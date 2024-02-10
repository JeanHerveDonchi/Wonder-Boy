#include "Vec2.h"
#include <cmath>

namespace 
{

	// fp comparisons by Donald Knuth from TAOCP
	// The floating-point comparison algorithm by Donald Knuth 
	// from "The Art of Computer Programming (TAOCP)" addresses issues 
	// commonly encountered when comparing floating-point numbers, such as rounding errors.
	const static float EPS{ 0.0000001f };

	bool approximatelyEqual(float a, float b, float epsilon = EPS)
	{
		return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
		// fabs() is used to get the absolute value of a or b
		// choose bigger absolute value, not smaller value
		// and check if the difference between a and b is less than or equal to the product of the bigger absolute value * epsilon
	}

	bool essentiallyEqual(float a, float b, float epsilon = EPS)
	{
		return fabs(a - b) <= ((fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
		// fabs() is used to get the absolute value of a or b
		// choose smaller absolute value, not bigger value
		// and check if the difference between a and b is less than or equal to the product of the smaller absolute value * epsilon
		// more precise than approximatelyEqual
	}

	bool definitelyGreaterThan(float a, float b, float epsilon = EPS)
	{
		return (a - b) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
		// fabs() is used to get the absolute value of a or b
		// choose bigger absolute value, not smaller value
		// and check if the difference between a and b is greater than the product of the bigger absolute value * epsilon
		// if it returns true, then a is definitely greater than b 
		// because using the bigger absolute value makes the tolerance range bigger
	}

	bool definitelyLessThan(float a, float b, float epsilon = EPS)
	{
		return (b - a) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
		// fabs() is used to get the absolute value of a or b
		// same as definitelyGreaterThan, but checks if b is definitely greater than a
		// if it returns true, then a is definitely less than b
		// because using the bigger absolute value makes the tolerance range bigger
	}
}

// trigonometric functions helper
const float PI = 3.14159265358979323846f;

// radian represents an angle formed by an arc whose length is equal to radius
// circumference(360 degree) = PI * 2 * radius
// 180 degree / PI = 1 radian
// ex. a rad = a * ( 360 / (2*PI) ) degree
inline float radToDeg(float radValue) 
{
	return radValue * (180.f / PI);
}

// PI/180 radian = 1 degree
// ex. a degree = a * ((PI *2) / 360) rad
inline float degToRad(float degValue) 
{
	return degValue * (PI / 180.f);
}

Vec2::Vec2()
{}

Vec2::Vec2(float x, float y)
	: x(x), y(y)
{}

Vec2::Vec2(int x, int y)
	: x(static_cast<float>(x)), y(static_cast<float>(y))
{}
// static cast is used to convert int to float with no loss of data

Vec2::Vec2(float bearing)
	: x(std::cos(bearing)), y(std::sin(bearing)) // if hypotenuse is 1, cos = x, sin = y
{}
// when include C++ cmath, sin and cos are defined in std namespace globally
// but using std:: is better for readability and team coding

