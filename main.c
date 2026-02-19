#include "boxUI/boxUI.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main()
{
	BX_Image image;
	image.size = bx_Vec2u(100, 100);
	image.pixels = malloc(image.size.x * image.size.y * 4);

	BX_Box root = bx_createRoot(bx_Rectf(0, 0, image.size.x, image.size.y));
	root.theme.bgColor = bx_rgba(0x00, 0x00, 0x00, 0xFF);

	BX_Theme theme;
	theme.fgColor =  bx_rgba(0xFF, 0xFF, 0xFF, 0xFF);
	theme.bgColor =  bx_rgba(0xFF, 0x00, 0x00, 0xFF);
	theme.outColor = bx_rgba(0xFF, 0xFF, 0xFF, 0x7F);
	theme.outThick = 2;
	theme.posMode = BX_PER_X | BX_PER_Y | BX_PER_W | BX_PER_H;

	BX_Box* first =  bx_createBox(&root, bx_Rectf(50.f, 50.f, 50.f, 50.f), theme);
	for (int i = 0; i < 4; i++)
	{
		theme.bgColor = bx_rgba(0x00, 0xFF, 0x00, i * 0x3F);
		bx_createBox(first, bx_Rectf(i * 20, 50.f, 20.f, 20.f), theme);
	}

	bx_drawBox(&root, image);

	stbi_write_bmp("out.bmp", image.size.x, image.size.y, 4, image.pixels);

	bx_deleteBox(&root);

	return 0;
}