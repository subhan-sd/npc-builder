#pragma once

#include "types.hpp"
#include "appearance.hpp"
#include "conditions.hpp"
#include "body_geometry.hpp"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace npc {

// Schema version for CharacterProfile
inline constexpr uint32_t PROFILE_SCHEMA_VERSION = 2;

// Anatomy configuration domain
struct AnatomyProfile {
    float height{0.5f};       // 0.0 (short) - 1.0 (tall)
    float body_mass{0.5f};    // 0.0 (skinny) - 1.0 (fat/heavy)
    float muscularity{0.5f};  // 0.0 (lean) - 1.0 (muscular)
    float posture{0.0f};      // 0.0 (upright) - 1.0 (hunched)
    float head_scale{0.5f};   // 0.0 - 1.0
    float torso_ratio{0.5f};  // 0.0 - 1.0
    float arm_ratio{0.5f};    // 0.0 - 1.0
    float leg_ratio{0.5f};    // 0.0 - 1.0

    bool operator==(const AnatomyProfile& other) const = default;

    // Convert from legacy BodyConfiguration
    static AnatomyProfile from_legacy(const BodyConfiguration& b) {
        return AnatomyProfile{
            .height = b.height,
            .body_mass = b.body_mass,
            .muscularity = b.muscularity,
            .posture = b.posture,
            .head_scale = b.head_scale,
            .torso_ratio = b.torso_ratio,
            .arm_ratio = b.arm_ratio,
            .leg_ratio = b.leg_ratio
        };
    }

    // Convert to legacy BodyConfiguration
    BodyConfiguration to_legacy() const {
        return BodyConfiguration{
            .height = height,
            .body_mass = body_mass,
            .muscularity = muscularity,
            .posture = posture,
            .head_scale = head_scale,
            .torso_ratio = torso_ratio,
            .arm_ratio = arm_ratio,
            .leg_ratio = leg_ratio
        };
    }
};

// Appearance configuration domain
struct AppearanceProfile {
    uint8_t eye_shape{0};
    uint8_t eye_spacing{1};
    ColorRGBA eye_color{40, 30, 20, 255};
    uint8_t nose_shape{0};
    uint8_t mouth_shape{0};
    uint8_t hair_style{0};    // 0: Bald, 1: Short, 2: Medium, 3: Long
    ColorRGBA hair_color{50, 40, 30, 255};
    ColorRGBA skin_color{225, 185, 150, 255};
    ColorRGBA shirt_color{60, 90, 140, 255};
    ColorRGBA pants_color{45, 50, 60, 255};
    Expression expression{Expression::Neutral};

    bool operator==(const AppearanceProfile& other) const = default;

    static AppearanceProfile from_legacy(const FaceConfiguration& f) {
        return AppearanceProfile{
            .eye_shape = f.eye_shape,
            .eye_spacing = f.eye_spacing,
            .eye_color = f.eye_color,
            .nose_shape = f.nose_shape,
            .mouth_shape = f.mouth_shape,
            .hair_style = f.hair_style,
            .hair_color = f.hair_color,
            .skin_color = f.skin_color,
            .shirt_color = f.shirt_color,
            .pants_color = f.pants_color,
            .expression = f.expression
        };
    }

    FaceConfiguration to_legacy() const {
        return FaceConfiguration{
            .eye_shape = eye_shape,
            .eye_spacing = eye_spacing,
            .eye_color = eye_color,
            .nose_shape = nose_shape,
            .mouth_shape = mouth_shape,
            .hair_style = hair_style,
            .hair_color = hair_color,
            .skin_color = skin_color,
            .shirt_color = shirt_color,
            .pants_color = pants_color,
            .expression = expression
        };
    }
};

// Physical condition configuration domain
struct PhysicalConditionProfile {
    PhysicalConditionSet conditions;

    bool operator==(const PhysicalConditionProfile& other) const {
        if (conditions.amputations != other.conditions.amputations) return false;
        if (conditions.injuries.size() != other.conditions.injuries.size()) return false;
        for (size_t i = 0; i < conditions.injuries.size(); ++i) {
            const auto& a = conditions.injuries[i];
            const auto& b = other.conditions.injuries[i];
            if (a.condition_id != b.condition_id ||
                a.body_part != b.body_part ||
                a.type != b.type ||
                std::fabs(a.severity - b.severity) > 1e-5f ||
                a.visual_seed != b.visual_seed ||
                a.active != b.active ||
                a.healed != b.healed) {
                return false;
            }
        }
        return true;
    }
};

