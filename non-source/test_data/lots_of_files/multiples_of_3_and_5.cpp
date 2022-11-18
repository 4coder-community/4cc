/*
 * YOUR INFO HERE!
 */

// FINAL VERSION:
int sum_mults_3_and_5(int top){
    int n = (top-1)/3;
    int m = ((n*n)+n)/2;
    int c = m*3;

    n = (top+4)/15;
    m = 15*n;
    c += m*n;
    m += 5;
    if (m < top){
        c += m;
    }
    
    return c;
}

#ifdef EULER_PROBLEM
// BUILDING AREA:
struct Euler_In{};

struct Euler_Result{
    int counter;
};

static const int TOP = 1000;

inline Euler_Result euler_main(Euler_In in){
    Euler_Result result;
    result.counter = sum_mults_3_and_5(TOP);
    return result;
}

void euler_print(Euler_Result answer, Euler_In in){
    printf("%d\n", answer.counter);
}

#define EULER_CHECK

bool euler_check(Euler_Result answer, Euler_In in){
    bool result;
    
    int counter = 0;
    for (int i = 0; i < TOP; ++i){
        if ((i % 3) == 0 || (i % 5) == 0){
            counter += i;
        }
    }
    
    printf("answer = %d\n", counter);
    result = (answer.counter == counter);
    
    return result;
}
#endif

