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
		f32 aspect = (f32)box.w / (f32)box.h;
		if (aspect < theme.aspect)
		{
			f32 newH = box.h * (aspect / theme.aspect);
			if (theme.posMode & BX_RECT_ALIGN_CY)
				box.y += (box.h - newH) * 0.5f;
			else if (theme.posMode & BX_RECT_ALIGN_B)
				box.y += (box.h - newH);
			box.h = newH;
		}
		else if (aspect > theme.aspect)
		{
			f32 newW = box.w * (theme.aspect / aspect);
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
	box->childCap = 0;
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

	case BX_TYPE_TEXT:
		bx_drawTextRec(box, image);
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

		if (!bx_addTo(parent, box))
		{
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

		if (!bx_addTo(parent, list))
		{
			free(list);
			list = NULL;
		}
	}
	else
		printf("malloc failed %d\n", __LINE__);
	return list;
}

BX_Text* bx_createText(BX_Box* parent, BX_Vec2f pos, const char* string, bool copy, BX_Theme theme, u16 fontSize, u8 order)
{
	BX_Text* text = malloc(sizeof(BX_Text));
	if (text)
	{
		bx_initBox(&text->box, parent, bx_Rectf(pos.x, pos.y, 0.f, 0.f), theme);
		text->box.type = BX_TYPE_TEXT;
		text->order = order;
		text->scrollX = 0.f;
		text->scrollY = 0.f;
		text->fontSize = fontSize;
		text->copy = copy;
		text->text = NULL;
		text->length = 0;

		bx_setText(text, string, copy);

		if (!bx_addTo(parent, text))
		{
			if (text->text && text->copy)
				free(text->text);
			free(text);
			text = NULL;
		}
	}
	else
		printf("malloc failed %d\n", __LINE__);
	return text;
}

bool bx_addTo(BX_Box* parent, BX_Box* box)
{
	if (parent->numChild + 1 > parent->childCap)
	{
		u64 newSize = parent->numChild * 2;
		if (newSize == 0)
			newSize = 1;
		BX_Box** temp = realloc(parent->child, newSize * sizeof(void*));
		if (temp)
		{
			parent->child = temp;
			parent->childCap = newSize;
		}
		else
		{
			printf("realloc failed %d\n", __LINE__);
			return false;
		}
	}
	parent->child[parent->numChild++] = box;
	box->depth = parent->depth + 1;

	bx_recalcBox(box);

	return true;
}

void bx_resizeRoot(BX_Box* root, BX_Rectf imageRect)
{
	root->rect = imageRect;
	root->calc = imageRect;
	root->crop = imageRect;

	bx_recalcBox(root);
}

void bx_recalcBox(BX_Box* box)
{
	if (box->type == BX_TYPE_ROOT)
	{
		for (u64 i = 0; i < box->numChild; ++i)
			bx_resizeRec(box->child[i]);
	}
	else
	{
		// Lists need to recalc all children after the one that changes
		// Wouldn't know how big the biggest child of the row/col is
		// Easier to do all
		// Could be bad for example:
		// A file viewer showing system32, but something is added/deleted
		if (box->par && box->par->type == BX_TYPE_LIST)
			bx_resizeRec(box->par);
		else
			bx_resizeRec(box);
	}
}

bool bx_setText(BX_Text* text, const char* string, bool copy)
{
	if (text->text && text->copy)
		free(text->text);
	text->copy = copy;

	if (!string)
	{
		text->length = 0;
		text->text = NULL;
		text->box.rect.w = 0;
		text->box.rect.h = 0;
		return true;
	}
	else
	{
		text->length = strlen(string);
		if (copy)
		{
			text->text = malloc(text->length + 1);
			if (text->text)
			{
				memcpy(text->text, string, text->length + 1);
				BX_Vec2f size = bx_measureText(string, text->box.par->calc, text->fontSize, text->order);
				text->box.rect.w = size.x;
				text->box.rect.h = size.y;
				return true;
			}
			else
			{
				text->length = 0;
				return false;
			}
		}
		else
		{
			text->text = string;
			BX_Vec2f size = bx_measureText(string, text->box.par->calc, text->fontSize, text->order);
			text->box.rect.w = size.x;
			text->box.rect.h = size.y;
			return true;
		}
	}
}

