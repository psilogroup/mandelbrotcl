#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
class TextRenderer
{
public:
	TTF_Font* font;
	SDL_Rect Message_rect;
	SDL_Surface* surface;
	SDL_Texture* Texture;
	SDL_Color Color;
    SDL_Renderer *rendererDest;

	void Draw(std::string text,int x,int y);

	TextRenderer (SDL_Renderer *renderer, std::string fontPath, int size, SDL_Color color );
	~TextRenderer();

};