// Generation configuration domain
struct GenerationProfile {
    uint32_t npc_id{1};
    uint64_t seed{12345};
    uint32_t generator_version{1};

    bool operator==(const GenerationProfile& other) const = default;
};

// Canonical Character Profile
struct CharacterProfile {
    uint32_t schema_version{PROFILE_SCHEMA_VERSION};
    GenerationProfile generation;
    AnatomyProfile anatomy;
    AppearanceProfile appearance;
    PhysicalConditionProfile physical;
    GeometryOverride geometry_override{};
    BodyGeometry body_geometry{compute_full_geometry(0.5f, 0.5f, 0.5f, 0.5f)};

    bool operator==(const CharacterProfile& other) const = default;

    // Helper to calculate or refresh full derived geometry based on anatomy + override
    BodyGeometry derived_geometry() const {
        return compute_full_geometry(
            anatomy.height,
            anatomy.body_mass,
            anatomy.muscularity,
            anatomy.leg_ratio,
            geometry_override
        );
    }

    static CharacterProfile create_default(uint32_t npc_id = 1, uint64_t seed = 12345) {
        CharacterProfile p;
        p.schema_version = PROFILE_SCHEMA_VERSION;
        p.generation.npc_id = npc_id;
        p.generation.seed = seed;
        p.generation.generator_version = 1;
        p.anatomy = AnatomyProfile{};
        p.appearance = AppearanceProfile{};
        p.physical = PhysicalConditionProfile{};
        p.geometry_override = GeometryOverride{};
        p.body_geometry = p.derived_geometry();
        return p;
    }

    // Convert from legacy HumanAppearance + PhysicalConditionSet
    static CharacterProfile from_legacy(const HumanAppearance& app, const PhysicalConditionSet& cond = {}) {
        CharacterProfile p;
        p.schema_version = PROFILE_SCHEMA_VERSION;
        p.generation.npc_id = app.npc_id;
        p.generation.seed = app.appearance_seed;
        p.generation.generator_version = app.generator_version;
        p.anatomy = AnatomyProfile::from_legacy(app.body);
        p.appearance = AppearanceProfile::from_legacy(app.face);
        p.physical.conditions = cond;
        p.geometry_override = GeometryOverride{};
        p.body_geometry = p.derived_geometry();
        return p;
    }

    // Convert to legacy HumanAppearance
    HumanAppearance to_legacy_appearance() const {
        HumanAppearance app;
        app.npc_id = generation.npc_id;
        app.appearance_seed = generation.seed;
        app.generator_version = generation.generator_version;
        app.body = anatomy.to_legacy();
        app.face = appearance.to_legacy();
        return app;
    }
};

// Validation outcome
enum class ValidationOutcome {
    Valid,
    ValidNormalized,
    Invalid
};

struct ValidationResult {
    ValidationOutcome outcome{ValidationOutcome::Valid};
    std::vector<std::string> issues;

    bool is_ok() const {
        return outcome != ValidationOutcome::Invalid;
    }
};

class ProfileValidator {
public:
    static ValidationResult validate(const CharacterProfile& profile) {
        ValidationResult result;

        if (profile.schema_version != PROFILE_SCHEMA_VERSION) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("Unsupported schema version: " + std::to_string(profile.schema_version));
            return result;
        }

        // Validate Anatomy
        auto check_float = [&](float val, const std::string& name) {
            if (std::isnan(val) || std::isinf(val)) {
                result.outcome = ValidationOutcome::Invalid;
                result.issues.push_back(name + " is NaN or Inf");
            } else if (val < 0.0f || val > 1.0f) {
                result.outcome = ValidationOutcome::Invalid;
                result.issues.push_back(name + " out of range [0.0, 1.0]: " + std::to_string(val));
            }
        };

        check_float(profile.anatomy.height, "anatomy.height");
        check_float(profile.anatomy.body_mass, "anatomy.body_mass");
        check_float(profile.anatomy.muscularity, "anatomy.muscularity");
        check_float(profile.anatomy.posture, "anatomy.posture");
        check_float(profile.anatomy.head_scale, "anatomy.head_scale");
        check_float(profile.anatomy.torso_ratio, "anatomy.torso_ratio");
        check_float(profile.anatomy.arm_ratio, "anatomy.arm_ratio");
        check_float(profile.anatomy.leg_ratio, "anatomy.leg_ratio");

