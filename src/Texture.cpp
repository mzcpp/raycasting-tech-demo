#include "Texture.hpp"
#include "Game.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

Texture::Texture(Game* game) : 
    game_(game), 
    texture_(nullptr), 
    surface_(nullptr), 
    width_(0), 
    height_(0)
{
}

Texture::~Texture()
{
    Free();
}

void Texture::Free()
{
    if (texture_ != nullptr)
    {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }

    if (surface_ != nullptr)
    {
        SDL_FreeSurface(surface_);
        surface_ = nullptr;
    }

    width_ = 0;
    height_ = 0;
}

bool Texture::LoadTextureFromFile(const char* path)
{
	if (!LoadPixelsFromFile(path))
	{
		printf("Failed to load pixels for %s!\n", path);
        return false;
	}

    if (!LoadTextureFromPixels())
    {
        printf("Failed to load texture from pixels from %s!\n", path);
        return false;
    }

    return true;
}

bool Texture::LoadPixelsFromFile(const char* path)
{
    Free();

    SDL_Surface* loaded_surface = IMG_Load(path);

    if (loaded_surface == nullptr)
    {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return false;
    }

    surface_ = SDL_ConvertSurfaceFormat(loaded_surface, SDL_GetWindowPixelFormat(game_->window_), 0);

    if (surface_ == nullptr)
    {
        printf("Unable to convert loaded surface to display format! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    width_ = surface_->w;
    height_ = surface_->h;
    SDL_FreeSurface(loaded_surface);

    return true;
}

bool Texture::LoadTextureFromPixels()
{
    if (surface_ == nullptr)
    {
		printf("No pixels loaded!\n");
        return false;
    }

    SDL_SetColorKey(surface_, SDL_TRUE, SDL_MapRGB(surface_->format, 0xff, 0, 0xff));

    texture_ = SDL_CreateTextureFromSurface(game_->renderer_, surface_);

    if (texture_ == nullptr)
    {
		printf("Unable to create texture from loaded pixels! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    width_ = surface_->w;
    height_ = surface_->h;
    
    SDL_FreeSurface(surface_);
    surface_ = nullptr;

    return true;
}

std::uint32_t* Texture::GetPixels32()
{
    std::uint32_t* pixels = nullptr;

	if (surface_ != nullptr)
	{
		pixels = static_cast<std::uint32_t*>(surface_->pixels);
	}

	return pixels;
}

std::uint32_t Texture::GetPitch32()
{
    std::uint32_t pitch = 0;

	if (surface_ != nullptr)
	{
		pitch = surface_->pitch / 4;
	}

	return pitch;
}

std::uint32_t Texture::MapRGBA(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
{
    std::uint32_t pixel = 0;

	if (surface_ != nullptr)
	{
		pixel = SDL_MapRGBA(surface_->format, r, g, b, a);
	}

	return pixel;
}
