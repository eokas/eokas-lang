
#ifndef  _EOKAS_BASE_RANDOM_H_
#define  _EOKAS_BASE_RANDOM_H_

#include "header.h"

_BeginNamespace(eokas)

class Random
{
public:
	Random();
	~Random();

public:
	i32_t next();
	i32_t next(i32_t minValue, i32_t maxValue);
	f32_t next(f32_t minValue, f32_t maxValue);
	f64_t next(f64_t minValue, f64_t maxValue);

private:
	unsigned int mSeed;
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_RANDOM_H_
