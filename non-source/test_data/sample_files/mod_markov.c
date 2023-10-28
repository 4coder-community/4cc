#define STB_SB_MMAP
#include "stb_sb.h"
#include "module.h"
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <regex.h>
#include <zlib.h>
#include "utils.h"

static bool markov_init (const IRCCoreCtx*);
static void markov_quit (void);
static void markov_join (const char*, const char*);
static void markov_cmd  (const char*, const char*, const char*, int);
static void markov_msg  (const char*, const char*, const char*);
static void markov_mod_msg(const char* sender, const IRCModMsg* msg);
static bool markov_save (FILE*);

enum { MARKOV_SAY, MARKOV_ASK, MARKOV_INTERVAL, MARKOV_STATUS };

const IRCModuleCtx irc_mod_ctx = {
    .name     = "markov",
    .desc     = "Says incomprehensible stuff",
    .flags    = IRC_MOD_DEFAULT,
    .on_init  = &markov_init,
    .on_quit  = &markov_quit,
    .on_cmd   = &markov_cmd,
    .on_msg   = &markov_msg,
    .on_join  = &markov_join,
    .on_save  = &markov_save,
    .on_mod_msg = &markov_mod_msg,
    .commands = DEFINE_CMDS (
        [MARKOV_SAY]      = CONTROL_CHAR "say",
        [MARKOV_ASK]      = CONTROL_CHAR "ask",
        [MARKOV_INTERVAL] = CONTROL_CHAR "interval " CONTROL_CHAR "gap",
        [MARKOV_STATUS]   = CONTROL_CHAR "status"
        )
};

static const IRCCoreCtx* ctx;

typedef uint32_t word_idx_t;

struct MarkovLinkKey_ {
    uint32_t val_idx;
    word_idx_t word_idx_1 : 24;
    word_idx_t word_idx_2 : 24;
} __attribute__ ((packed));
typedef struct MarkovLinkKey_ MarkovLinkKey;

typedef struct {
    word_idx_t word_idx : 24;
    uint8_t count;
    uint32_t next;
} MarkovLinkVal;

static char* word_mem;

static MarkovLinkKey* chain_keys;
static MarkovLinkVal* chain_vals;

static char rng_state_mem[256];
static struct random_data rng_state;

static regex_t url_regex;

static size_t max_chain_len = 16;
static size_t msg_chance = 150;

static word_idx_t start_sym_idx;
static word_idx_t end_sym_idx;

static uint32_t recent_hashes[128];
static size_t hash_idx;

static char** markov_nicks;

static uint32_t markov_rand(uint32_t limit){
    int32_t x;
    
    do {
        random_r(&rng_state, &x);
    } while (x >= (RAND_MAX - RAND_MAX % limit));
    
    return x % limit;
}

static bool find_word(const char* word, size_t word_len, word_idx_t* index){
    char* w = alloca(word_len + 2);
    w[0] = 0;
    memcpy(w + 1, word, word_len + 1);
    
    char* p = memmem(word_mem, sbmm_count(word_mem), w, word_len + 2);
    if(p){
        *index = (p + 1) - word_mem;
    }
    
    return p != NULL;
}

static word_idx_t find_or_add_word(const char* word, size_t word_len){
    word_idx_t index;
    if(!find_word(word, word_len, &index)){
        char* p = memcpy(sbmm_add(word_mem, word_len+1), word, word_len+1);
        index = p - word_mem;
    }
    return index;
}

static ssize_t find_key_idx(word_idx_t a, word_idx_t b){
    for(const MarkovLinkKey* k = chain_keys; k < sbmm_end(chain_keys); ++k){
        if(k->word_idx_1 == a && k->word_idx_2 == b){
            return k - chain_keys;
        }
    }
    return -1;
}

static const char* bad_end_words[] = {
    "and",
    "the",
    "a",
    "as",
    "if",
    "i",
    ",",
    "/",
    NULL
};

