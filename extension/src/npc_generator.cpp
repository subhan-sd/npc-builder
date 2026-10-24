#include "npc_generator.hpp"
#include "npc/generator.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <algorithm>

namespace godot {

void NPCGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("generate_appearance", "seed", "npc_id"), &NPCGenerator::generate_appearance);
    ClassDB::bind_method(D_METHOD("render_image", "appearance", "anim_state", "direction", "anim_time", "amputations", "injuries"), &NPCGenerator::render_image);
}

NPCGenerator::NPCGenerator() {}
NPCGenerator::~NPCGenerator() {}

Dictionary NPCGenerator::generate_appearance(uint64_t seed, int npc_id) {
    auto app = npc::CharacterGenerator::generate_from_seed(seed, static_cast<uint32_t>(npc_id));

    Dictionary dict;
    dict["npc_id"] = static_cast<int64_t>(app.npc_id);
    dict["appearance_seed"] = static_cast<int64_t>(app.appearance_seed);
    dict["generator_version"] = static_cast<int64_t>(app.generator_version);

    // Body
    Dictionary body;
    body["height"] = app.body.height;
    body["body_mass"] = app.body.body_mass;
    body["muscularity"] = app.body.muscularity;
    body["posture"] = app.body.posture;
    body["head_scale"] = app.body.head_scale;
    body["torso_ratio"] = app.body.torso_ratio;
    body["arm_ratio"] = app.body.arm_ratio;
    body["leg_ratio"] = app.body.leg_ratio;
    dict["body"] = body;

    // Face & Features
    Dictionary face;
    face["eye_shape"] = static_cast<int>(app.face.eye_shape);
    face["eye_spacing"] = static_cast<int>(app.face.eye_spacing);
    face["eye_color"] = Color(app.face.eye_color.r / 255.0f, app.face.eye_color.g / 255.0f, app.face.eye_color.b / 255.0f, 1.0f);
    face["nose_shape"] = static_cast<int>(app.face.nose_shape);
    face["mouth_shape"] = static_cast<int>(app.face.mouth_shape);
    face["hair_style"] = static_cast<int>(app.face.hair_style);
    face["hair_color"] = Color(app.face.hair_color.r / 255.0f, app.face.hair_color.g / 255.0f, app.face.hair_color.b / 255.0f, 1.0f);
    face["skin_color"] = Color(app.face.skin_color.r / 255.0f, app.face.skin_color.g / 255.0f, app.face.skin_color.b / 255.0f, 1.0f);
    face["shirt_color"] = Color(app.face.shirt_color.r / 255.0f, app.face.shirt_color.g / 255.0f, app.face.shirt_color.b / 255.0f, 1.0f);
    face["pants_color"] = Color(app.face.pants_color.r / 255.0f, app.face.pants_color.g / 255.0f, app.face.pants_color.b / 255.0f, 1.0f);
    face["expression"] = static_cast<int>(app.face.expression);
    dict["face"] = face;

    return dict;
}

