#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace npc {

// Forward declaration
struct AnatomyProfile;

// Canonical bounds for all geometry dimensions
struct GeometryBounds {
    static constexpr int SHOULDER_W_MIN = 9;
    static constexpr int SHOULDER_W_MAX = 18;
    static constexpr int WAIST_W_MIN = 6;
    static constexpr int WAIST_W_MAX = 16;
    static constexpr int HIPS_W_MIN = 6;
    static constexpr int HIPS_W_MAX = 16;
    static constexpr int THIGH_THICK_MIN = 2;
    static constexpr int THIGH_THICK_MAX = 4;
    static constexpr int NECK_W_MIN = 2;
    static constexpr int NECK_W_MAX = 3;
    static constexpr int ARM_BASE_THICK_MIN = 2;
    static constexpr int ARM_BASE_THICK_MAX = 3;
    static constexpr int LEG_GAP_MIN = 0;
    static constexpr int LEG_GAP_MAX = 1;

    static constexpr int TOTAL_HEIGHT_MIN = 36;
    static constexpr int TOTAL_HEIGHT_MAX = 42;
    static constexpr int LEG_LEN_MIN = 13;
    static constexpr int LEG_LEN_MAX = 17;
    static constexpr int TORSO_H_MIN = 11;
    static constexpr int TORSO_H_MAX = 19;
    static constexpr int ARM_LEN_MIN = 13;
    static constexpr int ARM_LEN_MAX = 21;
};

// Integer offsets for explicit overrides. 0 means formula default.
struct GeometryOverride {
    int shoulder_w{0};
    int waist_w{0};
    int hips_w{0};
    int thigh_thick{0};
    int neck_w{0};
    int arm_base_thick{0};
    int leg_gap{0};
    int total_height{0};
    int leg_len{0};
    int torso_h{0};
    int arm_len{0};

    bool operator==(const GeometryOverride& other) const = default;
};

// Core base body geometry calculated from anatomy parameters and optional overrides
struct BodyGeometry {
    // 7 horizontal / thickness values
    int shoulder_w{11};     // Shoulder width in pixels
    int waist_w{8};          // Waist width in pixels
    int hips_w{8};           // Hips/pelvis width in pixels
    int thigh_thick{3};      // Thigh thickness in pixels
    int neck_w{2};           // Neck width in pixels
    int arm_base_thick{2};   // Base arm thickness in pixels
    int leg_gap{0};          // Gap between legs (0 or 1) in pixels

    // 4 vertical / length values (computed in the same canonical function)
    int total_height{39};    // Total standing height (36 + round(h * 6))
    int leg_len{15};         // Leg length (13 + round(leg_ratio * 4))
    int torso_h{14};         // Torso height (max(11, total_height - 8 - 2 - leg_len))
    int arm_len{16};         // Arm length (torso_h + 2)

    bool operator==(const BodyGeometry& other) const = default;
};

// Compute complete body geometry (all 11 values) from anatomy parameters and optional overrides.
// Implements exact canonical formulas from generator.cpp render().
inline BodyGeometry compute_full_geometry(
    float height,
    float mass,
    float musc,
    float leg_ratio,
    const GeometryOverride& ovr = {}
) {
    BodyGeometry geo;

    // 1. Seven widths / thicknesses
    geo.neck_w = (musc > 0.6f || mass > 0.7f) ? 3 : 2;
    geo.neck_w = std::clamp(geo.neck_w + ovr.neck_w, GeometryBounds::NECK_W_MIN, GeometryBounds::NECK_W_MAX);

    int raw_shoulder = 9 + static_cast<int>(std::round(musc * 4.0f + mass * 1.5f));
    geo.shoulder_w = std::clamp(raw_shoulder + ovr.shoulder_w, GeometryBounds::SHOULDER_W_MIN, GeometryBounds::SHOULDER_W_MAX);

    int raw_waist = 6 + static_cast<int>(std::round(mass * 4.5f + (1.0f - musc) * 1.0f));
    if (musc > 0.5f && mass < 0.6f) {
        raw_waist = std::min(raw_waist, geo.shoulder_w - 4); // Distinct V-taper for Gym Jim
    }
    geo.waist_w = std::clamp(raw_waist + ovr.waist_w, GeometryBounds::WAIST_W_MIN, GeometryBounds::WAIST_W_MAX);

    int raw_hips = std::max(geo.waist_w, 7 + static_cast<int>(std::round(mass * 3.5f)));
    geo.hips_w = std::clamp(raw_hips + ovr.hips_w, GeometryBounds::HIPS_W_MIN, GeometryBounds::HIPS_W_MAX);

    geo.leg_gap = (musc > 0.6f || mass < 0.35f) ? 1 : 0;
    geo.leg_gap = std::clamp(geo.leg_gap + ovr.leg_gap, GeometryBounds::LEG_GAP_MIN, GeometryBounds::LEG_GAP_MAX);

    int raw_thigh = 3;
    if (mass > 0.7f && musc > 0.5f) {
        raw_thigh = 4;
    } else if (mass < 0.3f && musc < 0.3f) {
        raw_thigh = 2; // Slim Jim
    }
    geo.thigh_thick = std::clamp(raw_thigh + ovr.thigh_thick, GeometryBounds::THIGH_THICK_MIN, GeometryBounds::THIGH_THICK_MAX);

    int raw_arm = (musc > 0.65f || mass > 0.75f) ? 3 : 2;
    geo.arm_base_thick = std::clamp(raw_arm + ovr.arm_base_thick, GeometryBounds::ARM_BASE_THICK_MIN, GeometryBounds::ARM_BASE_THICK_MAX);

    // 2. Four vertical / length values (exact match with generator.cpp lines 117-127)
    int raw_total_h = 36 + static_cast<int>(std::round(height * 6.0f));
    geo.total_height = std::clamp(raw_total_h + ovr.total_height, GeometryBounds::TOTAL_HEIGHT_MIN, GeometryBounds::TOTAL_HEIGHT_MAX);

    int raw_leg_len = 13 + static_cast<int>(std::round(leg_ratio * 4.0f));
    geo.leg_len = std::clamp(raw_leg_len + ovr.leg_len, GeometryBounds::LEG_LEN_MIN, GeometryBounds::LEG_LEN_MAX);

    // Head is 8px, neck connection is 2px
    int raw_torso_h = geo.total_height - 8 - 2 - geo.leg_len;
    if (raw_torso_h < 11) raw_torso_h = 11;
    geo.torso_h = std::clamp(raw_torso_h + ovr.torso_h, GeometryBounds::TORSO_H_MIN, GeometryBounds::TORSO_H_MAX);

    int raw_arm_len = geo.torso_h + 2;
    geo.arm_len = std::clamp(raw_arm_len + ovr.arm_len, GeometryBounds::ARM_LEN_MIN, GeometryBounds::ARM_LEN_MAX);

    return geo;
}

// Compute base body geometry from anatomy mass and muscularity (legacy helper)
inline BodyGeometry compute_geometry(float mass, float musc) {
    return compute_full_geometry(0.5f, mass, musc, 0.5f);
}

} // namespace npc
