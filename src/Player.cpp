#include "Player.hpp"
#include "Level.hpp"
#include "Game.hpp"
#include "Constants.hpp"

#include <SDL2/SDL.h>

#include <iostream>
#include <cassert>

Player::Player(Game* game, Level* level) : 
	game_(game), 
	level_(level), 
	position_(2.0f, 2.0f), 
	velocity_(0.0f, 0.0f), 
	direction_(1.0f, 0.0f), 
	plane_(0.0f, 0.66f),
	walk_speed_(0.15f), 
	rotation_speed_(3.0f), 
	rotating_degrees_(0.0f), 
	rotating_(false), 
	moving_forwards_(false), 
	moving_backwards_(false)
{
}

Player::~Player()
{
}

void Player::HandleEvent(SDL_Event* e)
{
	if (e->type == SDL_KEYDOWN)
	{
		if (e->key.keysym.sym == SDLK_UP)
		{
			moving_forwards_ = true;
			velocity_.x_ = walk_speed_ * direction_.x_;
			velocity_.y_ = walk_speed_ * direction_.y_;
		}
		else if (e->key.keysym.sym == SDLK_DOWN)
		{
			moving_backwards_ = true;
			velocity_.x_ = walk_speed_ * -direction_.x_;
			velocity_.y_ = walk_speed_ * -direction_.y_;
		}
		
		if (e->key.keysym.sym == SDLK_LEFT)
		{	
			rotating_ = true;
			rotating_degrees_ = -rotation_speed_;
		}
		else if (e->key.keysym.sym == SDLK_RIGHT)
		{
			rotating_ = true;
			rotating_degrees_ = rotation_speed_;
		}
	}

	if (e->type == SDL_KEYUP)
	{
		if (e->key.keysym.sym == SDLK_UP)
		{
			moving_forwards_ = false;
			velocity_.x_ = 0.0f;
			velocity_.y_ = 0.0f;
		}

		if (e->key.keysym.sym == SDLK_DOWN)
		{
			moving_backwards_ = false;
			velocity_.x_ = 0.0f;
			velocity_.y_ = 0.0f;
		}
		
		if (e->key.keysym.sym == SDLK_LEFT || e->key.keysym.sym == SDLK_RIGHT)
		{
			rotating_ = false;
			rotating_degrees_ = 0.0f;
		}
	}
}

void Player::Tick()
{	
	level_->ClearVerticalLines();
	CastRayLines();

	if (rotating_)
	{
		const Vect2d<float> direction_point = { position_.x_ + direction_.x_, position_.y_ + direction_.y_ };
		const Vect2d<float> rotated_direction_point = RotatePoint(direction_point, position_, rotating_degrees_);
		
		const Vect2d<float> plane_point = { position_.x_ + direction_.x_ + plane_.x_, position_.y_ + direction_.y_ + plane_.y_ };
		const Vect2d<float> rotated_plane_point = RotatePoint(plane_point, position_, rotating_degrees_);
		
		direction_.x_ = rotated_direction_point.x_ - position_.x_;
		direction_.y_ = rotated_direction_point.y_ - position_.y_;
		plane_.x_ = rotated_plane_point.x_ - position_.x_ - direction_.x_;
		plane_.y_ = rotated_plane_point.y_ - position_.y_ - direction_.y_;

		if (moving_forwards_ || moving_backwards_)
		{
			velocity_.x_ = walk_speed_ * (moving_backwards_ ? -direction_.x_ : direction_.x_);
			velocity_.y_ = walk_speed_ * (moving_backwards_ ? -direction_.y_ : direction_.y_);
		}
	}

	if (moving_forwards_ || moving_backwards_)
	{
		const Tile* next_tile = level_->GetTile(position_.x_ + velocity_.x_, position_.y_ + velocity_.y_);

		if (next_tile != nullptr && !next_tile->is_wall_)
		{
			position_.x_ += velocity_.x_;
			position_.y_ += velocity_.y_;
		}
	}
}

void Player::Render()
{
	SDL_RenderSetViewport(game_->renderer_, NULL);

	int scale_factor = 16;
	// SDL_FRect player = { position_.x_ * static_cast<float>(scale_factor), position_.y_ * static_cast<float>(scale_factor), static_cast<float>(scale_factor), static_cast<float>(scale_factor) };
	// SDL_SetRenderDrawColor(game_->renderer_, 0xff, 0xff, 0xff, 0xff);
	// SDL_RenderFillRectF(game_->renderer_, &player);

	SDL_SetRenderDrawColor(game_->renderer_, 0xff, 0xff, 0xff, 0xff);
	
	const int x1 = 0;
	const int x2 = constants::screen_width - 1;

	const double ray_dir_x1 = direction_.x_ + plane_.x_ * (((2 * x1) / static_cast<double>(constants::screen_width)) - 1);
	const double ray_dir_y1 = direction_.y_ + plane_.y_ * (((2 * x1) / static_cast<double>(constants::screen_width)) - 1);

	const double ray_dir_x2 = direction_.x_ + plane_.x_ * (((2 * x2) / static_cast<double>(constants::screen_width)) - 1);
	const double ray_dir_y2 = direction_.y_ + plane_.y_ * (((2 * x2) / static_cast<double>(constants::screen_width)) - 1);

	const float pos_x_offset = position_.x_ * scale_factor;
	const float pos_y_offset = position_.y_ * scale_factor;

	SDL_RenderDrawLineF(game_->renderer_, pos_x_offset, pos_y_offset, pos_x_offset + (ray_dir_x1 * scale_factor), pos_y_offset + (ray_dir_y1 * scale_factor));
	SDL_RenderDrawLineF(game_->renderer_, pos_x_offset, pos_y_offset, pos_x_offset + (ray_dir_x2 * scale_factor), pos_y_offset + (ray_dir_y2 * scale_factor));
}