Ref<Image> NPCGenerator::render_image(
    const Dictionary& appearance,
    int anim_state,
    int direction,
    float anim_time,
    const Array& amputations,
    const Array& injuries
) {
    npc::HumanAppearance app;
    app.npc_id = static_cast<uint32_t>(appearance.get("npc_id", 0));
    app.appearance_seed = static_cast<uint64_t>(appearance.get("appearance_seed", 0));
    app.generator_version = static_cast<uint32_t>(appearance.get("generator_version", 1));

    if (appearance.has("body")) {
        Dictionary body = appearance["body"];
        app.body.height = body.get("height", 0.5f);
        app.body.body_mass = body.get("body_mass", 0.5f);
        app.body.muscularity = body.get("muscularity", 0.5f);
        app.body.posture = body.get("posture", 0.0f);
        app.body.head_scale = body.get("head_scale", 0.5f);
        app.body.torso_ratio = body.get("torso_ratio", 0.5f);
        app.body.arm_ratio = body.get("arm_ratio", 0.5f);
        app.body.leg_ratio = body.get("leg_ratio", 0.5f);
        app.body.clamp();
    }

    if (appearance.has("face")) {
        Dictionary face = appearance["face"];
        app.face.eye_shape = static_cast<uint8_t>(static_cast<int>(face.get("eye_shape", 0)));
        app.face.eye_spacing = static_cast<uint8_t>(static_cast<int>(face.get("eye_spacing", 1)));
        app.face.nose_shape = static_cast<uint8_t>(static_cast<int>(face.get("nose_shape", 0)));
        app.face.mouth_shape = static_cast<uint8_t>(static_cast<int>(face.get("mouth_shape", 0)));
        app.face.hair_style = static_cast<uint8_t>(static_cast<int>(face.get("hair_style", 0)));
        app.face.expression = static_cast<npc::Expression>(static_cast<int>(face.get("expression", 0)));

        auto to_color = [](const Variant& v, npc::ColorRGBA def) -> npc::ColorRGBA {
            if (v.get_type() == Variant::COLOR) {
                Color c = v;
                return {
                    static_cast<uint8_t>(std::clamp(c.r * 255.0f, 0.0f, 255.0f)),
                    static_cast<uint8_t>(std::clamp(c.g * 255.0f, 0.0f, 255.0f)),
                    static_cast<uint8_t>(std::clamp(c.b * 255.0f, 0.0f, 255.0f)),
                    255
                };
            }
            return def;
        };

        app.face.skin_color = to_color(face.get("skin_color", Variant()), app.face.skin_color);
        app.face.eye_color = to_color(face.get("eye_color", Variant()), app.face.eye_color);
        app.face.hair_color = to_color(face.get("hair_color", Variant()), app.face.hair_color);
        app.face.shirt_color = to_color(face.get("shirt_color", Variant()), app.face.shirt_color);
        app.face.pants_color = to_color(face.get("pants_color", Variant()), app.face.pants_color);
    }

    npc::PhysicalConditionSet cond;
    for (int i = 0; i < amputations.size(); ++i) {
        int part_idx = amputations[i];
        if (part_idx >= 0 && part_idx < static_cast<int>(npc::BodyPart::Count)) {
            cond.set_amputated(static_cast<npc::BodyPart>(part_idx), true);
        }
    }

    for (int i = 0; i < injuries.size(); ++i) {
        Dictionary inj_dict = injuries[i];
        int body_part = inj_dict.get("body_part", 1);
        int inj_type = inj_dict.get("type", 0);
        float severity = inj_dict.get("severity", 0.5f);

        if (body_part >= 0 && body_part < static_cast<int>(npc::BodyPart::Count) &&
            inj_type >= 0 && inj_type <= static_cast<int>(npc::ConditionType::BloodStain)) {
            npc::InjuryCondition inj;
            inj.condition_id = static_cast<uint32_t>(i + 1);
            inj.body_part = static_cast<npc::BodyPart>(body_part);
            inj.type = static_cast<npc::ConditionType>(inj_type);
            inj.severity = std::clamp(severity, 0.0f, 1.0f);
            inj.active = true;
            cond.injuries.push_back(inj);
        }
    }

    npc::AnimationCommand cmd;
    cmd.state = static_cast<npc::AnimationState>(std::clamp(anim_state, 0, 5));
    cmd.direction = static_cast<npc::Direction>(std::clamp(direction, 0, 3));
    cmd.anim_time = anim_time;
    cmd.speed = 1.0f;

    auto buf = npc::CharacterGenerator::render(app, cond, cmd);

    PackedByteArray byte_array;
    byte_array.resize(buf.width * buf.height * 4);
    uint8_t* ptr = byte_array.ptrw();
    for (size_t i = 0; i < buf.pixels.size(); ++i) {
        ptr[i * 4 + 0] = buf.pixels[i].r;
        ptr[i * 4 + 1] = buf.pixels[i].g;
        ptr[i * 4 + 2] = buf.pixels[i].b;
        ptr[i * 4 + 3] = buf.pixels[i].a;
    }

    Ref<Image> image = Image::create_from_data(buf.width, buf.height, false, Image::FORMAT_RGBA8, byte_array);
    return image;
}

} // namespace godot