static size_t markov_gen(char* buffer, size_t buffer_len){
    if(!buffer_len) return 0;
    *buffer = 0;
    
    ssize_t key_idx = find_key_idx(start_sym_idx, start_sym_idx);
    assert(key_idx != -1);
    
    MarkovLinkKey* key = chain_keys + key_idx;
    
    int chain_len = 1 + markov_rand(max_chain_len);
    int links = 0;
    bool should_end = false;
    
    do {
        size_t total = 0;
        size_t end_count = 0;
        
        MarkovLinkVal* val = chain_vals + key->val_idx;
        do {
            if(val->word_idx == end_sym_idx) end_count = val->count;
            total += val->count;
        } while(val->next != -1 && (val = chain_vals + val->next));
        
        assert(total);
        ssize_t count = markov_rand(total);
        
        should_end =
            (links >= chain_len && end_count > (total / 2)) ||
            (links >= chain_len * 1.5f && end_count) ||
            (links >= chain_len * 2.0f);
        
        val = chain_vals + key->val_idx;
        while((count -= val->count) >= 0){
            val = chain_vals + val->next;
        }
        
        if(val->word_idx == end_sym_idx){
            break;
        }
        
        const char* word = word_mem + val->word_idx;
        
        for(const char** c = bad_end_words; *c; ++c){
            if(strcmp(*c, word) == 0){
                should_end = false;
                break;
            }
        }
        
        if(*buffer && strcmp(word, ",") != 0){
            inso_strcat(buffer, buffer_len, " ");
        }
        inso_strcat(buffer, buffer_len, word);
        
        ssize_t new_key_idx = find_key_idx(key->word_idx_2, val->word_idx);
        assert(new_key_idx > 0);
        
        key = chain_keys + new_key_idx;
        
    } while(!should_end);
    
    return strlen(buffer);
}

static uint32_t markov_hash(const char* str, size_t len){
    uint32_t hash = 9229;
    for(int i = 0; i < len; ++i){
        hash *= 31U;
        hash += str[i];
    }
    return hash;
}

static void markov_add_hash(const char* str, size_t len){
    recent_hashes[hash_idx] = markov_hash(str, len);
    hash_idx = (hash_idx + 1) % ARRAY_SIZE(recent_hashes);
}

static bool markov_check_dup(const char* str, size_t len){
    uint32_t hash = markov_hash(str, len);
    for(int i = 0; i < ARRAY_SIZE(recent_hashes); ++i){
        if(recent_hashes[i] == hash) return true;
    }
    return false;
}

static const char* markov_get_punct(){
    
    size_t val = markov_rand(100);
    
    if(val < 67) return ".";
    if(val < 72) return "?";
    if(val < 85) return "!";
    if(val < 97) return "...";
    if(val < 98) return "‽";
    if(val < 99) return ". FailFish";
    
    return ". Kappa";
}

static bool markov_gen_formatted(char* msg, size_t msg_len){
    int num_sentences = markov_rand(10) < 8 ? 1 : 2;
    
    while(num_sentences--){
        int attempts = 0;
        
        size_t buff_len = msg_len;
        char* buff = alloca(msg_len);
        
        size_t tmp_len;
        
        do {
            tmp_len = markov_gen(buff, buff_len);
            
            if(*buff == ','){
                tmp_len -= 2;
                memmove(buff, buff + 2, tmp_len);
            }
        } while(attempts++ < 5 && markov_check_dup(buff, tmp_len));
        
        buff_len = tmp_len;
        
        if(attempts >= 5){
            puts("Couldn't get a good message, giving up.");
            return false;
        }
        
        markov_add_hash(buff, buff_len);
        
        *buff = toupper(*buff);
        memcpy(msg, buff, buff_len);
        msg[buff_len] = 0;
        
        msg += buff_len;
        msg_len -= buff_len;
        
        if(num_sentences){
            int written = 0;
            written = INSO_MAX(written, inso_strcat(msg, msg_len, markov_get_punct()));
            written = INSO_MAX(written, inso_strcat(msg, msg_len, " "));
            
            msg += written;
            msg_len -= written;
        }
    }
    
    inso_strcat(msg, msg_len, markov_get_punct());
    
    return true;
}

