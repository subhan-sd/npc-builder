#include "npc/generator.hpp"
#include "npc/serializer.hpp"
#include <cstring>

#if defined(_WIN32)
#define NPC_EXPORT __declspec(dllexport)
#else
#define NPC_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {

struct CNpcAppearance {
    uint32_t npc_id;
    uint64_t seed;
    float height;
    float body_mass;
    float muscularity;
    float posture;
    float head_scale;
    float torso_ratio;
    float arm_ratio;
    float leg_ratio;
    uint8_t hair_style;
    uint8_t expression;
    uint8_t skin_r, skin_g, skin_b;
    uint8_t hair_r, hair_g, hair_b;
    uint8_t shirt_r, shirt_g, shirt_b;
    uint8_t pants_r, pants_g, pants_b;
};

NPC_EXPORT CNpcAppearance npc_generate_appearance(uint64_t seed, uint32_t npc_id) {
    auto app = npc::CharacterGenerator::generate_from_seed(seed, npc_id);
    CNpcAppearance c_app{};
    c_app.npc_id = app.npc_id;
    c_app.seed = app.appearance_seed;
    c_app.height = app.body.height;
    c_app.body_mass = app.body.body_mass;
    c_app.muscularity = app.body.muscularity;
    c_app.posture = app.body.posture;
    c_app.head_scale = app.body.head_scale;
    c_app.torso_ratio = app.body.torso_ratio;
    c_app.arm_ratio = app.body.arm_ratio;
    c_app.leg_ratio = app.body.leg_ratio;
    c_app.hair_style = app.face.hair_style;
    c_app.expression = static_cast<uint8_t>(app.face.expression);

    c_app.skin_r = app.face.skin_color.r;
    c_app.skin_g = app.face.skin_color.g;
    c_app.skin_b = app.face.skin_color.b;

    c_app.hair_r = app.face.hair_color.r;
    c_app.hair_g = app.face.hair_color.g;
    c_app.hair_b = app.face.hair_color.b;

    c_app.shirt_r = app.face.shirt_color.r;
    c_app.shirt_g = app.face.shirt_color.g;
    c_app.shirt_b = app.face.shirt_color.b;

    c_app.pants_r = app.face.pants_color.r;
    c_app.pants_g = app.face.pants_color.g;
    c_app.pants_b = app.face.pants_color.b;
    return c_app;
}

NPC_EXPORT int npc_render_pixels(
    const CNpcAppearance* c_app,
    const uint8_t* amputations_mask, // 10 bytes (one per BodyPart)
    uint8_t anim_state,
    uint8_t direction,
    float anim_time,
    uint8_t* out_rgba_buffer, // 32 * 48 * 4 bytes
    int buffer_size
) {
    if (!c_app || !out_rgba_buffer || buffer_size < npc::CharacterGenerator::CANVAS_WIDTH * npc::CharacterGenerator::CANVAS_HEIGHT * 4) {
        return -1;
    }

    npc::HumanAppearance app;
    app.npc_id = c_app->npc_id;
    app.appearance_seed = c_app->seed;
    app.generator_version = 1;
    app.body.height = c_app->height;
    app.body.body_mass = c_app->body_mass;
    app.body.muscularity = c_app->muscularity;
    app.body.posture = c_app->posture;
    app.body.head_scale = c_app->head_scale;
    app.body.torso_ratio = c_app->torso_ratio;
    app.body.arm_ratio = c_app->arm_ratio;
    app.body.leg_ratio = c_app->leg_ratio;

    app.face.hair_style = c_app->hair_style;
    app.face.expression = static_cast<npc::Expression>(c_app->expression);
    app.face.skin_color = {c_app->skin_r, c_app->skin_g, c_app->skin_b, 255};
    app.face.hair_color = {c_app->hair_r, c_app->hair_g, c_app->hair_b, 255};
    app.face.shirt_color = {c_app->shirt_r, c_app->shirt_g, c_app->shirt_b, 255};
    app.face.pants_color = {c_app->pants_r, c_app->pants_g, c_app->pants_b, 255};

    npc::PhysicalConditionSet cond;
    if (amputations_mask) {
        for (size_t i = 0; i < static_cast<size_t>(npc::BodyPart::Count); ++i) {
            if (amputations_mask[i] != 0) {
                cond.set_amputated(static_cast<npc::BodyPart>(i), true);
            }
        }
    }

    npc::AnimationCommand cmd;
    cmd.state = static_cast<npc::AnimationState>(anim_state);
    cmd.direction = static_cast<npc::Direction>(direction);
    cmd.anim_time = anim_time;
    cmd.speed = 1.0f;

    auto buf = npc::CharacterGenerator::render(app, cond, cmd);
    for (size_t i = 0; i < buf.pixels.size(); ++i) {
        out_rgba_buffer[i * 4 + 0] = buf.pixels[i].r;
        out_rgba_buffer[i * 4 + 1] = buf.pixels[i].g;
        out_rgba_buffer[i * 4 + 2] = buf.pixels[i].b;
        out_rgba_buffer[i * 4 + 3] = buf.pixels[i].a;
    }

    return 0;
}

} // extern "C"
