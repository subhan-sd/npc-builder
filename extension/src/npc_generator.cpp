#include "npc_generator.hpp"
#include "npc/generator.hpp"
#include "npc/profile.hpp"
#include "npc/serializer.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>

namespace godot {

void NPCGenerator::_bind_methods() {
    // Legacy methods
    ClassDB::bind_method(D_METHOD("generate_appearance", "seed", "npc_id"), &NPCGenerator::generate_appearance);
    ClassDB::bind_method(D_METHOD("render_image", "appearance", "anim_state", "direction", "anim_time", "amputations", "injuries"), &NPCGenerator::render_image);

    // v0.3.1 Character Profile methods
    ClassDB::bind_method(D_METHOD("create_default_profile", "npc_id", "seed"), &NPCGenerator::create_default_profile);
    ClassDB::bind_method(D_METHOD("generate_profile", "seed", "npc_id"), &NPCGenerator::generate_profile);
    ClassDB::bind_method(D_METHOD("validate_profile", "profile"), &NPCGenerator::validate_profile);
    ClassDB::bind_method(D_METHOD("serialize_profile", "profile"), &NPCGenerator::serialize_profile);
    ClassDB::bind_method(D_METHOD("deserialize_profile", "serialized_data"), &NPCGenerator::deserialize_profile);
    ClassDB::bind_method(D_METHOD("render_profile", "profile", "anim_state", "direction", "anim_time"), &NPCGenerator::render_profile);
}

NPCGenerator::NPCGenerator() {}
NPCGenerator::~NPCGenerator() {}

// Helper to convert C++ CharacterProfile to Godot Dictionary
static Dictionary profile_to_dict(const npc::CharacterProfile& p) {
    Dictionary dict;
    dict["schema_version"] = static_cast<int64_t>(p.schema_version);

    // Generation
    Dictionary gen;
    gen["npc_id"] = static_cast<int64_t>(p.generation.npc_id);
    gen["seed"] = static_cast<int64_t>(p.generation.seed);
    gen["generator_version"] = static_cast<int64_t>(p.generation.generator_version);
    dict["generation"] = gen;

    // Anatomy
    Dictionary anatomy;
    anatomy["height"] = p.anatomy.height;
    anatomy["body_mass"] = p.anatomy.body_mass;
    anatomy["muscularity"] = p.anatomy.muscularity;
    anatomy["posture"] = p.anatomy.posture;
    anatomy["head_scale"] = p.anatomy.head_scale;
    anatomy["torso_ratio"] = p.anatomy.torso_ratio;
    anatomy["arm_ratio"] = p.anatomy.arm_ratio;
    anatomy["leg_ratio"] = p.anatomy.leg_ratio;
    dict["anatomy"] = anatomy;

    // Appearance
    Dictionary app;
    app["eye_shape"] = static_cast<int>(p.appearance.eye_shape);
    app["eye_spacing"] = static_cast<int>(p.appearance.eye_spacing);
    app["nose_shape"] = static_cast<int>(p.appearance.nose_shape);
    app["mouth_shape"] = static_cast<int>(p.appearance.mouth_shape);
    app["hair_style"] = static_cast<int>(p.appearance.hair_style);
    app["expression"] = static_cast<int>(p.appearance.expression);

    app["eye_color"] = Color(p.appearance.eye_color.r / 255.0f, p.appearance.eye_color.g / 255.0f, p.appearance.eye_color.b / 255.0f, 1.0f);
    app["hair_color"] = Color(p.appearance.hair_color.r / 255.0f, p.appearance.hair_color.g / 255.0f, p.appearance.hair_color.b / 255.0f, 1.0f);
    app["skin_color"] = Color(p.appearance.skin_color.r / 255.0f, p.appearance.skin_color.g / 255.0f, p.appearance.skin_color.b / 255.0f, 1.0f);
    app["shirt_color"] = Color(p.appearance.shirt_color.r / 255.0f, p.appearance.shirt_color.g / 255.0f, p.appearance.shirt_color.b / 255.0f, 1.0f);
    app["pants_color"] = Color(p.appearance.pants_color.r / 255.0f, p.appearance.pants_color.g / 255.0f, p.appearance.pants_color.b / 255.0f, 1.0f);
    dict["appearance"] = app;

    // Physical conditions
    Dictionary physical;
    Array amps;
    for (size_t i = 0; i < static_cast<size_t>(npc::BodyPart::Count); ++i) {
        if (p.physical.conditions.amputations[i]) {
            amps.push_back(static_cast<int>(i));
        }
    }
    physical["amputations"] = amps;

    Array injs;
    for (const auto& inj : p.physical.conditions.injuries) {
        Dictionary d_inj;
        d_inj["condition_id"] = static_cast<int64_t>(inj.condition_id);
        d_inj["body_part"] = static_cast<int>(inj.body_part);
        d_inj["type"] = static_cast<int>(inj.type);
        d_inj["severity"] = inj.severity;
        d_inj["visual_seed"] = static_cast<int64_t>(inj.visual_seed);
        d_inj["active"] = inj.active;
        d_inj["healed"] = inj.healed;
        injs.push_back(d_inj);
    }
    physical["injuries"] = injs;
    dict["physical"] = physical;

    // Body Geometry (v0.3.3)
    Dictionary geo;
    geo["shoulder_w"] = p.body_geometry.shoulder_w;
    geo["waist_w"] = p.body_geometry.waist_w;
    geo["hips_w"] = p.body_geometry.hips_w;
    geo["thigh_thick"] = p.body_geometry.thigh_thick;
    geo["neck_w"] = p.body_geometry.neck_w;
    geo["arm_base_thick"] = p.body_geometry.arm_base_thick;
    geo["leg_gap"] = p.body_geometry.leg_gap;
    dict["body_geometry"] = geo;

    return dict;
}

// Helper to convert Godot Dictionary to C++ CharacterProfile
static npc::CharacterProfile dict_to_profile(const Dictionary& dict) {
    npc::CharacterProfile p;
    p.schema_version = static_cast<uint32_t>(dict.get("schema_version", npc::PROFILE_SCHEMA_VERSION));

    if (dict.has("generation")) {
        Dictionary gen = dict["generation"];
        p.generation.npc_id = static_cast<uint32_t>(gen.get("npc_id", 1));
        p.generation.seed = static_cast<uint64_t>(gen.get("seed", 12345));
        p.generation.generator_version = static_cast<uint32_t>(gen.get("generator_version", 1));
    }

    if (dict.has("anatomy")) {
        Dictionary anatomy = dict["anatomy"];
        p.anatomy.height = anatomy.get("height", 0.5f);
        p.anatomy.body_mass = anatomy.get("body_mass", 0.5f);
        p.anatomy.muscularity = anatomy.get("muscularity", 0.5f);
        p.anatomy.posture = anatomy.get("posture", 0.0f);
        p.anatomy.head_scale = anatomy.get("head_scale", 0.5f);
        p.anatomy.torso_ratio = anatomy.get("torso_ratio", 0.5f);
        p.anatomy.arm_ratio = anatomy.get("arm_ratio", 0.5f);
        p.anatomy.leg_ratio = anatomy.get("leg_ratio", 0.5f);
    }

    if (dict.has("appearance")) {
        Dictionary app = dict["appearance"];
        p.appearance.eye_shape = static_cast<uint8_t>(static_cast<int>(app.get("eye_shape", 0)));
        p.appearance.eye_spacing = static_cast<uint8_t>(static_cast<int>(app.get("eye_spacing", 1)));
        p.appearance.nose_shape = static_cast<uint8_t>(static_cast<int>(app.get("nose_shape", 0)));
        p.appearance.mouth_shape = static_cast<uint8_t>(static_cast<int>(app.get("mouth_shape", 0)));
        p.appearance.hair_style = static_cast<uint8_t>(static_cast<int>(app.get("hair_style", 0)));
        p.appearance.expression = static_cast<npc::Expression>(static_cast<int>(app.get("expression", 0)));

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

        p.appearance.skin_color = to_color(app.get("skin_color", Variant()), p.appearance.skin_color);
        p.appearance.eye_color = to_color(app.get("eye_color", Variant()), p.appearance.eye_color);
        p.appearance.hair_color = to_color(app.get("hair_color", Variant()), p.appearance.hair_color);
        p.appearance.shirt_color = to_color(app.get("shirt_color", Variant()), p.appearance.shirt_color);
        p.appearance.pants_color = to_color(app.get("pants_color", Variant()), p.appearance.pants_color);
    }

    if (dict.has("physical")) {
        Dictionary physical = dict["physical"];
        if (physical.has("amputations")) {
            Array amps = physical["amputations"];
            for (int i = 0; i < amps.size(); ++i) {
                int part_idx = amps[i];
                if (part_idx >= 0 && part_idx < static_cast<int>(npc::BodyPart::Count)) {
                    p.physical.conditions.amputations[part_idx] = true;
                }
            }
        }
        if (physical.has("injuries")) {
            Array injs = physical["injuries"];
            for (int i = 0; i < injs.size(); ++i) {
                Dictionary d_inj = injs[i];
                int body_part = d_inj.get("body_part", 1);
                int inj_type = d_inj.get("type", 0);
                float severity = d_inj.get("severity", 0.5f);

                npc::InjuryCondition inj;
                inj.condition_id = static_cast<uint32_t>(d_inj.get("condition_id", i + 1));
                inj.body_part = static_cast<npc::BodyPart>(body_part);
                inj.type = static_cast<npc::ConditionType>(inj_type);
                inj.severity = severity;
                inj.visual_seed = static_cast<uint32_t>(d_inj.get("visual_seed", 0));
                inj.active = d_inj.get("active", true);
                inj.healed = d_inj.get("healed", false);
                p.physical.conditions.injuries.push_back(inj);
            }
        }
    }

    if (dict.has("body_geometry")) {
        Dictionary geo = dict["body_geometry"];
        p.body_geometry.shoulder_w = geo.get("shoulder_w", p.body_geometry.shoulder_w);
        p.body_geometry.waist_w = geo.get("waist_w", p.body_geometry.waist_w);
        p.body_geometry.hips_w = geo.get("hips_w", p.body_geometry.hips_w);
        p.body_geometry.thigh_thick = geo.get("thigh_thick", p.body_geometry.thigh_thick);
        p.body_geometry.neck_w = geo.get("neck_w", p.body_geometry.neck_w);
        p.body_geometry.arm_base_thick = geo.get("arm_base_thick", p.body_geometry.arm_base_thick);
        p.body_geometry.leg_gap = geo.get("leg_gap", p.body_geometry.leg_gap);
    } else {
        p.body_geometry = npc::compute_geometry(p.anatomy.body_mass, p.anatomy.muscularity);
    }

    return p;
}

Dictionary NPCGenerator::create_default_profile(int npc_id, uint64_t seed) {
    auto p = npc::CharacterProfile::create_default(static_cast<uint32_t>(npc_id), seed);
    return profile_to_dict(p);
}

Dictionary NPCGenerator::generate_profile(uint64_t seed, int npc_id) {
    auto p = npc::CharacterGenerator::generate_profile_from_seed(seed, static_cast<uint32_t>(npc_id));
    return profile_to_dict(p);
}

Dictionary NPCGenerator::validate_profile(const Dictionary& profile) {
    auto p = dict_to_profile(profile);
    auto res = npc::ProfileValidator::validate(p);

    Dictionary out;
    out["is_valid"] = res.is_ok();
    out["outcome"] = static_cast<int>(res.outcome);

    Array issues;
    for (const auto& issue : res.issues) {
        issues.push_back(String(issue.c_str()));
    }
    out["issues"] = issues;
    return out;
}

String NPCGenerator::serialize_profile(const Dictionary& profile) {
    auto p = dict_to_profile(profile);
    std::string s = npc::Serializer::serialize_profile(p);
    return String(s.c_str());
}

Dictionary NPCGenerator::deserialize_profile(const String& serialized_data) {
    std::string s = serialized_data.utf8().get_data();
    npc::CharacterProfile p;
    bool ok = npc::Serializer::deserialize_profile(s, p);

    Dictionary out;
    out["success"] = ok;
    if (ok) {
        out["profile"] = profile_to_dict(p);
    }
    return out;
}

Ref<Image> NPCGenerator::render_profile(
    const Dictionary& profile,
    int anim_state,
    int direction,
    float anim_time
) {
    auto p = dict_to_profile(profile);

    // Validate profile before rendering (FR-03: reject invalid profile before calling renderer)
    auto val_res = npc::ProfileValidator::validate(p);
    if (!val_res.is_ok()) {
        UtilityFunctions::push_error("NPCGenerator::render_profile: invalid profile rejected before rendering");
        return Ref<Image>();
    }

    npc::AnimationCommand cmd;
    cmd.state = static_cast<npc::AnimationState>(std::clamp(anim_state, 0, 5));
    cmd.direction = static_cast<npc::Direction>(std::clamp(direction, 0, 3));
    cmd.anim_time = anim_time;
    cmd.speed = 1.0f;

    auto buf = npc::CharacterGenerator::render_profile(p, cmd);

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
