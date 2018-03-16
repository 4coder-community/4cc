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

#ifdef EULER_PROBLEM
// BUILDING AREA:

typedef unsigned long long bigint;

struct Euler_In{};

struct Euler_Result{
    bigint sum;
};

static const bigint TOP = 2000000;

Euler_Result euler_main(Euler_In in){
    Sieve_Array sieve;
    sieve.size = (TOP-1)/2;
    sieve.is_composite = (bool*)malloc(sizeof(bool)*sieve.size);
    memset(sieve.is_composite, 0, sizeof(bool)*sieve.size);
    
    Primes ps;
    ps.count = 1;
    ps.max = 10;
    ps.primes = (bigint*)malloc(sizeof(bigint)*ps.max);
    ps.primes[0] = 2;    
    
    int i = sieve_partial(&ps, sieve, 0);
    while (i < sieve.size){
        int new_max = ps.max*2;
        bigint *new_primes = (bigint*)malloc(sizeof(bigint)*new_max);
        memcpy(new_primes, ps.primes, sizeof(bigint)*ps.max);
        free(ps.primes);
        ps.max = new_max;
        ps.primes = new_primes;

        i = sieve_partial(&ps, sieve, i);
    }
    
    bigint s=0;
    for (int i = 0; i < ps.count; ++i){
        s += ps.primes[i];
    }
    
    free(sieve.is_composite);
    free(ps.primes);
    
    Euler_Result result;
    result.sum = s;
    return result;
}

void euler_print(Euler_Result answer, Euler_In in){
    printf("%llu\n", answer.sum);
}

#define EULER_CHECK

bool euler_check(Euler_Result answer, Euler_In in){
    bool result = 1;
    
    bigint s=2;
    int size_ = TOP / 3;
    int *ps = (int*)malloc(sizeof(int)*size_);
    int pc = 1;
    ps[0] = 2;
    
    for (bigint n = 3; n < TOP; n += 2){
        bool prime = 1;
        for (int i = 0; i < pc; ++i){
            int p = ps[i];
            if ((n % p) == 0){
                prime = 0;
                break;
            }
        }
        if (prime){
            ps[pc++] = n;
            s += n;
        }
    }
    
    printf("answer = %llu\n", s);
    result = (s == answer.sum);
    
    return result;
}
#endif

