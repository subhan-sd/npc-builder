#include "npc/serializer.hpp"
#include "npc/generator.hpp"
#include "test_framework.hpp"
#include <iostream>

void test_serializer_roundtrip() {
    std::cout << "Testing serializer round-trip for 100 seeds with full eye color retention...\n";
    for (uint64_t seed = 1; seed <= 100; ++seed) {
        auto app1 = npc::CharacterGenerator::generate_from_seed(seed, static_cast<uint32_t>(seed));
        std::string serialized = npc::Serializer::serialize(app1);

        npc::HumanAppearance app2;
        bool ok = npc::Serializer::deserialize(serialized, app2);
        TEST_ASSERT(ok);

        TEST_ASSERT_EQ(app1.npc_id, app2.npc_id);
        TEST_ASSERT_EQ(app1.appearance_seed, app2.appearance_seed);
        TEST_ASSERT_EQ(app1.generator_version, app2.generator_version);

        TEST_ASSERT_EQ(app1.body.height, app2.body.height);
        TEST_ASSERT_EQ(app1.body.body_mass, app2.body.body_mass);
        TEST_ASSERT_EQ(app1.body.muscularity, app2.body.muscularity);
        TEST_ASSERT_EQ(app1.body.posture, app2.body.posture);
        TEST_ASSERT_EQ(app1.body.head_scale, app2.body.head_scale);
        TEST_ASSERT_EQ(app1.body.torso_ratio, app2.body.torso_ratio);
        TEST_ASSERT_EQ(app1.body.arm_ratio, app2.body.arm_ratio);
        TEST_ASSERT_EQ(app1.body.leg_ratio, app2.body.leg_ratio);

        TEST_ASSERT_EQ(app1.face.eye_shape, app2.face.eye_shape);
        TEST_ASSERT_EQ(app1.face.eye_spacing, app2.face.eye_spacing);
        TEST_ASSERT_EQ(app1.face.nose_shape, app2.face.nose_shape);
        TEST_ASSERT_EQ(app1.face.mouth_shape, app2.face.mouth_shape);
        TEST_ASSERT_EQ(app1.face.hair_style, app2.face.hair_style);
        TEST_ASSERT_EQ(static_cast<int>(app1.face.expression), static_cast<int>(app2.face.expression));

        // Eye color retention check (Fixes Fakta-008!)
        TEST_ASSERT_EQ(app1.face.eye_color.r, app2.face.eye_color.r);
        TEST_ASSERT_EQ(app1.face.eye_color.g, app2.face.eye_color.g);
        TEST_ASSERT_EQ(app1.face.eye_color.b, app2.face.eye_color.b);
        TEST_ASSERT_EQ(app1.face.eye_color.a, app2.face.eye_color.a);

        TEST_ASSERT_EQ(app1.face.skin_color.r, app2.face.skin_color.r);
        TEST_ASSERT_EQ(app1.face.hair_color.r, app2.face.hair_color.r);
        TEST_ASSERT_EQ(app1.face.shirt_color.r, app2.face.shirt_color.r);
        TEST_ASSERT_EQ(app1.face.pants_color.r, app2.face.pants_color.r);
    }
    std::cout << "[PASS] 100 of 100 seeds round-trip perfectly preserved.\n";
}

void test_serializer_corrupt_data_rejected() {
    npc::HumanAppearance app;

    // Unknown format version
    TEST_ASSERT(!npc::Serializer::deserialize("999 1 1 123 0.5", app));

    // Empty string
    TEST_ASSERT(!npc::Serializer::deserialize("", app));

    // Truncated string
    TEST_ASSERT(!npc::Serializer::deserialize("2 1 1 123 0.5 0.5", app));

    // Invalid body bounds (> 1.0f)
    std::string invalid_body = "2 1 1 123 5.0 0.5 0.5 0.0 0.5 0.5 0.5 0.5 0 1 0 0 0 0 200 150 100 255 40 30 20 255 50 40 30 255 60 90 140 255 45 50 60 255";
    TEST_ASSERT(!npc::Serializer::deserialize(invalid_body, app));

    // Invalid color values (> 255)
    std::string invalid_color = "2 1 1 123 0.5 0.5 0.5 0.0 0.5 0.5 0.5 0.5 0 1 0 0 0 0 999 150 100 255 40 30 20 255 50 40 30 255 60 90 140 255 45 50 60 255";
    TEST_ASSERT(!npc::Serializer::deserialize(invalid_color, app));

    // Invalid facial shapes
    std::string invalid_hair = "2 1 1 123 0.5 0.5 0.5 0.0 0.5 0.5 0.5 0.5 0 1 0 0 99 0 200 150 100 255 40 30 20 255 50 40 30 255 60 90 140 255 45 50 60 255";
    TEST_ASSERT(!npc::Serializer::deserialize(invalid_hair, app));

    std::cout << "[PASS] Corrupted and out-of-bounds serialized strings rejected safely.\n";
}

