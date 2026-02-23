#include "boxUI.h"
#include "boxUI_Int.h"

BX_Box bx_createRoot(BX_Rectf rect)
{
	BX_Box box = { 0 };
	box.rect = rect;
	box.calc = rect;
	box.crop = rect;
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

BX_Rectf bx_alignBoxMargin(BX_Rectf box, BX_Theme theme, BX_Rectf parent)
{
	BX_Rectf rect = bx_applyAspectRatio(bx_alignBox(box, theme.posMode, parent), theme);
	// Margin uses width and height as right and bottom
	// Convert from position to size

	BX_Rectf ret;

	// X
	if (theme.posMode & BX_MARG_PER_L)
		ret.x = rect.x + rect.w * (theme.margin.x * 0.01f);
	else
		ret.x = rect.x + theme.margin.x;

	if (theme.posMode & BX_MARG_PER_R)
		ret.w = rect.w * (1.f - theme.margin.w * 0.01f) - (ret.x - rect.x);
	else
		ret.w = rect.w - theme.margin.w - (ret.x - rect.x);

	// Y
	if (theme.posMode & BX_MARG_PER_T)
		ret.y = rect.y + rect.h * (theme.margin.y * 0.01f);
	else
		ret.y = rect.y + theme.margin.y;

	if (theme.posMode & BX_MARG_PER_B)
		ret.h = rect.h * (1.f - theme.margin.h * 0.01f) - (ret.y - rect.y);
	else
		ret.h = rect.h - theme.margin.h - (ret.y - rect.y);

	return ret;
}

BX_Rectf bx_applyAspectRatio(BX_Rectf box, BX_Theme theme)
{
	if (theme.aspect != 0.f)
	{
		float aspect = (float)box.w / (float)box.h;
		if (aspect < theme.aspect)
		{
			float newH = box.h * (aspect / theme.aspect);
			if (theme.posMode & BX_RECT_ALIGN_CY)
				box.y += (box.h - newH) * 0.5f;
			else if (theme.posMode & BX_RECT_ALIGN_B)
				box.y += (box.h - newH);
			box.h = newH;
		}
		else if (aspect > theme.aspect)
		{
			float newW = box.w * (theme.aspect / aspect);
			if (theme.posMode & BX_RECT_ALIGN_CX)
				box.x += (box.w - newW) * 0.5f;
			else if (theme.posMode & BX_RECT_ALIGN_R)
				box.x += (box.w - newW);
			box.w = newW;
		}
	}

	return box;
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

void bx_callDrawType(BX_Box* box, BX_Image image)
{
	switch (box->type)
	{
	case BX_TYPE_ROOT:
	case BX_TYPE_BOX:
	case BX_TYPE_LIST:
		bx_drawBoxRec(box, image);
		break;
	}
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
			printf("realloc failed %d\n", __LINE__);
			free(box);
			box = NULL;
		}
	}
	else
		printf("malloc failed %d\n", __LINE__);
	return box;
}

BX_List* bx_createList(BX_Box* parent, BX_Rectf rect, BX_Theme theme, u8 order)
{
	BX_List* list = malloc(sizeof(BX_List));
	if (list)
	{
		bx_initBox(&list->box, parent, rect, theme);
		list->box.type = BX_TYPE_LIST;
		list->order = order;
		list->scrollX = 0.f;
		list->scrollY = 0.f;

		BX_Box** temp = realloc(parent->child, (parent->numChild + 1) * sizeof(void*));
		if (temp)
		{
			parent->child = temp;
			parent->child[parent->numChild++] = list;
		}
		else
		{
			printf("realloc failed %d\n", __LINE__);
			free(list);
			list = NULL;
		}
	}
	else
		printf("malloc failed %d\n", __LINE__);
	return list;
}

void bx_resizeRoot(BX_Box* root, BX_Rectf imageRect)
{
	root->rect = imageRect;
	root->calc = imageRect;
	root->crop = imageRect;

	bx_recalcBox(root, imageRect);
}

