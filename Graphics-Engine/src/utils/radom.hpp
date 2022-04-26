#pragma once

#include <random>

static float Random(int32_t min, int32_t max)
{
	
    static std::random_device dev;
    static std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);
    return dist(rng);
}
