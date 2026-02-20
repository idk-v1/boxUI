#include "boxUI.h"
#include "boxUI_Int.h"

BX_Box bx_createRoot(BX_Rectf rect)
{
	BX_Box box = { 0 };
	box.rect = rect;
	box.type = BX_TYPE_ROOT;
	return box;
}

BX_Rectf bx_alignBox(BX_Rectf box, u16 posMode, BX_Rectf parent)
{
	BX_Rectf rect = box;

	// X
	if (posMode & BX_RECT_PER_W)
		rect.w = parent.w * (box.w * 0.01f);

	if (posMode & BX_RECT_PER_X)
		rect.x = parent.x + parent.w * (box.x * 0.01f);

	if (posMode & BX_RECT_ALIGN_CX)
		rect.x -= rect.w * 0.5f;
	else if (posMode & BX_RECT_ALIGN_R)
		rect.x -= rect.w;

	// Y
	if (posMode & BX_RECT_PER_H)
		rect.h = parent.h * (box.h * 0.01f);

	if (posMode & BX_RECT_PER_Y)
		rect.y = parent.y + parent.h * (box.y * 0.01f);

	if (posMode & BX_RECT_ALIGN_CX)
		rect.y -= rect.h * 0.5f;
	else if (posMode & BX_RECT_ALIGN_B)
		rect.y -= rect.h;

	return rect;
}

BX_Rectf bx_alignBoxMargin(BX_Rectf box, BX_Rectf margin, u16 posMode, BX_Rectf parent)
{
	BX_Rectf rect = bx_alignBox(box, posMode, parent);
	// Margin uses width and height as right and bottom
	// Convert from position to size

	BX_Rectf ret;

	// X
	if (posMode & BX_MARG_PER_X)
		ret.x = rect.x + rect.w * (margin.x * 0.01f);
	else
		ret.x = rect.x + margin.x;

	if (posMode & BX_MARG_PER_W)
		ret.w = rect.w * (1.f - margin.w * 0.01f) - (ret.x - rect.x);
	else
		ret.w = rect.w - margin.w - (ret.x - rect.x);

	// Y
	if (posMode & BX_MARG_PER_Y)
		ret.y = rect.y + rect.h * (margin.y * 0.01f);
	else
		ret.y = rect.y + margin.y;

	if (posMode & BX_MARG_PER_H)
		ret.h = rect.h * (1.f - margin.h * 0.01f) - (ret.y - rect.y);
	else
		ret.h = rect.h - margin.h - (ret.y - rect.y);

	return ret;
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
		for (u64 x = 0; x < rect.w; ++x)
			for (u64 y = 0; y < rect.h; ++y)
			{
				image.pixels[(x + rect.x) + (y + rect.y) * image.size.x] = 
					bx_blendAlpha(theme.bgColor, 
						bx_rgbaHex(image.pixels[(x + rect.x) + (y + rect.y) * image.size.x])
					).hex;
			}
	}
}

BX_Rectf bx_cropRect(BX_Rectf rect, BX_Rectf parent)
{
	BX_Rectf crop = rect;

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

	return crop;
}


void bx_drawBoxRec(BX_Box* box, BX_Image image, BX_Rectf parent)
{
	BX_Rectf margin = bx_alignBoxMargin(box->rect, box->theme.margin, 
		box->theme.posMode, parent);

	BX_Rectf crop = bx_cropRect(margin, parent);

	drawRect(image, bx_Rectu(roundf(crop.x), roundf(crop.y),
		roundf(crop.w), roundf(crop.h)), box->theme);
	if (box->hovered)
	{
		BX_Theme theme = box->theme;
		theme.bgColor = bx_rgba(0xFF, 0xFF, 0xFF, 0x3F);
		drawRect(image, bx_Rectu(roundf(crop.x), roundf(crop.y),
			roundf(crop.w), roundf(crop.h)), theme);
	}

	if (box->theme.outThick)
		bx_drawBoxOutline(box, image, parent, margin);

	for (u64 i = 0; i < box->numChild; ++i)
		bx_drawBoxRec(box->child[i], image, margin);
}

void bx_drawBoxOutline(BX_Box* box, BX_Image image, BX_Rectf parent, BX_Rectf bounds)
{
	BX_Box temp = { 0 };
	BX_Theme theme = { 0 };
	theme.bgColor = box->theme.outColor;
	BX_Rectf rect = { 0 };
	bx_initBox(&temp, NULL, rect, theme);
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

bool bx_updateBoxRec(BX_Box* box, BX_Rectf parent, BX_Vec2f mouse, bool hasChance)
{
	if (hasChance)
	{
		BX_Rectf margin = bx_alignBoxMargin(box->rect, box->theme.margin,
			box->theme.posMode, parent);

		BX_Rectf crop = bx_cropRect(margin, parent);

		bool hovered = bx_rectContains(crop, mouse);

		bool foundBetter = false;
		for (u64 i = 0; i < box->numChild; ++i)
			if (bx_updateBoxRec(box->child[i], margin, mouse, hovered))
				foundBetter = true;

		box->hovered = false;
		if (!foundBetter)
			box->hovered = hovered;

		return hovered;
	}
	else
	{
		box->hovered = false;

		for (u64 i = 0; i < box->numChild; ++i)
			bx_updateBoxRec(box->child[i], parent, mouse, box->hovered);
		
		return false;
	}
}

bool bx_rectContains(BX_Rectf rect, BX_Vec2f point)
{
	return (point.x >= rect.x && point.y >= rect.y &&
		point.x < rect.x + rect.w && point.y < rect.y + rect.h);
}

void bx_updateBox(BX_Box* root, BX_Vec2f mouse)
{
	// Size needs to be in pixels, just use root
	if (root->type == BX_TYPE_ROOT)
	{
		bx_updateBoxRec(root, root->rect, mouse, true);
	}
}

void bx_drawBox(BX_Box* root, BX_Image image)
{
	// Do nothing if not root, I don't wanna recurse all the way up to calc root bounds
	if (root->type == BX_TYPE_ROOT)
		bx_drawBoxRec(root, image, root->rect);
}

void bx_deleteBox(BX_Box* box)
{
	if (box)
	{
		if (box->par)
		{
			for (u64 i = 0; i < box->par->numChild; ++i)
			{
				if (box->par->child[i] == box)
				{
					box->par->child[i] = box->par->child[box->par->numChild - 1];
					--box->par->numChild;
					break;
				}
			}
		}

		for (u64 i = 0; i < box->numChild; ++i)
			bx_deleteBox(box->child[i]);
		if (box->child)
			free(box->child);
		if (box->type == BX_TYPE_ROOT)
			memset(box, 0, sizeof(BX_Box));
		else
			free(box);
	}
}