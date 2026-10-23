#pragma once
#include <cstdint>
#include <string>

namespace npc {

enum class Direction : uint8_t {
    South = 0, // Front
    North,     // Back
    East,      // Right
    West       // Left
};

enum class AnimationState : uint8_t {
    Idle = 0,
    Walk,
    Run,
    Attack,
    Fall,
    Swim
};

enum class BodyPart : uint8_t {
    Head = 0,
    Torso,
    LeftArm,
    RightArm,
    LeftHand,
    RightHand,
    LeftLeg,
    RightLeg,
    LeftFoot,
    RightFoot,
    Count
};

enum class Expression : uint8_t {
    Neutral = 0,
    Happy,
    Sad,
    Angry,
    InPain,
    Surprised
};

enum class ConditionType : uint8_t {
    Cut = 0,
    Bruise,
    Burn,
    Scar,
    Bandage,
    BloodStain
};

struct ColorRGBA {
    uint8_t r{255};
    uint8_t g{255};
    uint8_t b{255};
    uint8_t a{255};

    constexpr bool operator==(const ColorRGBA& other) const = default;
};

} // namespace npc
