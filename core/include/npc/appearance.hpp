#pragma once
#include "types.hpp"
#include <string>
#include <array>
#include <algorithm>

namespace npc {

struct BodyConfiguration {
    float height{0.5f};       // 0.0 (short) - 1.0 (tall)
    float body_mass{0.5f};    // 0.0 (skinny) - 1.0 (fat/heavy)
    float muscularity{0.5f};  // 0.0 (lean) - 1.0 (muscular)
    float posture{0.0f};      // 0.0 (upright) - 1.0 (hunched)
    float head_scale{0.5f};   // 0.0 - 1.0
    float torso_ratio{0.5f};  // 0.0 - 1.0
    float arm_ratio{0.5f};    // 0.0 - 1.0
    float leg_ratio{0.5f};    // 0.0 - 1.0

    void clamp() {
        height = std::clamp(height, 0.0f, 1.0f);
        body_mass = std::clamp(body_mass, 0.0f, 1.0f);
        muscularity = std::clamp(muscularity, 0.0f, 1.0f);
        posture = std::clamp(posture, 0.0f, 1.0f);
        head_scale = std::clamp(head_scale, 0.0f, 1.0f);
        torso_ratio = std::clamp(torso_ratio, 0.0f, 1.0f);
        arm_ratio = std::clamp(arm_ratio, 0.0f, 1.0f);
        leg_ratio = std::clamp(leg_ratio, 0.0f, 1.0f);
    }
};

struct FaceConfiguration {
    uint8_t eye_shape{0};
    uint8_t eye_spacing{1};
    ColorRGBA eye_color{40, 30, 20, 255};
    uint8_t nose_shape{0};
    uint8_t mouth_shape{0};
    uint8_t hair_style{0};    // 0: Bald, 1: Short, 2: Medium, 3: Long
    ColorRGBA hair_color{50, 40, 30, 255};
    ColorRGBA skin_color{225, 185, 150, 255};
    ColorRGBA shirt_color{60, 90, 140, 255};
    ColorRGBA pants_color{45, 50, 60, 255};
    Expression expression{Expression::Neutral};
};

struct HumanAppearance {
    uint32_t npc_id{0};
    uint64_t appearance_seed{12345};
    uint32_t generator_version{1};
    BodyConfiguration body;
    FaceConfiguration face;
};

} // namespace npc
