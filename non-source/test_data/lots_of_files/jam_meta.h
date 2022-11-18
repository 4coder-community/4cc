/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.21.2015 (mm.dd.yyyy)
 *
 * Meta File.
 */

#ifndef DB_META_H
#define DB_META_H

#include <stdint.h>
#include <limits.h>

typedef uint8_t u8;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;

typedef int8_t i8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;

typedef i32 bool32;
typedef i8 bool8;

typedef float real32;
typedef double real64;

typedef const char* c_str;

#define internal static
#define globalvar static
#define persist static

#define globalconst static const
#define localconst static const

#define S(X) #X
#define S_(X) S(X)
#define S__LINE__ S_(__LINE__)

#define AllowLocal(name) (void)name
#define ArrayCount(array) (sizeof(array)/sizeof(array[0]))
#define Assert(check) {if(!(check)){FILE *file = fopen("errorlog", "wb"); fprintf(file, "ASSERT!"); fclose(file); *((i32*)0) = 0;}}
#define TentativeAssert(check) {if(!(check)){*((i32*)0) = 0;}}

#define Min(a,b) (((a)<(b))?(a):(b))
#define Max(a,b) (((a)>(b))?(a):(b))

#define Swap(t,a,b) {t temp = a; a = b; b = temp;}

inline internal i32
Round(real32 x){ return (i32)(x+.5f); }

inline internal i32
Floor(real32 x){ return (i32)(x)-(x<0)*((i32)x != x); }

inline internal i32
Ceil(real32 x){ return (i32)(x)+(x>0)*((i32)x != x); }

inline internal real32
DecPart(real32 x){ return x - (i32)x; }

// TODO(allen): move to math.cpp
inline internal i32
LargeRoundUp(i32 n, i32 b){ return b*(n/b); }

#define PI32 3.1415926f
#define TAU32 6.2831853f

#define TMax(t,v) globalconst t max_##t = v
TMax(u8,  255);
TMax(u16, 65535);
TMax(u32, 4294967295);
TMax(u64, 18446744073709551615);

TMax(i8,  127);
TMax(i16, 32767);
TMax(i32, 2147483647);
TMax(i64, 9223372036854775807);
#undef TMax

#define TMin(t) globalconst t min_##t = 0
TMin(u8);
TMin(u16);
TMin(u32);
TMin(u64);
#undef TMin

#define TMin(t,v) globalconst t min_##t = ((t)v)
TMin(i8,  -128);
TMin(i16, -32768);
TMin(i32, INT_MIN);
TMin(i64, -9223372036854775808LL);
#undef TMin

#define CheckFlag(flags, mask) ((flags & mask) == mask)

#define Bit_0 (1 << 0)
#define Bit_1 (1 << 1)
#define Bit_2 (1 << 2)
#define Bit_3 (1 << 3)
#define Bit_4 (1 << 4)
#define Bit_5 (1 << 5)
#define Bit_6 (1 << 6)
#define Bit_7 (1 << 7)

#define Bit_8 (1 << 8)
#define Bit_9 (1 << 9)
#define Bit_10 (1 << 10)
#define Bit_11 (1 << 11)
#define Bit_12 (1 << 12)
#define Bit_13 (1 << 13)
#define Bit_14 (1 << 14)
#define Bit_15 (1 << 15)

#define Bit_16 (1 << 16)
#define Bit_17 (1 << 17)
#define Bit_18 (1 << 18)
#define Bit_19 (1 << 19)
#define Bit_20 (1 << 20)
#define Bit_21 (1 << 21)
#define Bit_22 (1 << 22)
#define Bit_23 (1 << 23)

#define Bit_24 (1 << 24)
#define Bit_25 (1 << 25)
#define Bit_26 (1 << 26)
#define Bit_27 (1 << 27)
#define Bit_28 (1 << 28)
#define Bit_29 (1 << 29)
#define Bit_30 (1 << 30)
#define Bit_31 (1 << 31)

#define Byte_0 (0xFFU)
#define Byte_1 (0xFFU << 8)
#define Byte_2 (0xFFU << 16)
#define Byte_3 (0xFFU << 24)
#define Byte_4 (0xFFU << 32)
#define Byte_5 (0xFFU << 40)
#define Byte_6 (0xFFU << 48)
#define Byte_7 (0xFFU << 56)

#define bytes(n) (n)
#define Kbytes(n) (bytes(n) * 1024)
#define Mbytes(n) (Kbytes(n) * 1024)
#define Gbytes(n) (Mbytes(n) * 1024)
#define Tbytes(n) (Gbytes(n) * 1024)

#endif
