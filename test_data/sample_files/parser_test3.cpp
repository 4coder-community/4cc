/*
 * THIS TESTS:
 *  - wrong number of arguments to macro
 */

#define NoArg() X
#define OneArg(a) (a + 1)

NoArg(5);
OneArg();


