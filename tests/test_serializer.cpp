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

int main() {
    std::cout << "Running Serializer verification tests...\n";
    test_serializer_roundtrip();
    test_serializer_corrupt_data_rejected();

    if (npc::test::failure_count() > 0) {
        std::cerr << "Serializer tests FAILED with " << npc::test::failure_count() << " error(s).\n";
        return 1;
    }

    std::cout << "All Serializer tests passed successfully!\n";
    return 0;
}
