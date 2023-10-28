/*
 * THIS TESTS:
 *  - this is a memory stress test
 */

#define FOO ZZZ
#define BAR YYY

#define ZZZ 1 + YYY
#define YYY 2 + ZZZ

#define A(word) (word)
#define B(word) (word + A(word))
#define C(word) (B(word) + A(word))
#define D(m, word) (C(word) * m(word))
#define E(a,b) (D(C, a##b) + a##_longer_thing)
#define F(a,b) (D(A, #a) + (D(B, #b)) + (E(b,a)))

F(0,1);
F(FOO, BAR);
F(FOO + BAR + 10, BAR * 10 + FOO);

#define Empty
#define G1(x) G2 Empty (x)
#define G2(x) G3 Empty (x)
#define G3(x) G4 Empty (x)
#define G4(x) G5 Empty (x)
#define G5(x) G6 Empty (x)
#define G6(x) G7 Empty (x)
#define G7(x) G8 Empty (x)
#define G8(x) G9 Empty (x)
#define G9(x) (x + x)
#define Expand(x) x

G1(0);
Expand(G1(1));
Expand(Expand(G1(2)));
Expand(Expand(Expand(G1(3))));
Expand(Expand(Expand(Expand(G1(4)))));
Expand(Expand(Expand(Expand(Expand(G1(5))))));
Expand(Expand(Expand(Expand(Expand(Expand(G1(6)))))));
Expand(Expand(Expand(Expand(Expand(Expand(Expand(G1(7))))))));
Expand(Expand(Expand(Expand(Expand(Expand(Expand(Expand(G1(8)))))))));
Expand(Expand(Expand(Expand(Expand(Expand(Expand(Expand(Expand(G1(9))))))))));

#define Comma ,
#define M(a) N(a)
#define N(a,b) a+b

M(100 Comma 200);

#define FOOD(name, ...) show(#__VA_ARGS__, __VA_ARGS__); ++name##_counter; show(#name": %d\n", name##_counter)

FOOD(bard, aplha, beta, gamma, delta);

Expand(Expand(Expand(Expand(Expand(Expand(Expand(Expand(Expand(F(FOO + BAR + 10, BAR * 10 + FOO))))))))));


