/*
 * THIS TESTS:
 *  - good #define
 *  - invoking good #define
 *  - using macro in arguments to itself
 *  - invoking macro with parens from a different expansion level
 *  - macro definitions with comments
 */

#define EMPTY
#define EMPTY_WITH_PAREN()
#define GOOD_NO_PAREN /*DON'T DUPLICATE ME*/ FoodBard
#define GOOD_WITH_PAREN() /*DON'T DUPLICATE ME*/ SuperDuper EMPTY
#define GOOD_WITH_ARG(a/*DON'T FRET ME*/) /*DON'T DUPLICATE ME*/ (a + 1) EMPTY_WITH_PAREN()
#define MASK GOOD_WITH_ARG(10)
#define UNUSUAL() 5 * GOOD_WITH_PAREN

GOOD_NO_PAREN;
EMPTY_WITH_PAREN;
GOOD_WITH_PAREN;
GOOD_WITH_PAREN();
GOOD_WITH_ARG(2);

GOOD_WITH_ARG(3*MASK);
UNUSUAL()();

GOOD_WITH_PAREN


