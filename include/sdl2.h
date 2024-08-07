#pragma once

#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

struct Noncopyable
{
	public:
		Noncopyable(){}
	private:
		Noncopyable(Noncopyable const &);
		Noncopyable & operator = (Noncopyable const &);

};


struct Window : Noncopyable
{
	int const width;
	int const height;
	
	Window(int width, int height)
	:
		width(width), height(height)
	{}

	~Window()
	{
		SDL_DestroyWindow(&window);
	}

	SDL_Window& _init_window()
	{
		SDL_Window* ptr = SDL_CreateWindow( "SDL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN );
		if (ptr == NULL) {
			std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
			SDL_Quit();
		}
		return *ptr;
	}
	SDL_Window& window = _init_window();
	//SDL_Surface& screen = *SDL_GetWindowSurface( &window );
};

struct Image : Noncopyable
{
	SDL_Surface& _init_surface(std::string path)
	{
		SDL_Surface* ptr = IMG_Load(path.c_str());
		if (ptr == NULL)
			std::cerr << "Unable to load image! SDL Error: \n" << SDL_GetError() << "\n";
	       	return	*ptr;
	}

	SDL_Surface& surface;

	Image(std::string path)
	:
		surface(_init_surface(path))
	{}

	~Image()
	{
		SDL_FreeSurface(&surface);
	}

	
	void blit(SDL_Surface& other_surface, int const x, int const y)
	{
		SDL_Rect rect;
		rect.x = x;
		rect.y = y;
		SDL_BlitSurface(&surface, NULL, &other_surface, &rect );
	}
};

struct Renderer : Noncopyable
{
	SDL_Renderer& _init_renderer(Window &window)
	{
		SDL_Renderer* ptr = SDL_CreateRenderer(&window.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (ptr == NULL)
                	std::cerr << "Renderer could not be created! SDL Error: \n" << SDL_GetError();
	       	return	*ptr;
	}

	SDL_Renderer& renderer;

	Renderer(Window& window)
	:
		renderer(_init_renderer(window))
	{}

	~Renderer()
	{
		SDL_DestroyRenderer(&renderer);
	}
};

struct Texture : Noncopyable
{
	SDL_Texture& _init_texture(Renderer &renderer, std::string &path)
	{
		SDL_Texture* ptr = IMG_LoadTexture(&renderer.renderer, path.c_str());
		if (ptr == NULL)
            		std::cerr << "Unable to create texture! SDL Error: \n" << path.c_str() << SDL_GetError();
		return *ptr;
	}

	SDL_Texture& texture;

	Texture(Renderer &renderer, std::string path)
	:
		texture(_init_texture(renderer, path))
	{}

	~Texture()
	{
		SDL_DestroyTexture(&texture);
	}
	
	void render(Renderer &renderer, int const x, int const y)
	{
		int w, h;
		SDL_QueryTexture(&texture, NULL, NULL, &w, &h);
		SDL_Rect rect { x, y, w, h };
		SDL_RenderCopy(&renderer.renderer, &texture, NULL, &rect);
	}

	void render(Renderer &renderer, int const x, int const y, int const w, int const h)
	{
		SDL_Rect rect { x, y, w, h };
		SDL_RenderCopy(&renderer.renderer, &texture, NULL, &rect);
	}
};

struct EmptyTexture : Noncopyable
{
	SDL_Texture& _init_texture(Renderer &renderer, int const w, int const h)
	{
		SDL_Texture* ptr = SDL_CreateTexture(&renderer.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
		if (ptr == NULL)
            		std::cerr << "Unable to create texture! SDL Error: \n" << SDL_GetError();
		return *ptr;
	}

	SDL_Texture& texture;

	void render(Renderer &renderer, int const x, int const y)
	{
		int w, h;
		SDL_QueryTexture(&texture, NULL, NULL, &w, &h);
		SDL_Rect rect { x, y, w, h };
		SDL_RenderCopy(&renderer.renderer, &texture, NULL, &rect);
	}

	void render(Renderer &renderer, int const x, int const y, int const w, int const h)
	{
		SDL_Rect rect { x, y, w, h };
		SDL_RenderCopy(&renderer.renderer, &texture, NULL, &rect);
	}

	EmptyTexture(Renderer &renderer, int const w, int const h)
	:
		texture(_init_texture(renderer, w, h))
	{}

	~EmptyTexture()
	{
		SDL_DestroyTexture(&texture);
	}
};
struct Font : Noncopyable
{
	TTF_Font& _init_font(std::string font, int const size)
	{
	        char const *pfont = font.c_str();
	        TTF_Font* ptr = TTF_OpenFont(pfont, size);
	        if (ptr == NULL)
	                std::cerr << "Unable to create Font! SDL Error: \n" << SDL_GetError();
	        return *ptr;
	}
	
	TTF_Font& font;
	
	Font(std::string font, int const size)
	:
	        font(_init_font(font, size))
	{}
	
	void render(Renderer &renderer, std::string text, int x, int y, uint8_t r, uint8_t g, uint8_t b)
	{
	        char const *ptext = text.c_str();
		SDL_Surface* surf_message = TTF_RenderText_Blended(&font, ptext, SDL_Color { r, g, b, 255 });
		SDL_Texture* message = SDL_CreateTextureFromSurface(&renderer.renderer, surf_message);
		SDL_Rect rect;
		rect.x = x;
		rect.y = y;
		TTF_SizeText(&font, ptext, &rect.w, &rect.h);

		SDL_RenderCopy(&renderer.renderer, message, NULL, &rect);
		SDL_FreeSurface(surf_message);
		SDL_DestroyTexture(message);
	}

	void render(Renderer &renderer, std::string text, int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b)
	{
	        char const *ptext = text.c_str();
		SDL_Surface* surf_message = TTF_RenderText_Blended(&font, ptext, SDL_Color { r, g, b, 255 });
		SDL_Texture* message = SDL_CreateTextureFromSurface(&renderer.renderer, surf_message);
		SDL_Rect rect { x, y, w, h };

		SDL_RenderCopy(&renderer.renderer, message, NULL, &rect);
		SDL_FreeSurface(surf_message);
		SDL_DestroyTexture(message);
	}
};
	
