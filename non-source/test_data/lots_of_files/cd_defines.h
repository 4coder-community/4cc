/*
 * Useful standard defines
 */

// TOP

#ifndef CD_DEFINES_H
#define CD_DEFINES_H

#include <stdint.h>

#define ArrayCount(a) (sizeof(a)/sizeof(*(a)))

#define AllowLocal(x) ((void)x)

#ifdef DEBUG
# define Assert(x) do{ if (!(x)) { *((int*)0) = 0; } }while(0)
#else
# define Assert(x)
#endif

#define InvalidCodePath Assert(!"Invalid code path!")
#define NotImplemented Assert(!"Implement this!")

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef i32 rptr32;
typedef i64 rptr64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u8 b8;
typedef u16 b16;
typedef u32 b32;

typedef float f32;
typedef double f64;

#define Kbytes(n) ((n) << 10)
#define Mbytes(n) ((n) << 20)
#define Gbytes(n) ((u64)(n) << 30)
#define Tbytes(n) ((u64)(n) << 40)

#define Min(a,b) (((a)<(b))?(a):(b))
#define Max(a,b) (((a)>(b))?(a):(b))

#define PtrRead(ptr, type) *(type*)ptr; ptr += sizeof(type)

#define max_u8  (u32)(0xFF)
#define max_u16 (u32)(0xFFFF)
#define max_u32 (u32)(0xFFFFFFFF)
#define max_u64 (u64)(0xFFFFFFFFFFFFFFFF)

#define min_i8  (i32)(0x80)
#define min_i16 (i32)(0x8000)
#define min_i32 (i32)(0x80000000)
#define min_i64 (i64)(0x8000000000000000)

#define max_i8  (i32)(0x7F)
#define max_i16 (i32)(0x7FFF)
#define max_i32 (i32)(0x7FFFFFFF)
#define max_i64 (i64)(0x7FFFFFFFFFFFFFFF)

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


//
// Resolution Defines
//

#define DEFAULT_WIDTH 1366
#define DEFAULT_HEIGHT 768

f32 visible_units = 20.f;
f32 unit_over_pixel = visible_units / DEFAULT_WIDTH;
f32 pixel_over_unit = DEFAULT_WIDTH / visible_units;
f32 visible_top = DEFAULT_HEIGHT * unit_over_pixel;

f32 x_origin_p = 120.f;
f32 y_origin_p = DEFAULT_HEIGHT / 2.f;

f32 x_origin_u = x_origin_p * unit_over_pixel;
f32 y_origin_u = y_origin_p * unit_over_pixel;

// TODO(allen): I want these in a "dev-global" section
// where they can be edited, exported, and seen by any
// module.
i32 lanes_per_rail = 5;
i32 total_lanes = lanes_per_rail * 9;
i32 half_visible_lanes = 2;
i32 total_visible_lanes = 2*half_visible_lanes + 1;


//
// Asset Backup Defines
//

// NOTE(allen): The maximum number of back up
// files to keep in existence in the backup
// folder. If this limit is exceeded the
// oldest backup is thrown out to make room.
#define MAX_BACKUP_COUNT 100

// NOTE(allen): The number of seconds between
// backup events.
#define BACKUP_FREQUENCY 120.f



#endif

// BOTTOM
