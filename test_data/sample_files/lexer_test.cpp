/*
 * THIS TESTS:
 * - block comment, line comment, comment merging
 * - preprocessor body, preprocessor include
 * - keyword detection
 * - integer constant, with and without suffix
 * - hex form integer constant
 * - floating constant, with and without suffix
 * - float constant with scientific notation
 * - boolean constant
 * - string constant
 * - more than 1 kb worth of token data
 */
// Each line comment and block comment is emitted separately
// but then merged in the cpp_push_token function

#define PREPROCESSOR
#include <nonsense.h>

#define MULTI_LINE\
    a(            \
        b+x, c+x, \
                  )

#define PP_OPS(n) printf(#n" %d\n",             \
                         counter_##n)

extern void* foob();
static inline void fbar();

#if (contets - here + labeled == as pp_body)

int main(){
    // constant expressions
    float x = 3.5883f;
    char c = 0x34;
    foob(.23e-10, -32ll, -32.);
    int d = 5;
    if (true){
        char stuff[] = "gotta get that string constant \\ with \" crazieness in there \n";
    }

    // filler to put the memory requirement for the tokens over 1KB
    int x,y,z,h;
    x=(y*x)+(-z*y)+h;
    h+=x<<z;
    z=++h+=x-10;
    y>>=x?z:h;
    return 0;
}

#endif

#include "teststuff.h"



