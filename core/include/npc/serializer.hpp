#pragma once
#include "appearance.hpp"
#include "profile.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace npc {

// Serializer / Deserializer for HumanAppearance and CharacterProfile
class Serializer {
public:
    static constexpr uint32_t SERIALIZER_FORMAT_VERSION = 2;
    static constexpr uint32_t PROFILE_FORMAT_VERSION = 3;

    // Serialize legacy HumanAppearance (Format V2)
    static std::string serialize(const HumanAppearance& app) {
        std::ostringstream ss;
        ss << SERIALIZER_FORMAT_VERSION << " "
           << app.generator_version << " "
           << app.npc_id << " "
           << app.appearance_seed << " "
           << std::setprecision(9)
           << app.body.height << " "
           << app.body.body_mass << " "
           << app.body.muscularity << " "
           << app.body.posture << " "
           << app.body.head_scale << " "
           << app.body.torso_ratio << " "
           << app.body.arm_ratio << " "
           << app.body.leg_ratio << " "
           << static_cast<int>(app.face.eye_shape) << " "
           << static_cast<int>(app.face.eye_spacing) << " "
           << static_cast<int>(app.face.nose_shape) << " "
           << static_cast<int>(app.face.mouth_shape) << " "
           << static_cast<int>(app.face.hair_style) << " "
           << static_cast<int>(app.face.expression) << " "
           << static_cast<int>(app.face.skin_color.r) << " "
           << static_cast<int>(app.face.skin_color.g) << " "
           << static_cast<int>(app.face.skin_color.b) << " "
           << static_cast<int>(app.face.skin_color.a) << " "
           << static_cast<int>(app.face.eye_color.r) << " "
           << static_cast<int>(app.face.eye_color.g) << " "
           << static_cast<int>(app.face.eye_color.b) << " "
           << static_cast<int>(app.face.eye_color.a) << " "
           << static_cast<int>(app.face.hair_color.r) << " "
           << static_cast<int>(app.face.hair_color.g) << " "
           << static_cast<int>(app.face.hair_color.b) << " "
           << static_cast<int>(app.face.hair_color.a) << " "
           << static_cast<int>(app.face.shirt_color.r) << " "
           << static_cast<int>(app.face.shirt_color.g) << " "
           << static_cast<int>(app.face.shirt_color.b) << " "
           << static_cast<int>(app.face.shirt_color.a) << " "
           << static_cast<int>(app.face.pants_color.r) << " "
           << static_cast<int>(app.face.pants_color.g) << " "
           << static_cast<int>(app.face.pants_color.b) << " "
           << static_cast<int>(app.face.pants_color.a);
        return ss.str();
    }

