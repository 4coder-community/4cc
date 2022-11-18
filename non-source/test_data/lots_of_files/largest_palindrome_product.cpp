/*
 * YOUR INFO HERE!
 */

// FINAL VERSION:
// generalization for this function is not so good

// req: 0 <= a,b,c < 10
inline int palindrom_5digit(int a, int b, int c){
    int result;
    result = a*10001 + b*1010 + c*100;
    return result;
}

// req: 0 <= a,b,c < 10
inline int palindrom_6digit(int a, int b, int c){
    int result;
    result = a*100001 + b*10010 + c*1100;
    return result;
}

// req: 100 <= x < 999
inline void extract_3digit(int x, int *a, int *b, int *c){
    int y;
    y = x/10;
    *c = (x - y*10);
    
    x = y;
    y /= 10;
    *b = (x - y*10);
    
    x = y;
    y /= 10;
    *a = (x - y*10);
}

inline int extract_digit(int x, int *a){
    int y=x/10;
    *a = (x - y*10);
    return y;
}

bool is_product_of_3digit_numbers(int n){
    bool result = 0;
    for (int x = 100; x < 999; ++x){
        if (n % x == 0){
            int y = n / x;
            if (y >= 100 && y < 1000){
                result = 1;
                break;
            }
        }
    }
    return result;
}

#ifdef EULER_PROBLEM
// BUILDING AREA:
struct Euler_In{};

struct Euler_Result{
    int largest_palindrome;
};

Euler_Result euler_main(Euler_In in){
    int palindrome = -1;
    
    // check 6 digit
    for (int x = 999; x >= 100; --x){
        int a,b,c,p;
        extract_3digit(x, &a, &b, &c);
        p = palindrom_6digit(a,b,c);
        if (is_product_of_3digit_numbers(p)){
            palindrome = p;
            break;
        }
    }
    
    if (palindrome == -1){
        // check 5 digit
        for (int x = 999; x >= 100; --x){
            int a,b,c,p;
            extract_3digit(x, &a, &b, &c);
            p = palindrom_5digit(a,b,c);
            if (is_product_of_3digit_numbers(p)){
                palindrome = p;
                break;
            }
        }
    }
    
    Euler_Result result;
    result.largest_palindrome = palindrome;
    return result;
}

void euler_print(Euler_Result answer, Euler_In in){
    printf("%d\n", answer.largest_palindrome);
}

#if 0
#define EULER_CHECK

bool euler_check(Euler_Result answer, Euler_In in){
    bool result = 1;
    
    return result;
}
#endif

#endif
