#include "boxUI/boxUI.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main()
{
	BX_Image image;
	image.size = bx_Vec2u(100, 100);
	image.pixels = malloc(image.size.x * image.size.y * 4);
	if (image.pixels)
	{
		memset(image.pixels, 0, image.size.x * image.size.y * 4);

		BX_Box root = bx_createRoot(bx_Rectf(0, 0, image.size.x, image.size.y));

		BX_Theme theme = { 0 };
		theme.fgColor = bx_rgba(0xFF, 0xFF, 0xFF, 0xFF);
		theme.bgColor = bx_rgba(0xFF, 0x00, 0x00, 0xFF);
		theme.outColor = bx_rgba(0xFF, 0xFF, 0xFF, 0xFF);
		theme.posMode = BX_RECT_PER_X | BX_RECT_PER_Y | BX_RECT_PER_W | BX_RECT_PER_H |
			BX_MARG_PER_X | BX_MARG_PER_Y | BX_MARG_PER_W | BX_MARG_PER_H;

		BX_Box* first = bx_createBox(&root, bx_Rectf(50.f, 50.f, 50.f, 50.f), theme);
		theme.outThick = 1;
		for (int i = 0; i < 4; i++)
		{
			theme.bgColor = bx_rgba(0x00, 0xFF, 0x00, 255.f / 3.f * i);
			bx_createBox(first, bx_Rectf(i * 20 + 20.f, 33.f, 20.f, 20.f), theme);
		}
		theme.margin = bx_Rectf(10.f, 10.f, 10.f, 10.f);
		for (int i = 0; i < 4; i++)
		{
			theme.bgColor = bx_rgba(0x00, 0xFF, 0x00, 255.f / 3.f * i);
			bx_createBox(first, bx_Rectf(i * 20 + 20.f, 66.f, 20.f, 20.f), theme);
		}

		bx_drawBox(&root, image);

		stbi_write_bmp("out.bmp", image.size.x, image.size.y, 4, image.pixels);

		bx_deleteBox(&root);

		free(image.pixels);
	}

	return 0;
}