void bx_recalcBox(BX_Box* box, BX_Rectf imageRect)
{
	if (box->type == BX_TYPE_ROOT)
	{
		for (u64 i = 0; i < box->numChild; ++i)
			bx_resizeRec(box->child[i], imageRect);
	}
	else
	{
		// Lists need to recalc all children after the one that changes, easier to do all
		// Could be bad for example:
		// A file viewer showing system32, but something is added/deleted
		if (box->par && box->par->type == BX_TYPE_LIST)
			bx_resizeRec(box->par, imageRect);
		else
			bx_resizeRec(box, imageRect);
	}
}

void bx_resizeRec(BX_Box* box, BX_Rectf imageRect)
{
	box->calc = bx_alignBoxMargin(box->rect, box->theme, box->par->calc);

	box->crop = bx_cropRect(box->calc, box->par->crop);

	if (box->type == BX_TYPE_LIST)
		bx_resizeListRec(box, imageRect);
	else
		for (u64 i = 0; i < box->numChild; ++i)
			bx_resizeRec(box->child[i], imageRect);
}

void bx_resizeListRec(BX_List* list, BX_Rectf imageRect)
{
	float childX = 0.f;
	if (list->order & BX_LIST_RIGHT)
		childX = list->box.calc.w;
	float childY = 0.f;
	if (list->order & BX_LIST_BOTTOM)
		childY = list->box.calc.h;
	float maxChildW = 0.f;
	float maxChildH = 0.f;
	for (u64 i = 0; i < list->box.numChild; ++i)
	{
		BX_Rectf child = bx_applyAspectRatio(bx_alignBox(list->box.child[i]->rect,
			list->box.child[i]->theme.posMode, list->box.calc), 
			list->box.child[i]->theme);

		if (list->order & BX_LIST_WRAP)
		{
			if (list->order & BX_LIST_COL)
			{
				if (list->order & BX_LIST_BOTTOM)
				{
					if (childY - child.h < 0)
					{
						if (list->order & BX_LIST_RIGHT)
							childX -= maxChildW;
						else // BX_LIST_LEFT
							childX += maxChildW;
						childY = list->box.calc.h;
						maxChildW = 0;
						maxChildH = 0;
					}
				}
				else // BX_LIST_TOP
				{
					if (childY + child.h > list->box.calc.h)
					{
						if (list->order & BX_LIST_RIGHT)
							childX -= maxChildW;
						else // BX_LIST_LEFT
							childX += maxChildW;
						childY = 0;
						maxChildW = 0;
						maxChildH = 0;
					}
				}
			}
			else // BX_LIST_ROW
			{
				if (list->order & BX_LIST_RIGHT)
				{
					if (childX - child.w < 0)
					{
						if (list->order & BX_LIST_BOTTOM)
							childY -= maxChildH;
						else // BX_LIST_TOP
							childY += maxChildH;
						childX = list->box.calc.w;
						maxChildW = 0;
						maxChildH = 0;
					}
				}
				else // BX_LIST_LEFT
				{
					if (childX + child.w > list->box.calc.w)
					{
						if (list->order & BX_LIST_BOTTOM)
							childY -= maxChildH;
						else // BX_LIST_TOP
							childY += maxChildH;
						childX = 0;
						maxChildW = 0;
						maxChildH = 0;
					}
				}
			}
		}

		BX_Rectf oldRect = list->box.child[i]->rect;
		u16 oldPosMode = list->box.child[i]->theme.posMode;
		list->box.child[i]->rect = bx_Rectf(list->box.calc.x + childX, 
			list->box.calc.y + childY, child.w, child.h);
		list->box.child[i]->theme.posMode = 0;
		if (list->order & BX_LIST_RIGHT)
			list->box.child[i]->theme.posMode |= BX_RECT_ALIGN_R;
		if (list->order & BX_LIST_BOTTOM)
			list->box.child[i]->theme.posMode |= BX_RECT_ALIGN_B;

		bx_resizeRec(list->box.child[i], imageRect);
		list->box.child[i]->rect = oldRect;
		list->box.child[i]->theme.posMode = oldPosMode;

		if (child.w > maxChildW)
			maxChildW = child.w;
		if (child.h > maxChildH)
			maxChildH = child.h;

		if (list->order & BX_LIST_COL)
		{
			if (list->order & BX_LIST_BOTTOM)
				childY -= child.h;
			else // BX_LIST_TOP
				childY += child.h;
		}
		else // BX_LIST_ROW
		{
			if (list->order & BX_LIST_RIGHT)
				childX -= child.w;
			else // BX_LIST_LEFT
				childX += child.w;
		}
	}
}


