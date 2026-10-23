#pragma once
#include <cstdint>

namespace npc {

// SplitMix64 deterministic PRNG (stdlib compatible, minimal, cross-platform)
class DeterministicRNG {
private:
    uint64_t state;

public:
    explicit DeterministicRNG(uint64_t seed = 0) : state(seed == 0 ? 0x9E3779B97F4A7C15ULL : seed) {}

    uint64_t next_u64() {
        uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }

    uint32_t next_u32() {
        return static_cast<uint32_t>(next_u64() >> 32);
    }

    float next_float() {
        return (next_u32() >> 8) * (1.0f / 16777216.0f); // [0.0, 1.0)
    }

    float range_float(float min_val, float max_val) {
        return min_val + next_float() * (max_val - min_val);
    }

    int range_int(int min_val, int max_val) {
        if (min_val >= max_val) return min_val;
        return min_val + static_cast<int>(next_u32() % (max_val - min_val + 1));
    }
};

} // namespace npc
