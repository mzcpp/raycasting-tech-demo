#include "Bitmap.hpp"

#include <SDL2/SDL.h>

#include <cstdint>
#include <cstring>
#include <iostream>

Bitmap::Bitmap(SDL_Renderer* renderer, std::size_t width, std::size_t height) : 
    renderer_(renderer), 
    width_(width), 
    height_(height)
{
    pixels_ = new std::uint32_t[width_ * height_];

    for (std::size_t i = 0; i < width_ * height_; ++i)
 	{
 		pixels_[i] = 0;
 	}

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width_, height_);

    //SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
    SDL_UpdateTexture(texture_, nullptr, pixels_, width_ * sizeof(std::uint32_t));
}

Bitmap::~Bitmap()
{
    SDL_DestroyTexture(texture_);
    texture_ = nullptr;

    delete[] pixels_;
    pixels_ = nullptr;
}

void Bitmap::DrawBitmap(const Bitmap& bitmap, int x_offset, int y_offset)
{
    for (int y = 0; y < static_cast<int>(bitmap.height_); ++y)
    {
        const int dest_y = y + y_offset;

        if (dest_y < 0 || dest_y > static_cast<int>(height_))
        {
            continue;
        }

        for (int x = 0; x < static_cast<int>(bitmap.width_); ++x)
        {
            const int dest_x = x + x_offset;

            if (dest_x < 0 || dest_x > static_cast<int>(width_))
            {
                continue;
            }

            pixels_[dest_y * width_ + dest_x] = bitmap.pixels_[y * bitmap.width_ + x];
        }   
    }
}

void Bitmap::DrawFillRect(int top_left_x, int top_left_y, int bottom_right_x, int bottom_right_y, std::uint32_t color)
{
    if (top_left_x > bottom_right_x || top_left_y > bottom_right_y)
    {
        return;
    }

    for (int y = top_left_y; y < bottom_right_y; ++y)
    {
        if (y < 0 || y > static_cast<int>(height_))
        {
            continue;
        }

        for (int x = top_left_x; x < bottom_right_x; ++x)
        {
            if (x < 0 || x > static_cast<int>(width_))
            {
                continue;
            }

            pixels_[y * width_ + x] = color;
        }
    }
}

void Bitmap::DrawPoint(int x, int y, std::uint32_t color)
{
    if (x < 0 || x > static_cast<int>(width_) || y < 0 || y > static_cast<int>(height_))
    {
        return;
    }

    pixels_[y * width_ + x] = color;
}

void Bitmap::DrawLine(int x1, int y1, int x2, int y2, std::uint32_t color)
{
    bool y_longer = false;
    int increment_val = 0;
    int end_val = 0;
	int short_len = y2 - y1;
	int long_len = x2 - x1;
	
    if (std::abs(short_len) > std::abs(long_len))
    {
		int swap = short_len;
		short_len = long_len;
		long_len = swap;
		y_longer = true;
	}
	
	end_val = long_len;
	
    if (long_len < 0)
    {
		increment_val = -1;
		long_len = -long_len;
	} 
    else
    { 
        increment_val = 1;
    }

	double dec_inc = 0.0;

	if (long_len == 0)
    {
        dec_inc = static_cast<double>(short_len);
    } 
	else
    { 
        dec_inc = (static_cast<double>(short_len) / static_cast<double>(long_len));
    }
	
    double j = 0.0;
	
    if (y_longer)
    {
		for (int i = 0; i != end_val; i += increment_val)
        {
            pixels_[(y1 + i) * width_ + (x1 + static_cast<int>(j))] = color;
			j += dec_inc;
		}
	} 
    else 
    {
		for (int i = 0; i != end_val; i += increment_val)
        {
            pixels_[(y1 + static_cast<int>(j)) * width_ + (x1 + i)] = color;
			j += dec_inc;
		}
	}
}

void Bitmap::Render()
{
    SDL_UpdateTexture(texture_, nullptr, pixels_, width_ * sizeof(std::uint32_t));
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
}

void Bitmap::Clear()
{
    SDL_Color background_color;
    background_color.r = 0;
    background_color.g = 0;
    background_color.b = 0;

    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    std::uint32_t color = (background_color.b << 24) + (background_color.g << 16) + (background_color.r << 8) + 255;
    #else
    std::uint32_t color = (255 << 24) + (background_color.r << 16) + (background_color.g << 8) + background_color.b;
    #endif

    for (std::size_t i = 0; i < width_ * height_; ++i)
    {
        pixels_[i] = color;
    }
}
