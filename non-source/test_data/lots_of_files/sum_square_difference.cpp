/*
 * YOUR INFO HERE!
 */

// FINAL VERSION:

#ifdef EULER_PROBLEM
// BUILDING AREA:
struct Euler_In{};

struct Euler_Result{
    int difference;
};

static const int NUMBER = 100;

inline Euler_Result euler_main(Euler_In in){
    int n,x,y,d;
    n = NUMBER;
    x = (n)*(n+1)/2;
    x *= x;
    y = n*(n+1)*(2*n+1)/6;
    d = x - y;
    
    Euler_Result result;
    result.difference = d;
    return result;
}

void euler_print(Euler_Result answer, Euler_In in){
    printf("%d\n", answer.difference);
}

#define EULER_CHECK

bool euler_check(Euler_Result answer, Euler_In in){
    bool result;
    
    int c=0,d=0;
    
    for (int i = 1; i <= NUMBER; ++i){
        c+=i*i;
        d+=i;
    }
    
    d *= d;
    d -= c;
    
    printf("answer = %d\n", d);
    result = (d == answer.difference);
    
    return result;
}
#endif