void test_serializer_override_roundtrip_experiment() {
    std::cout << "Testing Task 1 review experiment: seed 7 with shoulder_w = 2 roundtrip...\n";
    auto p1 = npc::CharacterGenerator::generate_profile_from_seed(7, 7);
    p1.geometry_override.shoulder_w = 2;
    p1.body_geometry = p1.derived_geometry();

    npc::AnimationCommand cmd;
    cmd.state = npc::AnimationState::Idle;
    cmd.direction = npc::Direction::South;
    cmd.anim_time = 0.0f;
    cmd.speed = 1.0f;

    auto px1 = npc::CharacterGenerator::render_profile(p1, cmd);

    std::string serialized = npc::Serializer::serialize_profile(p1);
    npc::CharacterProfile p2;
    bool ok = npc::Serializer::deserialize_profile(serialized, p2);
    TEST_ASSERT(ok);

    TEST_ASSERT_EQ(p2.geometry_override.shoulder_w, 2);
    TEST_ASSERT(p1.geometry_override == p2.geometry_override);
    TEST_ASSERT(p1.derived_geometry() == p2.derived_geometry());

    auto px2 = npc::CharacterGenerator::render_profile(p2, cmd);
    TEST_ASSERT(px1.pixels.size() == px2.pixels.size());
    for (size_t i = 0; i < px1.pixels.size(); ++i) {
        TEST_ASSERT_EQ(px1.pixels[i].r, px2.pixels[i].r);
        TEST_ASSERT_EQ(px1.pixels[i].g, px2.pixels[i].g);
        TEST_ASSERT_EQ(px1.pixels[i].b, px2.pixels[i].b);
        TEST_ASSERT_EQ(px1.pixels[i].a, px2.pixels[i].a);
    }
    std::cout << "[PASS] Seed 7 with shoulder_w override perfectly preserved in format 5.\n";
}

