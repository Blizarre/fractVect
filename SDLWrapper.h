#pragma once

#include <SDL.h>
#include <functional>
#include <map>
#include <iostream>
#include "memory"
#include "graphics.h"

#include <cstdio>
#include <iostream>
#include <SDL_ttf.h>

class SDLWrapper
{
public:

	SDLWrapper(size_t width, size_t height);

	unsigned int getTicks() { return SDL_GetTicks(); }

	pixel*  startWorkingOnTexture(int * pitch)
	{
		void* pixels;
		int res = SDL_LockTexture(m_screen, nullptr, &pixels, pitch);
		if (res != 0)
		{
			throw std::runtime_error("something bad happened");
		}

		return reinterpret_cast<pixel*>(pixels);
	}

	void renderTexture()
	{
		SDL_UnlockTexture(m_screen);
		SDL_RenderCopy(m_renderer, m_screen, nullptr, nullptr);

		//SDL_RenderCopy(m_renderer, m_overlay, nullptr, nullptr);
		SDL_DestroyTexture(m_overlay);
		m_overlay = nullptr;
		SDL_RenderPresent(m_renderer);
	}

	void writeText(char* text)
	{
		SDL_Surface* surf = TTF_RenderText_Shaded(m_font, text, m_white, m_black);
		m_overlay = SDL_CreateTextureFromSurface(m_renderer, surf);
		SDL_FreeSurface(surf);
	}

	/***
	* bind a key number to a std::function. When processEvent will be called,
	* this function will be called at every keypress of keyNum. Warning, only
	* one function can be bound to a keyNum. previous bindings for this keynum
	* will be discarded.
	**/
	void onKeyPress(unsigned char keyNum, std::function<void()> f)
	{
		m_keyboardEventBindings[keyNum] = f;
	}

	void onMouseMotion(std::function<void(int, int)> f)
	{
		m_onMouseMove = f;
	}

	void onMouseWheel(std::function<void(int, int)> f)
	{
		m_onMouseWheel = f;
	}

	void onQuitEvent(std::function<void()> f) { m_quitEvent = f; }

	void processEvents();

	~SDLWrapper()
	{
		SDL_DestroyTexture(m_screen);
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
	}

protected:
	std::map<Uint32, std::function<void(void)>> m_keyboardEventBindings;
	std::function<void(void)> m_quitEvent;
	std::function<void(int, int)> m_onMouseMove;
	std::function<void(int, int)> m_onMouseWheel;
	

	SDL_Texture* m_screen;
	SDL_Renderer* m_renderer;
	SDL_Window* m_window;
	SDL_Texture* m_overlay = nullptr;
	TTF_Font* m_font;
	SDL_Color m_black, m_white;
	size_t m_width, m_height;
};
