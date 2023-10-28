/*
 * THIS TESTS:
 *  - pasting two non-arguments
 *  - pasting argument to non-argument
 *  - pasting non-argument to argument
 *  - pasting two arguments
 *  - pasting arguments with extended bodies
 *  - applying both paste and not to one argument
 *  UNFINISHED
 *  - putting paste at the beginning of the body
 *  - putting paste at the end of the body
 */

#define NN() food##bard
#define AN(a) a##_counter
#define NA(a) ptr_##a
#define AA(a,b) a##b

NN();
AN(T1);
NA(T2);
AA(C,D)

#define S1 QQ
#define S2 RR

AN(x += S1);
NA(x += S2);
AA(x += S1, y += S2);
