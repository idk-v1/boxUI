#pragma once
#include "boxUI.h"

BX_Rectf bx_alignBox(BX_Rectf box, u16 posMode, BX_Rectf parent);

BX_Rectf bx_alignBoxMargin(BX_Rectf box, BX_Theme theme, BX_Rectf parent);

BX_Rectf bx_applyAspectRatio(BX_Rectf box, BX_Theme theme);

void bx_initBox(BX_Box* box, BX_Box* parent, BX_Rectf rect, BX_Theme theme);

void bx_callDrawType(BX_Box* box, BX_Image image, BX_Rectf parent);

void drawRect(BX_Image image, BX_Rectu rect, BX_Theme theme);

BX_Rectf bx_cropRect(BX_Rectf rect, BX_Rectf parent);

void bx_drawBoxRec(BX_Box* box, BX_Image image, BX_Rectf parent);

void bx_drawListRec(BX_List* box, BX_Image image, BX_Rectf parent);

void bx_drawBoxOutline(BX_Box* box, BX_Image image, BX_Rectf parent, BX_Rectf bounds);

bool bx_updateBoxRec(BX_Box* box, BX_Rectf parent, BX_Vec2f mouse, bool hasChance);

bool bx_rectContains(BX_Rectf rect, BX_Vec2f point);