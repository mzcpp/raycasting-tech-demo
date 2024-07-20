#ifndef LEVEL_HPP
#define LEVEL_HPP

#include "Vect2d.hpp"

#include <SDL2/SDL.h>

#include <vector>

struct Tile
{
	SDL_Rect rect_;
	bool is_wall_;
	SDL_Color color_;
};

class Game;
class Screen;
  
class Level
{
private:
	Game* game_;
	Screen* screen_;
	SDL_Surface* surface_pixels_;
	Uint32* pixels_;

	std::vector<Tile> board_;

	int pixel_width_;
	int pixel_height_;
	int pixel_count_;
	int tile_size_;

public:
	Level(Game* game, Screen* screen);
	
	~Level();

	void HandleEvents();
	
	void Tick();
	
	bool Load(const char* path);

	bool Initialize(const char* path);

	void Free();

	int GetPixelWidth();
	
	int GetPixelHeight();
	
	int GetPixelCount();

	int GetTileSize();
	
	SDL_Surface* GetPixelSurface();

	Uint32* GetPixels32();

	Uint32 GetPitch32();

	Tile* GetTile(int x, int y);

	// std::vector<Tile*> GetNeighborTiles(int x, int y);

	Uint32 GetPixel(SDL_Surface *surface, int x, int y);
};

#endif
