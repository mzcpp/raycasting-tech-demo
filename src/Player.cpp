#include "Player.hpp"
#include "Level.hpp"
#include "Game.hpp"
#include "Constants.hpp"

#include <SDL2/SDL.h>

#include <iostream>
#include <cassert>

Player::Player(Game* game, Screen* screen, Level* level) : 
	game_(game), 
	screen_(screen), 
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
		else if (e->key.keysym.sym == SDLK_w)
		{
			const double plane_len = plane_.GetLength();

			if (plane_len < 2.0)
			{
				plane_.SetLength(plane_len + 0.1);
			}
		}
		else if (e->key.keysym.sym == SDLK_s)
		{
			const double plane_len = plane_.GetLength();

			if (plane_len > 0.3)
			{
				plane_.SetLength(plane_len - 0.1);
			}
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

	if (game_->map_toggled_)
	{
		level_->Tick();
		const int scale_factor = 16;
		const std::uint32_t color1 = game_->GetColor({ 0xff, 0xff, 0xff, 0xff });
		screen_->bitmap_->DrawLine(position_.x_ * scale_factor, position_.y_ * scale_factor, (position_.x_ + direction_.x_ + plane_.x_) * scale_factor, (position_.y_ + direction_.y_ + plane_.y_) * scale_factor, color1);
		screen_->bitmap_->DrawLine(position_.x_ * scale_factor, position_.y_ * scale_factor, (position_.x_ + direction_.x_ - plane_.x_) * scale_factor, (position_.y_ + direction_.y_ - plane_.y_) * scale_factor, color1);
	}
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
	Vect2d<double> ray_step_size = { 0.0, 0.0 };

	if (game_->fisheye_effect_toggled_)
	{
		ray_step_size = { std::abs(1 / ray_dir.x_), std::abs(1 / ray_dir.y_) };
	}
	else
	{
		ray_step_size = { std::sqrt(1 + ((ray_dir.y_ / ray_dir.x_) * (ray_dir.y_ / ray_dir.x_))), std::sqrt(1 + ((ray_dir.x_ / ray_dir.y_) * (ray_dir.x_ / ray_dir.y_))) };
	}

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
	int pitch = 100;
	int draw_start = -line_height / 2 + constants::screen_height / 2 + pitch;

	if (draw_start < 0) 
	{
		draw_start = 0;
	}
	
	int draw_end = line_height / 2 + constants::screen_height / 2 + pitch;
	
	if (draw_end >= constants::screen_height)
	{
		draw_end = constants::screen_height - 1;
	}

	SDL_Color color = tile_hit->color_;

	if (game_->textures_toggled_)
	{
		const SDL_Color red = { 0xff, 0x00, 0x00, 0xff };
		const SDL_Color green = { 0x00, 0xff, 0x00, 0xff };
		const SDL_Color blue = { 0x00, 0x00, 0xff, 0xff };
		const SDL_Color yellow = { 0xff, 0xff, 0x00, 0xff };

		Texture* current_texture = nullptr;

		if (game_->ColorsEqual(color, red))
		{
			current_texture = game_->textures_[0].get();
		}
		else if (game_->ColorsEqual(color, green))
		{
			current_texture = game_->textures_[1].get();
		}
		else if (game_->ColorsEqual(color, blue))
		{
			current_texture = game_->textures_[2].get();
		}
		else if (game_->ColorsEqual(color, yellow))
		{
			current_texture = game_->textures_[3].get();
		}
		else
		{
			return;
		}

		std::uint32_t* tex_pixels = current_texture->GetPixels32();

		double wall_x = 0.0;
		int tex_width = 64;
		int tex_height = 64;

		if (wall_side == 0)
		{
			wall_x = position_.y_ + wall_dist * ray_dir.y_;
		}
		else
		{
			wall_x = position_.x_ + wall_dist * ray_dir.x_;
		}

		wall_x -= std::floor(wall_x);

		int tex_x = static_cast<int>(wall_x * static_cast<double>(tex_width));

		if (wall_side == 0 && ray_dir.x_ > 0)
		{
			tex_x = tex_width - tex_x - 1;
		}

		if (wall_side == 1 && ray_dir.y_ < 0)
		{
			tex_x = tex_width - tex_x - 1;
		}

		double tex_step = 1.0 * tex_height / line_height;
		double tex_pos = (draw_start - pitch - constants::screen_height / 2 + line_height - 2) * tex_step;

		for (int y = draw_start; y < draw_end; ++y)
		{
			int tex_y = static_cast<int>(tex_pos) & (tex_height - 1);
			tex_pos += tex_step;
			std::uint32_t pixel_color = tex_pixels[tex_height * tex_y + tex_x];

			if (wall_side == 1)
			{
				pixel_color = (pixel_color >> 1) & 8355711;
			}

			screen_->bitmap_->DrawPoint(x, y, pixel_color);
		}

	}
	else
	{
		if (wall_side == 1)
		{
			color.r /= 2;
			color.g /= 2;
			color.b /= 2;
		}

		screen_->bitmap_->DrawLine(x, draw_start, x, draw_end, game_->GetColor(color));
	}
}
