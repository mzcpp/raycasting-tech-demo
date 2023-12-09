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

struct VerticalLine
{
	VerticalLine() = default;

	VerticalLine(int x, int y1, int y2, SDL_Color color) : x_(x), y1_(y1), y2_(y2), color_(color)
	{
	}

	int x_;
	int y1_;
	int y2_;
	SDL_Color color_;
};

class Game;

class Level
{
private:
	Game* game_;
	SDL_Surface* surface_pixels_;
	Uint32* pixels_;

	std::vector<Tile> board_;
	std::vector<VerticalLine> vertical_lines_;

	int pixel_width_;
	int pixel_height_;
	int pixel_count_;
	int tile_size_;

public:
	Level(Game* game);
	
	~Level();

	void HandleEvents();
	
	void Tick();
	
	void Render();

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

	void ClearVerticalLines();

	void AddVerticalLine(int x, int y1, int y2, SDL_Color color);

};

#endif
