#ifndef GAME_HPP
#define GAME_HPP

#include "Level.hpp"
#include "Player.hpp"

#include <SDL2/SDL.h>

#include <memory>

class Game
{
private:
	bool initialized_;
	bool running_;

	std::unique_ptr<Level> level_;
	std::unique_ptr<Player> player_;

public:
	SDL_Window* window_;
	SDL_Renderer* renderer_;
	
	Game();

	~Game();

	bool Initialize();

	void Finalize();

	void Run();

	void HandleEvents();
	
	void Tick();
	
	void Render();
};

#endif