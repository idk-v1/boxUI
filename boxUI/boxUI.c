#include "boxUI.h"
#include "boxUI_Int.h"

BX_Box bx_createRoot(BX_Rectf rect)
{
	BX_Box box = { 0 };
	box.rect = rect;
	box.type = BX_TYPE_ROOT;
	return box;
}

BX_Rectf bx_alignBox(BX_Rectf box, u8 posMode, BX_Rectf parent)
{
	BX_Rectf rect = box;

	// X
	if (posMode & BX_PER_W)
		rect.w = parent.w * (box.w * 0.01f);

	if (posMode & BX_PER_X)
		rect.x = parent.x + parent.w * (box.x * 0.01f);

	if (posMode & BX_ALIGN_L);
	else if (posMode & BX_ALIGN_R)
		rect.x -= rect.w;
	else // BX_ALIGN_C
		rect.x -= rect.w * 0.5f;

	// Y
	if (posMode & BX_PER_H)
		rect.h = parent.h * (box.h * 0.01f);

	if (posMode & BX_PER_Y)
		rect.y = parent.y + parent.h * (box.y * 0.01f);

	if (posMode & BX_ALIGN_T);
	else if (posMode & BX_ALIGN_B)
		rect.y -= rect.h;
	else // BX_ALIGN_C
		rect.y -= rect.h * 0.5f;

	return rect;
}

void bx_initBox(BX_Box* box, BX_Box* parent, BX_Rectf rect, BX_Theme theme)
{
	memset(box, 0, sizeof(BX_Box));

	box->par = parent;
	box->rect = rect;
	box->theme = theme;

	box->child = NULL;
	box->numChild = 0;
}

BX_Box* bx_createBox(BX_Box* parent, BX_Rectf rect, BX_Theme theme)
{
	BX_Box* box = malloc(sizeof(BX_Box));
	if (box)
	{
		bx_initBox(box, parent, rect, theme);
		box->type = BX_TYPE_BOX;

		BX_Box** temp = realloc(parent->child, (parent->numChild + 1) * sizeof(void*));
		if (temp)
		{
			parent->child = temp;
			parent->child[parent->numChild++] = box;
		}
		else
		{
			free(box);
			box = NULL;
		}
	}
	return box;
}


void drawRect(BX_Image image, BX_Rectu rect, BX_Theme theme)
{
	if (theme.bgColor.a == 0); // Skip
	else if (theme.bgColor.a == 255) // Fast overwrite
	{
		for (u64 x = 0; x < rect.w; ++x)
			for (u64 y = 0; y < rect.h; ++y)
				image.pixels[(x + rect.x) + (y + rect.y) * image.size.x] = theme.bgColor.hex;
	}
	else // Slow blend
	{
		BX_RGBA color;
		color.a = 255;
		float alpha = theme.bgColor.a / 255.f;

		for (u64 x = 0; x < rect.w; ++x)
			for (u64 y = 0; y < rect.h; ++y)
			{
				BX_RGBA oldColor;
				oldColor.hex = image.pixels[(x + rect.x) + (y + rect.y) * image.size.x];

				color.r = alpha * theme.bgColor.r + (1.f - alpha) * oldColor.r;
				color.g = alpha * theme.bgColor.g + (1.f - alpha) * oldColor.g;
				color.b = alpha * theme.bgColor.b + (1.f - alpha) * oldColor.b;
				
				image.pixels[(x + rect.x) + (y + rect.y) * image.size.x] = color.hex;
			}
	}
}


