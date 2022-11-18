/*
 * YOUR INFO HERE!
 */

// FINAL VERSION:

struct Seive_Array{
    bool *is_composite;
    int size;
};

struct Primes{
    int *primes;
    int count, max;
};

void extend_sieve(Primes ps, Seive_Array seive, int start){
    int q,p,j,n,m;
    for (q = 1; q < ps.count; ++q){
        p = ps.primes[q];
        j = start;
        n = 2*j + 3;
        m = (n/p)*p;
        if (n != m){
            n = m+p;
            if (n % 2 == 0){
                n += p;
            }
            j = (n-3)/2;
        }
        
        for (; j < seive.size; j += p){
            seive.is_composite[j] = 1;
        }
    }
}

int seive_partial(Primes *ps_, Seive_Array seive, int start){
    Primes ps = *ps_;
    int i=start,n,j;
    for (; i < seive.size; ++i){
        if (!seive.is_composite[i]){
            n = i*2 + 3;
            ps.primes[ps.count++] = n;
            if (ps.count == ps.max){
                break;
            }
            for (j = i + n; j < seive.size; j += n){
                seive.is_composite[j] = 1;
            }
        }
    }
    ps_->count = ps.count;
    return i;
}

// TODO(allen): This is for an incrementally extended seive
struct Seive{
    Seive_Array seive;
    Primes ps;
    int i;
    int w;
};

// size of primes >= which_prime
// size of seive >= size > 0
Seive begin_seive(int *primes, int which_prime, bool *seive, int size){
    Seive result;
    
    result.seive.size = size;
    result.seive.is_composite = seive;
    
    result.ps.primes = primes;
    result.ps.count = 1;
    result.ps.max = which_prime;
    primes[0] = 2;
    
    result.i = 0;
    result.w = which_prime;
    
    return result;
}

inline bool seive_step(Seive *s){
    s->i = seive_partial(&s->ps, s->seive, s->i);
    return (s->ps.count != s->w);
}

inline void seive_grow(Seive *s, Seive_Array new_seive){
    extend_seive(s->ps, new_seive, s->seive.size);
    s->seive = new_seive;
}

inline int seive_init_size(int which_prime){
    return which_prime*5;
}

inline int seive_new_size(Seive s){
    return s.seive.size + s.w * (1 + ((s.w - s.ps.count) / 1000));
}

#ifdef EULER_PROBLEM
// BUILDING AREA:
struct Euler_In{};

struct Euler_Result{
    int prime;
};

static const int WHICH_PRIME = 10001;

Euler_Result euler_main(Euler_In in){
    char *chunk = (char*)malloc(1024*1024);
    
    int primes_[WHICH_PRIME];
    int size_ = seive_init_size(WHICH_PRIME);
    bool *seive_ = (bool*)chunk;
    memset(seive_, 0, size_);
    
    Seive s;
    s = begin_seive(primes_, WHICH_PRIME, seive_, size_);
    
    while (seive_step(&s)){
        Seive_Array new_seive;
        new_seive.size = seive_new_size(s);
        new_seive.is_composite = (bool*)chunk;
        memset(chunk + s.seive.size, 0, new_seive.size - s.seive.size);
        
        // free s.seive.is_composite, do not touch s.seive.size
        seive_grow(&s, new_seive);
    }
    
    free(chunk);
    
    Euler_Result result;
    result.prime = s.ps.primes[WHICH_PRIME - 1];
    
    return result;
}

void euler_print(Euler_Result answer, Euler_In in){
    printf("%d\n", answer.prime);
}

#if 0
#define EULER_CHECK

bool euler_check(Euler_Result answer, Euler_In in){
    bool result = 1;
    
    return result;
}
#endif

#endif
