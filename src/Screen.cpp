#include "Screen.hpp"
#include "Bitmap.hpp"
#include "Constants.hpp"
#include "Game.hpp"

Screen::Screen(Game* game) : 
    game_(game), 
    bitmap_(std::make_unique<Bitmap>(game->renderer_, constants::screen_width, constants::screen_height))
{
}

void Screen::Render()
{
    bitmap_->Render();
}
