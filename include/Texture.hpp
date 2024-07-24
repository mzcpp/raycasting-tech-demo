#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <SDL2/SDL.h>

#include <cstdint>

class Game;

class Texture
{
private:
    Game* game_;
    SDL_Texture* texture_;
    SDL_Surface* surface_;

public:
    std::size_t width_;
    std::size_t height_;

    Texture(Game* game);

    ~Texture();

    void Free();

    bool LoadTextureFromFile(const char* path);

    bool LoadPixelsFromFile(const char* path);

    bool LoadTextureFromPixels();

    std::uint32_t* GetPixels32();

    std::uint32_t GetPitch32();

    std::uint32_t MapRGBA(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a);
};

#endif
