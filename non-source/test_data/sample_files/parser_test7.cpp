/*
 * THIS TESTS:
 *  - true compliant variadic macro
 *      (for the various extensions test elsewhere!)
 *  - variadic macro, properly defined
 *  - variadic macro, properly invoked
 *  - variadic macro, properly invoked with blank parameter
 */

#define DOSHOW(str, ...) show("SUPER: "str"\n", __VA_ARGS__)

DOSHOW("%d, %d", x, y);

DOSHOW("else",);