static void markov_load(){
    gzFile f = gzopen(ctx->get_datafile(), "rb");
    uint32_t word_size = 0, key_size = 0, val_size = 0;
    
    if(gzread(f, &word_size, sizeof(word_size)) < 1) goto out;
    if(gzread(f, &key_size, sizeof(key_size)) < 1) goto out;
    if(gzread(f, &val_size, sizeof(val_size)) < 1) goto out;
    
    if(gzread(f, sbmm_add(word_mem, word_size), word_size) < word_size) goto out;
    if(gzread(f, sbmm_add(chain_keys, key_size), sizeof(MarkovLinkKey) * key_size) < key_size) goto out;
    if(gzread(f, sbmm_add(chain_vals, val_size), sizeof(MarkovLinkVal) * val_size) < val_size) goto out;
    
    gzclose(f);
    return;
    
    out:
    puts("markov: couldn't read file.");
    gzclose(f);
}

static bool markov_save(FILE* file){
    uint32_t word_size = sbmm_count(word_mem) - 1;
    uint32_t key_size  = sbmm_count(chain_keys);
    uint32_t val_size  = sbmm_count(chain_vals);
    
    gzFile f = gzdopen(dup(fileno(file)), "wb");
    
    if(gzwrite(f, &word_size, sizeof(word_size)) < 1) goto out;
    if(gzwrite(f, &key_size, sizeof(key_size)) < 1) goto out;
    if(gzwrite(f, &val_size, sizeof(val_size)) < 1) goto out;
    
    if(gzwrite(f, word_mem + 1, word_size) < word_size) goto out;
    if(gzwrite(f, chain_keys, sizeof(MarkovLinkKey) * key_size) < key_size) goto out;
    if(gzwrite(f, chain_vals, sizeof(MarkovLinkVal) * val_size) < val_size) goto out;
    
    gzclose(f);
    return true;
    
    out:
    puts("markov: error saving file.");
    gzclose(f);
    return false;
}

static bool markov_init(const IRCCoreCtx* _ctx){
    ctx = _ctx;
    
    unsigned int seed = rand();
    
    int fd = open("/dev/urandom", O_RDONLY);
    if(fd != -1){
        if(read(fd, &seed, sizeof(seed)) == -1){
            perror("markov_init: read");
        }
        close(fd);
    }
    
    initstate_r(seed, rng_state_mem, sizeof(rng_state_mem), &rng_state);
    setstate_r(rng_state_mem, &rng_state);
    
    sbmm_push(word_mem, 0);
    
    regcomp(&url_regex, "(www\\.|https?:\\/\\/|\\.com|\\.[a-zA-Z]\\/)", REG_ICASE | REG_EXTENDED | REG_NOSUB);
    
    markov_load();
    
    start_sym_idx = find_or_add_word("^", 1);
    end_sym_idx   = find_or_add_word("$", 1);
    
    return true;
}

static void markov_join(const char* chan, const char* name){
    
    if(strcasecmp(name, ctx->get_username()) == 0) return;
    
    for(int i = 0; i < sb_count(markov_nicks); ++i){
        if(strcasecmp(name, markov_nicks[i]) == 0){
            return;
        }
    }
    sb_push(markov_nicks, strdup(name));
}

static void markov_send(const char* chan){
    char buffer[256];
    if(!markov_gen_formatted(buffer, sizeof(buffer))) return;
    ctx->send_msg(chan, "%s", buffer);
}

static void markov_reply(const char* chan, const char* nick){
    char buffer[256];
    if(!markov_gen_formatted(buffer, sizeof(buffer))) return;
    ctx->send_msg(chan, "@%s: %s", nick, buffer);
}

static void markov_ask(const char* chan){
    char buffer[256];
    if(!markov_gen_formatted(buffer, sizeof(buffer))) return;
    
    size_t len = strlen(buffer);
    if(len && ispunct(buffer[len-1])){
        buffer[len-1] = '?';
    } else if(sizeof(buffer) - len > 1){
        buffer[len] = '?';
        buffer[len+1] = 0;
    }
    ctx->send_msg(chan, "Q: %s", buffer);
}

