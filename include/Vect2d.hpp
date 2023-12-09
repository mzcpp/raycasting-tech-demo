#ifndef VECT_2D
#define VECT_2D

#include <cmath>

#include <iostream>

template <typename T>
class Vect2d
{
public:
	T x_;
	T y_;

	Vect2d(T x, T y) : x_(x), y_(y)
	{
	}

	void Normalize()
	{
		T length = GetLength();
		x_ /= length;
		y_ /= length;
	}

	void SetLength(T length)
	{
		Normalize();
		x_ *= length;
		y_ *= length;
	}

	T GetLength()
	{
		return std::sqrt((x_ * x_) + (y_ * y_));
	}
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Vect2d<T>& vect)
{
    os << vect.x_ << ' ' << vect.y_ << std::endl;
    return os;
}

#endif