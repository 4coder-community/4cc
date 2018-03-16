
// TOP

#if defined(FSTRING_IMPLEMENTATION) && defined(FSTRING_GUARD)
#undef FSTRING_IMPLEMENTATION
#endif

#include <stdint.h>

#ifndef FSTRING_LINK
# define FSTRING_LINK static
#endif

#ifndef FSTRING_INLINE
# define FSTRING_INLINE inline
#endif

#ifndef FSTRING_STRUCT
#define FSTRING_STRUCT
typedef struct String{
    char *str;
    int32_t size;
    int32_t memory_size;
} String;

typedef struct Offset_String{
    int32_t offset;
    int32_t size;
} Offset_String;
#endif

#ifndef fstr_bool
# define fstr_bool int32_t
#endif

#ifndef literal
# define literal(s) (s), (sizeof(s)-1)
#endif

#ifndef FCODER_STRING_H
#define FCODER_STRING_H

FSTRING_INLINE  fstr_bool     char_is_slash(char c);
FSTRING_INLINE  char          char_to_upper(char c);
FSTRING_INLINE  char          char_to_lower(char c);
FSTRING_INLINE  fstr_bool     char_is_whitespace(char c);
FSTRING_INLINE  fstr_bool     char_is_alpha_numeric(char c);
FSTRING_INLINE  fstr_bool     char_is_alpha_numeric_true(char c);
FSTRING_INLINE  fstr_bool     char_is_alpha(char c);
FSTRING_INLINE  fstr_bool     char_is_alpha_true(char c);
FSTRING_INLINE  fstr_bool     char_is_hex(char c);
FSTRING_INLINE  fstr_bool     char_is_numeric(char c);
FSTRING_INLINE  String        string_zero(void);
FSTRING_INLINE  String        make_string_max(void *str, int32_t size, int32_t mem_size);
FSTRING_INLINE  String        make_string(void *str, int32_t size);
#ifndef   make_lit_string
# define  make_lit_string(s) (make_string_max((char*)(s), sizeof(s)-1, sizeof(s)))
#endif
#ifndef   make_fixed_width_string
# define  make_fixed_width_string(s) (make_string_max((char*)(s), 0, sizeof(s)))
#endif
#ifndef   expand_str
# define  expand_str(s) ((s).str), ((s).size)
#endif
FSTRING_LINK    int32_t       str_size(char *str);
FSTRING_INLINE  String        make_string_slowly(void *str);
FSTRING_INLINE  String        substr(String str, int32_t start);
FSTRING_INLINE  String        substr_start_size(String str, int32_t start, int32_t size);
FSTRING_LINK    String        skip_whitespace(String str);
FSTRING_LINK    String        chop_whitespace(String str);
FSTRING_LINK    String        skip_chop_whitespace(String str);
FSTRING_INLINE  String        tailstr(String str);
FSTRING_LINK    fstr_bool     match_cc(char *a, char *b);
FSTRING_LINK    fstr_bool     match_sc(String a, char *b);
FSTRING_INLINE  fstr_bool     match_cs(char *a, String b);
FSTRING_LINK    fstr_bool     match_ss(String a, String b);
FSTRING_LINK    fstr_bool     match_part_ccl(char *a, char *b, int32_t *len);
FSTRING_LINK    fstr_bool     match_part_scl(String a, char *b, int32_t *len);
FSTRING_INLINE  fstr_bool     match_part_cc(char *a, char *b);
FSTRING_INLINE  fstr_bool     match_part_sc(String a, char *b);
FSTRING_LINK    fstr_bool     match_part_cs(char *a, String b);
FSTRING_LINK    fstr_bool     match_part_ss(String a, String b);
FSTRING_LINK    fstr_bool     match_insensitive_cc(char *a, char *b);
FSTRING_LINK    fstr_bool     match_insensitive_sc(String a, char *b);
FSTRING_INLINE  fstr_bool     match_insensitive_cs(char *a, String b);
FSTRING_LINK    fstr_bool     match_insensitive_ss(String a, String b);
FSTRING_LINK    fstr_bool     match_part_insensitive_ccl(char *a, char *b, int32_t *len);
FSTRING_LINK    fstr_bool     match_part_insensitive_scl(String a, char *b, int32_t *len);
FSTRING_INLINE  fstr_bool     match_part_insensitive_cc(char *a, char *b);
FSTRING_INLINE  fstr_bool     match_part_insensitive_sc(String a, char *b);
FSTRING_LINK    fstr_bool     match_part_insensitive_cs(char *a, String b);
FSTRING_LINK    fstr_bool     match_part_insensitive_ss(String a, String b);
FSTRING_LINK    int32_t       compare_cc(char *a, char *b);
FSTRING_LINK    int32_t       compare_sc(String a, char *b);
FSTRING_INLINE  int32_t       compare_cs(char *a, String b);
FSTRING_LINK    int32_t       compare_ss(String a, String b);
FSTRING_LINK    int32_t       find_c(char *str, int32_t start, char character);
FSTRING_LINK    int32_t       find_s(String str, int32_t start, char character);
FSTRING_LINK    int32_t       find_chars_c(char *str, int32_t start, char *characters);
FSTRING_LINK    int32_t       find_chars_s(String str, int32_t start, char *characters);
FSTRING_LINK    int32_t       find_substr_c(char *str, int32_t start, String seek);
FSTRING_LINK    int32_t       find_substr_s(String str, int32_t start, String seek);
FSTRING_LINK    int32_t       rfind_substr(String str, int32_t start, String seek);
FSTRING_LINK    int32_t       find_substr_insensitive_c(char *str, int32_t start, String seek);
FSTRING_LINK    int32_t       find_substr_insensitive_s(String str, int32_t start, String seek);
FSTRING_INLINE  fstr_bool     has_substr_c(char *s, String seek);
FSTRING_INLINE  fstr_bool     has_substr_s(String s, String seek);
FSTRING_INLINE  fstr_bool     has_substr_insensitive_c(char *s, String seek);
FSTRING_INLINE  fstr_bool     has_substr_insensitive_s(String s, String seek);
FSTRING_LINK    int32_t       copy_fast_unsafe_cc(char *dest, char *src);
FSTRING_LINK    int32_t       copy_fast_unsafe_cs(char *dest, String src);
FSTRING_LINK    fstr_bool     copy_checked_ss(String *dest, String src);
FSTRING_LINK    fstr_bool     copy_partial_sc(String *dest, char *src);
FSTRING_LINK    fstr_bool     copy_partial_ss(String *dest, String src);
FSTRING_INLINE  int32_t       copy_cc(char *dest, char *src);
FSTRING_INLINE  void          copy_ss(String *dest, String src);
FSTRING_INLINE  void          copy_sc(String *dest, char *src);
FSTRING_LINK    fstr_bool     append_checked_ss(String *dest, String src);
FSTRING_LINK    fstr_bool     append_partial_sc(String *dest, char *src);
FSTRING_LINK    fstr_bool     append_partial_ss(String *dest, String src);
FSTRING_LINK    fstr_bool     append_char(String *dest, char c);
FSTRING_INLINE  fstr_bool     append_s(String *dest, String src);
FSTRING_INLINE  fstr_bool     append_c(String *dest, char *src);
FSTRING_LINK    fstr_bool     terminate_with_null(String *str);
FSTRING_LINK    fstr_bool     append_padding(String *dest, char c, int32_t target_size);
FSTRING_LINK    void          replace_char(String *str, char replace, char with);
FSTRING_LINK    int32_t       int_to_str_size(int32_t x);
FSTRING_LINK    fstr_bool     int_to_str(String *dest, int32_t x);
FSTRING_LINK    fstr_bool     append_int_to_str(String *dest, int32_t x);
FSTRING_LINK    int32_t       u64_to_str_size(uint64_t x);
FSTRING_LINK    fstr_bool     u64_to_str(String *dest, uint64_t x);
FSTRING_LINK    fstr_bool     append_u64_to_str(String *dest, uint64_t x);
FSTRING_LINK    int32_t       float_to_str_size(float x);
FSTRING_LINK    fstr_bool     append_float_to_str(String *dest, float x);
FSTRING_LINK    fstr_bool     float_to_str(String *dest, float x);
FSTRING_LINK    fstr_bool     str_is_int_s(String str);
FSTRING_LINK    int32_t       str_to_int_c(char *str);
FSTRING_LINK    int32_t       str_to_int_s(String str);
FSTRING_LINK    int32_t       hexchar_to_int(char c);
FSTRING_LINK    char          int_to_hexchar(int32_t x);
FSTRING_LINK    uint32_t      hexstr_to_int(String str);
FSTRING_LINK    fstr_bool     color_to_hexstr(String *s, uint32_t color);
FSTRING_LINK    fstr_bool     hexstr_to_color(String s, uint32_t *out);
FSTRING_LINK    int32_t       reverse_seek_slash_pos(String str, int32_t pos);
FSTRING_INLINE  int32_t       reverse_seek_slash(String str);
FSTRING_INLINE  String        front_of_directory(String dir);
FSTRING_INLINE  String        path_of_directory(String dir);
FSTRING_LINK    fstr_bool     set_last_folder_c(String *dir, char *folder_name, char slash);
FSTRING_LINK    fstr_bool     set_last_folder_s(String *dir, String folder_name, char slash);
FSTRING_LINK    String        file_extension(String str);
FSTRING_LINK    fstr_bool     remove_extension(String *str);
FSTRING_LINK    fstr_bool     remove_last_folder(String *str);
FSTRING_LINK    fstr_bool     string_set_match(String *str_set, int32_t count, String str, int32_t *match_index);

