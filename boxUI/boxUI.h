#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef  int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef  uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;

#define BX_Vec2(type, name) \
typedef struct BX_Vec2##name\
{ \
	type x, y; \
} BX_Vec2##name; \
static BX_Vec2##name bx_Vec2##name(type x, type y) \
{ \
	BX_Vec2##name vec = { x, y }; \
	return vec; \
}

#define BX_Rect(type, name) \
typedef struct BX_Rect##name\
{ \
	type x, y, w, h; \
} BX_Rect##name; \
static BX_Rect##name bx_Rect##name(type x, type y, type w, type h) \
{ \
	BX_Rect##name rect = { x, y, w, h }; \
	return rect; \
}

BX_Vec2(i32, i);
BX_Vec2(u32, u);
BX_Vec2(f32, f);

BX_Rect(i32, i);
BX_Rect(u32, u);
BX_Rect(f32, f);

typedef struct BX_RGBA
{
	union
	{
		u32 hex;
		// Change order depending on system
		struct { u8 r, g, b, a; };
	};
} BX_RGBA;

inline BX_RGBA bx_rgba(u8 r, u8 g, u8 b, u8 a)
{
	BX_RGBA rgba;
	rgba.r = r;
	rgba.g = g;
	rgba.b = b;
	rgba.a = a;
	return rgba;
}

enum
{
	BX_TYPE_ROOT,
	BX_TYPE_BOX,
};

typedef struct BX_Theme
{
	BX_Rectf margin;
	BX_RGBA fgColor, bgColor, outColor;
	i32 outThick; // positive draws inside, negative draws outside, 0 disables
	// bits [0-7] are for rect alignment
	// bits [8-F] are for margin alignment
	// bits [0-3] x0 y1 w2 h3 |  0 - use pixels, 1 - use percent
	// bits [4,5] x45         | 00 - centered,  10 - left, 01 right
	// bits [6,7] y67         | 00 - centered,  10 - top,  01 bottom
	// bits [8-B] x0 y1 w2 h3 |  0 - use pixels, 1 - use percent
	// bits [C,D] x45         | 00 - centered,  10 - left, 01 right
	// bits [E,F] y67         | 00 - centered,  10 - top,  01 bottom
	u16 posMode;
} BX_Theme;

enum
{
	BX_RECT_ALIGN_C =                0b0,
	BX_RECT_ALIGN_L =           0b100000,
	BX_RECT_ALIGN_R =            0b10000,
	BX_RECT_ALIGN_T =         0b10000000,
	BX_RECT_ALIGN_B =          0b1000000,
	BX_RECT_PIX_X   =                0b0,
	BX_RECT_PIX_Y   =                0b0,
	BX_RECT_PIX_W   =                0b0,
	BX_RECT_PIX_H   =                0b0,
	BX_RECT_PER_X   =                0b1,
	BX_RECT_PER_Y   =               0b10,
	BX_RECT_PER_W   =              0b100,
	BX_RECT_PER_H   =             0b1000,

	BX_MARG_ALIGN_C =                0b0,
	BX_MARG_ALIGN_L =   0b10000000000000,
	BX_MARG_ALIGN_R =    0b1000000000000,
	BX_MARG_ALIGN_T = 0b1000000000000000,
	BX_MARG_ALIGN_B =  0b100000000000000,
	BX_MARG_PIX_X   =                0b0,
	BX_MARG_PIX_Y   =                0b0,
	BX_MARG_PIX_W   =                0b0,
	BX_MARG_PIX_H   =                0b0,
	BX_MARG_PER_X   =        0b100000000,
	BX_MARG_PER_Y   =       0b1000000000,
	BX_MARG_PER_W   =      0b10000000000,
	BX_MARG_PER_H   =     0b100000000000,
};

typedef struct BX_Box
{
	BX_Rectf rect;
	
	BX_Theme theme;

	u16 type;

	u16 numChild;
	struct BX_Box* par;
	struct BX_Box** child;

} BX_Box;

typedef struct BX_Image
{
	u32* pixels;
	BX_Vec2u size;
} BX_Image;


BX_Box bx_createRoot(BX_Rectf rect);

BX_Box* bx_createBox(BX_Box* parent, BX_Rectf rect, BX_Theme theme);

void bx_drawBox(BX_Box* box, BX_Image image);

void bx_deleteBox(BX_Box* box);