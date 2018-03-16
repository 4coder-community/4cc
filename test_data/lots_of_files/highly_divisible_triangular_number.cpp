/*
 * YOUR INFO HERE!
 */

// FINAL VERSION:
typedef unsigned long long bigint;
struct Sieve_Array{
    bool *is_composite;
    bigint size;
};

struct Primes{
    bigint *primes;
    int count, max;
};

struct Sieve{
    Sieve_Array sieve;
    Primes ps;
    int i;
    int w;
};

void extend_sieve(Primes ps, Sieve_Array sieve, int start){
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
        
        for (; j < sieve.size; j += p){
            sieve.is_composite[j] = 1;
        }
    }
}

int sieve_partial(Primes *ps_, Sieve_Array sieve, int start){
    Primes ps = *ps_;
    int i=start,n,j;
    for (; i < sieve.size; ++i){
        if (!sieve.is_composite[i]){
            n = i*2 + 3;
            ps.primes[ps.count++] = n;
            for (j = i + n; j < sieve.size; j += n){
                sieve.is_composite[j] = 1;
            }
            if (ps.count == ps.max){
                ++i;
                break;
            }
        }
    }
    ps_->count = ps.count;
    return i;
}

// size of primes >= which_prime
// size of sieve >= size > 0
void begin_sieve(Sieve *s, int prime_size, int size){
    s->sieve.size = size;
    
    s->ps.count = 1;
    s->ps.max = prime_size;
    s->ps.primes[0] = 2;
    
    s->i = 0;
    s->w = prime_size;
}

inline bool sieve_step(Sieve *s){
    s->i = sieve_partial(&s->ps, s->sieve, s->i);
    return (s->ps.count != s->w);
}

inline void sieve_grow(Sieve *s, Sieve_Array new_sieve){
    extend_sieve(s->ps, new_sieve, s->sieve.size);
    s->sieve = new_sieve;
}

inline int sieve_init_size(int which_prime){
    return which_prime*5;
}

inline int sieve_new_size(Sieve s){
    return s.sieve.size + s.w * (1 + ((s.w - s.ps.count) / 1000));
}

#ifdef EULER_PROBLEM
// BUILDING AREA:
struct Euler_In{};

struct Euler_Result{
    int number;
};

int count_divisors(int x, int *d, Primes ps){
    int dc = 0;
    int c,p,i,j;
    
    p = ps.primes[0];
    for (i = 0; i < ps.count && p < x; ++i){
        p = ps.primes[i];
        c = 1;
        while (x % p == 0){
            x /= p;
            ++c;
        }
        if (c > 1){
            d[dc++] = c;
        }
    }
    
    c = 1;
    for (j = 0; j < dc; ++j){
        c *= d[j];
    }
    
    return c;
}

int count_divisors(int x, int *d){
    int dc = 0;
    int c,p,j;
    
    c = 1;
    while (x % 2 == 0){
        x /= 2;
        ++c;
    }
    if (c > 1){
        d[dc++] = c;
    }
    
    for (p = 3; p <= x; p += 2){
        c = 1;
        while (x % p == 0){
            x /= p;
            ++c;
        }
        if (c > 1){
            d[dc++] = c;
        }
    }
    
    c = 1;
    for (j = 0; j < dc; ++j){
        c *= d[j];
    }
    
    return c;
}

inline Euler_Result euler_main(Euler_In in){
    int d[500];
    int max = 1600;
    int *divs = (int*)malloc(sizeof(int)*max);
    divs[0] = 0;
    divs[1] = 1;
    divs[2] = 2;
    
    int x=1,n=2,c=0,a,b;
    a=1;
    b=1;
    
    while (1){
        if (c >= 500){
            break;
        }
        
        ++n;
        divs[n] = count_divisors(n, d);
        
        b += 2;
        x += 2*a;
        c = divs[a]*divs[b];
        
        if (c >= 500){
            break;
        }
        
        ++n;
        if (n >= max){
            int new_max = max*2;
            int *new_divs = (int*)malloc(sizeof(int)*new_max);
            memcpy(new_divs, divs, sizeof(int)*max);
            free(divs);
            divs = new_divs;
            max = new_max;
        }
        
        divs[n] = count_divisors(n, d);
        
        a += 1;
        x += b;
        c = divs[a]*divs[b];
    }
    
    Euler_Result result;
    result.number = x;
    return result;
}

void euler_print(Euler_Result answer, Euler_In in){
    printf("%d\n", answer.number);
}

#define EULER_CHECK

bool euler_check(Euler_Result answer, Euler_In in){
    bool result;
    
    int a = 76576500;
    
    printf("answer = %d\n", a);
    result = (a == answer.number);
    
    return result;
}
#endif

