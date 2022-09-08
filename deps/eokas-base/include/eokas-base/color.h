
#ifndef  _EOKAS_BASE_COLOR_H_
#define  _EOKAS_BASE_COLOR_H_

#include "header.h"

_BeginNamespace(eokas)

/*
An rgba f32_t-normal color
*/
class Color
{
public:
	Color(f32_t r=0.0f, f32_t g=0.0f, f32_t b=0.0f, f32_t a=0.0f);
	Color(const Color& c);
	~Color();

public:
	Color& operator=(const Color& c);
	Color operator-();

	Color operator+(const Color& c);
	Color operator-(const Color& c);
	Color operator*(const Color& c);

	Color& operator+=(const Color& c);
	Color& operator-=(const Color& c);
	Color& operator*=(const Color& c);

	bool operator==(const Color& c);
	bool operator!=(const Color& c);

public:
	Color& clamp(f32_t min = 0, f32_t max = 1);

public:
	f32_t r;
	f32_t g;
	f32_t b;
	f32_t a;
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_COLOR_H_
