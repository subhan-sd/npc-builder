#pragma once
#include <stdint.h>
#include <stddef.h>

#if defined(_WIN32)
#define NPC_C_EXPORT __declspec(dllexport)
#else
#define NPC_C_EXPORT __attribute__((visibility("default")))
#endif

#define NPC_API_VERSION 4

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

typedef struct CNpcColorRGBA {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} CNpcColorRGBA;

typedef struct CNpcAppearance {
    uint32_t struct_size;       // sizeof(CNpcAppearance)
    uint32_t api_version;       // NPC_API_VERSION
    uint32_t generator_version; // e.g. 1
    uint32_t npc_id;
    uint64_t seed;

    // Body Configuration
    float height;
    float body_mass;
    float muscularity;
    float posture;
    float head_scale;
    float torso_ratio;
    float arm_ratio;
    float leg_ratio;

    // Face & Features
    uint8_t eye_shape;
    uint8_t eye_spacing;
    uint8_t nose_shape;
    uint8_t mouth_shape;
    uint8_t hair_style;
    uint8_t expression;

    // Colors
    CNpcColorRGBA skin_color;
    CNpcColorRGBA eye_color;
    CNpcColorRGBA hair_color;
    CNpcColorRGBA shirt_color;
    CNpcColorRGBA pants_color;
} CNpcAppearance;

typedef struct CNpcInjury {
    uint32_t condition_id;
    uint8_t body_part;
    uint8_t type;
    float severity;
    uint32_t visual_seed;
    uint8_t active;
    uint8_t healed;
} CNpcInjury;

// CNpcProfile representing CharacterProfile C-boundary struct
typedef struct CNpcProfile {
    uint32_t struct_size;       // sizeof(CNpcProfile)
    uint32_t api_version;       // NPC_API_VERSION
    uint32_t schema_version;    // 1

    // Generation domain
    uint32_t npc_id;
    uint64_t seed;
    uint32_t generator_version;

    // Anatomy domain
    float height;
    float body_mass;
    float muscularity;
    float posture;
    float head_scale;
    float torso_ratio;
    float arm_ratio;
    float leg_ratio;

    // Appearance domain
    uint8_t eye_shape;
    uint8_t eye_spacing;
    uint8_t nose_shape;
    uint8_t mouth_shape;
    uint8_t hair_style;
    uint8_t expression;

    CNpcColorRGBA skin_color;
    CNpcColorRGBA eye_color;
    CNpcColorRGBA hair_color;
    CNpcColorRGBA shirt_color;
    CNpcColorRGBA pants_color;

    // Physical condition domain
    uint8_t amputations_mask[10]; // BodyPart::Count

    // Body geometry domain (v0.3.3 / API v4: 7 integer values)
    // NOTE: These fields are output cache only and ignored on input.
    // Geometry is always canonically derived from anatomy.
    int16_t geo_shoulder_w;
    int16_t geo_waist_w;
    int16_t geo_hips_w;
    int16_t geo_thigh_thick;
    int16_t geo_neck_w;
    int16_t geo_arm_base_thick;
    int16_t geo_leg_gap;
} CNpcProfile;

typedef struct CNpcRenderConfig {
    uint32_t struct_size;       // sizeof(CNpcRenderConfig)
    uint32_t api_version;       // NPC_API_VERSION
    uint8_t anim_state;         // 0..5
    uint8_t direction;          // 0..3
    float anim_time;
    float speed;
    const uint8_t* amputations_mask; // 10 bytes (BodyPart::Count)
    const CNpcInjury* injuries;
    uint32_t injury_count;
} CNpcRenderConfig;

#pragma pack(pop)

// API Error codes
enum {
    NPC_SUCCESS = 0,
    NPC_ERR_INVALID_ARGUMENT = -1,
    NPC_ERR_BUFFER_TOO_SMALL = -2,
    NPC_ERR_VERSION_MISMATCH = -3,
    NPC_ERR_INVALID_ENUM     = -4,
    NPC_ERR_VALIDATION_FAIL  = -5
};

NPC_C_EXPORT uint32_t npc_get_api_version(void);

// Profile API
NPC_C_EXPORT int npc_profile_create_default(
    uint32_t npc_id,
    uint64_t seed,
    CNpcProfile* out_profile
);

NPC_C_EXPORT int npc_profile_validate(
    const CNpcProfile* profile
);

NPC_C_EXPORT int npc_profile_render(
    const CNpcProfile* profile,
    const CNpcInjury* injuries,
    uint32_t injury_count,
    uint8_t anim_state,
    uint8_t direction,
    float anim_time,
    uint8_t* out_rgba_buffer,
    size_t buffer_size
);

NPC_C_EXPORT int npc_generate_appearance_v2(
    uint64_t seed,
    uint32_t npc_id,
    CNpcAppearance* out_app
);

NPC_C_EXPORT int npc_render_pixels_v2(
    const CNpcAppearance* app,
    const CNpcRenderConfig* config,
    uint8_t* out_rgba_buffer,
    size_t buffer_size
);

// Backward-compatible V1 signatures
NPC_C_EXPORT int npc_render_pixels(
    const void* legacy_app,
    const uint8_t* amputations_mask,
    uint8_t anim_state,
    uint8_t direction,
    float anim_time,
    uint8_t* out_rgba_buffer,
    int buffer_size
);

#ifdef __cplusplus
}
#endif