    static bool deserialize(const std::string& data, HumanAppearance& out_app) {
        std::istringstream ss(data);
        uint32_t format_version = 0;
        if (!(ss >> format_version)) {
            return false;
        }

        if (format_version == 2) {
            int eye_shape, eye_spacing, nose, mouth, hair_style, expr;
            int sr, sg, sb, sa;
            int er, eg, eb, ea;
            int hr, hg, hb, ha;
            int tr, tg, tb, ta;
            int pr, pg, pb, pa;

            if (!(ss >> out_app.generator_version
                     >> out_app.npc_id
                     >> out_app.appearance_seed
                     >> out_app.body.height
                     >> out_app.body.body_mass
                     >> out_app.body.muscularity
                     >> out_app.body.posture
                     >> out_app.body.head_scale
                     >> out_app.body.torso_ratio
                     >> out_app.body.arm_ratio
                     >> out_app.body.leg_ratio
                     >> eye_shape
                     >> eye_spacing
                     >> nose
                     >> mouth
                     >> hair_style
                     >> expr
                     >> sr >> sg >> sb >> sa
                     >> er >> eg >> eb >> ea
                     >> hr >> hg >> hb >> ha
                     >> tr >> tg >> tb >> ta
                     >> pr >> pg >> pb >> pa)) {
                return false;
            }

            // Validation of numerical & enum bounds
            if (out_app.body.height < 0.0f || out_app.body.height > 1.0f ||
                out_app.body.body_mass < 0.0f || out_app.body.body_mass > 1.0f ||
                out_app.body.muscularity < 0.0f || out_app.body.muscularity > 1.0f ||
                out_app.body.posture < 0.0f || out_app.body.posture > 1.0f ||
                out_app.body.head_scale < 0.0f || out_app.body.head_scale > 1.0f ||
                out_app.body.torso_ratio < 0.0f || out_app.body.torso_ratio > 1.0f ||
                out_app.body.arm_ratio < 0.0f || out_app.body.arm_ratio > 1.0f ||
                out_app.body.leg_ratio < 0.0f || out_app.body.leg_ratio > 1.0f) {
                return false;
            }

            if (expr < 0 || expr > static_cast<int>(Expression::Surprised)) {
                return false;
            }

            auto valid_color = [](int r, int g, int b, int a) {
                return r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255 && a >= 0 && a <= 255;
            };

            if (!valid_color(sr, sg, sb, sa) || !valid_color(er, eg, eb, ea) ||
                !valid_color(hr, hg, hb, ha) || !valid_color(tr, tg, tb, ta) ||
                !valid_color(pr, pg, pb, pa)) {
                return false;
            }

            out_app.face.eye_shape = static_cast<uint8_t>(eye_shape);
            out_app.face.eye_spacing = static_cast<uint8_t>(eye_spacing);
            out_app.face.nose_shape = static_cast<uint8_t>(nose);
            out_app.face.mouth_shape = static_cast<uint8_t>(mouth);
            out_app.face.hair_style = static_cast<uint8_t>(hair_style);
            out_app.face.expression = static_cast<Expression>(expr);

            out_app.face.skin_color = {static_cast<uint8_t>(sr), static_cast<uint8_t>(sg), static_cast<uint8_t>(sb), static_cast<uint8_t>(sa)};
            out_app.face.eye_color = {static_cast<uint8_t>(er), static_cast<uint8_t>(eg), static_cast<uint8_t>(eb), static_cast<uint8_t>(ea)};
            out_app.face.hair_color = {static_cast<uint8_t>(hr), static_cast<uint8_t>(hg), static_cast<uint8_t>(hb), static_cast<uint8_t>(ha)};
            out_app.face.shirt_color = {static_cast<uint8_t>(tr), static_cast<uint8_t>(tg), static_cast<uint8_t>(tb), static_cast<uint8_t>(ta)};
            out_app.face.pants_color = {static_cast<uint8_t>(pr), static_cast<uint8_t>(pg), static_cast<uint8_t>(pb), static_cast<uint8_t>(pa)};
            return true;
        }

        // Unknown format version
        return false;
    }