BX_Vec2f bx_measureText(const char* string, BX_Rectf parent, u16 fontSize, u8 order)
{
	f32 fontW = fontSize * 8.f;
	f32 fontH = fontSize * 10.f;
	f32 tx = 0.f, ty = 0.f;

	if (!string)
	{
		return bx_Vec2f(0, 0);
	}

	u64 length = strlen(string);
	for (u64 i = 0; i < length; ++i)
	{
		if (string[i] == '\n')
		{
			ty += fontH;
			tx = 0;
		}
		else
		{
			if (tx + fontW >= parent.w)
			{
				ty += fontH;
				tx = 0;
			}
			tx += fontW;
		}
	}

	return bx_Vec2f(tx, ty + fontH);
}

void bx_resizeRec(BX_Box* box)
{
	if (box->type == BX_TYPE_TEXT)
	{
		BX_Text* text = box;
		BX_Vec2f size = bx_measureText(text->text, box->par->calc, text->fontSize, text->order);
		box->rect.w = size.x;
		box->rect.h = size.y;
	}

	box->calc = bx_alignBoxMargin(box->rect, box->theme, box->par->calc);
	
	box->crop = bx_cropRect(box->calc, box->par->crop);

	if (box->type == BX_TYPE_LIST)
		bx_resizeListRec(box);
	else
		for (u64 i = 0; i < box->numChild; ++i)
			bx_resizeRec(box->child[i]);
}

void bx_resizeListRec(BX_List* list)
{
	f32 childX = 0.f;
	if (list->order & BX_LIST_RIGHT)
		childX = list->box.calc.w;
	f32 childY = 0.f;
	if (list->order & BX_LIST_BOTTOM)
		childY = list->box.calc.h;
	f32 maxChildW = 0.f;
	f32 maxChildH = 0.f;
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

		bx_resizeRec(list->box.child[i]);
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
	if (box->crop.w)
	{
		bx_drawRect(image, bx_Rectu(box->crop.x, box->crop.y,
			box->crop.w, box->crop.h), box->theme.bgColor);

		for (u64 i = 0; i < box->numChild; ++i)
			bx_callDrawType(box->child[i], image);
	}
	// Maybe the box is over the edge, but has a huge outline
	// Still have a chance to draw it
	if (box->theme.outThick)
		bx_drawBoxOutline(box, image);
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

void bx_drawTextRec(BX_Text* text, BX_Image image)
{
	if (text->box.crop.w)
	{
		bx_drawRect(image, bx_Rectu(text->box.crop.x, text->box.crop.y,
			text->box.crop.w, text->box.crop.h), text->box.theme.bgColor);

		f32 fontW = text->fontSize * 8.f;
		f32 fontH = text->fontSize * 10.f;
		f32 tx = text->box.calc.x, ty = text->box.calc.y;
		for (u64 i = 0; i < text->length; ++i)
		{
			if (text->text[i] == '\n')
			{
				ty += fontH;
				tx = text->box.calc.x;
			}
			else
			{
				if (tx + fontW >= text->box.par->calc.w)
				{
					ty += fontH;
					tx = text->box.calc.x;
				}

				BX_Rectf rect = bx_cropRect(bx_cropRect(bx_Rectf(tx, ty, fontW, fontH),
					text->box.calc), bx_Rectf(0, 0, image.size.x, image.size.y));
				bx_drawRect(image, bx_Rectu(rect.x, rect.y, rect.w, rect.h), 
					(i & 1 ? text->box.theme.fgColor : text->box.theme.bgColor));

				tx += fontW;
			}
		}
	}

	for (u64 i = 0; i < text->box.numChild; ++i)
		bx_callDrawType(text->box.child[i], image);

	// Maybe the box is over the edge, but has a huge outline
	// Still have a chance to draw it
	if (text->box.theme.outThick)
		bx_drawBoxOutline(&text->box, image);
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
			if (box->par->numChild == 0);
			else if (box->par->numChild == 1)
			{
				--box->par->numChild;
				free(box->par->child);
				box->par->childCap = 0;
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
				if (box->par->type == BX_TYPE_LIST)
					bx_resizeListRec(box->par);
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
		{
			if (box->type == BX_TYPE_TEXT)
			{
				BX_Text* text = box;
				if (text->copy)
					if (text->text)
						free(text->text);
			}

			free(box);
		}
	}
}