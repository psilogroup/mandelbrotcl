#include "TextRenderer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
using namespace std;

void TextRenderer::Draw(std::string text,int x,int y)
{
	
	
    
    
    if (surface != nullptr)
    {
		SDL_FreeSurface(surface);
        surface = nullptr;
    }
    
    if (Texture != nullptr)
    {
     SDL_DestroyTexture(Texture)   ;
    }

	


	surface = TTF_RenderText_Blended(font, text.c_str(), Color);
	Texture = SDL_CreateTextureFromSurface(rendererDest, surface);

	if (Texture != nullptr)
	{
		
		Message_rect.x = x;
		Message_rect.y = y;
		Message_rect.w = surface->w;
		Message_rect.h = surface->h;
		SDL_RenderCopy(rendererDest, Texture, NULL, &Message_rect);
	}
}

TextRenderer::TextRenderer(SDL_Renderer *renderer, std::string fontPath, int size, SDL_Color color)
{
	font = TTF_OpenFont(fontPath.c_str(), size);
	Color.a = color.a;
	Color.r = color.r;
	Color.g = color.g;
	Color.b = color.b;
	surface = nullptr;
    Texture = nullptr;
    rendererDest = renderer;

	
}

TextRenderer::~TextRenderer()
{
	if (font != nullptr)
		TTF_CloseFont(font);

	if (surface != nullptr)
		SDL_FreeSurface(surface);

	if (Texture != nullptr)
		SDL_DestroyTexture(Texture);
}