    // Serialize CharacterProfile (Format V3)
    // Structure:
    // [format_version 3] [schema_version] [npc_id] [seed] [gen_version]
    // [anatomy floats 8x: height body_mass muscularity posture head_scale torso_ratio arm_ratio leg_ratio]
    // [appearance shapes 5x: eye_shape eye_spacing nose_shape mouth_shape hair_style] [expression]
    // [skin RGBA 4x] [eye RGBA 4x] [hair RGBA 4x] [shirt RGBA 4x] [pants RGBA 4x]
    // [amputations_mask 10x]
    // [injury_count]
    // For each injury: [condition_id body_part type severity visual_seed active healed]
    static std::string serialize_profile(const CharacterProfile& p) {
        std::ostringstream ss;
        ss << PROFILE_FORMAT_VERSION << " "
           << p.schema_version << " "
           << p.generation.npc_id << " "
           << p.generation.seed << " "
           << p.generation.generator_version << " "
           << std::setprecision(9)
           << p.anatomy.height << " "
           << p.anatomy.body_mass << " "
           << p.anatomy.muscularity << " "
           << p.anatomy.posture << " "
           << p.anatomy.head_scale << " "
           << p.anatomy.torso_ratio << " "
           << p.anatomy.arm_ratio << " "
           << p.anatomy.leg_ratio << " "
           << static_cast<int>(p.appearance.eye_shape) << " "
           << static_cast<int>(p.appearance.eye_spacing) << " "
           << static_cast<int>(p.appearance.nose_shape) << " "
           << static_cast<int>(p.appearance.mouth_shape) << " "
           << static_cast<int>(p.appearance.hair_style) << " "
           << static_cast<int>(p.appearance.expression) << " "
           << static_cast<int>(p.appearance.skin_color.r) << " "
           << static_cast<int>(p.appearance.skin_color.g) << " "
           << static_cast<int>(p.appearance.skin_color.b) << " "
           << static_cast<int>(p.appearance.skin_color.a) << " "
           << static_cast<int>(p.appearance.eye_color.r) << " "
           << static_cast<int>(p.appearance.eye_color.g) << " "
           << static_cast<int>(p.appearance.eye_color.b) << " "
           << static_cast<int>(p.appearance.eye_color.a) << " "
           << static_cast<int>(p.appearance.hair_color.r) << " "
           << static_cast<int>(p.appearance.hair_color.g) << " "
           << static_cast<int>(p.appearance.hair_color.b) << " "
           << static_cast<int>(p.appearance.hair_color.a) << " "
           << static_cast<int>(p.appearance.shirt_color.r) << " "
           << static_cast<int>(p.appearance.shirt_color.g) << " "
           << static_cast<int>(p.appearance.shirt_color.b) << " "
           << static_cast<int>(p.appearance.shirt_color.a) << " "
           << static_cast<int>(p.appearance.pants_color.r) << " "
           << static_cast<int>(p.appearance.pants_color.g) << " "
           << static_cast<int>(p.appearance.pants_color.b) << " "
           << static_cast<int>(p.appearance.pants_color.a);

        // Amputations
        for (size_t i = 0; i < static_cast<size_t>(BodyPart::Count); ++i) {
            ss << " " << (p.physical.conditions.amputations[i] ? 1 : 0);
        }

        // Injuries
        ss << " " << p.physical.conditions.injuries.size();
        for (const auto& inj : p.physical.conditions.injuries) {
            ss << " " << inj.condition_id
               << " " << static_cast<int>(inj.body_part)
               << " " << static_cast<int>(inj.type)
               << " " << inj.severity
               << " " << inj.visual_seed
               << " " << (inj.active ? 1 : 0)
               << " " << (inj.healed ? 1 : 0);
        }

        return ss.str();
    }

