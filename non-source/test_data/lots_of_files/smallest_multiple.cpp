/*
 * YOUR INFO HERE!
 */

// FINAL VERSION:

// req: size of mem >= CEIL[top/2]*8
int smallest_multiple(int top, int *mem){
    int *factors = mem;
    int factor_count = 0;
    int n,m,o,r=2;
    factors[factor_count++] = 2;
    for (int i = 3; i <= top; ++i){
        n = i;
        for (int j = 0; j < factor_count && n > 1; ++j){
            m = factors[j];
            o = n / m;
            if (o*m == n){
                n = o;
            }
        }
        if (n > 1){
            factors[factor_count++] = n;
            r *= n;
        }
    }
    return r;
}

#ifdef EULER_PROBLEM
// BUILDING AREA:
struct Euler_In{};

struct Euler_Result{
    int smallest_multiple;
};

static const int TOP = 20;

inline Euler_Result euler_main(Euler_In in){
    int f[TOP];
    
    Euler_Result result;
    result.smallest_multiple = smallest_multiple(TOP, f);
    return result;
}

void euler_print(Euler_Result answer, Euler_In in){
    printf("%d\n", answer.smallest_multiple);
}

#define EULER_CHECK

bool euler_check(Euler_Result answer, Euler_In in){
    bool result;

    bool not_found = 1;
    int x = TOP - 1;
    while (not_found){
        ++x;
        not_found = 0;
        for (int y = 2; y <= TOP; ++y){
            if (x % y != 0){
                not_found = 1;
                break;
            }
        }
    }
    
    printf("answer = %d\n", x);
    result = (answer.smallest_multiple == x);
    
    return result;
}

#endif