void bx_drawBoxRec(BX_Box* box, BX_Image image, BX_Rectf parent)
{
	BX_Rectf bounds;

	if (box->type != BX_TYPE_ROOT)
		bounds = bx_alignBox(box->rect, box->theme.posMode, parent);
	else // Root is forced to be top left origin, using pixels, so no transformations
		bounds = box->rect;

	BX_Rectf crop = bounds;
	// Flip rect if negative size
	if (crop.w < 0)
	{
		crop.w = -crop.w;
		crop.x -= crop.w;
	}
	if (crop.h < 0)
	{
		crop.h = -crop.h;
		crop.y -= crop.h;
	}
	// Crop top left
	if (crop.x < parent.x)
	{
		crop.w += crop.x - parent.x;
		crop.x = parent.x;
	}
	if (crop.y < parent.y)
	{
		crop.h += crop.y - parent.y;
		crop.y = parent.y;
	}
	// Other out of bounds positions
	if (crop.x >= parent.w + parent.x)
		crop.w = 0;
	if (crop.y >= parent.h + parent.y)
		crop.h = 0;
	if (crop.w < 0)
		crop.w = 0;
	if (crop.h < 0)
		crop.h = 0;
	// Crop bottom right
	if (crop.x - parent.x + crop.w >= parent.w)
		crop.w += parent.w - (crop.x - parent.x + crop.w);
	if (crop.y - parent.y + crop.h >= parent.h)
		crop.h += parent.h - (crop.y - parent.y + crop.h);

	drawRect(image, bx_Rectu(roundf(crop.x), roundf(crop.y), roundf(crop.w), roundf(crop.h)), box->theme);

	if (box->theme.outThick)
		bx_drawBoxOutline(box, image, parent, bounds);

	for (u64 i = 0; i < box->numChild; ++i)
		bx_drawBoxRec(box->child[i], image, bounds);
}

void bx_drawBoxOutline(BX_Box* box, BX_Image image, BX_Rectf parent, BX_Rectf bounds)
{
	BX_Box temp;
	BX_Theme theme = { 0 };
	theme.bgColor = box->theme.outColor;
	theme.posMode = BX_ALIGN_L | BX_ALIGN_T;
	BX_Rectf rect = { 0 };
	bx_initBox(&temp, box, rect, theme);
	temp.type = BX_TYPE_BOX;

	if (box->theme.outThick < 0)
	{
		// Top
		rect.x = bounds.x + box->theme.outThick;
		rect.y = bounds.y + box->theme.outThick;
		rect.w = -box->theme.outThick * 2 + bounds.w;
		rect.h = -box->theme.outThick;
		temp.rect = rect;
		bx_drawBoxRec(&temp, image, parent);

		// Left
		rect.x = bounds.x + box->theme.outThick;
		rect.y = bounds.y;
		rect.w = -box->theme.outThick;
		rect.h = -box->theme.outThick + bounds.h;
		temp.rect = rect;
		bx_drawBoxRec(&temp, image, parent);

		// Right
		rect.x = bounds.x + bounds.w;
		rect.y = bounds.y;
		rect.w = -box->theme.outThick;
		rect.h = -box->theme.outThick + bounds.h;
		temp.rect = rect;
		bx_drawBoxRec(&temp, image, parent);

		// Bottom
		rect.x = bounds.x;
		rect.y = bounds.y + bounds.h;
		rect.w = bounds.w;
		rect.h = -box->theme.outThick;
		temp.rect = rect;
		bx_drawBoxRec(&temp, image, parent);
	}
	else if (box->theme.outThick > 0)
	{
		// Top
		rect.x = bounds.x;
		rect.y = bounds.y;
		rect.w = bounds.w;
		rect.h = box->theme.outThick;
		temp.rect = rect;
		bx_drawBoxRec(&temp, image, parent);

		// Left
		rect.x = bounds.x;
		rect.y = bounds.y + box->theme.outThick;
		rect.w = box->theme.outThick;
		rect.h = bounds.h - box->theme.outThick;
		temp.rect = rect;
		bx_drawBoxRec(&temp, image, parent);

		// Right
		rect.x = bounds.x + bounds.w - box->theme.outThick;
		rect.y = bounds.y + box->theme.outThick;
		rect.w = box->theme.outThick;
		rect.h = bounds.h - box->theme.outThick;
		temp.rect = rect;
		bx_drawBoxRec(&temp, image, parent);

		// Bottom
		rect.x = bounds.x + box->theme.outThick;
		rect.y = bounds.y + bounds.h - box->theme.outThick;
		rect.w = bounds.w - box->theme.outThick * 2;
		rect.h = box->theme.outThick;
		temp.rect = rect;
		bx_drawBoxRec(&temp, image, parent);
	}
}

void bx_drawBox(BX_Box* box, BX_Image image)
{
	// Do nothing if not root, I don't wanna recurse all the way up to calc root bounds
	if (box->type == BX_TYPE_ROOT)
		bx_drawBoxRec(box, image, box->rect);
}

void bx_deleteBox(BX_Box* box)
{
	if (box)
	{
		for (u64 i = 0; i < box->numChild; ++i)
			bx_deleteBox(box->child[i]);
		if (box->type == BX_TYPE_ROOT)
			memset(box, 0, sizeof(BX_Box));
		else
			free(box);
	}
}