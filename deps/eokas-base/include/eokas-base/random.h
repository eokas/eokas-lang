
#ifndef  _EOKAS_BASE_RANDOM_H_
#define  _EOKAS_BASE_RANDOM_H_

#include "header.h"

_BeginNamespace(eokas)

struct Random_Fake
{
    u32_t seed;

    Random_Fake();
    Random_Fake(u32_t seed);

    f32_t make() const;

    static Random_Fake globalRandom;

    static f32_t value();
    static i32_t range(i32_t min, i32_t max);
    static f32_t range(f32_t min, f32_t max);
    static f64_t range(f64_t min, f64_t max);
};

using Random = Random_Fake;

_EndNamespace(eokas)

#endif//_EOKAS_BASE_RANDOM_H_
