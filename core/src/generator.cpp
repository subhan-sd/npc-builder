#include "npc/generator.hpp"
#include <cmath>
#include <numbers>

namespace npc {

HumanAppearance CharacterGenerator::generate_from_seed(uint64_t seed, uint32_t npc_id) {
    DeterministicRNG rng(seed);
    HumanAppearance app;
    app.npc_id = npc_id;
    app.appearance_seed = seed;
    app.generator_version = 2; // Upgraded to v0.2 anatomical model

    // Body
    app.body.height = rng.range_float(0.2f, 0.8f);
    app.body.body_mass = rng.range_float(0.1f, 0.9f);
    app.body.muscularity = rng.range_float(0.1f, 0.9f);
    app.body.posture = (rng.next_float() < 0.2f) ? rng.range_float(0.4f, 0.9f) : 0.0f;
    app.body.head_scale = rng.range_float(0.45f, 0.55f);
    app.body.torso_ratio = rng.range_float(0.45f, 0.55f);
    app.body.arm_ratio = rng.range_float(0.45f, 0.55f);
    app.body.leg_ratio = rng.range_float(0.45f, 0.55f);
    app.body.clamp();

    // Palettes calibrated from Reference 1 (warm 5-step tone ramp)
    static const ColorRGBA skin_tones[] = {
        {217, 160, 120, 255}, // Base Trim/Gym Jim tone
        {240, 196, 154, 255}, // Pale tone
        {180, 125, 90, 255},  // Tanned tone
        {140, 90, 60, 255},   // Dark tone
        {100, 65, 45, 255},   // Deep bronze
        {235, 175, 140, 255}  // Warm peach
    };
    static const ColorRGBA hair_colors[] = {
        {73, 53, 47, 255},    // Dark brunette
        {119, 81, 59, 255},   // Chestnut brown
        {168, 121, 82, 255},  // Golden brown
        {200, 160, 95, 255},  // Blonde
        {145, 60, 40, 255},   // Auburn
        {160, 155, 150, 255}  // Silver grey
    };
    static const ColorRGBA eye_colors[] = {
        {50, 35, 30, 255}, {45, 70, 95, 255}, {55, 80, 50, 255}, {75, 50, 30, 255}
    };
    static const ColorRGBA shirt_colors[] = {
        {73, 106, 155, 255}, // Muted slate blue
        {140, 65, 60, 255},  // Brick red
        {65, 105, 75, 255},  // Forest green
        {145, 120, 65, 255}, // Ochre
        {95, 80, 110, 255},  // Slate purple
        {170, 165, 155, 255} // Linen grey
    };
    static const ColorRGBA pants_colors[] = {
        {55, 60, 75, 255},   // Denim dark
        {75, 60, 50, 255},   // Brown leather
        {50, 55, 50, 255},   // Charcoal olive
        {80, 80, 85, 255}    // Slate grey
    };

    app.face.skin_color = skin_tones[rng.range_int(0, 5)];
    app.face.hair_color = hair_colors[rng.range_int(0, 5)];
    app.face.eye_color = eye_colors[rng.range_int(0, 3)];
    app.face.shirt_color = shirt_colors[rng.range_int(0, 5)];
    app.face.pants_color = pants_colors[rng.range_int(0, 3)];

    app.face.hair_style = static_cast<uint8_t>(rng.range_int(0, 3));
    app.face.eye_shape = static_cast<uint8_t>(rng.range_int(0, 2));
    app.face.eye_spacing = static_cast<uint8_t>(rng.range_int(1, 2));
    app.face.nose_shape = static_cast<uint8_t>(rng.range_int(0, 1));
    app.face.mouth_shape = static_cast<uint8_t>(rng.range_int(0, 1));
    app.face.expression = Expression::Neutral;

    return app;
}

// 5-Color Ramp derived from Base Tone (matching Reference 1 palette discipline)
struct ColorRamp {
    ColorRGBA highlight;
    ColorRGBA base;
    ColorRGBA shadow;
    ColorRGBA deep;
    ColorRGBA outline;
};

static ColorRamp make_ramp(ColorRGBA base, ColorRGBA outline_tint = {73, 53, 47, 255}) {
    ColorRamp r;
    r.base = base;
    r.highlight = {
        static_cast<uint8_t>(std::min(255.0f, base.r * 1.18f + 20.0f)),
        static_cast<uint8_t>(std::min(255.0f, base.g * 1.18f + 20.0f)),
        static_cast<uint8_t>(std::min(255.0f, base.b * 1.18f + 20.0f)),
        base.a
    };
    r.shadow = {
        static_cast<uint8_t>(base.r * 0.76f),
        static_cast<uint8_t>(base.g * 0.68f),
        static_cast<uint8_t>(base.b * 0.66f),
        base.a
    };
    r.deep = {
        static_cast<uint8_t>(base.r * 0.55f),
        static_cast<uint8_t>(base.g * 0.45f),
        static_cast<uint8_t>(base.b * 0.42f),
        base.a
    };
    r.outline = outline_tint;
    return r;
}

PixelBuffer CharacterGenerator::render(
    const HumanAppearance& app,
    const PhysicalConditionSet& cond,
    const AnimationCommand& cmd
) {
    PixelBuffer buf(CANVAS_WIDTH, CANVAS_HEIGHT, {0, 0, 0, 0});

    // 1. ANATOMICAL PARAMETERS & PROFILES (Ref 1: Trim, Gym, Slim, Chin Jim)
    float h = app.body.height;
    float mass = app.body.body_mass;
    float musc = app.body.muscularity;
    float post = app.body.posture;

    // Canvas vertical dimensions (baseline ~38px height)
    int total_height = 34 + static_cast<int>(std::round(h * 8.0f));

    // Torso & Shoulder widths
    // Muscular (Gym Jim): Broad shoulder delta, tapered waist
    // Heavy (Chin Jim): Broad waist & hips, rounded abdomen
    // Slim (Slim Jim): Narrow shoulders, narrow waist
    int shoulder_w = 8 + static_cast<int>(std::round(musc * 3.0f + mass * 1.5f));
    int waist_w = 6 + static_cast<int>(std::round(mass * 4.0f + (1.0f - musc) * 1.0f));
    if (musc > 0.6f && mass < 0.6f) {
        waist_w = std::min(waist_w, shoulder_w - 2); // V-taper
    }

    int head_w = 7;
    int head_h = 8;
    int neck_w = (musc > 0.6f || mass > 0.7f) ? 3 : 2;

    int leg_len = 10 + static_cast<int>(std::round(app.body.leg_ratio * 4.0f));
    int torso_h = total_height - head_h - 2 - leg_len;
    if (torso_h < 9) torso_h = 9;
    int arm_len = torso_h + 1;

    // Kinematics & Animation offsets
    float t = cmd.anim_time * cmd.speed;
    int leg_l_dy = 0, leg_r_dy = 0;
    int arm_l_dy = 0, arm_r_dy = 0;
    int bob_y = 0;
    int hunched_offset_x = (post > 0.3f) ? 1 : 0;
    if (cmd.direction == Direction::West) hunched_offset_x = -hunched_offset_x;
    else if (cmd.direction == Direction::North || cmd.direction == Direction::South) hunched_offset_x = 0;

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

    int cx = CANVAS_WIDTH / 2;
    int ground_y = CANVAS_HEIGHT - 3;
    int base_y = ground_y - total_height - bob_y;

    // Head position
    int head_x = cx - head_w / 2 + hunched_offset_x;
    int head_y = base_y + ((post > 0.3f) ? 1 : 0);

    // Neck & Torso positions
    int neck_y = head_y + head_h - 1;
    int torso_y = neck_y + 1;
    int hips_y = torso_y + torso_h;

    // Palette ramps
    ColorRamp skin = make_ramp(app.face.skin_color);
    ColorRamp shirt = make_ramp(app.face.shirt_color);
    ColorRamp pants = make_ramp(app.face.pants_color);
    ColorRamp hair = make_ramp(app.face.hair_color);

    // 2. LEGS & FEET RENDERING (with calf & thigh anatomical shaping)
    int leg_thick = (mass > 0.6f || musc > 0.7f) ? 3 : 2;
    int leg_l_x = cx - waist_w / 2;
    int leg_r_x = cx + waist_w / 2 - leg_thick;

    // Left Leg
    if (!cond.is_amputated(BodyPart::LeftLeg)) {
        int cur_y = hips_y;
        int cur_h = leg_len + leg_l_dy;
        for (int y = 0; y < cur_h; ++y) {
            int py = cur_y + y;
            // Knee taper and calf bulge
            int lx = leg_l_x;
            int lw = leg_thick;
            if (y > cur_h / 2 && y < cur_h - 2 && (musc > 0.5f || mass > 0.5f)) {
                lw = leg_thick + 1; // Calf curve
            }
            buf.draw_span(py, lx, lx + lw - 1, pants.base, pants.outline, pants.shadow);
        }
        if (!cond.is_amputated(BodyPart::LeftFoot)) {
            // Ankle & Shoe base
            int fy = cur_y + cur_h - 1;
            int fx = leg_l_x + (cmd.direction == Direction::West ? -1 : 0);
            buf.draw_span(fy, fx, fx + leg_thick, pants.deep, pants.outline, pants.outline);
        }
    }

    // Right Leg
    if (!cond.is_amputated(BodyPart::RightLeg)) {
        int cur_y = hips_y;
        int cur_h = leg_len + leg_r_dy;
        for (int y = 0; y < cur_h; ++y) {
            int py = cur_y + y;
            int rx = leg_r_x;
            int rw = leg_thick;
            if (y > cur_h / 2 && y < cur_h - 2 && (musc > 0.5f || mass > 0.5f)) {
                rw = leg_thick + 1; // Calf curve
            }
            buf.draw_span(py, rx, rx + rw - 1, pants.base, pants.outline, pants.shadow);
        }
        if (!cond.is_amputated(BodyPart::RightFoot)) {
            int fy = cur_y + cur_h - 1;
            int fx = leg_r_x + (cmd.direction == Direction::East ? 1 : 0);
            buf.draw_span(fy, fx, fx + leg_thick, pants.deep, pants.outline, pants.outline);
        }
    }

    // 3. TORSO RENDERING (Anatomical taper from shoulders down to waist & hips)
    if (!cond.is_amputated(BodyPart::Torso)) {
        for (int y = 0; y < torso_h; ++y) {
            int py = torso_y + y;
            float progress = static_cast<float>(y) / static_cast<float>(torso_h);
            // Linear interpolate shoulder to waist
            int cur_w = static_cast<int>(shoulder_w * (1.0f - progress) + waist_w * progress);
            int tx = cx - cur_w / 2;

            // Light from upper-left (highlight on left, shadow on right)
            ColorRGBA mid_fill = (progress < 0.6f && y > 1) ? shirt.highlight : shirt.base;
            buf.draw_span(py, tx, tx + cur_w - 1, mid_fill, shirt.outline, shirt.shadow);
        }
        // Belt line / transition to pants
        int belt_w = waist_w;
        buf.draw_span(hips_y - 1, cx - belt_w / 2, cx - belt_w / 2 + belt_w - 1, pants.deep, pants.outline, pants.outline);
    }

    // 4. NECK
    buf.draw_span(neck_y, cx - neck_w / 2, cx - neck_w / 2 + neck_w - 1, skin.shadow, skin.outline, skin.shadow);

    // 5. ARMS & HANDS (with Deltoid & Forearm contour)
    int arm_thick = (musc > 0.6f || mass > 0.7f) ? 3 : 2;
    int arm_l_x = cx - shoulder_w / 2 - arm_thick;
    int arm_r_x = cx + shoulder_w / 2;
    int arm_y = torso_y;

    // Left Arm
    if (!cond.is_amputated(BodyPart::LeftArm)) {
        int ay = arm_y + arm_l_dy;
        for (int y = 0; y < arm_len - 2; ++y) {
            int py = ay + y;
            ColorRGBA fill = (y < 2) ? shirt.base : skin.base; // Short sleeve transition
            buf.draw_span(py, arm_l_x, arm_l_x + arm_thick - 1, fill, skin.outline, skin.shadow);
        }
        if (!cond.is_amputated(BodyPart::LeftHand)) {
            // Hand bud with palm shadow
            int hy = ay + arm_len - 2;
            buf.draw_span(hy, arm_l_x, arm_l_x + arm_thick - 1, skin.base, skin.outline, skin.shadow);
            buf.set_pixel(arm_l_x + arm_thick / 2, hy + 1, skin.deep);
        }
    }

    // Right Arm
    if (!cond.is_amputated(BodyPart::RightArm)) {
        int ay = arm_y + arm_r_dy;
        for (int y = 0; y < arm_len - 2; ++y) {
            int py = ay + y;
            ColorRGBA fill = (y < 2) ? shirt.base : skin.base;
            buf.draw_span(py, arm_r_x, arm_r_x + arm_thick - 1, fill, skin.outline, skin.shadow);
        }
        if (!cond.is_amputated(BodyPart::RightHand)) {
            int hy = ay + arm_len - 2;
            buf.draw_span(hy, arm_r_x, arm_r_x + arm_thick - 1, skin.base, skin.outline, skin.shadow);
            buf.set_pixel(arm_r_x + arm_thick / 2, hy + 1, skin.deep);
        }
    }

    // 6. HEAD & FACIAL FEATURES (Contoured skull, chin, ears, eyes, nose, mouth)
    if (!cond.is_amputated(BodyPart::Head)) {
        // Skull contour (oval rounded crown and tapered jaw)
        // Row 0: crown (narrow)
        buf.draw_span(head_y, head_x + 1, head_x + head_w - 2, skin.base, skin.outline, skin.shadow);
        // Row 1-5: cranium & temples
        for (int y = 1; y <= 5; ++y) {
            buf.draw_span(head_y + y, head_x, head_x + head_w - 1, (y <= 2 ? skin.highlight : skin.base), skin.outline, skin.shadow);
        }
        // Row 6-7: jaw and chin (taper inward by 1px)
        buf.draw_span(head_y + 6, head_x + 1, head_x + head_w - 2, skin.base, skin.outline, skin.shadow);
        buf.draw_span(head_y + 7, head_x + 2, head_x + head_w - 3, skin.shadow, skin.outline, skin.deep);

        // HAIR SILHOUETTE (Styles: 0=Bald, 1=Trim/Short, 2=Medium, 3=Long)
        if (app.face.hair_style > 0) {
            // Crown & Bangs
            buf.draw_span(head_y - 1, head_x + 1, head_x + head_w - 2, hair.highlight, hair.outline, hair.outline);
            buf.draw_span(head_y, head_x, head_x + head_w - 1, hair.base, hair.outline, hair.shadow);
            buf.set_pixel(head_x + 1, head_y + 1, hair.base);
            buf.set_pixel(head_x + head_w - 2, head_y + 1, hair.shadow);

            if (app.face.hair_style >= 2) { // Medium sideburns
                buf.set_pixel(head_x, head_y + 2, hair.outline);
                buf.set_pixel(head_x + head_w - 1, head_y + 2, hair.shadow);
            }
            if (app.face.hair_style == 3) { // Long hair locks behind shoulders
                buf.draw_span(head_y + 5, head_x - 1, head_x, hair.shadow, hair.outline, hair.deep);
                buf.draw_span(head_y + 5, head_x + head_w - 1, head_x + head_w, hair.deep, hair.deep, hair.outline);
            }
            if (cmd.direction == Direction::North) { // Full hair back cover
                for (int y = 0; y <= 5; ++y) {
                    buf.draw_span(head_y + y, head_x, head_x + head_w - 1, hair.base, hair.outline, hair.shadow);
                }
            }
        }

        // DIRECTION AWARE FACIAL DETAILS
        if (cmd.direction != Direction::North) {
            int eye_y = head_y + 3;
            int eye_l_x = head_x + 1;
            int eye_r_x = head_x + head_w - 2;

            if (cmd.direction == Direction::South) {
                // Eye sockets & pupils
                buf.set_pixel(eye_l_x, eye_y, app.face.eye_color);
                buf.set_pixel(eye_r_x, eye_y, app.face.eye_color);
                // Nose shadow cluster
                buf.set_pixel(head_x + head_w / 2, eye_y + 1, skin.shadow);

                // Expressive mouth & chin
                int mouth_y = head_y + 5;
                int mouth_x = head_x + head_w / 2;
                ColorRGBA m_col = skin.deep;
                switch (app.face.expression) {
                    case Expression::Happy:
                        buf.set_pixel(mouth_x - 1, mouth_y - 1, m_col);
                        buf.set_pixel(mouth_x, mouth_y, m_col);
                        buf.set_pixel(mouth_x + 1, mouth_y - 1, m_col);
                        break;
                    case Expression::Sad:
                    case Expression::InPain:
                        buf.set_pixel(mouth_x - 1, mouth_y, m_col);
                        buf.set_pixel(mouth_x, mouth_y - 1, m_col);
                        buf.set_pixel(mouth_x + 1, mouth_y, m_col);
                        break;
                    case Expression::Angry:
                        buf.set_pixel(eye_l_x, eye_y - 1, hair.outline); // Brow furrow
                        buf.set_pixel(eye_r_x, eye_y - 1, hair.outline);
                        buf.set_pixel(mouth_x, mouth_y, m_col);
                        break;
                    default:
                        buf.set_pixel(mouth_x, mouth_y, m_col);
                        break;
                }
            } else if (cmd.direction == Direction::East) {
                // Profile facing Right: eye, nose contour protrusion
                buf.set_pixel(head_x + head_w - 2, eye_y, app.face.eye_color);
                buf.set_pixel(head_x + head_w, eye_y + 1, skin.shadow); // Nose bridge
                buf.set_pixel(head_x + 1, eye_y + 1, skin.shadow);      // Ear lobe
            } else if (cmd.direction == Direction::West) {
                // Profile facing Left
                buf.set_pixel(head_x + 1, eye_y, app.face.eye_color);
                buf.set_pixel(head_x - 1, eye_y + 1, skin.shadow);      // Nose bridge
                buf.set_pixel(head_x + head_w - 2, eye_y + 1, skin.shadow); // Ear lobe
            }
        }
    }

    // 7. PHYSICAL CONDITIONS OVERLAYS (Injuries, Wounds, Bandages, Scars)
    for (const auto& inj : cond.injuries) {
        if (!inj.active && !inj.healed) continue;

        int part_x = cx - shoulder_w / 2, part_y = torso_y, part_w = shoulder_w, part_h = torso_h;
        if (inj.body_part == BodyPart::Head) {
            part_x = head_x; part_y = head_y; part_w = head_w; part_h = head_h;
        } else if (inj.body_part == BodyPart::LeftArm) {
            part_x = arm_l_x; part_y = arm_y + arm_l_dy; part_w = arm_thick; part_h = arm_len;
        } else if (inj.body_part == BodyPart::RightArm) {
            part_x = arm_r_x; part_y = arm_y + arm_r_dy; part_w = arm_thick; part_h = arm_len;
        } else if (inj.body_part == BodyPart::LeftLeg) {
            part_x = leg_l_x; part_y = hips_y + leg_l_dy; part_w = leg_thick; part_h = leg_len;
        } else if (inj.body_part == BodyPart::RightLeg) {
            part_x = leg_r_x; part_y = hips_y + leg_r_dy; part_w = leg_thick; part_h = leg_len;
        }

        DeterministicRNG inj_rng(inj.visual_seed != 0 ? inj.visual_seed : static_cast<uint64_t>(inj.condition_id));
        int ox = part_x + inj_rng.range_int(0, std::max(0, part_w - 1));
        int oy = part_y + inj_rng.range_int(0, std::max(0, part_h - 1));

        ColorRGBA effect_col = {180, 25, 25, 240}; // Blood / Cut red
        if (inj.type == ConditionType::Bandage) effect_col = {235, 230, 220, 255}; // Off-white bandage
        else if (inj.type == ConditionType::Scar) effect_col = {150, 110, 90, 220}; // Scar tissue
        else if (inj.type == ConditionType::Bruise) effect_col = {80, 50, 95, 200}; // Purple bruise

        buf.set_pixel(ox, oy, effect_col);
        if (inj.severity > 0.4f) {
            buf.set_pixel(ox + 1, oy, effect_col);
        }
        if (inj.type == ConditionType::Bandage) {
            // Horizontal wrap across part span
            for (int bx = 0; bx < part_w; ++bx) {
                buf.set_pixel(part_x + bx, oy, effect_col);
            }
        }
    }

    return buf;
}

} // namespace npc
