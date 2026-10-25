#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace npc {

// Forward declaration
struct AnatomyProfile;

// Core base body geometry calculated from anatomy parameters
struct BodyGeometry {
    int shoulder_w{11};     // Shoulder width in pixels
    int waist_w{8};          // Waist width in pixels
    int hips_w{8};           // Hips/pelvis width in pixels
    int thigh_thick{3};      // Thigh thickness in pixels
    int neck_w{2};           // Neck width in pixels
    int arm_base_thick{2};   // Base arm thickness in pixels
    int leg_gap{0};          // Gap between legs (0 or 1) in pixels

    bool operator==(const BodyGeometry& other) const = default;
};

// Compute base body geometry from anatomy parameters.
// Implements exact canonical formulas from generator.cpp.
inline BodyGeometry compute_geometry(float mass, float musc) {
    BodyGeometry geo;

    geo.neck_w = (musc > 0.6f || mass > 0.7f) ? 3 : 2;

    geo.shoulder_w = 9 + static_cast<int>(std::round(musc * 4.0f + mass * 1.5f));
    geo.waist_w = 6 + static_cast<int>(std::round(mass * 4.5f + (1.0f - musc) * 1.0f));
    if (musc > 0.5f && mass < 0.6f) {
        geo.waist_w = std::min(geo.waist_w, geo.shoulder_w - 4); // Distinct V-taper for Gym Jim
    }
    geo.hips_w = std::max(geo.waist_w, 7 + static_cast<int>(std::round(mass * 3.5f)));

    geo.leg_gap = (musc > 0.6f || mass < 0.35f) ? 1 : 0;
    geo.thigh_thick = 3;
    if (mass > 0.7f && musc > 0.5f) {
        geo.thigh_thick = 4;
    } else if (mass < 0.3f && musc < 0.3f) {
        geo.thigh_thick = 2; // Slim Jim
    }

    geo.arm_base_thick = (musc > 0.65f || mass > 0.75f) ? 3 : 2;

    return geo;
}

} // namespace npc
