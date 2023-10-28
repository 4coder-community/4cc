/*
 * YOUR INFO HERE!
 */

// FINAL VERSION:
unsigned long long largest_prime_factor(unsigned long long n){
    unsigned long long lpf, p, s, t;
    lpf = 1;
    p = 2;
    s = 1;
    t = n / p;
    
    while (p <= t){
        if (n % p == 0){
            lpf = p;
            n /= p;
            while (n % p == 0){
                n /= p;
            }
        }
        p += s;
        s = 2;
        t = n / p;
    }
    
    if (n > lpf){
        lpf = n;
    }
    
    return lpf;
}

#ifdef EULER_PROBLEM
// BUILDING AREA:
struct Euler_In{
    unsigned long long n;
};

struct Euler_Result{
    unsigned long long lpf;
};

static const unsigned long long NUMBER = 600851475143;

Euler_Result euler_main(Euler_In in){
    Euler_Result result;
    result.lpf = largest_prime_factor(NUMBER);
    return result;
}

void euler_print(Euler_Result answer, Euler_In in){
    printf("%llu\n", answer.lpf);
}

#if 0
#define EULER_CHECK

bool euler_check(Euler_Result answer, Euler_In in){
    bool result = 1;
    
    return result;
}
#endif

#endif

