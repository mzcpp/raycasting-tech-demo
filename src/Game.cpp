#include "Game.hpp"
#include "Constants.hpp"
#include "Player.hpp"
#include "Level.hpp"
#include "Texture.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cstdint>
#include <iostream>
#include <memory>

Game::Game() : 
	initialized_(false), 
	running_(false), 
	map_toggled_(true), 
	fisheye_effect_toggled_(false), 
	textures_toggled_(false)
{
	initialized_ = InitializeSDL();

	screen_ = std::make_unique<Screen>(this);
	level_ = std::make_unique<Level>(this, screen_.get());
	level_->Initialize("res/gfx/level.png");
	player_ = std::make_unique<Player>(this, screen_.get(), level_.get());

	for (std::size_t i = 0; i < 4; ++i)
	{
		textures_.emplace_back(std::make_unique<Texture>(this));
	}

	textures_[0]->LoadPixelsFromFile("res/gfx/red_tex.png");
	textures_[1]->LoadPixelsFromFile("res/gfx/green_tex.png");
	textures_[2]->LoadPixelsFromFile("res/gfx/blue_tex.png");
	textures_[3]->LoadPixelsFromFile("res/gfx/yellow_tex.png");
}

Game::~Game()
{
	Finalize();
}

bool Game::InitializeSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not be initialized! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"))
	{
		printf("%s\n", "Warning: Texture filtering is not enabled!");
	}

	window_ = SDL_CreateWindow(constants::game_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, constants::screen_width, constants::screen_height, SDL_WINDOW_SHOWN);

	if (window_ == nullptr)
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);

	if (renderer_ == nullptr)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	
	//SDL_RenderSetLogicalSize(renderer_, 48, 48);

	constexpr int img_flags = IMG_INIT_PNG;

	if (!(IMG_Init(img_flags) & img_flags))
	{
		printf("SDL_image could not be initialized! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	return true;
}

void Game::Finalize()
{
	SDL_DestroyWindow(window_);
	window_ = nullptr;
	
	SDL_DestroyRenderer(renderer_);
	renderer_ = nullptr;

	SDL_Quit();
	IMG_Quit();
}

void Game::Run()
{
	if (!initialized_)
	{
		return;
	}

	running_ = true;

	constexpr double ms = 1.0 / 60.0;
	std::uint64_t last_time = SDL_GetPerformanceCounter();
	long double delta = 0.0;

	double timer = SDL_GetTicks();

	int frames = 0;
	int ticks = 0;

	while (running_)
	{
		const std::uint64_t now = SDL_GetPerformanceCounter();
		const long double elapsed = static_cast<long double>(now - last_time) / static_cast<long double>(SDL_GetPerformanceFrequency());

		last_time = now;
		delta += elapsed;

		HandleEvents();

		while (delta >= ms)
		{
			Tick();
			delta -= ms;
			++ticks;
		}

		//printf("%Lf\n", delta / ms);
		Render();
		++frames;

		if (SDL_GetTicks() - timer > 1000.0)
		{
			timer += 1000.0;
			//printf("Frames: %d, Ticks: %d\n", frames, ticks);
			frames = 0;
			ticks = 0;
		}
	}
}

void Game::HandleEvents()
{
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
		{
			running_ = false;
			return;
		}
		if (e.type == SDL_KEYDOWN)
		{
			if (e.key.keysym.sym == SDLK_m)
			{
				map_toggled_ = !map_toggled_;
			}
			else if (e.key.keysym.sym == SDLK_f)
			{
				fisheye_effect_toggled_ = !fisheye_effect_toggled_;
			}
			else if (e.key.keysym.sym == SDLK_t)
			{
				textures_toggled_ = !textures_toggled_;
			}
		}

		player_->HandleEvent(&e);
	}
}

void Game::Tick()
{
	screen_->bitmap_->Clear();
	player_->Tick();
}

void Game::Render()
{
	SDL_RenderSetViewport(renderer_, NULL);
	SDL_SetRenderDrawColor(renderer_, 0xff, 0xff, 0xff, 0xff);
	SDL_RenderClear(renderer_);

	screen_->Render();

	SDL_RenderPresent(renderer_);
}

std::uint32_t Game::GetColor(const SDL_Color& color)
{
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	std::uint32_t uint32_color = (color.b << 24) + (color.g << 16) + (color.r << 8) + 255;
	#else
	std::uint32_t uint32_color = (255 << 24) + (color.r << 16) + (color.g << 8) + color.b;
	#endif

	return uint32_color;
}

bool Game::ColorsEqual(const SDL_Color& color1, const SDL_Color& color2)
{
	return color1.r == color2.r && color1.g == color2.g && color1.b == color2.b && color1.a == color2.a;
}

