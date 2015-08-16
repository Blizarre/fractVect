#pragma once

#include <SDL.h>
#include <functional>
#include <map>
#include <iostream>
#include "memory"
#include "graphics.h"

#include <cstdio>
#include <iostream>

class SDLWrapper
{
public:

	SDLWrapper(size_t width, size_t height);

	unsigned int getTicks() { return SDL_GetTicks(); }

	pixel*  startWorkingOnTexture(int * pitch)
	{
		void* pixels;
		int res = SDL_LockTexture(m_screen, NULL, &pixels, pitch);
		if (res != 0)
		{
			std::cout << "something bad happenned" << std::endl;
		}

		return reinterpret_cast<pixel*>(pixels);
	}

	void renderTexture()
	{
		SDL_UnlockTexture(m_screen);
		SDL_RenderCopy(m_renderer, m_screen, NULL, NULL);
		SDL_RenderPresent(m_renderer);
	}

	void updateImage(pixel* data)
	{
		// Will do for now, TODO: use better function
		SDL_UpdateTexture(m_screen, nullptr, data, m_width * 4);
		SDL_RenderCopy(m_renderer, m_screen, NULL, NULL);
		SDL_RenderPresent(m_renderer);
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

	void onMouseMotion(std::function<void(size_t, size_t)> f)
	{
		m_onMouseMove = f;
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
	std::function<void(size_t, size_t)> m_onMouseMove;

	SDL_Texture* m_screen;
	SDL_Renderer* m_renderer;
	SDL_Window* m_window;

	int m_width, m_height;
};
