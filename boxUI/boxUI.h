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
#if 0 // using stb_image
		struct { u8 r, g, b, a; };
#elif defined _WIN32
		struct { u8 b, g, r, a; };
#endif
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

inline BX_RGBA bx_rgbaHex(u32 color)
{
	BX_RGBA ret;
	ret.hex = color;
	return ret;
}

inline float bx_clamp(float min, float val, float max)
{
	if (val < min)
		return min;
	if (val > max)
		return max;
	return val;
}

inline float bx_lerp(float a, float b, float v)
{
	return (a * v + b * (1.f - v));
}

inline BX_RGBA bx_colorLerp(BX_RGBA a, BX_RGBA b, float v)
{
	BX_RGBA ret;
	ret.r = bx_lerp(a.r, b.r, v);
	ret.g = bx_lerp(a.g, b.g, v);
	ret.b = bx_lerp(a.b, b.b, v);
	ret.a = bx_lerp(a.a, b.a, v);
	return ret;
}

inline BX_RGBA bx_blendAlpha(BX_RGBA color, BX_RGBA old)
{
	float alpha = color.a / 255.f;
	BX_RGBA out;
	out.a = 255;
	out.r = alpha * color.r + (1.f - alpha) * old.r;
	out.g = alpha * color.g + (1.f - alpha) * old.g;
	out.b = alpha * color.b + (1.f - alpha) * old.b;
	return out;
}


enum
{
	BX_TYPE_ROOT,
	BX_TYPE_BOX,
	BX_TYPE_LIST,
};

typedef struct BX_Theme
{
	BX_Rectf margin;
	BX_RGBA fgColor, bgColor, outColor;
	i32 outThick; // positive draws inside, negative draws outside, 0 disables
	float aspect; // width/height | 0 disables, negative is undefined
	// bits [0-3] x0 y1 w2 h3 |  0 - use pixels, 1 - use percent     | Rect
	// bits [4,5] x45         | 10 - centered,  00 - left, 01 right  | Rect
	// bits [6,7] y67         | 10 - centered,  00 - top,  01 bottom | Rect
	// bits [8-B] x0 y1 w2 h3 |  0 - use pixels, 1 - use percent     | Margin
	u16 posMode;
} BX_Theme;

enum
{
	BX_RECT_ALIGN_L  =            0b0, // Origin is left side
	BX_RECT_ALIGN_CX =       0b100000, // Origin is horizontal center
	BX_RECT_ALIGN_R  =        0b10000, // Origin is right side
	BX_RECT_ALIGN_T  =            0b0, // Origin is top side
	BX_RECT_ALIGN_CY =     0b10000000, // Origin is vertical center
	BX_RECT_ALIGN_B  =      0b1000000, // Origin is bottom side

	BX_RECT_PIX_X    =            0b0, // X position uses pixels
	BX_RECT_PIX_Y    =            0b0, // Y position uses pixels
	BX_RECT_PIX_W    =            0b0, // Width uses pixels
	BX_RECT_PIX_H    =            0b0, // Height uses pixels

	BX_RECT_PER_X    =            0b1, // X position uses percents
	BX_RECT_PER_Y    =           0b10, // Y position uses percents
	BX_RECT_PER_W    =          0b100, // Width uses percents
	BX_RECT_PER_H    =         0b1000, // Height uses percents

	BX_MARG_PIX_L    =            0b0, // Left margin uses pixels
	BX_MARG_PIX_T    =            0b0, // Top margin uses pixels
	BX_MARG_PIX_R    =            0b0, // Right margin uses pixels
	BX_MARG_PIX_B    =            0b0, // Bottom margin uses pixels
	
	BX_MARG_PER_L    =    0b100000000, // Left margin uses percents
	BX_MARG_PER_T    =   0b1000000000, // Top margin uses percents
	BX_MARG_PER_R    =  0b10000000000, // Right margin uses percents
	BX_MARG_PER_B    = 0b100000000000, // Bottom margin uses percents
};

typedef struct BX_Box
{
	BX_Rectf calc, crop;
	BX_Rectf rect;
	
	BX_Theme theme;

	u8 type;
	bool hovered;

	u16 numChild;
	struct BX_Box* par;
	struct BX_Box** child;

} BX_Box;

enum
{
	BX_LIST_ROW    =    0b0, // Fill row before column
	BX_LIST_COL    =    0b1, // Fill column before row
	BX_LIST_LEFT   =    0b0, // Fill left to right
	BX_LIST_RIGHT  =   0b10, // FIll right to left
	BX_LIST_TOP    =    0b0, // Fill top to bottom
	BX_LIST_BOTTOM =  0b100, // Fill bottom to top
	BX_LIST_CLIP   =    0b0, // Clip out of bounds, use scrolling
	BX_LIST_WRAP   = 0b1000, // Place out of bounds on next line, if not able, clip instead
	/*
		Using wrap
		+-----+
		|# # #|
		|# #  |
		+-----+

		Using wrap (full)
		+-----+
		|# # #|
		|# # #|
		|# # #|
		+#----+ <- has to clip here

		Using clip
		+-----+
		|# # #|# #
		+-----+
	*/
};

// Child positioning and alignment is ignored
typedef struct BX_List
{
	BX_Box box;
	float scrollX, scrollY;
	// bit 0 | 0 - fill row first, 1 - fill column first
	// bit 1 | 0 - fill left first, 1 - fill right first
	// bit 2 | 0 - fill top first, 1 - fill bottom first
	// bit 3 | 0 - clip, 1 - wrap
	u8 order;
} BX_List;



typedef struct BX_Image
{
	u32* pixels;
	BX_Vec2u size;
} BX_Image;


BX_Box bx_createRoot(BX_Rectf rect);

BX_Box* bx_createBox(BX_Box* parent, BX_Rectf rect, BX_Theme theme);

BX_List* bx_createList(BX_Box* parent, BX_Rectf rect, BX_Theme theme, u8 order);

void bx_resizeRoot(BX_Box* root, BX_Rectf imageRect);

void bx_updateBox(BX_Box* root, BX_Vec2f mouse);

void bx_drawBox(BX_Box* root, BX_Image image);

void bx_deleteBox(BX_Box* box);