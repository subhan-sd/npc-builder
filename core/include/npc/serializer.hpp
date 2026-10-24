#pragma once
#include "appearance.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace npc {

// Serializer / Deserializer for HumanAppearance
// Format:
// [format_version] [gen_version] [npc_id] [seed] [body floats 8x] [face shapes 5x] [expression]
// [skin RGBA 4x] [eye RGBA 4x] [hair RGBA 4x] [shirt RGBA 4x] [pants RGBA 4x]
class Serializer {
public:
    static constexpr uint32_t SERIALIZER_FORMAT_VERSION = 2;

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
};

} // namespace npc
