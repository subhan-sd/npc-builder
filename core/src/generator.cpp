#include "npc/generator.hpp"
#include <cmath>
#include <numbers>

namespace npc {

HumanAppearance CharacterGenerator::generate_from_seed(uint64_t seed, uint32_t npc_id) {
    DeterministicRNG rng(seed);
    HumanAppearance app;
    app.npc_id = npc_id;
    app.appearance_seed = seed;
    app.generator_version = 1;

    // Body
    app.body.height = rng.range_float(0.1f, 0.9f);
    app.body.body_mass = rng.range_float(0.1f, 0.9f);
    app.body.muscularity = rng.range_float(0.1f, 0.9f);
    app.body.posture = (rng.next_float() < 0.2f) ? rng.range_float(0.4f, 0.9f) : 0.0f;
    app.body.head_scale = rng.range_float(0.4f, 0.6f);
    app.body.torso_ratio = rng.range_float(0.4f, 0.6f);
    app.body.arm_ratio = rng.range_float(0.4f, 0.6f);
    app.body.leg_ratio = rng.range_float(0.4f, 0.6f);
    app.body.clamp();

    // Palettes
    static const ColorRGBA skin_tones[] = {
        {245, 215, 190, 255}, {225, 185, 150, 255}, {195, 145, 105, 255},
        {160, 110, 75, 255},  {115, 75, 45, 255},   {75, 45, 25, 255}
    };
    static const ColorRGBA hair_colors[] = {
        {30, 25, 20, 255},    {70, 50, 35, 255},    {120, 80, 50, 255},
        {190, 140, 70, 255},  {160, 60, 40, 255},   {180, 180, 185, 255}
    };
    static const ColorRGBA eye_colors[] = {
        {40, 30, 20, 255}, {50, 75, 110, 255}, {60, 95, 50, 255}, {80, 55, 30, 255}
    };
    static const ColorRGBA shirt_colors[] = {
        {60, 90, 140, 255}, {130, 55, 50, 255}, {60, 110, 70, 255},
        {140, 120, 60, 255}, {80, 70, 90, 255}, {180, 175, 165, 255}
    };
    static const ColorRGBA pants_colors[] = {
        {45, 50, 60, 255}, {75, 60, 45, 255}, {40, 45, 40, 255}, {90, 90, 95, 255}
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

static ColorRGBA shade(ColorRGBA c, float factor) {
    return {
        static_cast<uint8_t>(std::clamp(c.r * factor, 0.0f, 255.0f)),
        static_cast<uint8_t>(std::clamp(c.g * factor, 0.0f, 255.0f)),
        static_cast<uint8_t>(std::clamp(c.b * factor, 0.0f, 255.0f)),
        c.a
    };
}

PixelBuffer CharacterGenerator::render(
    const HumanAppearance& app,
    const PhysicalConditionSet& cond,
    const AnimationCommand& cmd
) {
    PixelBuffer buf(CANVAS_WIDTH, CANVAS_HEIGHT, {0, 0, 0, 0});

    // Body dimensions computation
    // Height: base 32px to 42px
    int total_height = 32 + static_cast<int>(std::round(app.body.height * 10.0f));
    // Width modifiers
    int torso_width = 6 + static_cast<int>(std::round(app.body.body_mass * 4.0f + app.body.muscularity * 2.0f));
    int arm_thickness = (app.body.muscularity > 0.6f || app.body.body_mass > 0.7f) ? 3 : 2;
    int leg_thickness = (app.body.muscularity > 0.6f || app.body.body_mass > 0.7f) ? 3 : 2;

    int head_size = 7 + static_cast<int>(std::round(app.body.head_scale * 2.0f)); // 7 to 9
    int leg_len = 8 + static_cast<int>(std::round(app.body.leg_ratio * 4.0f));
    int torso_len = total_height - head_size - leg_len;
    if (torso_len < 8) torso_len = 8;
    int arm_len = torso_len + 1 + static_cast<int>(std::round(app.body.arm_ratio * 2.0f));

    // Animation evaluation
    float t = cmd.anim_time * cmd.speed;
    int leg_l_dy = 0, leg_r_dy = 0;
    int arm_l_dy = 0, arm_r_dy = 0;
    int bob_y = 0;
    int hunched_offset_x = (app.body.posture > 0.3f) ? 1 : 0;
    if (cmd.direction == Direction::West) hunched_offset_x = -hunched_offset_x;
    else if (cmd.direction == Direction::North || cmd.direction == Direction::South) hunched_offset_x = 0;

    int center_x = CANVAS_WIDTH / 2;
    int ground_y = CANVAS_HEIGHT - 3;
    int base_y = ground_y - total_height;

    // State specific kinematics
    switch (cmd.state) {
        case AnimationState::Idle: {
            bob_y = static_cast<int>(std::sin(t * 3.0f) * 0.8f);
            break;
        }
        case AnimationState::Walk: {
            float cycle = std::sin(t * 8.0f);
            leg_l_dy = static_cast<int>(cycle * 2.0f);
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
            arm_r_dy = -static_cast<int>(std::abs(cycle) * 5.0f);
            break;
        }
        case AnimationState::Fall: {
            // Prone / fallen pose
            base_y += total_height - 10;
            torso_len = 5;
            leg_len = 4;
            head_size = 6;
            break;
        }
        case AnimationState::Swim: {
            float cycle = std::sin(t * 5.0f);
            bob_y = static_cast<int>(cycle * 1.5f);
            leg_l_dy = static_cast<int>(std::sin(t * 6.0f) * 1.5f);
            leg_r_dy = -leg_l_dy;
            arm_l_dy = static_cast<int>(std::cos(t * 6.0f) * 2.0f);
            arm_r_dy = -arm_l_dy;
            break;
        }
    }

    base_y -= bob_y;

    // Hunched posture shifts head forward & down slightly
    int head_hunched_y = (app.body.posture > 0.3f) ? 1 : 0;
    int head_x = center_x - head_size / 2 + hunched_offset_x;
    int head_y = base_y + head_hunched_y;

    int torso_x = center_x - torso_width / 2;
    int torso_y = head_y + head_size - 1;

    int hips_y = torso_y + torso_len;
    int leg_l_x = torso_x + 1;
    int leg_r_x = torso_x + torso_width - leg_thickness - 1;

    int arm_l_x = torso_x - arm_thickness;
    int arm_r_x = torso_x + torso_width;
    int arm_y = torso_y + 1;

    // Palette shades
    ColorRGBA skin_dark = shade(app.face.skin_color, 0.85f);
    ColorRGBA shirt_dark = shade(app.face.shirt_color, 0.82f);
    ColorRGBA pants_dark = shade(app.face.pants_color, 0.82f);

    // 1. LEGS & FEET (Layer underneath or behind depending on direction)
    // Left Leg
    if (!cond.is_amputated(BodyPart::LeftLeg)) {
        int ly = hips_y;
        int lh = leg_len + leg_l_dy;
        buf.fill_rect(leg_l_x, ly, leg_thickness, lh, app.face.pants_color);
        if (!cond.is_amputated(BodyPart::LeftFoot)) {
            buf.fill_rect(leg_l_x, ly + lh - 1, leg_thickness + (cmd.direction == Direction::West ? -1 : 1), 1, pants_dark);
        }
    }
    // Right Leg
    if (!cond.is_amputated(BodyPart::RightLeg)) {
        int ry = hips_y;
        int rh = leg_len + leg_r_dy;
        buf.fill_rect(leg_r_x, ry, leg_thickness, rh, app.face.pants_color);
        if (!cond.is_amputated(BodyPart::RightFoot)) {
            buf.fill_rect(leg_r_x, ry + rh - 1, leg_thickness + (cmd.direction == Direction::East ? 1 : -1), 1, pants_dark);
        }
    }

    // 2. TORSO (Body)
    if (!cond.is_amputated(BodyPart::Torso)) {
        buf.fill_rect(torso_x, torso_y, torso_width, torso_len, app.face.shirt_color);
        // Belt line or shading
        buf.fill_rect(torso_x, hips_y - 2, torso_width, 2, shirt_dark);
    }

    // 3. ARMS
    // Left Arm
    if (!cond.is_amputated(BodyPart::LeftArm)) {
        int al_y = arm_y + arm_l_dy;
        buf.fill_rect(arm_l_x, al_y, arm_thickness, arm_len - 2, app.face.shirt_color);
        if (!cond.is_amputated(BodyPart::LeftHand)) {
            buf.fill_rect(arm_l_x, al_y + arm_len - 2, arm_thickness, 2, app.face.skin_color);
        }
    }
    // Right Arm
    if (!cond.is_amputated(BodyPart::RightArm)) {
        int ar_y = arm_y + arm_r_dy;
        buf.fill_rect(arm_r_x, ar_y, arm_thickness, arm_len - 2, app.face.shirt_color);
        if (!cond.is_amputated(BodyPart::RightHand)) {
            buf.fill_rect(arm_r_x, ar_y + arm_len - 2, arm_thickness, 2, app.face.skin_color);
        }
    }

    // 4. HEAD & FACE
    if (!cond.is_amputated(BodyPart::Head)) {
        buf.fill_rect(head_x, head_y, head_size, head_size, app.face.skin_color);

        // HAIR (styles: 0=Bald, 1=Short, 2=Medium, 3=Long)
        if (app.face.hair_style > 0) {
            // Hair top
            buf.fill_rect(head_x - 1, head_y - 1, head_size + 2, 2, app.face.hair_color);
            if (app.face.hair_style >= 2) { // Medium/Long sides
                buf.fill_rect(head_x - 1, head_y, 1, head_size - 2, app.face.hair_color);
                buf.fill_rect(head_x + head_size, head_y, 1, head_size - 2, app.face.hair_color);
            }
            if (app.face.hair_style == 3) { // Long back
                buf.fill_rect(head_x - 1, head_y + head_size - 2, head_size + 2, 2, app.face.hair_color);
            }
            if (cmd.direction == Direction::North) { // Back view shows full hair cover
                buf.fill_rect(head_x, head_y, head_size, head_size - 2, app.face.hair_color);
            }
        }

        // FACIAL FEATURES (South / East / West only; North is back of head)
        if (cmd.direction != Direction::North) {
            int eye_y = head_y + head_size / 2 - 1;
            int eye_l_x = head_x + 1;
            int eye_r_x = head_x + head_size - 2;

            if (cmd.direction == Direction::South) {
                // Eyes
                buf.set_pixel(eye_l_x, eye_y, app.face.eye_color);
                buf.set_pixel(eye_r_x, eye_y, app.face.eye_color);

                // Expression variations on mouth
                int mouth_y = head_y + head_size - 2;
                int mouth_x = head_x + head_size / 2 - 1;
                ColorRGBA mouth_col = skin_dark;

                switch (app.face.expression) {
                    case Expression::Happy:
                        buf.set_pixel(mouth_x - 1, mouth_y - 1, mouth_col);
                        buf.set_pixel(mouth_x, mouth_y, mouth_col);
                        buf.set_pixel(mouth_x + 1, mouth_y - 1, mouth_col);
                        break;
                    case Expression::Sad:
                    case Expression::InPain:
                        buf.set_pixel(mouth_x - 1, mouth_y, mouth_col);
                        buf.set_pixel(mouth_x, mouth_y - 1, mouth_col);
                        buf.set_pixel(mouth_x + 1, mouth_y, mouth_col);
                        break;
                    case Expression::Angry:
                        buf.set_pixel(eye_l_x, eye_y - 1, app.face.hair_color);
                        buf.set_pixel(eye_r_x, eye_y - 1, app.face.hair_color);
                        buf.set_pixel(mouth_x, mouth_y, mouth_col);
                        break;
                    default: // Neutral
                        buf.set_pixel(mouth_x, mouth_y, mouth_col);
                        break;
                }
            } else if (cmd.direction == Direction::East) {
                // Looking right
                buf.set_pixel(head_x + head_size - 2, eye_y, app.face.eye_color);
                buf.set_pixel(head_x + head_size - 1, eye_y + 1, skin_dark); // Nose profile
            } else if (cmd.direction == Direction::West) {
                // Looking left
                buf.set_pixel(head_x + 1, eye_y, app.face.eye_color);
                buf.set_pixel(head_x - 1, eye_y + 1, skin_dark); // Nose profile
            }
        }
    }

    // 5. PHYSICAL CONDITIONS (Injuries, Blood, Bandages, Scars)
    for (const auto& inj : cond.injuries) {
        if (!inj.active && !inj.healed) continue;

        int part_x = torso_x, part_y = torso_y, part_w = torso_width, part_h = torso_len;
        if (inj.body_part == BodyPart::Head) {
            part_x = head_x; part_y = head_y; part_w = head_size; part_h = head_size;
        } else if (inj.body_part == BodyPart::LeftArm) {
            part_x = arm_l_x; part_y = arm_y + arm_l_dy; part_w = arm_thickness; part_h = arm_len;
        } else if (inj.body_part == BodyPart::RightArm) {
            part_x = arm_r_x; part_y = arm_y + arm_r_dy; part_w = arm_thickness; part_h = arm_len;
        } else if (inj.body_part == BodyPart::LeftLeg) {
            part_x = leg_l_x; part_y = hips_y + leg_l_dy; part_w = leg_thickness; part_h = leg_len;
        } else if (inj.body_part == BodyPart::RightLeg) {
            part_x = leg_r_x; part_y = hips_y + leg_r_dy; part_w = leg_thickness; part_h = leg_len;
        }

        // Deterministic offset within part bounds
        DeterministicRNG inj_rng(inj.visual_seed != 0 ? inj.visual_seed : static_cast<uint64_t>(inj.condition_id));
        int ox = part_x + inj_rng.range_int(0, std::max(0, part_w - 1));
        int oy = part_y + inj_rng.range_int(0, std::max(0, part_h - 1));

        ColorRGBA effect_col = {180, 20, 20, 240}; // Blood / Cut red
        if (inj.type == ConditionType::Bandage) effect_col = {235, 230, 220, 255}; // Off-white bandage
        else if (inj.type == ConditionType::Scar) effect_col = {150, 110, 90, 220}; // Scar tissue
        else if (inj.type == ConditionType::Bruise) effect_col = {80, 50, 95, 200}; // Purple bruise

        buf.set_pixel(ox, oy, effect_col);
        if (inj.severity > 0.4f) {
            buf.set_pixel(ox + 1, oy, effect_col);
        }
        if (inj.type == ConditionType::Bandage) {
            // Bandage wraps horizontally across the limb
            for (int bx = 0; bx < part_w; ++bx) {
                buf.set_pixel(part_x + bx, oy, effect_col);
            }
        }
    }

    return buf;
}

} // namespace npc