#endif


//
// Character Helpers
//

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
char_is_slash(char c)
{
    return (c == '\\' || c == '/');
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE char
char_to_upper(char c)
{
    return (c >= 'a' && c <= 'z') ? c + (char)('A' - 'a') : c;
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE char
char_to_lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? c - (char)('A' - 'a') : c;
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
char_is_whitespace(char c)
{
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
char_is_alpha_numeric(char c)
{
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9' || c == '_');
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
char_is_alpha_numeric_true(char c)
{
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9');
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
char_is_alpha(char c)
{
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_');
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
char_is_alpha_true(char c)
{
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z');
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
char_is_hex(char c)
{
    return c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f';
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
char_is_numeric(char c)
{
    return (c >= '0' && c <= '9');
}
#endif


//
// String Making Functions
//

#ifndef FSTRING_GUARD
FSTRING_INLINE String
string_zero()
{
    String str={0};
    return(str);
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE String
make_string_max(void *str, int32_t size, int32_t mem_size)
{
    String result;
    result.str = (char*)str;
    result.size = size;
    result.memory_size = mem_size;
    return result;
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE String
make_string(void *str, int32_t size){
    String result;
    result.str = (char*)str;
    result.size = size;
    result.memory_size = size;
    return result;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
str_size(char *str)
{
    int32_t i = 0;
    while (str[i]) ++i;
    return i;
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE String
make_string_slowly(void *str)
{
    String result;
    result.str = (char*)str;
    result.size = str_size((char*)str);
    result.memory_size = result.size;
    return result;
}
#endif

// TODO(allen): I don't love the substr rule, I chose
// substr(String, start) and substr(String, start, size)
// but I wish I had substr(String, start) and substr(String, start, end)

#ifndef FSTRING_GUARD
FSTRING_INLINE String
substr(String str, int32_t start)
{
    String result;
    result.str = str.str + start;
    result.size = str.size - start;
    result.memory_size = 0;
    return(result);
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE String
substr_start_size(String str, int32_t start, int32_t size)
{
    String result;
    result.str = str.str + start;
    result.size = size;
    if (start + size > str.size){
        result.size = str.size - start;
    }
    result.memory_size = 0;
    return(result);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK String
skip_whitespace(String str)
{
    String result = {0};
    int32_t i = 0;
    for (; i < str.size && char_is_whitespace(str.str[i]); ++i);
    result = substr_start_size(str, i, str.size - i);
    return(result);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK String
chop_whitespace(String str)
{
    String result = {0};
    int32_t i = str.size;
    for (; i > 0 && char_is_whitespace(str.str[i-1]); --i);
    result = substr_start_size(str, 0, i);
    return(result);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK String
skip_chop_whitespace(String str)
{
    str = skip_whitespace(str);
    str = chop_whitespace(str);
    return(str);
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE String
tailstr(String str)
{
    String result;
    result.str = str.str + str.size;
    result.memory_size = str.memory_size - str.size;
    result.size = 0;
    return(result);
}
#endif


//
// String Comparison
//

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_cc(char *a, char *b){
    for (int32_t i = 0;; ++i){
        if (a[i] != b[i]){
            return 0;
        }
        if (a[i] == 0){
            return 1;
        }
    }
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_sc(String a, char *b){
    int32_t i = 0;
    for (; i < a.size; ++i){
        if (a.str[i] != b[i]){
            return 0;
        }
    }
    if (b[i] != 0){
        return 0;
    }
    return 1;
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
match_cs(char *a, String b){
    return(match_sc(b,a));
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_ss(String a, String b){
    if (a.size != b.size){
        return 0;
    }
    for (int32_t i = 0; i < b.size; ++i){
        if (a.str[i] != b.str[i]){
            return 0;
        }
    }
    return 1;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_part_ccl(char *a, char *b, int32_t *len){
    int32_t i;
    for (i = 0; b[i] != 0; ++i){
        if (a[i] != b[i]){
            return 0;
        }
    }
    *len = i;
    return 1;
}
#endif


#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_part_scl(String a, char *b, int32_t *len){
    int32_t i;
    for (i = 0; b[i] != 0; ++i){
        if (a.str[i] != b[i] || i == a.size){
            return 0;
        }
    }
    *len = i;
    return 1;
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
match_part_cc(char *a, char *b){
    int32_t x;
    return match_part_ccl(a,b,&x);
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
match_part_sc(String a, char *b){
    int32_t x;
    return match_part_scl(a,b,&x);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_part_cs(char *a, String b){
    for (int32_t i = 0; i != b.size; ++i){
        if (a[i] != b.str[i]){
            return 0;
        }
    }
    return 1;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_part_ss(String a, String b){
    if (a.size < b.size){
        return 0;
    }
    for (int32_t i = 0; i < b.size; ++i){
        if (a.str[i] != b.str[i]){
            return 0;
        }
    }
    return 1;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_insensitive_cc(char *a, char *b){
    for (int32_t i = 0;; ++i){
        if (char_to_upper(a[i]) !=
            char_to_upper(b[i])){
            return 0;
        }
        if (a[i] == 0){
            return 1;
        }
    }
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_insensitive_sc(String a, char *b){
    int32_t i = 0;
    for (; i < a.size; ++i){
        if (char_to_upper(a.str[i]) !=
            char_to_upper(b[i])){
            return 0;
        }
    }
    if (b[i] != 0){
        return 0;
    }
    return 1;
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
match_insensitive_cs(char *a, String b){
    return match_insensitive_sc(b,a);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_insensitive_ss(String a, String b){
    if (a.size != b.size){
        return 0;
    }
    for (int32_t i = 0; i < b.size; ++i){
        if (char_to_upper(a.str[i]) !=
            char_to_upper(b.str[i])){
            return 0;
        }
    }
    return 1;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_part_insensitive_ccl(char *a, char *b, int32_t *len){
    int32_t i;
    for (i = 0; b[i] != 0; ++i){
        if (char_to_upper(a[i]) != char_to_upper(b[i])){
            return 0;
        }
    }
    *len = i;
    return 1;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_part_insensitive_scl(String a, char *b, int32_t *len){
    int32_t i;
    for (i = 0; b[i] != 0; ++i){
        if (char_to_upper(a.str[i]) != char_to_upper(b[i]) ||
            i == a.size){
            return 0;
        }
    }
    *len = i;
    return 1;
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
match_part_insensitive_cc(char *a, char *b){
    int32_t x;
    return match_part_ccl(a,b,&x);
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
match_part_insensitive_sc(String a, char *b){
    int32_t x;
    return match_part_scl(a,b,&x);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_part_insensitive_cs(char *a, String b){
    for (int32_t i = 0; i != b.size; ++i){
        if (char_to_upper(a[i]) != char_to_upper(b.str[i])){
            return 0;
        }
    }
    return 1;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
match_part_insensitive_ss(String a, String b){
    if (a.size < b.size){
        return 0;
    }
    for (int32_t i = 0; i < b.size; ++i){
        if (char_to_upper(a.str[i]) != char_to_upper(b.str[i])){
            return 0;
        }
    }
    return 1;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
compare_cc(char *a, char *b){
    int32_t i = 0;
    while (a[i] == b[i] && a[i] != 0){
        ++i;
    }
    return (a[i] > b[i]) - (a[i] < b[i]);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
compare_sc(String a, char *b){
    int32_t i = 0;
    while (i < a.size && a.str[i] == b[i]){
        ++i;
    }
    if (i < a.size){
        return (a.str[i] > b[i]) - (a.str[i] < b[i]);
    }
    else{
        if (b[i] == 0){
            return 0;
        }
        else{
            return -1;
        }
    }
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE int32_t
compare_cs(char *a, String b){
    return -compare_sc(b,a);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
compare_ss(String a, String b){
    int32_t i = 0;
    while (i < a.size && i < b.size && a.str[i] == b.str[i]){
        ++i;
    }
    if (i < a.size && i < b.size){
        return (a.str[i] > b.str[i]) - (a.str[i] < b.str[i]);
    }
    else{
        return (a.size > b.size) - (a.size < b.size);
    }
}
#endif

//
// Finding Characters and Substrings
//

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
find_c(char *str, int32_t start, char character){
    int32_t i = start;
    while (str[i] != character && str[i] != 0) ++i;
    return i;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
find_s(String str, int32_t start, char character){
    int32_t i = start;
    while (i < str.size && str.str[i] != character) ++i;
    return i;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
find_chars_c(char *str, int32_t start, char *characters){
    int32_t i = start, j;
    while (str[i] != 0){
        for (j = 0; characters[j]; ++j){
            if (str[i] == characters[j]){
                return i;
            }
        }
        ++i;
    }
    return i;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
find_chars_s(String str, int32_t start, char *characters){
    int32_t i = start, j;
    while (i < str.size){
        for (j = 0; characters[j]; ++j){
            if (str.str[i] == characters[j]){
                return i;
            }
        }
        ++i;
    }
    return i;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
find_substr_c(char *str, int32_t start, String seek){
    int32_t i, j, k;
    fstr_bool hit;
    
    if (seek.size == 0){
        return str_size(str);
    }
    for (i = start; str[i]; ++i){
        if (str[i] == seek.str[0]){
            hit = 1;
            for (j = 1, k = i+1; j < seek.size; ++j, ++k){
                if (str[k] != seek.str[j]){
                    hit = 0;
                    break;
                }
            }
            if (hit){
                return i;
            }
        }
    }
    return i;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
find_substr_s(String str, int32_t start, String seek){
    int32_t stop_at, i, j, k;
    fstr_bool hit;
    
    if (seek.size == 0){
        return str.size;
    }
    stop_at = str.size - seek.size + 1;
    for (i = start; i < stop_at; ++i){
        if (str.str[i] == seek.str[0]){
            hit = 1;
            for (j = 1, k = i+1; j < seek.size; ++j, ++k){
                if (str.str[k] != seek.str[j]){
                    hit = 0;
                    break;
                }
            }
            if (hit){
                return i;
            }
        }
    }
    return str.size;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
rfind_substr(String str, int32_t start, String seek){
    int32_t i, j, k;
    fstr_bool hit;
    
    if (seek.size == 0){
        return -1;
    }
    if (start + seek.size > str.size){
        start = str.size - seek.size;
    }
    for (i = start; i >= 0; --i){
        if (str.str[i] == seek.str[0]){
            hit = 1;
            for (j = 1, k = i+1; j < seek.size; ++j, ++k){
                if (str.str[k] != seek.str[j]){
                    hit = 0;
                    break;
                }
            }
            if (hit){
                return i;
            }
        }
    }
    return -1;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
find_substr_insensitive_c(char *str, int32_t start, String seek){
    int32_t i, j, k;
    fstr_bool hit;
    char a_upper, b_upper;
    
    if (seek.size == 0){
        return str_size(str);
    }
    for (i = start; str[i]; ++i){
        if (str[i] == seek.str[0]){
            hit = 1;
            for (j = 1, k = i+1; j < seek.size; ++j, ++k){
                a_upper = char_to_upper(str[k]);
                b_upper = char_to_upper(seek.str[j]);
                if (a_upper != b_upper){
                    hit = 0;
                    break;
                }
            }
            if (hit){
                return i;
            }
        }
    }
    return i;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
find_substr_insensitive_s(String str, int32_t start, String seek){
    int32_t i, j, k;
    int32_t stop_at;
    fstr_bool hit;
    char a_upper, b_upper;
    
    if (seek.size == 0){
        return str.size;
    }
    stop_at = str.size - seek.size + 1;
    for (i = start; i < stop_at; ++i){
        if (str.str[i] == seek.str[0]){
            hit = 1;
            for (j = 1, k = i+1; j < seek.size; ++j, ++k){
                a_upper = char_to_upper(str.str[k]);
                b_upper = char_to_upper(seek.str[j]);
                if (a_upper != b_upper){
                    hit = 0;
                    break;
                }
            }
            if (hit){
                return i;
            }
        }
    }
    return str.size;
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
has_substr_c(char *s, String seek){
    return (s[find_substr_c(s, 0, seek)] != 0);
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
has_substr_s(String s, String seek){
    return (find_substr_s(s, 0, seek) < s.size);
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
has_substr_insensitive_c(char *s, String seek){
    return (s[find_substr_insensitive_c(s, 0, seek)] != 0);
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
has_substr_insensitive_s(String s, String seek){
    return (find_substr_insensitive_s(s, 0, seek) < s.size);
}
#endif

//
// String Copies and Appends
//

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
copy_fast_unsafe_cc(char *dest, char *src){
    char *start = dest;
    while (*src != 0){
        *dest = *src;
        ++dest;
        ++src;
    }
    return (int32_t)(dest - start);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
copy_fast_unsafe_cs(char *dest, String src){
    int32_t i = 0;
    while (i != src.size){
        dest[i] = src.str[i];
        ++i;
    }
    return(src.size);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
copy_checked_ss(String *dest, String src){
    char *dest_str;
    int32_t i;
    if (dest->memory_size < src.size){
        return 0;
    }
    dest_str = dest->str;
    for (i = 0; i < src.size; ++i){
        dest_str[i] = src.str[i];
    }
    dest->size = src.size;
    return 1;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
copy_partial_sc(String *dest, char *src){
    int32_t i = 0;
    int32_t memory_size = dest->memory_size;
    char *dest_str = dest->str;
    while (src[i] != 0){
        if (i >= memory_size){
            return 0;
        }
        dest_str[i] = src[i];
        ++i;
    }
    dest->size = i;
    return 1;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
copy_partial_ss(String *dest, String src){
    char *dest_str = dest->str;
    int32_t memory_size = dest->memory_size;
    fstr_bool result = 0;
    if (memory_size >= src.size){
        result = 1;
        memory_size = src.size;
    }
    for (int32_t i = 0; i < memory_size; ++i){
        dest_str[i] = src.str[i];
    }
    dest->size = memory_size;
    return result;
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE int32_t
copy_cc(char *dest, char *src){
    return copy_fast_unsafe_cc(dest, src);
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE void
copy_ss(String *dest, String src){
    copy_checked_ss(dest, src);
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE void
copy_sc(String *dest, char *src){
    copy_partial_sc(dest, src);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
append_checked_ss(String *dest, String src){
    String end = tailstr(*dest);
    fstr_bool result = copy_checked_ss(&end, src);
    // NOTE(allen): This depends on end.size still being 0 if
    // the check failed and no coppy occurred.
    dest->size += end.size;
    return result;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
append_partial_sc(String *dest, char *src){
    String end = tailstr(*dest);
    fstr_bool result = copy_partial_sc(&end, src);
    dest->size += end.size;
    return result;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
append_partial_ss(String *dest, String src){
    String end = tailstr(*dest);
    fstr_bool result = copy_partial_ss(&end, src);
    dest->size += end.size;
    return result;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
append_char(String *dest, char c){
    fstr_bool result = 0;
    if (dest->size < dest->memory_size){
        dest->str[dest->size++] = c;
        result = 1;
    }
    return result;
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
append_ss(String *dest, String src){
    return append_partial_ss(dest, src);
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE fstr_bool
append_sc(String *dest, char *src){
    return append_partial_sc(dest, src);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
terminate_with_null(String *str){
    fstr_bool result = 0;
    if (str->size < str->memory_size){
        str->str[str->size] = 0;
        result = 1;
    }
    return(result);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
append_padding(String *dest, char c, int32_t target_size){
    fstr_bool result = 1;
    int32_t offset = target_size - dest->size;
    int32_t r = 0;
    if (offset > 0){
        for (r = 0; r < offset; ++r){
            if (append_char(dest, c) == 0){
                result = 0;
                break;
            }
        }
    }
    return(result);
}
#endif


//
// Other Edits
//

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK void
replace_char(String *str, char replace, char with){
    char *s = str->str;
    int32_t i = 0;
    for (i = 0; i < str->size; ++i, ++s){
        if (*s == replace) *s = with;
    }
}
#endif

//
//  String <-> Number Conversions
//

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
int_to_str_size(int32_t x){
    int32_t size = 1;
    if (x < 0){
        size = 2;
    }
    x /= 10;
    while (x != 0){
        x /= 10;
        ++size;
    }
    return(size);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
int_to_str(String *dest, int32_t x){
    fstr_bool result = 1;
    char *str = dest->str;
    int32_t memory_size = dest->memory_size;
    int32_t size, i, j;
    fstr_bool negative;
    
    if (x == 0){
        str[0] = '0';
        dest->size = 1;
    }
    else{
        size = 0;
        negative = 0;
        if (x < 0){
            negative = 1;
            x = -x;
            str[size++] = '-';
        }
        while (x != 0){
            if (size == memory_size){
                result = 0;
                break;
            }
            i = x % 10;
            x /= 10;
            str[size++] = (char)('0' + i);
        }
        if (result){
            // NOTE(allen): Start i = 0 if not negative, start i = 1 if is negative
            // because - should not be flipped if it is negative :)
            for (i = negative, j = size-1; i < j; ++i, --j){
                char temp = str[i];
                str[i] = str[j];
                str[j] = temp;
            }
            dest->size = size;
        }
        else{
            dest->size = 0;
        }
    }
    return(result);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
append_int_to_str(String *dest, int32_t x){
    String last_part = tailstr(*dest);
    fstr_bool result = int_to_str(&last_part, x);
    if (result){
        dest->size += last_part.size;
    }
    return(result);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
u64_to_str_size(uint64_t x){
    int32_t size;
    if (x < 0){
        size = 0;
    }
    else{
        size = 1;
        x /= 10;
        while (x != 0){
            x /= 10;
            ++size;
        }
    }
    return(size);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
u64_to_str(String *dest, uint64_t x){
    fstr_bool result = 1;
    char *str = dest->str;
    int32_t memory_size = dest->memory_size;
    int32_t size, i, j;
    
    if (x == 0){
        str[0] = '0';
        dest->size = 1;
    }
    else{
        size = 0;
        while (x != 0){
            if (size == memory_size){
                result = 0;
                break;
            }
            i = x % 10;
            x /= 10;
            str[size++] = (char)('0' + i);
        }
        if (result){
            for (i = 0, j = size-1; i < j; ++i, --j){
                char temp = str[i];
                str[i] = str[j];
                str[j] = temp;
            }
            dest->size = size;
        }
        else{
            dest->size = 0;
        }
    }
    return(result);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
append_u64_to_str(String *dest, uint64_t x){
    String last_part = tailstr(*dest);
    fstr_bool result = u64_to_str(&last_part, x);
    if (result){
        dest->size += last_part.size;
    }
    return(result);
}
#endif

#ifndef FSTRING_GUARD
typedef struct Float_To_Str_Variables{
    fstr_bool negative;
    int32_t int_part;
    int32_t dec_part;
} Float_To_Str_Variables;

Float_To_Str_Variables
get_float_vars(float x){
    Float_To_Str_Variables vars = {0};
    
    if (x < 0){
        vars.negative = 1;
        x = -x;
    }
    
    vars.int_part = (int32_t)(x);
    vars.dec_part = (int32_t)((x - vars.int_part) * 1000);
    
    return(vars);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
float_to_str_size(float x){
    Float_To_Str_Variables vars = get_float_vars(x);
    int32_t size =
        vars.negative + int_to_str_size(vars.int_part) + 1 + int_to_str_size(vars.dec_part);
    return(size);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
append_float_to_str(String *dest, float x){
    fstr_bool result = 1;
    Float_To_Str_Variables vars = get_float_vars(x);
    
    if (vars.negative){
        append_char(dest, '-');
    }
    
    append_int_to_str(dest, vars.int_part);
    append_char(dest, '.');
    append_int_to_str(dest, vars.dec_part);
    
    return(result);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
float_to_str(String *dest, float x){
    fstr_bool result = 1;
    dest->size = 0;
    append_float_to_str(dest, x);
    return(result);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
str_is_int(String str){
    fstr_bool result = 1;
    for (int32_t i = 0; i < str.size; ++i){
        if (!char_is_numeric(str.str[i])){
            result = 0;
            break;
        }
    }
    return(result);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
str_to_int_c(char *str){
    int32_t x = 0;
    for (; *str; ++str){
        if (*str >= '0' || *str <= '9'){
            x *= 10;
            x += *str - '0';
        }
        else{
            x = 0;
            break;
        }
    }
    return(x);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
str_to_int_s(String str){
    int32_t x, i;
    if (str.size == 0){
        x = 0;
    }
    else{
        x = str.str[0] - '0';
        for (i = 1; i < str.size; ++i){
            x *= 10;
            x += str.str[i] - '0';
        }
    }
    return(x);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
hexchar_to_int(char c){
    int32_t x = 0;
    if (c >= '0' && c <= '9'){
        x = c-'0';
    }
    else if (c > 'F'){
        x = c+(10-'a');
    }
    else{
        x = c+(10-'A');
    }
    return(x);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK char
int_to_hexchar(int32_t x){
    return (x<10)?((char)x+'0'):((char)x+'a'-10);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK uint32_t
hexstr_to_int(String str){
    uint32_t x;
    int32_t i;
    if (str.size == 0){
        x = 0;
    }
    else{
        x = hexchar_to_int(str.str[0]);
        for (i = 1; i < str.size; ++i){
            x *= 0x10;
            x += hexchar_to_int(str.str[i]);
        }
    }
    return(x);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
color_to_hexstr(String *s, uint32_t color){
    fstr_bool result = 0;
    int32_t i;
    
    if (s->memory_size == 7 || s->memory_size == 8){
        result = 1;
        s->size = 6;
        s->str[6] = 0;
        color = color & 0x00FFFFFF;
        for (i = 5; i >= 0; --i){
            s->str[i] = int_to_hexchar(color & 0xF);
            color >>= 4;
        }
    }
    else if (s->memory_size > 8){
        result = 1;
        s->size = 8;
        s->str[8] = 0;
        for (i = 7; i >= 0; --i){
            s->str[i] = int_to_hexchar(color & 0xF);
            color >>= 4;
        }
    }
    return(result);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
hexstr_to_color(String s, uint32_t *out){
    fstr_bool result = 0;
    uint32_t color = 0;
    if (s.size == 6){
        result = 1;
        color = (unsigned int)hexstr_to_int(s);
        color |= (0xFF << 24);
        *out = color;
    }
    else if (s.size == 8){
        result = 1;
        color = (unsigned int)hexstr_to_int(s);
        *out = color;
    }
    return(result);
}
#endif

//
// Directory String Management
//

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK int32_t
reverse_seek_slash_pos(String str, int32_t pos){
    int32_t i = str.size - 1 - pos;
    while (i >= 0 && !char_is_slash(str.str[i])){
        --i;
    }
    return i;
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE int32_t
reverse_seek_slash(String str){
    return(reverse_seek_slash_pos(str, 0));
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE String
front_of_directory(String dir){
    return substr(dir, reverse_seek_slash(dir) + 1);
}
#endif

#ifndef FSTRING_GUARD
FSTRING_INLINE String
path_of_directory(String dir){
    return substr_start_size(dir, 0, reverse_seek_slash(dir) + 1);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
set_last_folder_c(String *dir, char *folder_name, char slash){
    fstr_bool result = 0;
    int32_t size = reverse_seek_slash(*dir) + 1;
    dir->size = size;
    if (append_sc(dir, folder_name)){
        if (append_char(dir, slash)){
            result = 1;
        }
    }
    if (!result){
        dir->size = size;
    }
    return result;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
set_last_folder_s(String *dir, String folder_name, char slash){
    fstr_bool result = 0;
    int32_t size = reverse_seek_slash(*dir) + 1;
    dir->size = size;
    if (append_ss(dir, folder_name)){
        if (append_char(dir, slash)){
            result = 1;
        }
    }
    if (!result){
        dir->size = size;
    }
    return result;
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK String
file_extension(String str){
    int32_t i;
    for (i = str.size - 1; i >= 0; --i){
        if (str.str[i] == '.') break;
    }
    ++i;
    return make_string(str.str+i, str.size-i);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
remove_extension(String *str){
    fstr_bool result = 0;
    char *s = str->str;
    int32_t i;
    for (i = str->size - 1; i >= 0; --i){
        if (s[i] == '.') break;
    }
    if (i >= 0){
        result = 1;
        str->size = i;
    }
    return(result);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
remove_last_folder(String *str){
    fstr_bool result = 0;
    int32_t end = reverse_seek_slash_pos(*str, 1);
    if (end >= 0){
        result = 1;
        str->size = end + 1;
    }
    return(result);
}
#endif

// TODO(allen): Add hash-table extension to string sets.
#ifdef FSTRING_IMPLEMENTATION
FSTRING_LINK fstr_bool
string_set_match(String *str_set, int32_t count, String str, int32_t *match_index){
    fstr_bool result = 0;
    int32_t i = 0;
    for (; i < count; ++i, ++str_set){
        if (match_ss(*str_set, str)){
            *match_index = i;
            result = 1;
            break;
        }
    }
    return(result);
}
#endif

#ifdef FSTRING_IMPLEMENTATION
#undef FSTRING_IMPLEMENTATION
#endif

#ifndef FSTRING_GUARD
#define FSTRING_GUARD
#endif

// BOTTOM