        // Validate Appearance
        if (profile.appearance.eye_shape > 2) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("appearance.eye_shape out of bounds [0, 2]: " + std::to_string(profile.appearance.eye_shape));
        }
        if (profile.appearance.eye_spacing < 1 || profile.appearance.eye_spacing > 2) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("appearance.eye_spacing out of bounds [1, 2]: " + std::to_string(profile.appearance.eye_spacing));
        }
        if (profile.appearance.nose_shape > 1) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("appearance.nose_shape out of bounds [0, 1]: " + std::to_string(profile.appearance.nose_shape));
        }
        if (profile.appearance.mouth_shape > 1) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("appearance.mouth_shape out of bounds [0, 1]: " + std::to_string(profile.appearance.mouth_shape));
        }
        if (profile.appearance.hair_style > 3) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("appearance.hair_style out of bounds [0, 3]: " + std::to_string(profile.appearance.hair_style));
        }
        if (static_cast<uint8_t>(profile.appearance.expression) > static_cast<uint8_t>(Expression::Surprised)) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("appearance.expression invalid enum");
        }

        // Validate Physical Condition Consistency (Amputations)
        if (profile.physical.conditions.is_amputated(BodyPart::Head)) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("physical.conditions Head cannot be amputated");
        }
        if (profile.physical.conditions.is_amputated(BodyPart::Torso)) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("physical.conditions Torso cannot be amputated");
        }
        if (profile.physical.conditions.is_amputated(BodyPart::LeftArm) && !profile.physical.conditions.is_amputated(BodyPart::LeftHand)) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("physical.conditions LeftHand must be amputated if LeftArm is amputated");
        }
        if (profile.physical.conditions.is_amputated(BodyPart::RightArm) && !profile.physical.conditions.is_amputated(BodyPart::RightHand)) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("physical.conditions RightHand must be amputated if RightArm is amputated");
        }
        if (profile.physical.conditions.is_amputated(BodyPart::LeftLeg) && !profile.physical.conditions.is_amputated(BodyPart::LeftFoot)) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("physical.conditions LeftFoot must be amputated if LeftLeg is amputated");
        }
        if (profile.physical.conditions.is_amputated(BodyPart::RightLeg) && !profile.physical.conditions.is_amputated(BodyPart::RightFoot)) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("physical.conditions RightFoot must be amputated if RightLeg is amputated");
        }

        // Validate Injuries
        for (size_t i = 0; i < profile.physical.conditions.injuries.size(); ++i) {
            const auto& inj = profile.physical.conditions.injuries[i];
            if (static_cast<uint8_t>(inj.body_part) >= static_cast<uint8_t>(BodyPart::Count)) {
                result.outcome = ValidationOutcome::Invalid;
                result.issues.push_back("injury[" + std::to_string(i) + "].body_part out of bounds");
            } else if (profile.physical.conditions.is_amputated(inj.body_part)) {
                result.outcome = ValidationOutcome::Invalid;
                result.issues.push_back("injury[" + std::to_string(i) + "] placed on amputated body part " + std::to_string(static_cast<int>(inj.body_part)));
            }

            if (static_cast<uint8_t>(inj.type) > static_cast<uint8_t>(ConditionType::BloodStain)) {
                result.outcome = ValidationOutcome::Invalid;
                result.issues.push_back("injury[" + std::to_string(i) + "].type out of bounds");
            }
            if (std::isnan(inj.severity) || std::isinf(inj.severity) || inj.severity < 0.0f || inj.severity > 1.0f) {
                result.outcome = ValidationOutcome::Invalid;
                result.issues.push_back("injury[" + std::to_string(i) + "].severity out of bounds [0.0, 1.0]");
            }
        }

        // Validate BodyGeometry bounds using canonical definitions from GeometryBounds
        if (profile.body_geometry.shoulder_w < GeometryBounds::SHOULDER_W_MIN || profile.body_geometry.shoulder_w > GeometryBounds::SHOULDER_W_MAX) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("body_geometry.shoulder_w out of bounds");
        }
        if (profile.body_geometry.waist_w < GeometryBounds::WAIST_W_MIN || profile.body_geometry.waist_w > GeometryBounds::WAIST_W_MAX) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("body_geometry.waist_w out of bounds");
        }
        if (profile.body_geometry.hips_w < GeometryBounds::HIPS_W_MIN || profile.body_geometry.hips_w > GeometryBounds::HIPS_W_MAX) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("body_geometry.hips_w out of bounds");
        }
        if (profile.body_geometry.thigh_thick < GeometryBounds::THIGH_THICK_MIN || profile.body_geometry.thigh_thick > GeometryBounds::THIGH_THICK_MAX) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("body_geometry.thigh_thick out of bounds");
        }
        if (profile.body_geometry.neck_w < GeometryBounds::NECK_W_MIN || profile.body_geometry.neck_w > GeometryBounds::NECK_W_MAX) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("body_geometry.neck_w out of bounds");
        }
        if (profile.body_geometry.arm_base_thick < GeometryBounds::ARM_BASE_THICK_MIN || profile.body_geometry.arm_base_thick > GeometryBounds::ARM_BASE_THICK_MAX) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("body_geometry.arm_base_thick out of bounds");
        }
        if (profile.body_geometry.leg_gap < GeometryBounds::LEG_GAP_MIN || profile.body_geometry.leg_gap > GeometryBounds::LEG_GAP_MAX) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("body_geometry.leg_gap out of bounds");
        }
        if (profile.body_geometry.total_height < GeometryBounds::TOTAL_HEIGHT_MIN || profile.body_geometry.total_height > GeometryBounds::TOTAL_HEIGHT_MAX) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("body_geometry.total_height out of bounds");
        }
        if (profile.body_geometry.leg_len < GeometryBounds::LEG_LEN_MIN || profile.body_geometry.leg_len > GeometryBounds::LEG_LEN_MAX) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("body_geometry.leg_len out of bounds");
        }
        if (profile.body_geometry.torso_h < GeometryBounds::TORSO_H_MIN || profile.body_geometry.torso_h > GeometryBounds::TORSO_H_MAX) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("body_geometry.torso_h out of bounds");
        }
        if (profile.body_geometry.arm_len < GeometryBounds::ARM_LEN_MIN || profile.body_geometry.arm_len > GeometryBounds::ARM_LEN_MAX) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("body_geometry.arm_len out of bounds");
        }

        return result;
    }

    // Normalization policy: clamps floats within [0.0, 1.0] if finite, clamps enums, returns outcome
    static ValidationResult normalize(CharacterProfile& profile) {
        ValidationResult result;

        if (profile.schema_version != PROFILE_SCHEMA_VERSION) {
            result.outcome = ValidationOutcome::Invalid;
            result.issues.push_back("Unsupported schema version: " + std::to_string(profile.schema_version));
            return result;
        }

        bool normalized = false;

        auto normalize_float = [&](float& val, const std::string& name) {
            if (std::isnan(val) || std::isinf(val)) {
                result.outcome = ValidationOutcome::Invalid;
                result.issues.push_back(name + " is NaN or Inf");
            } else if (val < 0.0f || val > 1.0f) {
                val = std::clamp(val, 0.0f, 1.0f);
                normalized = true;
                result.issues.push_back(name + " clamped to [0.0, 1.0]");
            }
        };

        normalize_float(profile.anatomy.height, "anatomy.height");
        normalize_float(profile.anatomy.body_mass, "anatomy.body_mass");
        normalize_float(profile.anatomy.muscularity, "anatomy.muscularity");
        normalize_float(profile.anatomy.posture, "anatomy.posture");
        normalize_float(profile.anatomy.head_scale, "anatomy.head_scale");
        normalize_float(profile.anatomy.torso_ratio, "anatomy.torso_ratio");
        normalize_float(profile.anatomy.arm_ratio, "anatomy.arm_ratio");
        normalize_float(profile.anatomy.leg_ratio, "anatomy.leg_ratio");

        if (result.outcome == ValidationOutcome::Invalid) {
            return result;
        }

        if (profile.appearance.eye_shape > 2) {
            profile.appearance.eye_shape = 2;
            normalized = true;
            result.issues.push_back("appearance.eye_shape clamped to 2");
        }
        if (profile.appearance.eye_spacing < 1 || profile.appearance.eye_spacing > 2) {
            profile.appearance.eye_spacing = std::clamp<uint8_t>(profile.appearance.eye_spacing, 1, 2);
            normalized = true;
            result.issues.push_back("appearance.eye_spacing clamped to [1, 2]");
        }
        if (profile.appearance.nose_shape > 1) {
            profile.appearance.nose_shape = 1;
            normalized = true;
            result.issues.push_back("appearance.nose_shape clamped to 1");
        }
        if (profile.appearance.mouth_shape > 1) {
            profile.appearance.mouth_shape = 1;
            normalized = true;
            result.issues.push_back("appearance.mouth_shape clamped to 1");
        }
        if (profile.appearance.hair_style > 3) {
            profile.appearance.hair_style = 3;
            normalized = true;
            result.issues.push_back("appearance.hair_style clamped to 3");
        }
        if (static_cast<uint8_t>(profile.appearance.expression) > static_cast<uint8_t>(Expression::Surprised)) {
            profile.appearance.expression = Expression::Neutral;
            normalized = true;
            result.issues.push_back("appearance.expression normalized to Neutral");
        }

        // Amputations normalization
        if (profile.physical.conditions.is_amputated(BodyPart::Head)) {
            profile.physical.conditions.amputations[static_cast<size_t>(BodyPart::Head)] = false;
            normalized = true;
            result.issues.push_back("Head amputation reverted");
        }
        if (profile.physical.conditions.is_amputated(BodyPart::Torso)) {
            profile.physical.conditions.amputations[static_cast<size_t>(BodyPart::Torso)] = false;
            normalized = true;
            result.issues.push_back("Torso amputation reverted");
        }
        if (profile.physical.conditions.is_amputated(BodyPart::LeftArm) && !profile.physical.conditions.is_amputated(BodyPart::LeftHand)) {
            profile.physical.conditions.amputations[static_cast<size_t>(BodyPart::LeftHand)] = true;
            normalized = true;
            result.issues.push_back("LeftHand cascade-amputated with LeftArm");
        }
        if (profile.physical.conditions.is_amputated(BodyPart::RightArm) && !profile.physical.conditions.is_amputated(BodyPart::RightHand)) {
            profile.physical.conditions.amputations[static_cast<size_t>(BodyPart::RightHand)] = true;
            normalized = true;
            result.issues.push_back("RightHand cascade-amputated with RightArm");
        }
        if (profile.physical.conditions.is_amputated(BodyPart::LeftLeg) && !profile.physical.conditions.is_amputated(BodyPart::LeftFoot)) {
            profile.physical.conditions.amputations[static_cast<size_t>(BodyPart::LeftFoot)] = true;
            normalized = true;
            result.issues.push_back("LeftFoot cascade-amputated with LeftLeg");
        }
        if (profile.physical.conditions.is_amputated(BodyPart::RightLeg) && !profile.physical.conditions.is_amputated(BodyPart::RightFoot)) {
            profile.physical.conditions.amputations[static_cast<size_t>(BodyPart::RightFoot)] = true;
            normalized = true;
            result.issues.push_back("RightFoot cascade-amputated with RightLeg");
        }

        // Injuries normalization
        auto& injs = profile.physical.conditions.injuries;
        for (auto it = injs.begin(); it != injs.end(); ) {
            if (static_cast<uint8_t>(it->body_part) >= static_cast<uint8_t>(BodyPart::Count)) {
                result.outcome = ValidationOutcome::Invalid;
                result.issues.push_back("injury body_part out of bounds");
                return result;
            }
            if (profile.physical.conditions.is_amputated(it->body_part)) {
                it = injs.erase(it);
                normalized = true;
                result.issues.push_back("Removed injury on amputated body part");
                continue;
            }
            if (static_cast<uint8_t>(it->type) > static_cast<uint8_t>(ConditionType::BloodStain)) {
                result.outcome = ValidationOutcome::Invalid;
                result.issues.push_back("injury type out of bounds");
                return result;
            }
            if (std::isnan(it->severity) || std::isinf(it->severity)) {
                result.outcome = ValidationOutcome::Invalid;
                result.issues.push_back("injury severity NaN or Inf");
                return result;
            }
            if (it->severity < 0.0f || it->severity > 1.0f) {
                it->severity = std::clamp(it->severity, 0.0f, 1.0f);
                normalized = true;
                result.issues.push_back("injury severity clamped to [0.0, 1.0]");
            }
            ++it;
        }

        // Recompute body geometry if anatomy was normalized
        profile.body_geometry = profile.derived_geometry();

        result.outcome = normalized ? ValidationOutcome::ValidNormalized : ValidationOutcome::Valid;
        return result;
    }
};

} // namespace npc
