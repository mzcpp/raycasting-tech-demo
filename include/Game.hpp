#ifndef GAME_HPP
#define GAME_HPP

#include "Level.hpp"
#include "Player.hpp"
#include "Screen.hpp"
#include "Texture.hpp"

#include <SDL2/SDL.h>

#include <memory>
#include <vector>

class Game
{
	friend class Level;
	
private:
	bool initialized_;
	bool running_;

	std::unique_ptr<Level> level_;
	std::unique_ptr<Player> player_;
	std::unique_ptr<Screen> screen_;

public:
	std::vector<std::unique_ptr<Texture>> textures_;
	bool map_toggled_;
	bool fisheye_effect_toggled_;
	bool textures_toggled_;

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

	bool ColorsEqual(const SDL_Color& color1, const SDL_Color& color2);
};

#endif