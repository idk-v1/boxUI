#include "boxUI/boxUI.h"

#include <SDL3/SDL.h>

int main()
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow("BoxUI", 800, 600, SDL_WINDOW_RESIZABLE);
	SDL_Surface* surface = SDL_GetWindowSurface(window);

	BX_Image image;
	image.size.x = surface->w;
	image.size.y = surface->h;
	image.pixels = surface->pixels;

	BX_Box root = bx_createRoot(bx_Rectf(0, 0, surface->w, surface->h));

	BX_Theme theme = { 0 };
	theme.fgColor = bx_rgba(0xFF, 0xFF, 0xFF, 0xFF);
	theme.bgColor = bx_rgba(0xFF, 0x00, 0x00, 0xFF);
	theme.outColor = bx_rgba(0xFF, 0xFF, 0xFF, 0xFF);
	theme.posMode = BX_RECT_ALIGN_CX | BX_RECT_ALIGN_CY |
		BX_RECT_PER_X | BX_RECT_PER_Y | BX_RECT_PER_W | BX_RECT_PER_H |
		BX_MARG_PIX_L | BX_MARG_PIX_T | BX_MARG_PIX_R | BX_MARG_PIX_B;

	//theme.aspect = 1.f;
	BX_Box* list = bx_createList(&root, bx_Rectf(50.f, 50.f, 75.f, 75.f), theme, 
		BX_LIST_WRAP | BX_LIST_ROW | BX_LIST_LEFT | BX_LIST_TOP);

	theme.aspect = 1.5f;
	theme.outThick = 1;
	theme.margin = bx_Rectf(2.f, 2.f, 2.f, 2.f);
	for (int i = 0; i < 5; i++)
	{
		theme.bgColor = bx_colorLerp(bx_rgba(0x00, 0xFF, 0x00, 0xFF), bx_rgba(0xFF, 0x00, 0x00, 0xFF), i / 11.f);
		bx_createBox(list, bx_Rectf(0.f, 0.f, 20.f + i * 2.f, 20.f + i * 2.f), theme);
	}

	bx_recalcBox(&root, bx_Rectf(0.f, 0.f, surface->w, surface->h));

	BX_Vec2f mouse = { 0 };
	BX_Box* hovered = NULL;

	bool running = true;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				running = false;
				break;

			case SDL_EVENT_WINDOW_RESIZED:
				surface = SDL_GetWindowSurface(window);
				image.size.x = surface->w;
				image.size.y = surface->h;
				image.pixels = surface->pixels;
				bx_resizeRoot(&root, bx_Rectf(0.f, 0.f, surface->w, surface->h));
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				switch (event.button.button)
				{
				case SDL_BUTTON_LEFT:
					if (hovered && hovered != list && hovered != &root)
					{
						bx_deleteBox(hovered);
						bx_recalcBox(list, bx_Rectf(0.f, 0.f, surface->w, surface->h));
					}
					break;

				case SDL_BUTTON_RIGHT:
					theme.aspect = 0.f;
					theme.bgColor.a = 255;
					theme.bgColor.r = rand() % 128 + 128;
					theme.bgColor.g = rand() % 128 + 128;
					theme.bgColor.b = rand() % 128 + 128;
					bx_createBox(list, 
						bx_Rectf(0.f, 0.f, rand() % 5 * 5.f + 20.f, rand() % 5 * 5.f + 20.f), 
						theme);
					bx_recalcBox(list, bx_Rectf(0.f, 0.f, surface->w, surface->h));
					break;
				}
			}
		}

		SDL_GetMouseState(&mouse.x, &mouse.y);
		hovered = bx_updateBox(&root, mouse);

		memset(image.pixels, 0, surface->w * surface->h * 4);
		bx_drawBox(&root, image);

		if (hovered)
			bx_drawRect(image, bx_Rectu(hovered->crop.x, hovered->crop.y, 
				hovered->crop.w, hovered->crop.h), bx_rgba(0xFF, 0xFF, 0xFF, 0x3F));

		SDL_UpdateWindowSurface(window);

		SDL_Delay(10);
	}

	SDL_DestroyWindow(window);

	bx_deleteBox(&root);

	return 0;
}