    // Deserializer for CharacterProfile
    // Supports Format V3 (full profile) and Format V2 (legacy migration into profile)
    static bool deserialize_profile(const std::string& data, CharacterProfile& out_p) {
        std::istringstream ss(data);
        uint32_t format_version = 0;
        if (!(ss >> format_version)) {
            return false;
        }

        if (format_version == PROFILE_FORMAT_VERSION) {
            uint32_t schema = 0;
            if (!(ss >> schema)) return false;
            if (schema != PROFILE_SCHEMA_VERSION) return false;

            out_p.schema_version = schema;
            if (!(ss >> out_p.generation.npc_id
                     >> out_p.generation.seed
                     >> out_p.generation.generator_version
                     >> out_p.anatomy.height
                     >> out_p.anatomy.body_mass
                     >> out_p.anatomy.muscularity
                     >> out_p.anatomy.posture
                     >> out_p.anatomy.head_scale
                     >> out_p.anatomy.torso_ratio
                     >> out_p.anatomy.arm_ratio
                     >> out_p.anatomy.leg_ratio)) {
                return false;
            }

            int eye_shape, eye_spacing, nose, mouth, hair_style, expr;
            int sr, sg, sb, sa;
            int er, eg, eb, ea;
            int hr, hg, hb, ha;
            int tr, tg, tb, ta;
            int pr, pg, pb, pa;

            if (!(ss >> eye_shape >> eye_spacing >> nose >> mouth >> hair_style >> expr
                     >> sr >> sg >> sb >> sa
                     >> er >> eg >> eb >> ea
                     >> hr >> hg >> hb >> ha
                     >> tr >> tg >> tb >> ta
                     >> pr >> pg >> pb >> pa)) {
                return false;
            }

            auto valid_color = [](int r, int g, int b, int a) {
                return r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255 && a >= 0 && a <= 255;
            };

            if (!valid_color(sr, sg, sb, sa) || !valid_color(er, eg, eb, ea) ||
                !valid_color(hr, hg, hb, ha) || !valid_color(tr, tg, tb, ta) ||
                !valid_color(pr, pg, pb, pa)) {
                return false;
            }

            out_p.appearance.eye_shape = static_cast<uint8_t>(eye_shape);
            out_p.appearance.eye_spacing = static_cast<uint8_t>(eye_spacing);
            out_p.appearance.nose_shape = static_cast<uint8_t>(nose);
            out_p.appearance.mouth_shape = static_cast<uint8_t>(mouth);
            out_p.appearance.hair_style = static_cast<uint8_t>(hair_style);
            out_p.appearance.expression = static_cast<Expression>(expr);

            out_p.appearance.skin_color = {static_cast<uint8_t>(sr), static_cast<uint8_t>(sg), static_cast<uint8_t>(sb), static_cast<uint8_t>(sa)};
            out_p.appearance.eye_color = {static_cast<uint8_t>(er), static_cast<uint8_t>(eg), static_cast<uint8_t>(eb), static_cast<uint8_t>(ea)};
            out_p.appearance.hair_color = {static_cast<uint8_t>(hr), static_cast<uint8_t>(hg), static_cast<uint8_t>(hb), static_cast<uint8_t>(ha)};
            out_p.appearance.shirt_color = {static_cast<uint8_t>(tr), static_cast<uint8_t>(tg), static_cast<uint8_t>(tb), static_cast<uint8_t>(ta)};
            out_p.appearance.pants_color = {static_cast<uint8_t>(pr), static_cast<uint8_t>(pg), static_cast<uint8_t>(pb), static_cast<uint8_t>(pa)};

            // Amputations
            for (size_t i = 0; i < static_cast<size_t>(BodyPart::Count); ++i) {
                int amp = 0;
                if (!(ss >> amp)) return false;
                out_p.physical.conditions.amputations[i] = (amp != 0);
            }

            // Injuries
            size_t injury_count = 0;
            if (!(ss >> injury_count)) return false;
            out_p.physical.conditions.injuries.clear();
            out_p.physical.conditions.injuries.reserve(injury_count);

            for (size_t i = 0; i < injury_count; ++i) {
                InjuryCondition inj;
                int part, type, act, hld;
                if (!(ss >> inj.condition_id >> part >> type >> inj.severity >> inj.visual_seed >> act >> hld)) {
                    return false;
                }
                inj.body_part = static_cast<BodyPart>(part);
                inj.type = static_cast<ConditionType>(type);
                inj.active = (act != 0);
                inj.healed = (hld != 0);
                out_p.physical.conditions.injuries.push_back(inj);
            }

            auto res = ProfileValidator::validate(out_p);
            return res.is_ok();
        } else if (format_version == SERIALIZER_FORMAT_VERSION) {
            // Backward compatibility: import Format V2 into CharacterProfile
            HumanAppearance legacy_app;
            if (!deserialize(data, legacy_app)) {
                return false;
            }
            out_p = CharacterProfile::from_legacy(legacy_app);
            auto res = ProfileValidator::validate(out_p);
            return res.is_ok();
        }

        return false;
    }
};

} // namespace npc