static const int say_cooldown = 300;
static time_t last_say;

static void markov_cmd(const char* chan, const char* name, const char* arg, int cmd){
    time_t now = time(0);
    
    bool admin = inso_is_admin(ctx, name);
    
    switch(cmd){
        
        case MARKOV_SAY: {
            if(admin || now - last_say >= say_cooldown){
                markov_send(chan);
                last_say = now;
            }
        } break;
        
        case MARKOV_ASK: {
            if(admin || now - last_say >= say_cooldown){
                markov_ask(chan);
                last_say = now;
            }
        } break;
        
        case MARKOV_INTERVAL: {
            if(!admin) break;
            
            if(*arg++){
                int chance = strtoul(arg, NULL, 0);
                if(chance != 0){
                    msg_chance = chance;
                }
            }
            
            ctx->send_msg(chan, "%s: interval = %zu.", name, msg_chance);
        } break;
        
        case MARKOV_STATUS: {
            if(!admin) break;
            
            ctx->send_msg(
                chan,
                "%s: markov status: %d keys, %d chains, %dKB word mem.",
                name,
                sbmm_count(chain_keys),
                sbmm_count(chain_vals),
                sbmm_count(word_mem) / 1024
                );
            
        } break;
    }
    
}

static void markov_replace(char** msg, const char* from, const char* to){
    size_t from_len = strlen(from);
    size_t to_len = strlen(to);
    size_t msg_len = sb_count(*msg);
    
    char* p;
    size_t off = 0;
    
    while((p = strstr(*msg + off, from))){
        
        off = p - *msg;
        
        if(to_len > from_len){
            memset(sb_add(*msg, to_len - from_len), 0, to_len - from_len);
        } else {
            stb__sbn(*msg) -= (from_len - to_len);
        }
        
        p = *msg + off;
        
        const char* end_p = *msg + msg_len;
        memmove(p + to_len, p + from_len, end_p - (p + from_len));
        memcpy(p, to, to_len);
        
        off += to_len;
        
        msg_len += (to_len - from_len);
    }
    
}

static void markov_add(word_idx_t indices[static 3]){
    
    ssize_t key_idx = find_key_idx(indices[0], indices[1]);
    
    if(key_idx == -1){
        
        MarkovLinkVal val = {
            .word_idx = indices[2],
            .count = 1,
            .next = -1
        };
        sbmm_push(chain_vals, val);
        
        MarkovLinkKey key = {
            .word_idx_1 = indices[0],
            .word_idx_2 = indices[1],
            .val_idx  = sbmm_count(chain_vals) - 1
        };
        sbmm_push(chain_keys, key);
        
    } else {
        bool found = false;
        size_t last_idx = key_idx;
        
        for(uint32_t i = chain_keys[key_idx].val_idx; i != -1; i = chain_vals[i].next){
            if(chain_vals[i].word_idx == indices[2]){
                if(chain_vals[i].count < UCHAR_MAX) ++chain_vals[i].count;
                found = true;
                break;
            }
            
            last_idx = i;
        }
        
        if(!found){
            MarkovLinkVal val = {
                .word_idx = indices[2],
                .count = 1,
                .next = -1
            };
            sbmm_push(chain_vals, val);
            chain_vals[last_idx].next = sbmm_count(chain_vals) - 1;
        }
    }
}

static const char* ignores[] = {
    "hmh_bot",
    "hmd_bot",
    "drakebot_",
    NULL
};

static const char* skip_words[] = { "p", "d", "b", "o", "-p", "-d", "-b", "-o", NULL };

