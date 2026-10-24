#include "npc/c_api.h"
#include "npc/generator.hpp"
#include "npc/serializer.hpp"
#include <cstring>
#include <algorithm>

extern "C" {

NPC_C_EXPORT uint32_t npc_get_api_version(void) {
    return NPC_API_VERSION;
}

NPC_C_EXPORT int npc_generate_appearance_v2(
    uint64_t seed,
    uint32_t npc_id,
    CNpcAppearance* out_app
) {
    if (!out_app) {
        return NPC_ERR_INVALID_ARGUMENT;
    }
    if (out_app->struct_size != sizeof(CNpcAppearance)) {
        return NPC_ERR_VERSION_MISMATCH;
    }

    auto app = npc::CharacterGenerator::generate_from_seed(seed, npc_id);

    out_app->api_version = NPC_API_VERSION;
    out_app->generator_version = app.generator_version;
    out_app->npc_id = app.npc_id;
    out_app->seed = app.appearance_seed;

    out_app->height = app.body.height;
    out_app->body_mass = app.body.body_mass;
    out_app->muscularity = app.body.muscularity;
    out_app->posture = app.body.posture;
    out_app->head_scale = app.body.head_scale;
    out_app->torso_ratio = app.body.torso_ratio;
    out_app->arm_ratio = app.body.arm_ratio;
    out_app->leg_ratio = app.body.leg_ratio;

    out_app->eye_shape = app.face.eye_shape;
    out_app->eye_spacing = app.face.eye_spacing;
    out_app->nose_shape = app.face.nose_shape;
    out_app->mouth_shape = app.face.mouth_shape;
    out_app->hair_style = app.face.hair_style;
    out_app->expression = static_cast<uint8_t>(app.face.expression);

    out_app->skin_color = {app.face.skin_color.r, app.face.skin_color.g, app.face.skin_color.b, app.face.skin_color.a};
    out_app->eye_color = {app.face.eye_color.r, app.face.eye_color.g, app.face.eye_color.b, app.face.eye_color.a};
    out_app->hair_color = {app.face.hair_color.r, app.face.hair_color.g, app.face.hair_color.b, app.face.hair_color.a};
    out_app->shirt_color = {app.face.shirt_color.r, app.face.shirt_color.g, app.face.shirt_color.b, app.face.shirt_color.a};
    out_app->pants_color = {app.face.pants_color.r, app.face.pants_color.g, app.face.pants_color.b, app.face.pants_color.a};

    return NPC_SUCCESS;
}

NPC_C_EXPORT int npc_render_pixels_v2(
    const CNpcAppearance* c_app,
    const CNpcRenderConfig* config,
    uint8_t* out_rgba_buffer,
    size_t buffer_size
) {
    if (!c_app || !config || !out_rgba_buffer) {
        return NPC_ERR_INVALID_ARGUMENT;
    }
    if (c_app->struct_size != sizeof(CNpcAppearance) || config->struct_size != sizeof(CNpcRenderConfig)) {
        return NPC_ERR_VERSION_MISMATCH;
    }
    if (buffer_size < static_cast<size_t>(npc::CharacterGenerator::CANVAS_WIDTH * npc::CharacterGenerator::CANVAS_HEIGHT * 4)) {
        return NPC_ERR_BUFFER_TOO_SMALL;
    }
    if (config->anim_state > static_cast<uint8_t>(npc::AnimationState::Swim) ||
        config->direction > static_cast<uint8_t>(npc::Direction::West)) {
        return NPC_ERR_INVALID_ENUM;
    }

    npc::HumanAppearance app;
    app.npc_id = c_app->npc_id;
    app.appearance_seed = c_app->seed;
    app.generator_version = c_app->generator_version;

    app.body.height = c_app->height;
    app.body.body_mass = c_app->body_mass;
    app.body.muscularity = c_app->muscularity;
    app.body.posture = c_app->posture;
    app.body.head_scale = c_app->head_scale;
    app.body.torso_ratio = c_app->torso_ratio;
    app.body.arm_ratio = c_app->arm_ratio;
    app.body.leg_ratio = c_app->leg_ratio;
    app.body.clamp();

    app.face.eye_shape = c_app->eye_shape;
    app.face.eye_spacing = c_app->eye_spacing;
    app.face.nose_shape = c_app->nose_shape;
    app.face.mouth_shape = c_app->mouth_shape;
    app.face.hair_style = c_app->hair_style;
    app.face.expression = static_cast<npc::Expression>(c_app->expression);

    app.face.skin_color = {c_app->skin_color.r, c_app->skin_color.g, c_app->skin_color.b, c_app->skin_color.a};
    app.face.eye_color = {c_app->eye_color.r, c_app->eye_color.g, c_app->eye_color.b, c_app->eye_color.a};
    app.face.hair_color = {c_app->hair_color.r, c_app->hair_color.g, c_app->hair_color.b, c_app->hair_color.a};
    app.face.shirt_color = {c_app->shirt_color.r, c_app->shirt_color.g, c_app->shirt_color.b, c_app->shirt_color.a};
    app.face.pants_color = {c_app->pants_color.r, c_app->pants_color.g, c_app->pants_color.b, c_app->pants_color.a};

    npc::PhysicalConditionSet cond;
    if (config->amputations_mask) {
        for (size_t i = 0; i < static_cast<size_t>(npc::BodyPart::Count); ++i) {
            if (config->amputations_mask[i] != 0) {
                cond.set_amputated(static_cast<npc::BodyPart>(i), true);
            }
        }
    }

    if (config->injuries && config->injury_count > 0) {
        for (uint32_t i = 0; i < config->injury_count; ++i) {
            const auto& c_inj = config->injuries[i];
            if (c_inj.body_part < static_cast<uint8_t>(npc::BodyPart::Count) &&
                c_inj.type <= static_cast<uint8_t>(npc::ConditionType::BloodStain)) {
                npc::InjuryCondition inj;
                inj.condition_id = c_inj.condition_id;
                inj.body_part = static_cast<npc::BodyPart>(c_inj.body_part);
                inj.type = static_cast<npc::ConditionType>(c_inj.type);
                inj.severity = std::clamp(c_inj.severity, 0.0f, 1.0f);
                inj.visual_seed = c_inj.visual_seed;
                inj.active = c_inj.active != 0;
                inj.healed = c_inj.healed != 0;
                cond.injuries.push_back(inj);
            }
        }
    }

    npc::AnimationCommand cmd;
    cmd.state = static_cast<npc::AnimationState>(config->anim_state);
    cmd.direction = static_cast<npc::Direction>(config->direction);
    cmd.anim_time = config->anim_time;
    cmd.speed = (config->speed <= 0.0f) ? 1.0f : config->speed;

    auto buf = npc::CharacterGenerator::render(app, cond, cmd);
    for (size_t i = 0; i < buf.pixels.size(); ++i) {
        out_rgba_buffer[i * 4 + 0] = buf.pixels[i].r;
        out_rgba_buffer[i * 4 + 1] = buf.pixels[i].g;
        out_rgba_buffer[i * 4 + 2] = buf.pixels[i].b;
        out_rgba_buffer[i * 4 + 3] = buf.pixels[i].a;
    }

    return NPC_SUCCESS;
}

// Backward-compatible V1 implementation
struct CNpcAppearanceV1 {
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

NPC_C_EXPORT int npc_render_pixels(
    const void* legacy_app,
    const uint8_t* amputations_mask,
    uint8_t anim_state,
    uint8_t direction,
    float anim_time,
    uint8_t* out_rgba_buffer,
    int buffer_size
) {
    if (!legacy_app || !out_rgba_buffer || buffer_size < npc::CharacterGenerator::CANVAS_WIDTH * npc::CharacterGenerator::CANVAS_HEIGHT * 4) {
        return NPC_ERR_INVALID_ARGUMENT;
    }

    const auto* c_app = static_cast<const CNpcAppearanceV1*>(legacy_app);
    CNpcAppearance app_v2{};
    app_v2.struct_size = sizeof(CNpcAppearance);
    app_v2.api_version = NPC_API_VERSION;
    app_v2.generator_version = 1;
    app_v2.npc_id = c_app->npc_id;
    app_v2.seed = c_app->seed;
    app_v2.height = c_app->height;
    app_v2.body_mass = c_app->body_mass;
    app_v2.muscularity = c_app->muscularity;
    app_v2.posture = c_app->posture;
    app_v2.head_scale = c_app->head_scale;
    app_v2.torso_ratio = c_app->torso_ratio;
    app_v2.arm_ratio = c_app->arm_ratio;
    app_v2.leg_ratio = c_app->leg_ratio;
    app_v2.hair_style = c_app->hair_style;
    app_v2.expression = c_app->expression;
    app_v2.skin_color = {c_app->skin_r, c_app->skin_g, c_app->skin_b, 255};
    app_v2.hair_color = {c_app->hair_r, c_app->hair_g, c_app->hair_b, 255};
    app_v2.shirt_color = {c_app->shirt_r, c_app->shirt_g, c_app->shirt_b, 255};
    app_v2.pants_color = {c_app->pants_r, c_app->pants_g, c_app->pants_b, 255};

    CNpcRenderConfig config{};
    config.struct_size = sizeof(CNpcRenderConfig);
    config.api_version = NPC_API_VERSION;
    config.anim_state = anim_state;
    config.direction = direction;
    config.anim_time = anim_time;
    config.speed = 1.0f;
    config.amputations_mask = amputations_mask;

    return npc_render_pixels_v2(&app_v2, &config, out_rgba_buffer, static_cast<size_t>(buffer_size));
}

} // extern "C"
