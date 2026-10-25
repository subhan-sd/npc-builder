#include "npc/generator.hpp"
#include "npc/body_geometry.hpp"
#include <cmath>
#include <algorithm>

namespace npc {

HumanAppearance CharacterGenerator::generate_from_seed(uint64_t seed, uint32_t npc_id) {
    DeterministicRNG rng(seed);
    HumanAppearance app;
    app.npc_id = npc_id;
    app.appearance_seed = seed;
    app.generator_version = 2;

    app.body.height = rng.range_float(0.2f, 0.8f);
    app.body.body_mass = rng.range_float(0.1f, 0.9f);
    app.body.muscularity = rng.range_float(0.1f, 0.9f);
    app.body.posture = (rng.next_float() < 0.2f) ? rng.range_float(0.4f, 0.9f) : 0.0f;
    app.body.head_scale = rng.range_float(0.45f, 0.55f);
    app.body.torso_ratio = rng.range_float(0.45f, 0.55f);
    app.body.arm_ratio = rng.range_float(0.45f, 0.55f);
    app.body.leg_ratio = rng.range_float(0.45f, 0.55f);
    app.body.clamp();

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
        {73, 106, 155, 255}, {140, 65, 60, 255}, {65, 105, 75, 255},
        {145, 120, 65, 255}, {95, 80, 110, 255}, {170, 165, 155, 255}
    };
    static const ColorRGBA pants_colors[] = {
        {55, 60, 75, 255}, {75, 60, 50, 255}, {50, 55, 50, 255}, {80, 80, 85, 255}
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

struct ColorRamp {
    ColorRGBA highlight;
    ColorRGBA base;
    ColorRGBA shadow;
    ColorRGBA deep;
    ColorRGBA outline;
};

// 5-Color Ramp derived from Base Tone (Reference 1 Body Shape Studies)
static ColorRamp make_ramp(ColorRGBA base, ColorRGBA outline_tint = {73, 53, 47, 255}) {
    ColorRamp r;
    r.base = base;
    r.highlight = {
        static_cast<uint8_t>(std::min(255.0f, base.r * 1.14f + 16.0f)),
        static_cast<uint8_t>(std::min(255.0f, base.g * 1.14f + 16.0f)),
        static_cast<uint8_t>(std::min(255.0f, base.b * 1.14f + 16.0f)),
        base.a
    };
    r.shadow = {
        static_cast<uint8_t>(base.r * 0.78f),
        static_cast<uint8_t>(base.g * 0.70f),
        static_cast<uint8_t>(base.b * 0.67f),
        base.a
    };
    r.deep = {
        static_cast<uint8_t>(base.r * 0.58f),
        static_cast<uint8_t>(base.g * 0.48f),
        static_cast<uint8_t>(base.b * 0.44f),
        base.a
    };
    r.outline = outline_tint;
    return r;
}

namespace detail {

PixelBuffer render_internal(
    const HumanAppearance& app,
    const PhysicalConditionSet& cond,
    const AnimationCommand& cmd,
    const BodyGeometry* override_geo
) {
    PixelBuffer buf(CharacterGenerator::CANVAS_WIDTH, CharacterGenerator::CANVAS_HEIGHT, {0, 0, 0, 0});

    float h = app.body.height;
    float mass = app.body.body_mass;
    float musc = app.body.muscularity;
    float post = app.body.posture;

    BodyGeometry geo = override_geo ? *override_geo : compute_full_geometry(h, mass, musc, app.body.leg_ratio, app.body.torso_ratio, app.body.arm_ratio);

    int total_height = geo.total_height;

    // Ref 1 Proportions (Head: 8px, Torso: 11-13px, Legs: 14-16px)
    int head_w = 7;
    int head_h = 8;
    int neck_w = geo.neck_w;

    int leg_len = geo.leg_len;
    int torso_h = geo.torso_h;
    int arm_len = geo.arm_len;

    // Kinematics & Animation offsets
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

    int cx = CharacterGenerator::CANVAS_WIDTH / 2;
    int ground_y = CharacterGenerator::CANVAS_HEIGHT - 3;
    int base_y = ground_y - total_height - bob_y;

    // Posture multi-part derivation (forward tilt in side view, trapezius compression in front view)
    int dir_sign = (cmd.direction == Direction::East) ? 1 : ((cmd.direction == Direction::West) ? -1 : 0);
    int head_lean_x = static_cast<int>(std::round(post * 3.0f)) * dir_sign;
    int torso_lean_x = static_cast<int>(std::round(post * 1.5f)) * dir_sign;
    int head_drop_y = (post > 0.2f) ? static_cast<int>(std::round(post * 1.5f)) : 0;

    int head_x = cx - head_w / 2 + head_lean_x;
    int head_y = base_y + head_drop_y;
    int neck_y = head_y + head_h - 1;
    int torso_y = neck_y + 1;
    int hips_y = torso_y + torso_h;

    ColorRamp skin = make_ramp(app.face.skin_color);
    ColorRamp hair = make_ramp(app.face.hair_color);

    // Anatomical profile widths from BodyGeometry (Ref 1: Trim, Gym, Slim, Chin Jim)
    int shoulder_w = geo.shoulder_w;
    int waist_w = geo.waist_w;
    int hips_w = geo.hips_w;

    // 1. LEGS & FEET RENDERING (thigh, knee, lateral calf curve, ankle pinch, foot platform)
    int leg_gap = geo.leg_gap;
    int thigh_thick = geo.thigh_thick;

    int leg_l_center = cx - 2 - leg_gap;
    int leg_r_center = cx + 2 + leg_gap;

    // Left Leg
    if (!cond.is_amputated(BodyPart::LeftLeg)) {
        int cur_y = hips_y;
        int cur_h = leg_len + leg_l_dy;
        int knee_y = cur_h * 4 / 10;
        int calf_y = cur_h * 7 / 10;

        for (int y = 0; y < cur_h; ++y) {
            int py = cur_y + y;
            int lw = thigh_thick;
            int lx = leg_l_center - lw / 2;

            if (y < knee_y) {
                lw = thigh_thick;
            } else if (y == knee_y) {
                lw = std::max(2, thigh_thick - 1);
            } else if (y <= calf_y) {
                lw = thigh_thick + (musc > 0.4f || mass > 0.4f ? 1 : 0);
                lx -= 1; // Lateral swell
            } else {
                lw = 2; // Ankle pinch
            }

            buf.draw_span(py, lx, lx + lw - 1, skin.base, skin.outline, skin.shadow);
        }

        if (!cond.is_amputated(BodyPart::LeftFoot)) {
            int fy = cur_y + cur_h - 1;
            int fx = leg_l_center - thigh_thick / 2 - (cmd.direction == Direction::West ? 2 : 1);
            int fw = thigh_thick + 2;
            buf.draw_span(fy, fx, fx + fw - 1, skin.shadow, skin.outline, skin.deep);
        }
    }

    // Right Leg
    if (!cond.is_amputated(BodyPart::RightLeg)) {
        int cur_y = hips_y;
        int cur_h = leg_len + leg_r_dy;
        int knee_y = cur_h * 4 / 10;
        int calf_y = cur_h * 7 / 10;

        for (int y = 0; y < cur_h; ++y) {
            int py = cur_y + y;
            int rw = thigh_thick;
            int rx = leg_r_center - rw / 2;

            if (y < knee_y) {
                rw = thigh_thick;
            } else if (y == knee_y) {
                rw = std::max(2, thigh_thick - 1);
            } else if (y <= calf_y) {
                rw = thigh_thick + (musc > 0.4f || mass > 0.4f ? 1 : 0);
            } else {
                rw = 2;
            }

            buf.draw_span(py, rx, rx + rw - 1, skin.base, skin.outline, skin.shadow);
        }

        if (!cond.is_amputated(BodyPart::RightFoot)) {
            int fy = cur_y + cur_h - 1;
            int fx = leg_r_center - thigh_thick / 2;
            int fw = thigh_thick + 2;
            buf.draw_span(fy, fx, fx + fw - 1, skin.shadow, skin.outline, skin.deep);
        }
    }

    // 2. TORSO RENDERING (Organic sloped shoulder transitions and natural curves)
    if (!cond.is_amputated(BodyPart::Torso)) {
        for (int y = 0; y < torso_h; ++y) {
            int py = torso_y + y;
            float progress = static_cast<float>(y) / static_cast<float>(torso_h);

            int cur_w;
            if (y == 0) {
                // Row 0: Trapezius slope (narrower than full shoulder)
                cur_w = shoulder_w - 2;
            } else if (progress < 0.45f) {
                // Chest / Pectorals: reaches full shoulder at row 1, then gentle taper
                float chest_p = (progress - 0.1f) / 0.35f;
                cur_w = static_cast<int>(shoulder_w * (1.0f - std::max(0.0f, chest_p) * 0.15f));
            } else if (progress < 0.78f) {
                // Waist & Abdomen
                float waist_p = (progress - 0.45f) / 0.33f;
                int chest_bot = static_cast<int>(shoulder_w * 0.85f);
                cur_w = static_cast<int>(chest_bot * (1.0f - waist_p) + waist_w * waist_p);
                if (mass > 0.65f) cur_w += static_cast<int>((1.0f - std::abs(waist_p - 0.5f) * 2.0f) * 2.5f);
            } else {
                // Pelvis / Hips flare
                float hip_p = (progress - 0.78f) / 0.22f;
                cur_w = static_cast<int>(waist_w * (1.0f - hip_p) + hips_w * hip_p);
            }

            int spine_offset = static_cast<int>(std::round(torso_lean_x * (1.0f - progress)));
            int tx = cx - cur_w / 2 + spine_offset;

            ColorRGBA mid_fill = skin.base;
            if (cmd.direction == Direction::North) {
                mid_fill = (progress < 0.4f) ? skin.base : skin.shadow;
            } else if (cmd.direction == Direction::East || cmd.direction == Direction::West) {
                mid_fill = (post > 0.3f && progress < 0.55f) ? skin.shadow : skin.base;
            }

            buf.draw_span(py, tx, tx + cur_w - 1, mid_fill, skin.outline, skin.shadow);

            // Subtle organic markers
            if (cmd.direction == Direction::South) {
                // Pectoral / Chest underline:
                if (geo.chest_curve > 0) {
                    // Pronounced chest contour for female morphology
                    if (y == torso_h * 38 / 100 && cur_w >= 8) {
                        buf.set_pixel(cx - 2 + spine_offset, py, skin.shadow);
                        buf.set_pixel(cx - 1 + spine_offset, py, skin.highlight);
                        buf.set_pixel(cx + 1 + spine_offset, py, skin.highlight);
                        buf.set_pixel(cx + 2 + spine_offset, py, skin.shadow);
                    } else if (y == torso_h * 42 / 100 && cur_w >= 8 && geo.chest_curve >= 2) {
                        buf.set_pixel(cx - 2 + spine_offset, py, skin.shadow);
                        buf.set_pixel(cx + 2 + spine_offset, py, skin.shadow);
                    }
                } else if (y == torso_h * 38 / 100 && musc > 0.45f && cur_w >= 8) {
                    // 1 dot under each peck for muscular male
                    buf.set_pixel(cx - 2 + spine_offset, py, skin.shadow);
                    buf.set_pixel(cx + 2 + spine_offset, py, skin.shadow);
                }
                // Abdominal midline seam
                if (y >= torso_h * 48 / 100 && y <= torso_h * 72 / 100 && musc > 0.55f) {
                    buf.set_pixel(cx + spine_offset, py, skin.shadow);
                    if (y % 3 == 0 && cur_w >= 7) {
                        buf.set_pixel(cx - 1 + spine_offset, py, skin.highlight);
                    }
                }
                // Navel for Chin Jim
                if (mass > 0.65f && y == torso_h * 65 / 100) {
                    buf.set_pixel(cx + spine_offset, py, skin.deep);
                }
            } else if ((cmd.direction == Direction::East || cmd.direction == Direction::West) && geo.chest_curve > 0) {
                // Lateral chest contour in profile view
                if (y >= torso_h * 35 / 100 && y <= torso_h * 42 / 100) {
                    int front_x = (cmd.direction == Direction::East) ? (tx + cur_w - 1) : tx;
                    buf.set_pixel(front_x, py, skin.highlight);
                }
            }
        }
        buf.draw_span(hips_y - 1, cx - hips_w / 2, cx - hips_w / 2 + hips_w - 1, skin.shadow, skin.outline, skin.deep);
    }

    // 3. NECK & TRAPEZIUS
    int neck_offset = static_cast<int>(std::round(torso_lean_x * 0.9f));
    buf.draw_span(neck_y, cx - neck_w / 2 + neck_offset, cx - neck_w / 2 + neck_w - 1 + neck_offset, skin.shadow, skin.outline, skin.shadow);

    // 4. ARMS & HANDS (Deltoid attaches at row 1, seamlessly aligned with shoulder slope)
    int arm_base_thick = geo.arm_base_thick;
    int arm_y = torso_y + 1; // Starts at row 1 under the trapezius slope
    int arm_l_x = cx - shoulder_w / 2 - arm_base_thick + torso_lean_x;
    int arm_r_x = cx + shoulder_w / 2 + torso_lean_x;

    // Left Arm
    if (!cond.is_amputated(BodyPart::LeftArm)) {
        int ay = arm_y + arm_l_dy;
        for (int y = 0; y < arm_len - 2; ++y) {
            int py = ay + y;
            int cur_w = arm_base_thick;
            int ax = arm_l_x;

            if (y == 0 || y == 1) {
                // Deltoid bulb
                cur_w = arm_base_thick + (musc > 0.4f ? 1 : 0);
                ax -= (musc > 0.4f ? 1 : 0);
            } else if (y == arm_len / 2) {
                // Elbow
                cur_w = std::max(2, arm_base_thick - 1);
            } else if (y == arm_len / 2 + 1 && musc > 0.4f) {
                // Forearm swell
                cur_w = arm_base_thick;
            }

            ColorRGBA fill = skin.base;
            buf.draw_span(py, ax, ax + cur_w - 1, fill, skin.outline, skin.shadow);
        }
        if (!cond.is_amputated(BodyPart::LeftHand)) {
            int hy = ay + arm_len - 2;
            buf.draw_span(hy, arm_l_x, arm_l_x + arm_base_thick, skin.base, skin.outline, skin.shadow);
            buf.set_pixel(arm_l_x - (cmd.direction == Direction::West ? 1 : 0), hy, skin.shadow); // Thumb
            buf.set_pixel(arm_l_x + arm_base_thick / 2, hy + 1, skin.deep);
        }
    }

    // Right Arm
    if (!cond.is_amputated(BodyPart::RightArm)) {
        int ay = arm_y + arm_r_dy;
        for (int y = 0; y < arm_len - 2; ++y) {
            int py = ay + y;
            int cur_w = arm_base_thick;
            int ax = arm_r_x;

            if (y == 0 || y == 1) {
                cur_w = arm_base_thick + (musc > 0.4f ? 1 : 0);
            } else if (y == arm_len / 2) {
                cur_w = std::max(2, arm_base_thick - 1);
            } else if (y == arm_len / 2 + 1 && musc > 0.4f) {
                cur_w = arm_base_thick;
            }

            ColorRGBA fill = skin.base;
            buf.draw_span(py, ax, ax + cur_w - 1, fill, skin.outline, skin.shadow);
        }
        if (!cond.is_amputated(BodyPart::RightHand)) {
            int hy = ay + arm_len - 2;
            buf.draw_span(hy, arm_r_x, arm_r_x + arm_base_thick, skin.base, skin.outline, skin.shadow);
            buf.set_pixel(arm_r_x + arm_base_thick + (cmd.direction == Direction::East ? 0 : -1), hy, skin.shadow);
            buf.set_pixel(arm_r_x + arm_base_thick / 2, hy + 1, skin.deep);
        }
    }

    // 5. HEAD & FACIAL FEATURES
    if (!cond.is_amputated(BodyPart::Head)) {
        buf.draw_span(head_y, head_x + 1, head_x + head_w - 2, skin.base, skin.outline, skin.shadow);
        for (int y = 1; y <= 5; ++y) {
            buf.draw_span(head_y + y, head_x, head_x + head_w - 1, (y <= 2 ? skin.highlight : skin.base), skin.outline, skin.shadow);
        }
        buf.draw_span(head_y + 6, head_x + 1, head_x + head_w - 2, skin.base, skin.outline, skin.shadow);
        buf.draw_span(head_y + 7, head_x + 2, head_x + head_w - 3, skin.shadow, skin.outline, skin.deep);

        // HAIR SILHOUETTE
        if (app.face.hair_style > 0) {
            buf.draw_span(head_y - 1, head_x + 1, head_x + head_w - 2, hair.highlight, hair.outline, hair.outline);
            buf.draw_span(head_y, head_x, head_x + head_w - 1, hair.base, hair.outline, hair.shadow);
            buf.set_pixel(head_x + 1, head_y + 1, hair.base);
            buf.set_pixel(head_x + head_w - 2, head_y + 1, hair.shadow);

            if (app.face.hair_style >= 2) {
                buf.set_pixel(head_x, head_y + 2, hair.outline);
                buf.set_pixel(head_x + head_w - 1, head_y + 2, hair.shadow);
            }
            if (app.face.hair_style == 3) {
                buf.draw_span(head_y + 5, head_x - 1, head_x, hair.shadow, hair.outline, hair.deep);
                buf.draw_span(head_y + 5, head_x + head_w - 1, head_x + head_w, hair.deep, hair.deep, hair.outline);
            }
            if (cmd.direction == Direction::North) {
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
                buf.set_pixel(eye_l_x, eye_y, app.face.eye_color);
                buf.set_pixel(eye_r_x, eye_y, app.face.eye_color);
                buf.set_pixel(head_x + head_w / 2, eye_y + 1, skin.shadow);

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
                        buf.set_pixel(eye_l_x, eye_y - 1, hair.outline);
                        buf.set_pixel(eye_r_x, eye_y - 1, hair.outline);
                        buf.set_pixel(mouth_x, mouth_y, m_col);
                        break;
                    default:
                        buf.set_pixel(mouth_x, mouth_y, m_col);
                        break;
                }
            } else if (cmd.direction == Direction::East) {
                buf.set_pixel(head_x + head_w - 2, eye_y, app.face.eye_color);
                buf.set_pixel(head_x + head_w, eye_y + 1, skin.shadow);
                buf.set_pixel(head_x + 1, eye_y + 1, skin.shadow);
            } else if (cmd.direction == Direction::West) {
                buf.set_pixel(head_x + 1, eye_y, app.face.eye_color);
                buf.set_pixel(head_x - 1, eye_y + 1, skin.shadow);
                buf.set_pixel(head_x + head_w - 2, eye_y + 1, skin.shadow);
            }
        }
    }

    // 6. PHYSICAL CONDITIONS OVERLAYS
    for (const auto& inj : cond.injuries) {
        if (!inj.active && !inj.healed) continue;

        // Skip injury if target body part is amputated
        if (cond.is_amputated(inj.body_part)) continue;

        int part_x = cx - shoulder_w / 2, part_y = torso_y, part_w = shoulder_w, part_h = torso_h;
        if (inj.body_part == BodyPart::Head) {
            part_x = head_x; part_y = head_y; part_w = head_w; part_h = head_h;
        } else if (inj.body_part == BodyPart::LeftArm) {
            part_x = arm_l_x; part_y = arm_y + arm_l_dy; part_w = arm_base_thick; part_h = arm_len - 2;
        } else if (inj.body_part == BodyPart::RightArm) {
            part_x = arm_r_x; part_y = arm_y + arm_r_dy; part_w = arm_base_thick; part_h = arm_len - 2;
        } else if (inj.body_part == BodyPart::LeftHand) {
            part_x = arm_l_x; part_y = arm_y + arm_l_dy + arm_len - 2; part_w = arm_base_thick + 1; part_h = 2;
        } else if (inj.body_part == BodyPart::RightHand) {
            part_x = arm_r_x; part_y = arm_y + arm_r_dy + arm_len - 2; part_w = arm_base_thick + 1; part_h = 2;
        } else if (inj.body_part == BodyPart::LeftLeg) {
            part_x = leg_l_center - thigh_thick / 2; part_y = hips_y + leg_l_dy; part_w = thigh_thick; part_h = leg_len - 1;
        } else if (inj.body_part == BodyPart::RightLeg) {
            part_x = leg_r_center - thigh_thick / 2; part_y = hips_y + leg_r_dy; part_w = thigh_thick; part_h = leg_len - 1;
        } else if (inj.body_part == BodyPart::LeftFoot) {
            part_x = leg_l_center - thigh_thick / 2 - (cmd.direction == Direction::West ? 2 : 1);
            part_y = hips_y + leg_len + leg_l_dy - 1;
            part_w = thigh_thick + 2;
            part_h = 1;
        } else if (inj.body_part == BodyPart::RightFoot) {
            part_x = leg_r_center - thigh_thick / 2;
            part_y = hips_y + leg_len + leg_r_dy - 1;
            part_w = thigh_thick + 2;
            part_h = 1;
        }

        DeterministicRNG inj_rng(inj.visual_seed != 0 ? inj.visual_seed : static_cast<uint64_t>(inj.condition_id));
        int ox = part_x + inj_rng.range_int(0, std::max(0, part_w - 1));
        int oy = part_y + inj_rng.range_int(0, std::max(0, part_h - 1));

        ColorRGBA effect_col = {180, 25, 25, 240};
        if (inj.type == ConditionType::Bandage) effect_col = {235, 230, 220, 255};
        else if (inj.type == ConditionType::Scar) effect_col = {150, 110, 90, 220};
        else if (inj.type == ConditionType::Bruise) effect_col = {80, 50, 95, 200};

        buf.set_pixel(ox, oy, effect_col);
        if (inj.severity > 0.4f) {
            buf.set_pixel(ox + 1, oy, effect_col);
        }
        if (inj.type == ConditionType::Bandage) {
            for (int bx = 0; bx < part_w; ++bx) {
                buf.set_pixel(part_x + bx, oy, effect_col);
            }
        }
    }

    return buf;
}

} // namespace detail
} // namespace npc
