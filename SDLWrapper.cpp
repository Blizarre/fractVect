#include "SDLWrapper.h"

#include <stdexcept>
#include <iostream>

/*
* Initialiser SDL : renvoie la surface principale
**/
SDLWrapper::SDLWrapper(size_t width, size_t height): m_width(width), m_height(height) {
	if (TTF_Init() == -1) {
		throw std::runtime_error(TTF_GetError());
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)< 0) {
		throw std::runtime_error(SDL_GetError());
	}
	else {
		std::clog << "Audio & Video modules initialized correctly" << std::endl;
	}

	m_window = SDL_CreateWindow("Vector Fractal generation", 100, 100, static_cast<int>(width), static_cast<int>(height), 0);
	SDL_SetWindowBordered(m_window, SDL_TRUE);
	m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE);
	Uint32 format = SDL_GetWindowPixelFormat(m_window);
	if (format != SDL_PIXELFORMAT_RGB888)
	{
		std::cerr << "Window pixel format unsupported, will be slower !" << std::endl;
	}

	m_screen = SDL_CreateTexture(m_renderer,
		SDL_PIXELFORMAT_RGB888,
		SDL_TEXTUREACCESS_STREAMING,
		static_cast<int>(width), static_cast<int>(height)
		);

	std::atexit(SDL_Quit);

	std::clog << "SDL initialization OK" << std::endl;
}


void SDLWrapper::processEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type) {

		case SDL_MOUSEMOTION:
			if (m_onMouseMove)
				m_onMouseMove(event.motion.x, event.motion.y);
			break;

		case SDL_MOUSEWHEEL:
			if(m_onMouseWheel)
				m_onMouseWheel(event.wheel.x, event.wheel.y);
			break;

		case SDL_KEYDOWN:
			std::clog << "sym " << (int)(event.key.keysym.sym) << " scancode " << event.key.keysym.scancode << std::endl;
			{
				auto itFind = m_keyboardEventBindings.find(event.key.keysym.sym);
				if (itFind != m_keyboardEventBindings.end())
					itFind->second();
			}
			break;

		case SDL_QUIT:
			if (m_quitEvent)
				m_quitEvent();
		}
	}
}
