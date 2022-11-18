/*
 * YOUR INFO HERE!
 */

// FINAL VERSION:
int sum_even_fibonacci(int top){
    int counter = 0;
    
    int a, b, c;
    a = 1;
    b = 1;
    c = 2;
    
    for (;c < top;){
        counter += c;
        
        a = b + c;
        b = c + a;
        c = a + b;
    }
}

#ifdef EULER_PROBLEM
// BUILDING AREA:
struct Euler_In{};

struct Euler_Result{
    int counter;
};

static const int TOP = 4000000;

inline Euler_Result euler_main(Euler_In in){
    Euler_Result result;
    result.counter = sum_even_fibonacci(TOP);
    return result;
}

void euler_print(Euler_Result answer, Euler_In in){
    printf("%d\n", answer.counter);
}

#define EULER_CHECK

bool euler_check(Euler_Result answer, Euler_In in){
    bool result;
    
    int counter = 0;
    
    int a, b, c;
    a = 0;
    b = 1;
    c = 0;
    
    for (;c < TOP;){
        if (c % 2 == 0){
            counter += c;
        }
        c = a + b;
        a = b;
        b = c;
    }
    
    printf("answer = %d\n", counter);
    result = (counter == answer.counter);
    
    return result;
}
#endif