static void markov_msg(const char* chan, const char* name, const char* _msg){
    
    markov_join(chan, name);
    
    if(*_msg == '!' || *_msg == '\\'){
        puts("skipping command.");
        return;
    }
    
    if(regexec(&url_regex, _msg, 0, NULL, 0) == 0){
        puts("skipping url.");
        return;
    }
    
    for(const char** n = ignores; *n; ++n){
        if(strcasecmp(*n, name) == 0){
            return;
        }
    }
    
    size_t msg_len = strlen(_msg);
    char* msg = NULL;
    memcpy(sb_add(msg, msg_len + 1), _msg, msg_len + 1);
    
    for(char* c = msg; c < sb_end(msg); ++c){
        *c = tolower(*c);
    }
    
    const char* bot_name = ctx->get_username();
    size_t      bot_name_len = strlen(bot_name);
    const char* name_pats[] = { "@%s", "%s:", "%s," };
    char        name_buf[256];
    bool        found_name = false;
    
    assert(bot_name_len + 2 < sizeof(name_buf));
    for(size_t i = 0; i < ARRAY_SIZE(name_pats); ++i){
        snprintf(name_buf, sizeof(name_buf), name_pats[i], bot_name);
        if(strcasestr(msg, name_buf)){
            found_name = true;
            break;
        }
    }
    
    if(found_name && markov_rand(3)){
        markov_reply(chan, name);
    }
    
    if(*msg == '@') *msg = ' ';
    
    for(char* p = msg; *p; ++p){
        if(*p < ' ' || *p >= 127) *p = ' ';
        else if(*p == '$') *p = '@';
    }
    
    markov_replace(&msg, ". ", " $ ");
    markov_replace(&msg, "! ", " $ ");
    markov_replace(&msg, "? ", " $ ");
    
    markov_replace(&msg, ",", " , ");
    
    for(char* p = msg; *p; ++p){
        if(strchr(".!?@:;`^(){}[]\"", *p)) *p = ' ';
    }
    
    markov_replace(&msg, "  ", " ");
    
    word_idx_t words[] = { start_sym_idx, start_sym_idx, 0 };
    
    char* state = NULL;
    char* word = strtok_r(msg, " ", &state);
    
    printf("Adding:");
    
    for(; word; word = strtok_r(NULL, " ", &state)){
        
        bool skip = false;
        for(const char** c = skip_words; *c; ++c){
            if(strcmp(word, *c) == 0){
                skip = true;
                break;
            }
        }
        
        if(skip) continue;
        
        printf(" [%s]", word);
        
        size_t len = strlen(word);
        word_idx_t idx = 0;
        
        if(len > 24){
            len = 9;
            idx = find_or_add_word("something", len);
        } else {
            //TODO: remove ++ --
            
            for(char** c = markov_nicks; c < sb_end(markov_nicks); ++c){
                if(strcasecmp(word, *c) == 0){
                    skip = true;
                    break;
                }
            }
            
            if(skip) continue;
            
            idx = find_or_add_word(word, len);
        }
        
        if(idx == end_sym_idx && words[1] == start_sym_idx) continue;
        
        if(idx == words[1] && words[1] == words[0]) continue;
        
        words[2] = idx;
        markov_add(words);
        
        if(idx == end_sym_idx){
            words[0] = start_sym_idx;
            words[1] = start_sym_idx;
        } else {
            words[0] = words[1];
            words[1] = words[2];
        }
    }
    
    puts(".");
    
    words[2] = end_sym_idx;
    if(words[1] != start_sym_idx) markov_add(words);
    
    if(markov_rand(msg_chance) == 0){
        markov_send(chan);
    }
    
    sb_free(msg);
}

static void markov_mod_msg(const char* sender, const IRCModMsg* msg){
    if(strcmp(msg->cmd, "markov_gen") == 0){
        
        char* buffer = malloc(256);
        if(!markov_gen(buffer, 256)){
            free(buffer);
            return;
        }
        
        msg->callback((intptr_t)buffer, msg->cb_arg);
    }
}

static void markov_quit(void){
    sbmm_free(word_mem);
    sbmm_free(chain_keys);
    sbmm_free(chain_vals);
    
    for(int i = 0; i < sb_count(markov_nicks); ++i){
        free(markov_nicks[i]);
    }
    sb_free(markov_nicks);
    
    regfree(&url_regex);
}
