#pragma once

#include <cstdint>

class RandomGenerator
{
public:
    RandomGenerator() = default;

    void setSeed(uint32_t seed)
    {
        state = (seed == 0u) ? 0x6d2b79f5u : seed;
    }

    uint32_t nextUInt()
    {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;
        return x;
    }

    float nextFloat01()
    {
        return static_cast<float>(nextUInt()) / static_cast<float>(0xffffffffu);
    }

    float nextFloatSigned()
    {
        return nextFloat01() * 2.0f - 1.0f;
    }

    float nextFloatRange(float minValue, float maxValue)
    {
        return minValue + (maxValue - minValue) * nextFloat01();
    }

private:
    uint32_t state { 0x6d2b79f5u };
};
