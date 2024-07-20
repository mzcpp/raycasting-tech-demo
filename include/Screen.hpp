#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "Bitmap.hpp"

#include <memory>

class Game;

class Screen
{
private:
    Game* game_;

public:
    std::unique_ptr<Bitmap> bitmap_;

    Screen(Game* game);

    void Render();

};

#endif
