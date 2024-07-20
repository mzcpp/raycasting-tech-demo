#ifndef BITMAP_HPP
#define BITMAP_HPP

#include <SDL2/SDL.h>

#include <cstdint>

class Bitmap
{
private:
    SDL_Renderer* renderer_;
    
public:
    std::size_t width_;
    std::size_t height_;
    std::uint32_t* pixels_;
	SDL_Texture* texture_;

    Bitmap(SDL_Renderer* renderer, std::size_t width, std::size_t height);

    ~Bitmap();

    void DrawBitmap(const Bitmap& bitmap, int x_offset, int y_offset);

    void DrawFillRect(int top_left_x, int top_left_y, int bottom_right_x, int bottom_right_y, std::uint32_t color);

    void DrawLine(int x, int y, int x2, int y2, std::uint32_t color);

    void DrawPoint(int x, int y, std::uint32_t color);

    void Render();

    void Clear();
};

#endif