void bx_drawRect(BX_Image image, BX_Rectu rect, BX_RGBA color)
{
	if (color.a == 0); // Skip
	else if (color.a == 255) // Fast overwrite
	{
		for (u64 x = 0; x < rect.w; ++x)
			for (u64 y = 0; y < rect.h; ++y)
				image.pixels[(x + rect.x) + (y + rect.y) * image.size.x] = color.hex;
	}
	else // Slow blend
	{
		for (u64 x = 0; x < rect.w; ++x)
			for (u64 y = 0; y < rect.h; ++y)
			{
				image.pixels[(x + rect.x) + (y + rect.y) * image.size.x] = 
					bx_blendAlpha(color, 
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
	if (crop.w < 0)
		crop.w = 0;
	if (crop.h < 0)
		crop.h = 0;

	return crop;
}


void bx_drawBoxRec(BX_Box* box, BX_Image image)
{
	bx_drawRect(image, bx_Rectu(box->crop.x, box->crop.y, 
		box->crop.w, box->crop.h), box->theme.bgColor);

	if (box->theme.outThick)
		bx_drawBoxOutline(box, image);

	for (u64 i = 0; i < box->numChild; ++i)
		bx_callDrawType(box->child[i], image);
}

void bx_drawBoxOutline(BX_Box* box, BX_Image image)
{
	BX_Theme theme = { 0 };
	theme.bgColor = box->theme.outColor;
	BX_Rectf rect = { 0 };
	BX_Rectf bounds = box->calc;

	if (box->theme.outThick < 0)
	{
		// Top
		rect.x = bounds.x + box->theme.outThick;
		rect.y = bounds.y + box->theme.outThick;
		rect.w = -box->theme.outThick * 2 + bounds.w;
		rect.h = -box->theme.outThick;
		rect = bx_cropRect(bx_cropRect(rect, box->par->crop), 
			bx_Rectf(0, 0, image.size.x, image.size.y));
		bx_drawRect(image, bx_Rectu(rect.x, rect.y, rect.w, rect.h), theme.bgColor);

		// Left
		rect.x = bounds.x + box->theme.outThick;
		rect.y = bounds.y;
		rect.w = -box->theme.outThick;
		rect.h = -box->theme.outThick + bounds.h;
		rect = bx_cropRect(bx_cropRect(rect, box->par->crop),
			bx_Rectf(0, 0, image.size.x, image.size.y));
		bx_drawRect(image, bx_Rectu(rect.x, rect.y, rect.w, rect.h), theme.bgColor);

		// Right
		rect.x = bounds.x + bounds.w;
		rect.y = bounds.y;
		rect.w = -box->theme.outThick;
		rect.h = -box->theme.outThick + bounds.h;
		rect = bx_cropRect(bx_cropRect(rect, box->par->crop),
			bx_Rectf(0, 0, image.size.x, image.size.y));
		bx_drawRect(image, bx_Rectu(rect.x, rect.y, rect.w, rect.h), theme.bgColor);

		// Bottom
		rect.x = bounds.x;
		rect.y = bounds.y + bounds.h;
		rect.w = bounds.w;
		rect.h = -box->theme.outThick;
		rect = bx_cropRect(bx_cropRect(rect, box->par->crop),
			bx_Rectf(0, 0, image.size.x, image.size.y));
		bx_drawRect(image, bx_Rectu(rect.x, rect.y, rect.w, rect.h), theme.bgColor);
	}
	else if (box->theme.outThick > 0)
	{
		// Top
		rect.x = bounds.x;
		rect.y = bounds.y;
		rect.w = bounds.w;
		rect.h = box->theme.outThick;
		rect = bx_cropRect(bx_cropRect(rect, box->par->crop),
			bx_Rectf(0, 0, image.size.x, image.size.y));
		bx_drawRect(image, bx_Rectu(rect.x, rect.y, rect.w, rect.h), theme.bgColor);

		// Left
		rect.x = bounds.x;
		rect.y = bounds.y + box->theme.outThick;
		rect.w = box->theme.outThick;
		rect.h = bounds.h - box->theme.outThick;
		rect = bx_cropRect(bx_cropRect(rect, box->par->crop),
			bx_Rectf(0, 0, image.size.x, image.size.y));
		bx_drawRect(image, bx_Rectu(rect.x, rect.y, rect.w, rect.h), theme.bgColor);

		// Right
		rect.x = bounds.x + bounds.w - box->theme.outThick;
		rect.y = bounds.y + box->theme.outThick;
		rect.w = box->theme.outThick;
		rect.h = bounds.h - box->theme.outThick;
		rect = bx_cropRect(bx_cropRect(rect, box->par->crop),
			bx_Rectf(0, 0, image.size.x, image.size.y));
		bx_drawRect(image, bx_Rectu(rect.x, rect.y, rect.w, rect.h), theme.bgColor);

		// Bottom
		rect.x = bounds.x + box->theme.outThick;
		rect.y = bounds.y + bounds.h - box->theme.outThick;
		rect.w = bounds.w - box->theme.outThick * 2;
		rect.h = box->theme.outThick;
		rect = bx_cropRect(bx_cropRect(rect, box->par->crop),
			bx_Rectf(0, 0, image.size.x, image.size.y));
		bx_drawRect(image, bx_Rectu(rect.x, rect.y, rect.w, rect.h), theme.bgColor);
	}
}

BX_Box* bx_updateBoxRec(BX_Box* box, BX_Vec2f mouse, bool hasChance)
{
	box->hovered = false;
	if (hasChance)
	{
		bool hovered = bx_rectContains(box->crop, mouse);
		if (hovered)
		{
			BX_Box* ret = NULL;
			for (u64 i = 0; i < box->numChild; ++i)
			{
				BX_Box* child = bx_updateBoxRec(box->child[i], mouse, hovered);
				if (child)
					ret = child;
			}

			if (!ret)
			{
				box->hovered = hovered;
				ret = box;
			}

			return ret;
		}
	}
	else
	{
		for (u64 i = 0; i < box->numChild; ++i)
			bx_updateBoxRec(box->child[i], mouse, false);
	}
	return NULL;
}

bool bx_rectContains(BX_Rectf rect, BX_Vec2f point)
{
	return (point.x >= rect.x && point.y >= rect.y &&
		point.x < rect.x + rect.w && point.y < rect.y + rect.h);
}

BX_Box* bx_updateBox(BX_Box* root, BX_Vec2f mouse)
{
	return bx_updateBoxRec(root, mouse, true);
}

void bx_drawBox(BX_Box* root, BX_Image image)
{
	bx_drawBoxRec(root, image);
}

void bx_deleteBox(BX_Box* box)
{
	if (box)
	{
		if (box->par)
		{
			if (box->par->numChild == 1)
			{
				--box->par->numChild;
				free(box->par->child);
				box->par->child = NULL;
			}
			else
			{
				for (u64 i = 0; i < box->par->numChild; ++i)
				{
					if (box->par->child[i] == box)
					{
						memmove(&box->par->child[i], &box->par->child[i + 1],
							(box->par->numChild - i - 1) * sizeof(void*));
						--box->par->numChild;
						break;
					}
				}
			}
		}

		// Inefficient child trying to delete itself if this.numChild isn't 0
		u64 numChild = box->numChild;
		box->numChild = 0;

		for (u64 i = 0; i < numChild; ++i)
			bx_deleteBox(box->child[i]);
		if (box->child)
			free(box->child);

		if (box->type == BX_TYPE_ROOT)
			memset(box, 0, sizeof(BX_Box));
		else
			free(box);
	}
}