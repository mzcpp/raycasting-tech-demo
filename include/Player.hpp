#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Vect2d.hpp"

#include <SDL2/SDL.h>

class Game;
class Level;
class Screen;
struct Tile;

class Player
{
private:
	Game* game_;
	Screen* screen_;
	Level* level_;

	Vect2d<float> position_;
	Vect2d<float> velocity_;
	Vect2d<float> direction_;
	Vect2d<float> plane_;

	float walk_speed_;
	float rotation_speed_;
	float rotating_degrees_;
	bool rotating_;
	bool moving_forwards_; 
	bool moving_backwards_;

public:
	Player(Game* game, Screen* screen, Level* level);

	~Player();

	void HandleEvent(SDL_Event* e);

	void Tick();

	Vect2d<float> RotatePoint(const Vect2d<float>& rotating_point, const Vect2d<float>& pivot, int degrees);

	void CastRayLines();

	void DigitalDifferentialAnalysis(int x, Vect2d<double> ray_dir);
};

#endif