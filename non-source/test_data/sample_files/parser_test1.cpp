/*
 * THIS TESTS:
 *  - #define errors
 *  - attempting to invoke bad #define
 */

#define
#define "THIS IS BAD"
#define GOT_BAD_PAREN1(
#define GOT_BAD_PAREN2(a
#define GOT_BAD_PAREN3(a,
#define GOT_BAD_PAREN4(a,)
#define GOT_BAD_PAREN5(,b)
#define GOT_BAD_PAREN6(+a,b)
#define GOT_BAD_PAREN7(a b)
#define GOT_BAD_PAREN8(a,+b)

GOT_BAD_PAREN1();
GOT_BAD_PAREN2();
GOT_BAD_PAREN3();
GOT_BAD_PAREN4();
GOT_BAD_PAREN5();
GOT_BAD_PAREN6();
GOT_BAD_PAREN7();
GOT_BAD_PAREN8();

