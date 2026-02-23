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
		BX_MARG_PER_L | BX_MARG_PER_T | BX_MARG_PER_R | BX_MARG_PER_B;

	theme.aspect = 1.f;
	BX_Box* first = bx_createList(&root, bx_Rectf(50.f, 50.f, 75.f, 75.f), theme, 
		BX_LIST_WRAP | BX_LIST_ROW | BX_LIST_LEFT | BX_LIST_TOP);

	theme.aspect = 0.f;
	theme.outThick = 1;
	theme.margin = bx_Rectf(5.f, 5.f, 5.f, 5.f);
	for (int i = 0; i < 10; i++)
	{
		theme.bgColor = bx_colorLerp(bx_rgba(0x00, 0xFF, 0x00, 0xFF), bx_rgba(0xFF, 0x00, 0x00, 0xFF), i / 11.f);
		bx_createBox(first, bx_Rectf(0.f, 0.f, 20.f + i * 2.f, 20.f + i * 2.f), theme);
	}

	bx_recalcBox(&root, bx_Rectf(0.f, 0.f, surface->w, surface->h));

	BX_Vec2f mouse = { 0 };

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
			}
		}

		SDL_GetMouseState(&mouse.x, &mouse.y);
		BX_Box* hovered = bx_updateBox(&root, mouse);

		memset(image.pixels, 0, surface->w * surface->h * 4);
		bx_drawBox(&root, image);

		if (hovered)
			bx_drawRect(image, bx_Rectu(hovered->crop.x, hovered->crop.y, 
				hovered->crop.w, hovered->crop.h), bx_rgba(0xFF, 0xFF, 0xFF, 0x3F));

		SDL_UpdateWindowSurface(window);

		//SDL_Delay(10);
	}

	SDL_DestroyWindow(window);

	bx_deleteBox(&root);

	return 0;
}