Vect2d<float> Player::RotatePoint(const Vect2d<float>& rotating_point, const Vect2d<float>& pivot, int degrees)
{
	Vect2d<float> result_point = { rotating_point.x_, rotating_point.y_ };

	const double pi = std::acos(-1);
	const double deg_to_rad = static_cast<double>(degrees) * pi / 180.0;
	const double sin_degrees = std::sin(deg_to_rad);
	const double cos_degrees = std::cos(deg_to_rad);

	const double new_x = (result_point.x_ - pivot.x_) * cos_degrees - (result_point.y_ - pivot.y_) * sin_degrees;
	const double new_y = (result_point.x_ - pivot.x_) * sin_degrees + (result_point.y_ - pivot.y_) * cos_degrees;

	result_point.x_ = new_x + pivot.x_;
	result_point.y_ = new_y + pivot.y_;

	return result_point;
}

void Player::CastRayLines()
{
	for (int x = 0; x < constants::screen_width; ++x)
	{
		const double camera_x = ((2 * x) / static_cast<double>(constants::screen_width)) - 1;
		const double ray_dir_x = direction_.x_ + plane_.x_ * camera_x;
		const double ray_dir_y = direction_.y_ + plane_.y_ * camera_x;

		DigitalDifferentialAnalysis(x, { ray_dir_x, ray_dir_y });
	}
}

void Player::DigitalDifferentialAnalysis(int x, Vect2d<double> ray_dir)
{
	Vect2d<int> map_check = { static_cast<int>(position_.x_), static_cast<int>(position_.y_) };
	// const Vect2d<double> ray_step_size = { std::abs(1 / ray_dir.x_), std::abs(1 / ray_dir.y_) };
	const Vect2d<double> ray_step_size = { std::sqrt(1 + ((ray_dir.y_ / ray_dir.x_) * (ray_dir.y_ / ray_dir.x_))), std::sqrt(1 + ((ray_dir.x_ / ray_dir.y_) * (ray_dir.x_ / ray_dir.y_))) };
	Vect2d<double> ray_length = { 0.0, 0.0 };
	Vect2d<int> step = { 0, 0 };

	bool wall_hit = false;
	int wall_side = -1;

	if (ray_dir.x_ < 0)
	{
		step.x_ = -1;
		ray_length.x_ = (position_.x_ - map_check.x_) * ray_step_size.x_;
	}
	else
	{
		step.x_ = 1;
		ray_length.x_ = (map_check.x_ + 1 - position_.x_) * ray_step_size.x_;
	}

	if (ray_dir.y_ < 0)
	{
		step.y_ = -1;
		ray_length.y_ = (position_.y_ - map_check.y_) * ray_step_size.y_;
	}
	else
	{
		step.y_ = 1;
		ray_length.y_ = (map_check.y_ + 1 - position_.y_) * ray_step_size.y_;
	}

	int loop_guard = 0;

	while (!wall_hit)
	{
		if (++loop_guard == 10000)
		{
			assert(false);
		}

		if (ray_length.x_ < ray_length.y_)
		{
			ray_length.x_ += ray_step_size.x_;
			map_check.x_ += step.x_;
			wall_side = 0;
		}
		else
		{
			ray_length.y_ += ray_step_size.y_;
			map_check.y_ += step.y_;
			wall_side = 1;
		}

		wall_hit = level_->GetTile(map_check.x_, map_check.y_)->is_wall_;
	}

	assert(wall_side != -1);

	Tile* tile_hit = level_->GetTile(map_check.x_, map_check.y_);

	if (wall_side == 0)
	{
		ray_length.x_ -= ray_step_size.x_;
	}
	else
	{
		ray_length.y_ -= ray_step_size.y_;
	}
	
	const double pi = std::acos(-1);
	const double dot = std::clamp(((ray_dir.x_ * direction_.x_) + (ray_dir.y_ * direction_.y_)) / (ray_dir.GetLength() * direction_.GetLength()), -1.0, 1.0);
	const double rad_angle = std::acos(dot);
	[[maybe_unused]] const double deg_angle = (rad_angle * (180.0 / pi));
	
	double wall_dist = wall_side == 0 ? ray_length.x_ : ray_length.y_;
	wall_dist *= std::cos(rad_angle);	

	int line_height = static_cast<int>(constants::screen_height / wall_dist);

	int draw_start = -line_height / 2 + constants::screen_height / 2;

	if (draw_start < 0) 
	{
		draw_start = 0;
	}
	
	int draw_end = line_height / 2 + constants::screen_height / 2;
	
	if (draw_end >= constants::screen_height)
	{
		draw_end = constants::screen_height - 1;
	}

	SDL_Color color = tile_hit->color_;

	if (wall_side == 1)
	{
		color.r /= 2;
		color.g /= 2;
		color.b /= 2;
	}

	level_->AddVerticalLine(x, draw_start, draw_end, color);
}
