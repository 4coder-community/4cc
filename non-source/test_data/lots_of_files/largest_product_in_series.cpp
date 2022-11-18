/*
 * YOUR INFO HERE!
 */

// FINAL VERSION:
#ifdef EULER_PROBLEM
// BUILDING AREA:
typedef unsigned long long bigint;

struct Euler_In{};

struct Euler_Result{
    bigint largest_product;
};

static const char DIGITS[] =
"73167176531330624919225119674426574742355349194934"
"96983520312774506326239578318016984801869478851843"
"85861560789112949495459501737958331952853208805511"
"12540698747158523863050715693290963295227443043557"
"66896648950445244523161731856403098711121722383113"
"62229893423380308135336276614282806444486645238749"
"30358907296290491560440772390713810515859307960866"
"70172427121883998797908792274921901699720888093776"
"65727333001053367881220235421809751254540594752243"
"52584907711670556013604839586446706324415722155397"
"53697817977846174064955149290862569321978468622482"
"83972241375657056057490261407972968652414535100474"
"82166370484403199890008895243450658541227588666881"
"16427171479924442928230863465674813919123162824586"
"17866458359124566529476545682848912883142607690042"
"24219022671055626321111109370544217506941658960408"
"07198403850962455444362981230987879927244284909188"
"84580156166097919133875499200524063689912560717606"
"05886116467109405077541002256983155200055935729725"
"71636269561882670428252483600823257530420752963450";

static const int DIGITS_SUB = 13;

static const int DIGITS_LEN = sizeof(DIGITS)-1;

Euler_Result euler_main(Euler_In in){
    char ds[DIGITS_LEN];
    bigint c=0,x=1,b=0,d;
    for (int i = 0; i < DIGITS_LEN; ++i){
        d = ds[i] = DIGITS[i] - '0';
        if (d == 0){
            c = 0;
            x = 1;
        }
        else{
            if (c < DIGITS_SUB){
                x *= d;
                ++c;
                if (c == DIGITS_SUB &&x > b){
                    b = x;
                }
            }
            else{
                x /= ds[i - DIGITS_SUB];
                x *= d;
                if (x > b){
                    b = x;
                }
            }
        }
    }
    
    Euler_Result result;
    result.largest_product = b;
    return result;
}

void euler_print(Euler_Result answer, Euler_In in){
    printf("%llu\n", answer.largest_product);
}

#define EULER_CHECK

bool euler_check(Euler_Result answer, Euler_In in){
    bool result = 1;
    
    bigint x,b=0;
    int stop = DIGITS_LEN - DIGITS_SUB + 1;
    
    for (int i = 0; i < stop; ++i){
        x = 1;
        for (int j = 0; j < DIGITS_SUB; ++j){
            x *= (DIGITS[i+j] - '0');
        }
        if (x > b){
            b = x;
        }
    }
    
    printf("answer = %llu\n", b);
    result = (b == answer.largest_product);
    
    return result;
}
#endif