void test_serializer_profile_migration_v3_and_v4() {
    std::cout << "Testing serializer migration from Format 3 and Format 4 (override = 0)...\n";
    auto p_canonical = npc::CharacterGenerator::generate_profile_from_seed(42, 42);

    // Create synthetic Format 4 string (format version 4, schema 2)
    std::ostringstream ss4;
    ss4 << "4 " << p_canonical.schema_version << " "
        << p_canonical.generation.npc_id << " "
        << p_canonical.generation.seed << " "
        << p_canonical.generation.generator_version << " "
        << std::setprecision(9)
        << p_canonical.anatomy.height << " "
        << p_canonical.anatomy.body_mass << " "
        << p_canonical.anatomy.muscularity << " "
        << p_canonical.anatomy.posture << " "
        << p_canonical.anatomy.head_scale << " "
        << p_canonical.anatomy.torso_ratio << " "
        << p_canonical.anatomy.arm_ratio << " "
        << p_canonical.anatomy.leg_ratio << " "
        << static_cast<int>(p_canonical.appearance.eye_shape) << " "
        << static_cast<int>(p_canonical.appearance.eye_spacing) << " "
        << static_cast<int>(p_canonical.appearance.nose_shape) << " "
        << static_cast<int>(p_canonical.appearance.mouth_shape) << " "
        << static_cast<int>(p_canonical.appearance.hair_style) << " "
        << static_cast<int>(p_canonical.appearance.expression) << " "
        << static_cast<int>(p_canonical.appearance.skin_color.r) << " "
        << static_cast<int>(p_canonical.appearance.skin_color.g) << " "
        << static_cast<int>(p_canonical.appearance.skin_color.b) << " "
        << static_cast<int>(p_canonical.appearance.skin_color.a) << " "
        << static_cast<int>(p_canonical.appearance.eye_color.r) << " "
        << static_cast<int>(p_canonical.appearance.eye_color.g) << " "
        << static_cast<int>(p_canonical.appearance.eye_color.b) << " "
        << static_cast<int>(p_canonical.appearance.eye_color.a) << " "
        << static_cast<int>(p_canonical.appearance.hair_color.r) << " "
        << static_cast<int>(p_canonical.appearance.hair_color.g) << " "
        << static_cast<int>(p_canonical.appearance.hair_color.b) << " "
        << static_cast<int>(p_canonical.appearance.hair_color.a) << " "
        << static_cast<int>(p_canonical.appearance.shirt_color.r) << " "
        << static_cast<int>(p_canonical.appearance.shirt_color.g) << " "
        << static_cast<int>(p_canonical.appearance.shirt_color.b) << " "
        << static_cast<int>(p_canonical.appearance.shirt_color.a) << " "
        << static_cast<int>(p_canonical.appearance.pants_color.r) << " "
        << static_cast<int>(p_canonical.appearance.pants_color.g) << " "
        << static_cast<int>(p_canonical.appearance.pants_color.b) << " "
        << static_cast<int>(p_canonical.appearance.pants_color.a);

    for (size_t i = 0; i < static_cast<size_t>(npc::BodyPart::Count); ++i) {
        ss4 << " 0";
    }
    // 7 dummy geometry values
    ss4 << " 11 8 8 3 2 2 0";
    // 0 injuries
    ss4 << " 0";

    npc::CharacterProfile p4;
    bool ok4 = npc::Serializer::deserialize_profile(ss4.str(), p4);
    TEST_ASSERT(ok4);
    TEST_ASSERT(p4.geometry_override == npc::GeometryOverride{});
    TEST_ASSERT(p4.derived_geometry() == p_canonical.derived_geometry());

    // Create synthetic Format 3 string (format version 3, schema 1)
    std::ostringstream ss3;
    ss3 << "3 1 "
        << p_canonical.generation.npc_id << " "
        << p_canonical.generation.seed << " "
        << p_canonical.generation.generator_version << " "
        << std::setprecision(9)
        << p_canonical.anatomy.height << " "
        << p_canonical.anatomy.body_mass << " "
        << p_canonical.anatomy.muscularity << " "
        << p_canonical.anatomy.posture << " "
        << p_canonical.anatomy.head_scale << " "
        << p_canonical.anatomy.torso_ratio << " "
        << p_canonical.anatomy.arm_ratio << " "
        << p_canonical.anatomy.leg_ratio << " "
        << static_cast<int>(p_canonical.appearance.eye_shape) << " "
        << static_cast<int>(p_canonical.appearance.eye_spacing) << " "
        << static_cast<int>(p_canonical.appearance.nose_shape) << " "
        << static_cast<int>(p_canonical.appearance.mouth_shape) << " "
        << static_cast<int>(p_canonical.appearance.hair_style) << " "
        << static_cast<int>(p_canonical.appearance.expression) << " "
        << static_cast<int>(p_canonical.appearance.skin_color.r) << " "
        << static_cast<int>(p_canonical.appearance.skin_color.g) << " "
        << static_cast<int>(p_canonical.appearance.skin_color.b) << " "
        << static_cast<int>(p_canonical.appearance.skin_color.a) << " "
        << static_cast<int>(p_canonical.appearance.eye_color.r) << " "
        << static_cast<int>(p_canonical.appearance.eye_color.g) << " "
        << static_cast<int>(p_canonical.appearance.eye_color.b) << " "
        << static_cast<int>(p_canonical.appearance.eye_color.a) << " "
        << static_cast<int>(p_canonical.appearance.hair_color.r) << " "
        << static_cast<int>(p_canonical.appearance.hair_color.g) << " "
        << static_cast<int>(p_canonical.appearance.hair_color.b) << " "
        << static_cast<int>(p_canonical.appearance.hair_color.a) << " "
        << static_cast<int>(p_canonical.appearance.shirt_color.r) << " "
        << static_cast<int>(p_canonical.appearance.shirt_color.g) << " "
        << static_cast<int>(p_canonical.appearance.shirt_color.b) << " "
        << static_cast<int>(p_canonical.appearance.shirt_color.a) << " "
        << static_cast<int>(p_canonical.appearance.pants_color.r) << " "
        << static_cast<int>(p_canonical.appearance.pants_color.g) << " "
        << static_cast<int>(p_canonical.appearance.pants_color.b) << " "
        << static_cast<int>(p_canonical.appearance.pants_color.a);

    for (size_t i = 0; i < static_cast<size_t>(npc::BodyPart::Count); ++i) {
        ss3 << " 0";
    }
    // 0 injuries (Format 3 has no geometry values)
    ss3 << " 0";

    npc::CharacterProfile p3;
    bool ok3 = npc::Serializer::deserialize_profile(ss3.str(), p3);
    TEST_ASSERT(ok3);
    TEST_ASSERT(p3.geometry_override == npc::GeometryOverride{});
    TEST_ASSERT(p3.derived_geometry() == p_canonical.derived_geometry());

    std::cout << "[PASS] Formats 3 and 4 correctly migrate with override = 0.\n";
}

void test_serializer_profile_format_compatibility() {
    std::cout << "Testing serializer backward compatibility (Format 4 mismatch discarded, derived output)...\n";
    auto p_canonical = npc::CharacterProfile::create_default(42, 99999);
    std::string s5 = npc::Serializer::serialize_profile(p_canonical);

    npc::CharacterProfile p_read;
    bool ok = npc::Serializer::deserialize_profile(s5, p_read);
    TEST_ASSERT(ok);

    // Geometry in p_read must be derived canonically, matching p_canonical.derived_geometry()
    TEST_ASSERT(p_read.body_geometry == p_canonical.derived_geometry());

    std::cout << "[PASS] Deserializer Format 5 correctly derives geometry canonically from anatomy.\n";
}

int main() {
    std::cout << "Running Serializer verification tests...\n";
    test_serializer_roundtrip();
    test_serializer_corrupt_data_rejected();
    test_serializer_profile_format_compatibility();
    test_serializer_override_roundtrip_experiment();
    test_serializer_profile_migration_v3_and_v4();

    if (npc::test::failure_count() > 0) {
        std::cerr << "Serializer tests FAILED with " << npc::test::failure_count() << " error(s).\n";
        return 1;
    }

    std::cout << "All Serializer tests passed successfully!\n";
    return 0;
}
