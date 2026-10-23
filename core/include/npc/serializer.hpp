#pragma once
#include "appearance.hpp"
#include <sstream>
#include <iomanip>

namespace npc {

// Serializer / Deserializer for HumanAppearance
class Serializer {
public:
    static std::string serialize(const HumanAppearance& app) {
        std::ostringstream ss;
        ss << std::setprecision(9)
           << app.generator_version << " "
           << app.npc_id << " "
           << app.appearance_seed << " "
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
           << static_cast<int>(app.face.hair_color.r) << " "
           << static_cast<int>(app.face.hair_color.g) << " "
           << static_cast<int>(app.face.hair_color.b) << " "
           << static_cast<int>(app.face.shirt_color.r) << " "
           << static_cast<int>(app.face.shirt_color.g) << " "
           << static_cast<int>(app.face.shirt_color.b) << " "
           << static_cast<int>(app.face.pants_color.r) << " "
           << static_cast<int>(app.face.pants_color.g) << " "
           << static_cast<int>(app.face.pants_color.b);
        return ss.str();
    }

    static bool deserialize(const std::string& data, HumanAppearance& out_app) {
        std::istringstream ss(data);
        int eye_shape, eye_spacing, nose, mouth, hair_style, expr;
        int sr, sg, sb, hr, hg, hb, tr, tg, tb, pr, pg, pb;

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
                 >> sr >> sg >> sb
                 >> hr >> hg >> hb
                 >> tr >> tg >> tb
                 >> pr >> pg >> pb)) {
            return false;
        }

        out_app.face.eye_shape = static_cast<uint8_t>(eye_shape);
        out_app.face.eye_spacing = static_cast<uint8_t>(eye_spacing);
        out_app.face.nose_shape = static_cast<uint8_t>(nose);
        out_app.face.mouth_shape = static_cast<uint8_t>(mouth);
        out_app.face.hair_style = static_cast<uint8_t>(hair_style);
        out_app.face.expression = static_cast<Expression>(expr);
        out_app.face.skin_color = {static_cast<uint8_t>(sr), static_cast<uint8_t>(sg), static_cast<uint8_t>(sb), 255};
        out_app.face.hair_color = {static_cast<uint8_t>(hr), static_cast<uint8_t>(hg), static_cast<uint8_t>(hb), 255};
        out_app.face.shirt_color = {static_cast<uint8_t>(tr), static_cast<uint8_t>(tg), static_cast<uint8_t>(tb), 255};
        out_app.face.pants_color = {static_cast<uint8_t>(pr), static_cast<uint8_t>(pg), static_cast<uint8_t>(pb), 255};
        return true;
    }
};

} // namespace npc
