/*
 * THIS TESTS:
 *  - recursive macros
 */

#define M(m,n) n + m(M,n-1)

M(M,7);

#define X a + X
#define Y b + Z
#define Z c + Y

X;
Y;

