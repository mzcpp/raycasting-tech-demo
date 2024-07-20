#include "Game.hpp"
#include "Level.hpp"
#include "Constants.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>

Level::Level(Game* game, Screen* screen) :
	game_(game), 
	screen_(screen), 
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
	std::for_each(board_.begin(), board_.end(), [this](Tile tile)
	{
		int scale_factor = 16;
		tile.rect_.x *= scale_factor;
		tile.rect_.y *= scale_factor;
		tile.rect_.w *= scale_factor;
		tile.rect_.h *= scale_factor;

		screen_->bitmap_->DrawFillRect(tile.rect_.x, tile.rect_.y, tile.rect_.x + tile.rect_.w, tile.rect_.y + tile.rect_.h, game_->GetColor(tile.color_));
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

	int tile_x = 0;
	int tile_y = 0;

	for (int x = 0; x < pixel_width_; ++x)
	{
		for (int y = 0; y < pixel_height_; ++y)
		{
			SDL_Color rgb;
			Uint32 data = GetPixel(surface_pixels_, x, y);
			SDL_GetRGB(data, surface_pixels_->format, &rgb.r, &rgb.g, &rgb.b);

			const int index = y * pixel_width_ + x;

			board_[index].is_wall_ = true;

			if (static_cast<int>(rgb.r) == 0 && static_cast<int>(rgb.g) == 0 && static_cast<int>(rgb.b) == 0)
			{
				board_[y * pixel_width_ + x].is_wall_ = false;
			}

			board_[index].color_.r = static_cast<int>(rgb.r);
			board_[index].color_.g = static_cast<int>(rgb.g);
			board_[index].color_.b = static_cast<int>(rgb.b);
			board_[index].color_.a = 255;
			board_[index].rect_.x = tile_x;
			board_[index].rect_.y = tile_y;
			board_[index].rect_.w = tile_size_;
			board_[index].rect_.h = board_[index].rect_.w;
			
			tile_y += tile_size_;
		}
		
		tile_x += tile_size_;
		tile_y = 0;
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

// void Level::ClearVerticalLines()
// {
// 	vertical_lines_.clear();
// }

// void Level::AddVerticalLine(int x, int y1, int y2, SDL_Color color)
// {
// 	vertical_lines_.emplace_back( x, y1, y2, color );
// }

Uint32 Level::GetPixel(SDL_Surface *surface, int x, int y)
{
    int bytes_per_pixel = surface->format->BytesPerPixel;
    Uint8* pixel = (Uint8*) surface->pixels + y * surface->pitch + x * bytes_per_pixel;

    if (bytes_per_pixel == 1)
    {
        return *pixel;
    }
    else if (bytes_per_pixel == 2)
    {
        return *(Uint16*) pixel;
    }
    else if (bytes_per_pixel == 3)
    {
    	if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            return pixel[0] << 16 | pixel[1] << 8 | pixel[2];
        }
        else
        {
            return pixel[0] | pixel[1] << 8 | pixel[2] << 16;
        }
    }
    else if (bytes_per_pixel == 4)
    {
        return *(Uint32*) pixel;
    }

    return 0;
}