#include "npc/c_api.h"
#include "npc/generator.hpp"
#include "npc/serializer.hpp"
#include <cstring>
#include <algorithm>

extern "C" {

NPC_C_EXPORT uint32_t npc_get_api_version(void) {
    return NPC_API_VERSION;
}

NPC_C_EXPORT int npc_profile_create_default(
    uint32_t npc_id,
    uint64_t seed,
    CNpcProfile* out_profile
) {
    if (!out_profile) return NPC_ERR_INVALID_ARGUMENT;
    if (out_profile->struct_size != sizeof(CNpcProfile)) return NPC_ERR_VERSION_MISMATCH;

    auto p = npc::CharacterProfile::create_default(npc_id, seed);

    out_profile->api_version = NPC_API_VERSION;
    out_profile->schema_version = p.schema_version;
    out_profile->npc_id = p.generation.npc_id;
    out_profile->seed = p.generation.seed;
    out_profile->generator_version = p.generation.generator_version;

    out_profile->height = p.anatomy.height;
    out_profile->body_mass = p.anatomy.body_mass;
    out_profile->muscularity = p.anatomy.muscularity;
    out_profile->posture = p.anatomy.posture;
    out_profile->head_scale = p.anatomy.head_scale;
    out_profile->torso_ratio = p.anatomy.torso_ratio;
    out_profile->arm_ratio = p.anatomy.arm_ratio;
    out_profile->leg_ratio = p.anatomy.leg_ratio;

    out_profile->eye_shape = p.appearance.eye_shape;
    out_profile->eye_spacing = p.appearance.eye_spacing;
    out_profile->nose_shape = p.appearance.nose_shape;
    out_profile->mouth_shape = p.appearance.mouth_shape;
    out_profile->hair_style = p.appearance.hair_style;
    out_profile->expression = static_cast<uint8_t>(p.appearance.expression);

    out_profile->skin_color = {p.appearance.skin_color.r, p.appearance.skin_color.g, p.appearance.skin_color.b, p.appearance.skin_color.a};
    out_profile->eye_color = {p.appearance.eye_color.r, p.appearance.eye_color.g, p.appearance.eye_color.b, p.appearance.eye_color.a};
    out_profile->hair_color = {p.appearance.hair_color.r, p.appearance.hair_color.g, p.appearance.hair_color.b, p.appearance.hair_color.a};
    out_profile->shirt_color = {p.appearance.shirt_color.r, p.appearance.shirt_color.g, p.appearance.shirt_color.b, p.appearance.shirt_color.a};
    out_profile->pants_color = {p.appearance.pants_color.r, p.appearance.pants_color.g, p.appearance.pants_color.b, p.appearance.pants_color.a};

    for (size_t i = 0; i < 10; ++i) {
        out_profile->amputations_mask[i] = p.physical.conditions.amputations[i] ? 1 : 0;
    }

    out_profile->geo_shoulder_w = static_cast<int16_t>(p.body_geometry.shoulder_w);
    out_profile->geo_waist_w = static_cast<int16_t>(p.body_geometry.waist_w);
    out_profile->geo_hips_w = static_cast<int16_t>(p.body_geometry.hips_w);
    out_profile->geo_thigh_thick = static_cast<int16_t>(p.body_geometry.thigh_thick);
    out_profile->geo_neck_w = static_cast<int16_t>(p.body_geometry.neck_w);
    out_profile->geo_arm_base_thick = static_cast<int16_t>(p.body_geometry.arm_base_thick);
    out_profile->geo_leg_gap = static_cast<int16_t>(p.body_geometry.leg_gap);

    return NPC_SUCCESS;
}

NPC_C_EXPORT int npc_profile_validate(
    const CNpcProfile* profile
) {
    if (!profile) return NPC_ERR_INVALID_ARGUMENT;
    if (profile->struct_size != sizeof(CNpcProfile)) return NPC_ERR_VERSION_MISMATCH;
    if (profile->schema_version != npc::PROFILE_SCHEMA_VERSION) return NPC_ERR_VALIDATION_FAIL;

    npc::CharacterProfile p;
    p.schema_version = profile->schema_version;
    p.generation.npc_id = profile->npc_id;
    p.generation.seed = profile->seed;
    p.generation.generator_version = profile->generator_version;

    p.anatomy.height = profile->height;
    p.anatomy.body_mass = profile->body_mass;
    p.anatomy.muscularity = profile->muscularity;
    p.anatomy.posture = profile->posture;
    p.anatomy.head_scale = profile->head_scale;
    p.anatomy.torso_ratio = profile->torso_ratio;
    p.anatomy.arm_ratio = profile->arm_ratio;
    p.anatomy.leg_ratio = profile->leg_ratio;

    p.appearance.eye_shape = profile->eye_shape;
    p.appearance.eye_spacing = profile->eye_spacing;
    p.appearance.nose_shape = profile->nose_shape;
    p.appearance.mouth_shape = profile->mouth_shape;
    p.appearance.hair_style = profile->hair_style;
    p.appearance.expression = static_cast<npc::Expression>(profile->expression);

    p.appearance.skin_color = {profile->skin_color.r, profile->skin_color.g, profile->skin_color.b, profile->skin_color.a};
    p.appearance.eye_color = {profile->eye_color.r, profile->eye_color.g, profile->eye_color.b, profile->eye_color.a};
    p.appearance.hair_color = {profile->hair_color.r, profile->hair_color.g, profile->hair_color.b, profile->hair_color.a};
    p.appearance.shirt_color = {profile->shirt_color.r, profile->shirt_color.g, profile->shirt_color.b, profile->shirt_color.a};
    p.appearance.pants_color = {profile->pants_color.r, profile->pants_color.g, profile->pants_color.b, profile->pants_color.a};

    for (size_t i = 0; i < 10; ++i) {
        if (profile->amputations_mask[i] > 1) {
            return NPC_ERR_VALIDATION_FAIL;
        }
        p.physical.conditions.amputations[i] = (profile->amputations_mask[i] != 0);
    }

    p.body_geometry.shoulder_w = profile->geo_shoulder_w;
    p.body_geometry.waist_w = profile->geo_waist_w;
    p.body_geometry.hips_w = profile->geo_hips_w;
    p.body_geometry.thigh_thick = profile->geo_thigh_thick;
    p.body_geometry.neck_w = profile->geo_neck_w;
    p.body_geometry.arm_base_thick = profile->geo_arm_base_thick;
    p.body_geometry.leg_gap = profile->geo_leg_gap;

    auto res = npc::ProfileValidator::validate(p);
    return res.is_ok() ? NPC_SUCCESS : NPC_ERR_VALIDATION_FAIL;
}

NPC_C_EXPORT int npc_profile_render(
    const CNpcProfile* c_profile,
    const CNpcInjury* injuries,
    uint32_t injury_count,
    uint8_t anim_state,
    uint8_t direction,
    float anim_time,
    uint8_t* out_rgba_buffer,
    size_t buffer_size
) {
    if (!c_profile || !out_rgba_buffer) return NPC_ERR_INVALID_ARGUMENT;
    if (injury_count > 0 && !injuries) return NPC_ERR_INVALID_ARGUMENT;
    if (c_profile->struct_size != sizeof(CNpcProfile)) return NPC_ERR_VERSION_MISMATCH;
    if (buffer_size < static_cast<size_t>(npc::CharacterGenerator::CANVAS_WIDTH * npc::CharacterGenerator::CANVAS_HEIGHT * 4)) {
        return NPC_ERR_BUFFER_TOO_SMALL;
    }
    if (anim_state > static_cast<uint8_t>(npc::AnimationState::Swim) ||
        direction > static_cast<uint8_t>(npc::Direction::West)) {
        return NPC_ERR_INVALID_ENUM;
    }

    npc::CharacterProfile p;
    p.schema_version = c_profile->schema_version;
    p.generation.npc_id = c_profile->npc_id;
    p.generation.seed = c_profile->seed;
    p.generation.generator_version = c_profile->generator_version;

    p.anatomy.height = c_profile->height;
    p.anatomy.body_mass = c_profile->body_mass;
    p.anatomy.muscularity = c_profile->muscularity;
    p.anatomy.posture = c_profile->posture;
    p.anatomy.head_scale = c_profile->head_scale;
    p.anatomy.torso_ratio = c_profile->torso_ratio;
    p.anatomy.arm_ratio = c_profile->arm_ratio;
    p.anatomy.leg_ratio = c_profile->leg_ratio;

    p.appearance.eye_shape = c_profile->eye_shape;
    p.appearance.eye_spacing = c_profile->eye_spacing;
    p.appearance.nose_shape = c_profile->nose_shape;
    p.appearance.mouth_shape = c_profile->mouth_shape;
    p.appearance.hair_style = c_profile->hair_style;
    p.appearance.expression = static_cast<npc::Expression>(c_profile->expression);

    p.appearance.skin_color = {c_profile->skin_color.r, c_profile->skin_color.g, c_profile->skin_color.b, c_profile->skin_color.a};
    p.appearance.eye_color = {c_profile->eye_color.r, c_profile->eye_color.g, c_profile->eye_color.b, c_profile->eye_color.a};
    p.appearance.hair_color = {c_profile->hair_color.r, c_profile->hair_color.g, c_profile->hair_color.b, c_profile->hair_color.a};
    p.appearance.shirt_color = {c_profile->shirt_color.r, c_profile->shirt_color.g, c_profile->shirt_color.b, c_profile->shirt_color.a};
    p.appearance.pants_color = {c_profile->pants_color.r, c_profile->pants_color.g, c_profile->pants_color.b, c_profile->pants_color.a};

    for (size_t i = 0; i < 10; ++i) {
        if (c_profile->amputations_mask[i] > 1) {
            return NPC_ERR_VALIDATION_FAIL;
        }
        p.physical.conditions.amputations[i] = (c_profile->amputations_mask[i] != 0);
    }

    p.body_geometry.shoulder_w = c_profile->geo_shoulder_w;
    p.body_geometry.waist_w = c_profile->geo_waist_w;
    p.body_geometry.hips_w = c_profile->geo_hips_w;
    p.body_geometry.thigh_thick = c_profile->geo_thigh_thick;
    p.body_geometry.neck_w = c_profile->geo_neck_w;
    p.body_geometry.arm_base_thick = c_profile->geo_arm_base_thick;
    p.body_geometry.leg_gap = c_profile->geo_leg_gap;

    if (injuries && injury_count > 0) {
        p.physical.conditions.injuries.reserve(injury_count);
        for (uint32_t i = 0; i < injury_count; ++i) {
            const auto& c_inj = injuries[i];
            npc::InjuryCondition inj;
            inj.condition_id = c_inj.condition_id;
            inj.body_part = static_cast<npc::BodyPart>(c_inj.body_part);
            inj.type = static_cast<npc::ConditionType>(c_inj.type);
            inj.severity = c_inj.severity;
            inj.visual_seed = c_inj.visual_seed;
            inj.active = c_inj.active != 0;
            inj.healed = c_inj.healed != 0;
            p.physical.conditions.injuries.push_back(inj);
        }
    }

    // Validate profile before rendering (FR-03: reject invalid profile before calling renderer)
    auto val_res = npc::ProfileValidator::validate(p);
    if (!val_res.is_ok()) {
        return NPC_ERR_VALIDATION_FAIL;
    }

    npc::AnimationCommand cmd;
    cmd.state = static_cast<npc::AnimationState>(anim_state);
    cmd.direction = static_cast<npc::Direction>(direction);
    cmd.anim_time = anim_time;
    cmd.speed = 1.0f;

    auto buf = npc::CharacterGenerator::render_profile(p, cmd);
    for (size_t i = 0; i < buf.pixels.size(); ++i) {
        out_rgba_buffer[i * 4 + 0] = buf.pixels[i].r;
        out_rgba_buffer[i * 4 + 1] = buf.pixels[i].g;
        out_rgba_buffer[i * 4 + 2] = buf.pixels[i].b;
        out_rgba_buffer[i * 4 + 3] = buf.pixels[i].a;
    }

    return NPC_SUCCESS;
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
