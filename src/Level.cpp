#include "Game.hpp"
#include "Level.hpp"
#include "Constants.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>
#include <cstdlib>

Level::Level(Game* game, Screen* screen) :
	game_(game), 
	screen_(screen), 
	surface_pixels_(nullptr), 
	pixels_(nullptr), 
	tiles_col_count_(0), 
	tiles_row_count_(0), 
	tiles_count_(0), 
	tile_size_(1)
{
	std::srand(std::time(nullptr));
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

	tiles_col_count_ = surface_pixels_->w;
	tiles_row_count_ = surface_pixels_->h;
	tiles_count_ = tiles_col_count_ * tiles_row_count_;

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

	for (int x = 0; x < tiles_col_count_; ++x)
	{
		for (int y = 0; y < tiles_row_count_; ++y)
		{
			SDL_Color rgb;
			Uint32 data = GetPixel(surface_pixels_, x, y);
			SDL_GetRGB(data, surface_pixels_->format, &rgb.r, &rgb.g, &rgb.b);

			const int index = y * tiles_col_count_ + x;

			board_[index].is_wall_ = true;

			if (static_cast<int>(rgb.r) == 0 && static_cast<int>(rgb.g) == 0 && static_cast<int>(rgb.b) == 0)
			{
				board_[y * tiles_col_count_ + x].is_wall_ = false;
			}

			board_[index].color_.r = rgb.r;
			board_[index].color_.g = rgb.g;
			board_[index].color_.b = rgb.b;
			board_[index].color_.a = 0xff;
			board_[index].rect_.x = tile_x;
			board_[index].rect_.y = tile_y;
			board_[index].rect_.w = tile_size_;
			board_[index].rect_.h = tile_size_;
			
			tile_y += tile_size_;
		}
		
		tile_x += tile_size_;
		tile_y = 0;
	}

	return true;
}

void Level::GenerateMazeHuntAndKill()
{
	tiles_col_count_ = 29;
	tiles_row_count_ = 27;

	tiles_col_count_ -= tiles_col_count_ % 2;
	++tiles_col_count_;

	tiles_row_count_ -= tiles_row_count_ % 2;
	++tiles_row_count_;

	tiles_count_ = tiles_col_count_ * tiles_row_count_;

	board_.clear();
	board_.resize(tiles_count_);

	int tile_x = 0;
	int tile_y = 0;

	for (int y = 0; y < tiles_row_count_; ++y)
	{
		for (int x = 0; x < tiles_col_count_; ++x)
		{
			const int index = y * tiles_col_count_ + x;
			
			board_[index].is_wall_ = true;
			board_[index].color_.r = 0x00;
			board_[index].color_.g = 0xff;
			board_[index].color_.b = 0x00;
			board_[index].color_.a = 0xff;
			board_[index].rect_.x = tile_x;
			board_[index].rect_.y = tile_y;
			board_[index].rect_.w = tile_size_;
			board_[index].rect_.h = tile_size_;

			if (x == 0 || x == tiles_col_count_ - 1 || y == 0 || y == tiles_row_count_ - 1)
			{
				board_[index].color_.r = 0xff;
				board_[index].color_.g = 0x00;
				board_[index].color_.b = 0x00;
			}

			tile_x += tile_size_;

		}

		tile_y += tile_size_;
		tile_x = 0;
	}

	int current_tile_index = 1 * tiles_col_count_ + 1;

	while (!BoardComplete())
	{
		DeleteWall(current_tile_index);
		const std::vector<int>& neighbor_indices = GetNeighborTilesIndices(current_tile_index);

		const bool all_uncovered = std::all_of(neighbor_indices.begin(), neighbor_indices.end(), [this](int neighbor_index)
			{
				return !board_[neighbor_index].is_wall_;
			});

		if (neighbor_indices.empty() || all_uncovered)
		{
			bool found = false;

			for (int y = 1; y < tiles_row_count_; y += 2)
			{
				for (int x = 1; x < tiles_col_count_; x += 2)
				{
					const int index = y * tiles_col_count_ + x;

					if (!board_[index].is_wall_)
					{
						continue;
					}

					for (int neighbor_index : GetNeighborTilesIndices(index))
					{
						if (board_[neighbor_index].is_wall_)
						{
							continue;	
						}
						
						const int delta = (neighbor_index - index) / 2;
						DeleteWall(neighbor_index);
						DeleteWall(index + delta);
						current_tile_index = index;
						break;
					}

					if (current_tile_index == index)
					{
						found = true;
						break;
					}
				}
				
				if (found)
				{
					break;
				}
			}
		}
		else
		{
			int random_neighbor_index = -1;
			
			do 
			{
				random_neighbor_index = neighbor_indices[std::rand() % neighbor_indices.size()];
			}
			while (!board_[random_neighbor_index].is_wall_);

			const int delta = (random_neighbor_index - current_tile_index) / 2;
			
			DeleteWall(random_neighbor_index);
			DeleteWall(current_tile_index + delta);

			current_tile_index = random_neighbor_index;
		}
	}

	game_->player_->SetPos({ 1.5f, 1.5f });
}

bool Level::BoardComplete()
{
	for (int y = 1; y < tiles_row_count_; y += 2)
	{
		for (int x = 1; x < tiles_col_count_; x += 2)
		{
			if (board_[y * tiles_col_count_ + x].is_wall_)
			{
				return false;
			}
		}
	}

	return true;
}

void Level::DeleteWall(std::size_t index)
{
	board_[index].is_wall_ = false;
	board_[index].color_.r = 0x00;
	board_[index].color_.g = 0x00;
	board_[index].color_.b = 0x00;
}

std::vector<int> Level::GetNeighborTilesIndices(int index)
{
	std::vector<int> result;

	const int north_neighbor_index = index - (2 * tiles_col_count_);
	const int east_neighbor_index = index + 2;
	const int south_neighbor_index = index + (2 * tiles_col_count_);
	const int west_neighbor_index = index - 2;

	if (index > (tiles_col_count_ * 3 - 1))
	{
		result.push_back(north_neighbor_index);
	}

	if ((index - (tiles_col_count_ - 2)) % tiles_col_count_ != 0 && (index - (tiles_col_count_ - 3)) % tiles_col_count_ != 0)
	{
		result.push_back(east_neighbor_index);
	}

	if (index < (tiles_col_count_ * (tiles_row_count_ - 3) - 1))
	{
		result.push_back(south_neighbor_index);
	}

	if ((index - 1) % tiles_col_count_ != 0 && (index - 2) % tiles_col_count_ != 0)
	{
		result.push_back(west_neighbor_index);
	}
	
	return result;
}

void Level::Free()
{
	if (surface_pixels_ != nullptr)
	{
		SDL_FreeSurface(surface_pixels_);
		surface_pixels_ = nullptr;
	}
}

int Level::GetColumnCount()
{
	return tiles_col_count_;
}
	
int Level::GetRowCount()
{
	return tiles_row_count_;
}

int Level::GetPixelCount()
{
	return tiles_count_;
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
	if (x < 0 || y < 0 || x >= (GetColumnCount() * GetTileSize()) || y >= (GetRowCount() * GetTileSize()))
	{
		return nullptr;
	}

	const int index = (y / tile_size_) * GetColumnCount() + (x / tile_size_);

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