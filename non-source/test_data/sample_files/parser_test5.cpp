/*
 * THIS TESTS:
 *  - putting stringify at the end of the body
 */

#define M(a) show(#a)
#define N(a) show(#a" = ", a)
#define FOO 10

M(x);
N(y);
N(FOO);

M("hello\n");
M('\\' + "");
M(L"string");

