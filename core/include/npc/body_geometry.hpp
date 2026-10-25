#pragma once
#include "types.hpp"
#include "conditions.hpp"
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace npc {

// Forward declarations
struct AnatomyProfile;
struct AnimationCommand;

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

    static constexpr int CHEST_CURVE_MIN = 0;
    static constexpr int CHEST_CURVE_MAX = 4;
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
    int chest_curve{0};

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

    // Morphological contour / female anatomy support (v0.3.4 / FR-05)
    int chest_curve{0};      // 0: Flat/Standard, 1..4: Pronounced chest volume/contour

    bool operator==(const BodyGeometry& other) const = default;
};

// Compute complete body geometry (all 11 values) from anatomy parameters and optional overrides.
// Implements exact canonical formulas from generator.cpp render().
inline BodyGeometry compute_full_geometry(
    float height,
    float mass,
    float musc,
    float leg_ratio,
    float torso_ratio = 0.5f,
    float arm_ratio = 0.5f,
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

    // Leg length is influenced by leg_ratio and inversely by torso_ratio (torso_ratio=0.5f gives 0 offset)
    int torso_adj = static_cast<int>(std::round((0.5f - torso_ratio) * 2.0f));
    int raw_leg_len = 13 + static_cast<int>(std::round(leg_ratio * 4.0f)) + torso_adj;
    geo.leg_len = std::clamp(raw_leg_len + ovr.leg_len, GeometryBounds::LEG_LEN_MIN, GeometryBounds::LEG_LEN_MAX);

    // Head is 8px, neck connection is 2px
    int raw_torso_h = geo.total_height - 8 - 2 - geo.leg_len;
    if (raw_torso_h < 11) raw_torso_h = 11;
    geo.torso_h = std::clamp(raw_torso_h + ovr.torso_h, GeometryBounds::TORSO_H_MIN, GeometryBounds::TORSO_H_MAX);

    // Arm length is torso_h + 2 by default, modulated by arm_ratio (arm_ratio=0.5f gives 0 offset)
    int arm_adj = static_cast<int>(std::round((arm_ratio - 0.5f) * 2.0f));
    int raw_arm_len = geo.torso_h + 2 + arm_adj;
    geo.arm_len = std::clamp(raw_arm_len + ovr.arm_len, GeometryBounds::ARM_LEN_MIN, GeometryBounds::ARM_LEN_MAX);

    // 3. Chest curve contour
    geo.chest_curve = std::clamp(ovr.chest_curve, GeometryBounds::CHEST_CURVE_MIN, GeometryBounds::CHEST_CURVE_MAX);

    return geo;
}

// Compute base body geometry from anatomy mass and muscularity (legacy helper)
inline BodyGeometry compute_geometry(float mass, float musc) {
    return compute_full_geometry(0.5f, mass, musc, 0.5f);
}

// -------------------------------------------------------------
// Clothing & Equipment Foundation Types (v0.3.3.1)
// -------------------------------------------------------------

// Rectangle in pixel space
struct PixelRect {
    int x{0};
    int y{0};
    int width{0};
    int height{0};

    bool operator==(const PixelRect& other) const = default;
};

// 2D anchor point
struct AnchorPoint {
    int x{0};
    int y{0};

    bool operator==(const AnchorPoint& other) const = default;
};

// Explicit rendering layer order (z-index) for modular layering
enum class LayerOrder : uint8_t {
    Background = 0,
    Undergarment,
    BaseBody,
    InnerClothing, // Shirt, pants
    OuterClothing, // Coat, vest
    Armor,
    Accessories,
    PhysicalConditions, // Bandages, blood, scars
    Count
};

// Placement of a single body part in a posed frame
struct BodyPartPlacement {
    PixelRect bounds;
    AnchorPoint origin;
    AnchorPoint attach_point;
    bool is_present{true}; // False if amputated (including cascade)
    LayerOrder base_layer{LayerOrder::BaseBody};

    bool operator==(const BodyPartPlacement& other) const = default;
};

// Complete posed placement for all 10 canonical body parts for one animation frame
struct PosedPlacement {
    // Array indexed by BodyPart enum (0..9)
    BodyPartPlacement parts[10];

    const BodyPartPlacement& get(BodyPart part) const {
        return parts[static_cast<size_t>(part)];
    }

    BodyPartPlacement& get(BodyPart part) {
        return parts[static_cast<size_t>(part)];
    }
};

