#include "Game.hpp"
#include "Level.hpp"
#include "Constants.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>

Level::Level(Game* game) :
	game_(game), 
	surface_pixels_(nullptr), 
	pixels_(nullptr), 
	pixel_width_(0), 
	pixel_height_(0), 
	pixel_count_(0), 
	tile_size_(1)
{
}
	
Level::~Level()
{
	Free();
}

void Level::HandleEvents()
{

}

void Level::Tick()
{

}

void Level::Render()
{
	std::for_each(vertical_lines_.begin(), vertical_lines_.end(), [this](const VerticalLine& v_line)
	{
		SDL_SetRenderDrawColor(game_->renderer_, v_line.color_.r, v_line.color_.g, v_line.color_.b, v_line.color_.a);
		SDL_RenderDrawLine(game_->renderer_, v_line.x_, v_line.y1_, v_line.x_, v_line.y2_);
	});

	std::for_each(board_.begin(), board_.end(), [this](Tile tile)
	{
		int scale_factor = 16;
		tile.rect_.x *= scale_factor;
		tile.rect_.y *= scale_factor;
		tile.rect_.w *= scale_factor;
		tile.rect_.h *= scale_factor;
		SDL_SetRenderDrawColor(game_->renderer_, tile.color_.r, tile.color_.g, tile.color_.b, tile.color_.a);
		SDL_RenderFillRect(game_->renderer_, &tile.rect_);
	});
}

bool Level::Load(const char* path)
{
	Free();

	surface_pixels_ = IMG_Load(path);

	if (surface_pixels_ == nullptr)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
		return false;
	}

	surface_pixels_ = SDL_ConvertSurfaceFormat(surface_pixels_, SDL_GetWindowPixelFormat(game_->window_), 0);

	pixel_width_ = surface_pixels_->w;
	pixel_height_ = surface_pixels_->h;
	pixel_count_ = pixel_width_ * pixel_height_;

	return true;
}

bool Level::Initialize(const char* path)
{
	if (!Load(path))
	{
		return false;
	}

	board_.resize(GetPixelCount());
	vertical_lines_.reserve(constants::screen_width);

	const Uint32 empty = SDL_MapRGBA(surface_pixels_->format, 0x00, 0x00, 0x00, 0xff);
	const Uint32 green_wall = SDL_MapRGBA(surface_pixels_->format, 0x00, 0xff, 0x00, 0xff);
	const Uint32 red_wall = SDL_MapRGBA(surface_pixels_->format, 0xff, 0x00, 0x00, 0xff);
	const Uint32 blue_wall = SDL_MapRGBA(surface_pixels_->format, 0x00, 0x00, 0xff, 0xff);
	const Uint32 yellow_wall = SDL_MapRGBA(surface_pixels_->format, 0xff, 0xff, 0x00, 0xff);

	const Uint32* pixels = GetPixels32();

	int tile_x = 0;
	int tile_y = 0;

	for (int i = 0; i < GetPixelCount(); ++i)
	{
		board_[i].is_wall_ = true;

		if (pixels[i] == empty)
		{
			board_[i].color_ = { 0x00, 0x00, 0x00, 0xff };
			board_[i].is_wall_ = false;
			//std::cout << ".";
		}
		else if (pixels[i] == green_wall)
		{
			board_[i].color_ = { 0x00, 0xff, 0x00, 0xff };
			//std::cout << "G";
		}
		else if (pixels[i] == red_wall)
		{
			board_[i].color_ = { 0xff, 0x00, 0x00, 0xff };
			//std::cout << "R";
		}
		else if (pixels[i] == blue_wall)
		{
			board_[i].color_ = { 0x00, 0x00, 0xff, 0xff };
			//std::cout << "B";
		}
		else if (pixels[i] == yellow_wall)
		{
			board_[i].color_ = { 0xff, 0xff, 0x00, 0xff };
			//std::cout << "Y";
		}

		board_[i].rect_.x = tile_x;
		board_[i].rect_.y = tile_y;
		board_[i].rect_.w = tile_size_;
		board_[i].rect_.h = board_[i].rect_.w;

		tile_x += tile_size_;

		if ((i + 1) % GetPixelWidth() == 0)
		{
			tile_x = 0;
			tile_y += tile_size_;

			//std::cout << std::endl;
		}
	}

	return true;
}

void Level::Free()
{
	if (surface_pixels_ != nullptr)
	{
		SDL_FreeSurface(surface_pixels_);
		surface_pixels_ = nullptr;
	}
}

int Level::GetPixelWidth()
{
	return pixel_width_;
}
	
int Level::GetPixelHeight()
{
	return pixel_height_;
}

int Level::GetPixelCount()
{
	return pixel_count_;
}

int Level::GetTileSize()
{
	return tile_size_;
}

SDL_Surface* Level::GetPixelSurface()
{
	return surface_pixels_;
}

Uint32* Level::GetPixels32()
{
	pixels_ = nullptr;

	if (surface_pixels_ != nullptr)
	{
		pixels_ = static_cast<Uint32*>(surface_pixels_->pixels);
	}

	return pixels_;
}

Uint32 Level::GetPitch32()
{
	Uint32 pitch = 0;

	if (surface_pixels_ != nullptr)
	{
		pitch = surface_pixels_->pitch / 4;
	}

	return pitch;
}

Tile* Level::GetTile(int x, int y)
{
	if (x < 0 || y < 0 || x >= (GetPixelWidth() * GetTileSize()) || y >= (GetPixelHeight() * GetTileSize()))
	{
		return nullptr;
	}

	const int index = (y / tile_size_) * GetPixelWidth() + (x / tile_size_);

	if (index < 0 || index >= static_cast<int>(board_.size()))
	{
		return nullptr;
	}
	
	return &board_[index];
}

// std::vector<Tile*> Level::GetNeighborTiles(int x, int y)
// {
// 	return { 
// 		GetTile(x - tile_size_, y), 
// 		GetTile(x + tile_size_, y), 
// 		GetTile(x, y - tile_size_), 
// 		GetTile(x, y + tile_size_), 
// 		GetTile(x - tile_size_, y - tile_size_), 
// 		GetTile(x + tile_size_, y - tile_size_), 
// 		GetTile(x - tile_size_, y + tile_size_), 
// 		GetTile(x + tile_size_, y + tile_size_) };
// }

void Level::ClearVerticalLines()
{
	vertical_lines_.clear();
}

void Level::AddVerticalLine(int x, int y1, int y2, SDL_Color color)
{
	vertical_lines_.emplace_back( x, y1, y2, color );
}