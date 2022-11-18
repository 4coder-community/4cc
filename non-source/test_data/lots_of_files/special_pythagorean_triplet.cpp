/*
 * YOUR INFO HERE!
 */

// FINAL VERSION:
#ifdef EULER_PROBLEM
// BUILDING AREA:

struct Euler_In{};

struct Euler_Result{
    int product;
};

static const int PYTHAG_SUM = 1000;

Euler_Result euler_main(Euler_In in){
#if 0
    int a,b,c=1;
    int sa,sb,sc;
    int g=1;
    
    while (g){
        ++c;
        
    }
#endif    
#if 1
    int a,b,c;
    int sa,sb,sc;
    int e;
    int g=1;
    
    c = ((PYTHAG_SUM+2)/3) + 1;
    
    while (g){
        ++c;
        sc = c*c;
        b = c-1;
        a = PYTHAG_SUM-b-c;
        while (a<b){
            sa = a*a;
            sb = b*b;
            if (sa+sb == sc){
                g=0;
                break;
            }
            --b;
            a = PYTHAG_SUM-b-c;
        }
    }
    
#endif
    
    int p=a*b*c;
    Euler_Result result;
    result.product = p;
    return result;
}

void euler_print(Euler_Result answer, Euler_In in){
    printf("%d\n", answer.product);
}

#define EULER_CHECK

bool euler_check(Euler_Result answer, Euler_In in){
    bool result = 1;
    
    int a,b,c=0;
    int sa,sb,sc;
    int e;
    int g=1;
    
    while (g){
        ++c;
        sc = c*c;
        a = 0;
        b = PYTHAG_SUM-a-c;
        while (a<b){
            sa = a*a;
            sb = b*b;
            if (sa+sb == sc){
                g=0;
                break;
            }
            ++a;
            b = PYTHAG_SUM-a-c;
        }
    }
    
    int p=a*b*c;
    
    printf("answer = %d, a=%d, b=%d, c=%d\n", p, a, b, c);
    result = (p == answer.product);
    
    return result;
}
#endif

