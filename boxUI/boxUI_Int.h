#pragma once
#include "boxUI.h"

BX_Rectf bx_alignBox(BX_Rectf box, u16 posMode, BX_Rectf parent);

void bx_initBox(BX_Box* box, BX_Box* parent, BX_Rectf rect, BX_Theme theme);

void drawRect(BX_Image image, BX_Rectu rect, BX_Theme theme);

void bx_drawBoxRec(BX_Box* box, BX_Image image, BX_Rectf parent);

void bx_drawBoxOutline(BX_Box* box, BX_Image image, BX_Rectf parent, BX_Rectf bounds);