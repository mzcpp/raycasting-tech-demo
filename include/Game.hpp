#ifndef GAME_HPP
#define GAME_HPP

#include "Level.hpp"
#include "Player.hpp"
#include "Screen.hpp"

#include <SDL2/SDL.h>

#include <memory>

class Game
{
private:
	bool initialized_;
	bool running_;

	std::unique_ptr<Level> level_;
	std::unique_ptr<Player> player_;
	std::unique_ptr<Screen> screen_;

public:
	bool map_toggled_;
	bool fisheye_effect_;

	SDL_Window* window_;
	SDL_Renderer* renderer_;
	
	Game();

	~Game();

	bool InitializeSDL();

	void Finalize();

	void Run();

	void HandleEvents();
	
	void Tick();
	
	void Render();

	std::uint32_t GetColor(const SDL_Color& color);
	
};

#endif