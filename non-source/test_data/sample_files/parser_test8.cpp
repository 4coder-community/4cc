s/*
 * THIS TESTS:
 *  - true compliant variadic macro
 *      (for the various extensions test elsewhere!)
 *  - variadic macro, improperly invoked
 *  - variadic macro, improperly defined
 */

#define DOSHOW(str, ...) show("SUPER: "str"\n", __VA_ARGS__)

DOSHOW("something");

#define DONTSHOW(str, ..., b) show("SUPER: "str" %d\n", __VA_ARGS__, b)

DONTSHOW("heyo! %f", 0.5, 10);
