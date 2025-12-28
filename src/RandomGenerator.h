#pragma once

#include <cstdint>
#include <cstring>

class RandomGenerator
{
public:
    void setSeed(double seedValue)
    {
        const auto seedBits = hashDouble(seedValue);
        const std::uint64_t combined = mixSeed(seedBits);
        state = combined != 0u ? combined : 0x6a09e667f3bcc909ULL;
    }

    std::uint64_t nextUInt64()
    {
        auto x = state;
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        state = x;
        return x * 2685821657736338717ULL;
    }

    double nextFloat01()
    {
        return static_cast<double>(nextUInt64() >> 11) * (1.0 / 9007199254740992.0);
    }

private:
    static std::uint64_t hashDouble(double value)
    {
        std::uint64_t bits = 0u;
        std::memcpy(&bits, &value, sizeof(bits));
        return mixSeed(bits);
    }

    static std::uint64_t mixSeed(std::uint64_t value)
    {
        value += 0x9e3779b97f4a7c15ULL;
        value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
        return value ^ (value >> 31);
    }

    std::uint64_t state = 0x6a09e667f3bcc909ULL;
};