// Compute posed placement matching exactly the coordinates used by generator.cpp render().
// Pure foundation helper, does not alter render() execution path.
inline PosedPlacement compute_posed_placement(
    const BodyGeometry& geo,
    float posture,
    const PhysicalConditionSet& conditions,
    const AnimationCommand& cmd
) {
    PosedPlacement placement;

    int total_height = geo.total_height;
    int head_w = 7;
    int head_h = 8;
    int leg_len = geo.leg_len;
    int torso_h = geo.torso_h;
    int arm_len = geo.arm_len;

    float t = cmd.anim_time * cmd.speed;
    int leg_l_dy = 0, leg_r_dy = 0;
    int arm_l_dy = 0, arm_r_dy = 0;
    int bob_y = 0;

    switch (cmd.state) {
        case AnimationState::Idle:
            bob_y = static_cast<int>(std::sin(t * 3.0f) * 0.8f);
            break;
        case AnimationState::Walk: {
            float cycle = std::sin(t * 8.0f);
            leg_l_dy = static_cast<int>(cycle * 2.5f);
            leg_r_dy = -leg_l_dy;
            arm_l_dy = -leg_l_dy;
            arm_r_dy = leg_l_dy;
            bob_y = static_cast<int>(std::abs(cycle) * 1.0f);
            break;
        }
        case AnimationState::Run: {
            float cycle = std::sin(t * 14.0f);
            leg_l_dy = static_cast<int>(cycle * 4.0f);
            leg_r_dy = -leg_l_dy;
            arm_l_dy = -leg_l_dy;
            arm_r_dy = leg_l_dy;
            bob_y = static_cast<int>(std::abs(cycle) * 2.0f);
            break;
        }
        case AnimationState::Attack: {
            float cycle = std::sin(t * 10.0f);
            arm_r_dy = -static_cast<int>(std::abs(cycle) * 6.0f);
            break;
        }
        case AnimationState::Fall: {
            total_height = 14;
            head_h = 6;
            torso_h = 5;
            leg_len = 3;
            break;
        }
        case AnimationState::Swim: {
            bob_y = static_cast<int>(std::sin(t * 5.0f) * 1.5f);
            leg_l_dy = static_cast<int>(std::sin(t * 6.0f) * 2.0f);
            leg_r_dy = -leg_l_dy;
            arm_l_dy = static_cast<int>(std::cos(t * 6.0f) * 3.0f);
            arm_r_dy = -arm_l_dy;
            break;
        }
    }

    int cx = 32 / 2;
    int ground_y = 48 - 3;
    int base_y = ground_y - total_height - bob_y;

    float post = posture;
    int dir_sign = (cmd.direction == Direction::East) ? 1 : ((cmd.direction == Direction::West) ? -1 : 0);
    int head_lean_x = static_cast<int>(std::round(post * 3.0f)) * dir_sign;
    int torso_lean_x = static_cast<int>(std::round(post * 1.5f)) * dir_sign;
    int head_drop_y = (post > 0.2f) ? static_cast<int>(std::round(post * 1.5f)) : 0;

    int head_x = cx - head_w / 2 + head_lean_x;
    int head_y = base_y + head_drop_y;
    int neck_y = head_y + head_h - 1;
    int torso_y = neck_y + 1;
    int hips_y = torso_y + torso_h;

    int shoulder_w = geo.shoulder_w;
    int arm_base_thick = geo.arm_base_thick;
    int arm_y = torso_y + 1;
    int arm_l_x = cx - shoulder_w / 2 - arm_base_thick + torso_lean_x;
    int arm_r_x = cx + shoulder_w / 2 + torso_lean_x;

    int leg_gap = geo.leg_gap;
    int thigh_thick = geo.thigh_thick;
    int leg_l_center = cx - 2 - leg_gap;
    int leg_r_center = cx + 2 + leg_gap;

    // BodyPart::Head
    auto& p_head = placement.get(BodyPart::Head);
    p_head.bounds = {head_x, head_y, head_w, head_h};
    p_head.origin = {head_x, head_y};
    p_head.attach_point = {head_x + head_w / 2, neck_y};
    p_head.is_present = !conditions.is_amputated(BodyPart::Head);

    // BodyPart::Torso
    auto& p_torso = placement.get(BodyPart::Torso);
    p_torso.bounds = {cx - shoulder_w / 2, torso_y, shoulder_w, torso_h};
    p_torso.origin = {cx - shoulder_w / 2, torso_y};
    p_torso.attach_point = {cx, hips_y};
    p_torso.is_present = !conditions.is_amputated(BodyPart::Torso);

    // BodyPart::LeftArm
    auto& p_larm = placement.get(BodyPart::LeftArm);
    p_larm.bounds = {arm_l_x, arm_y + arm_l_dy, arm_base_thick, arm_len - 2};
    p_larm.origin = {arm_l_x, arm_y + arm_l_dy};
    p_larm.attach_point = {arm_l_x + arm_base_thick / 2, arm_y + arm_l_dy + arm_len - 2};
    p_larm.is_present = !conditions.is_amputated(BodyPart::LeftArm);

    // BodyPart::RightArm
    auto& p_rarm = placement.get(BodyPart::RightArm);
    p_rarm.bounds = {arm_r_x, arm_y + arm_r_dy, arm_base_thick, arm_len - 2};
    p_rarm.origin = {arm_r_x, arm_y + arm_r_dy};
    p_rarm.attach_point = {arm_r_x + arm_base_thick / 2, arm_y + arm_r_dy + arm_len - 2};
    p_rarm.is_present = !conditions.is_amputated(BodyPart::RightArm);

    // BodyPart::LeftHand
    auto& p_lhand = placement.get(BodyPart::LeftHand);
    p_lhand.bounds = {arm_l_x, arm_y + arm_l_dy + arm_len - 2, arm_base_thick + 1, 2};
    p_lhand.origin = {arm_l_x, arm_y + arm_l_dy + arm_len - 2};
    p_lhand.attach_point = {arm_l_x, arm_y + arm_l_dy + arm_len - 2};
    p_lhand.is_present = !conditions.is_amputated(BodyPart::LeftHand) && !conditions.is_amputated(BodyPart::LeftArm);

    // BodyPart::RightHand
    auto& p_rhand = placement.get(BodyPart::RightHand);
    p_rhand.bounds = {arm_r_x, arm_y + arm_r_dy + arm_len - 2, arm_base_thick + 1, 2};
    p_rhand.origin = {arm_r_x, arm_y + arm_r_dy + arm_len - 2};
    p_rhand.attach_point = {arm_r_x, arm_y + arm_r_dy + arm_len - 2};
    p_rhand.is_present = !conditions.is_amputated(BodyPart::RightHand) && !conditions.is_amputated(BodyPart::RightArm);

    // BodyPart::LeftLeg
    auto& p_lleg = placement.get(BodyPart::LeftLeg);
    p_lleg.bounds = {leg_l_center - thigh_thick / 2, hips_y + leg_l_dy, thigh_thick, leg_len - 1};
    p_lleg.origin = {leg_l_center - thigh_thick / 2, hips_y + leg_l_dy};
    p_lleg.attach_point = {leg_l_center, hips_y + leg_len + leg_l_dy - 1};
    p_lleg.is_present = !conditions.is_amputated(BodyPart::LeftLeg);

    // BodyPart::RightLeg
    auto& p_rleg = placement.get(BodyPart::RightLeg);
    p_rleg.bounds = {leg_r_center - thigh_thick / 2, hips_y + leg_r_dy, thigh_thick, leg_len - 1};
    p_rleg.origin = {leg_r_center - thigh_thick / 2, hips_y + leg_r_dy};
    p_rleg.attach_point = {leg_r_center, hips_y + leg_len + leg_r_dy - 1};
    p_rleg.is_present = !conditions.is_amputated(BodyPart::RightLeg);

    // BodyPart::LeftFoot
    auto& p_lfoot = placement.get(BodyPart::LeftFoot);
    int lfoot_x = leg_l_center - thigh_thick / 2 - (cmd.direction == Direction::West ? 2 : 1);
    p_lfoot.bounds = {lfoot_x, hips_y + leg_len + leg_l_dy - 1, thigh_thick + 2, 1};
    p_lfoot.origin = {lfoot_x, hips_y + leg_len + leg_l_dy - 1};
    p_lfoot.attach_point = {leg_l_center, hips_y + leg_len + leg_l_dy - 1};
    p_lfoot.is_present = !conditions.is_amputated(BodyPart::LeftFoot) && !conditions.is_amputated(BodyPart::LeftLeg);

    // BodyPart::RightFoot
    auto& p_rfoot = placement.get(BodyPart::RightFoot);
    int rfoot_x = leg_r_center - thigh_thick / 2;
    p_rfoot.bounds = {rfoot_x, hips_y + leg_len + leg_r_dy - 1, thigh_thick + 2, 1};
    p_rfoot.origin = {rfoot_x, hips_y + leg_len + leg_r_dy - 1};
    p_rfoot.attach_point = {leg_r_center, hips_y + leg_len + leg_r_dy - 1};
    p_rfoot.is_present = !conditions.is_amputated(BodyPart::RightFoot) && !conditions.is_amputated(BodyPart::RightLeg);

    return placement;
}

} // namespace npc
