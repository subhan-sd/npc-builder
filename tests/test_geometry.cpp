#include "npc/body_geometry.hpp"
#include "npc/profile.hpp"
#include "npc/generator.hpp"
#include "test_framework.hpp"
#include <iostream>

void test_compute_geometry_accuracy() {
    std::cout << "Testing compute_geometry against canonical formulas for 100 seeds...\n";
    for (uint64_t seed = 1; seed <= 100; ++seed) {
        auto p = npc::CharacterGenerator::generate_profile_from_seed(seed, static_cast<uint32_t>(seed));
        float mass = p.anatomy.body_mass;
        float musc = p.anatomy.muscularity;

        auto geo = npc::compute_geometry(mass, musc);

        int exp_neck_w = (musc > 0.6f || mass > 0.7f) ? 3 : 2;
        int exp_shoulder_w = 9 + static_cast<int>(std::round(musc * 4.0f + mass * 1.5f));
        int exp_waist_w = 6 + static_cast<int>(std::round(mass * 4.5f + (1.0f - musc) * 1.0f));
        if (musc > 0.5f && mass < 0.6f) {
            exp_waist_w = std::min(exp_waist_w, exp_shoulder_w - 4);
        }
        int exp_hips_w = std::max(exp_waist_w, 7 + static_cast<int>(std::round(mass * 3.5f)));
        int exp_leg_gap = (musc > 0.6f || mass < 0.35f) ? 1 : 0;
        int exp_thigh_thick = 3;
        if (mass > 0.7f && musc > 0.5f) exp_thigh_thick = 4;
        else if (mass < 0.3f && musc < 0.3f) exp_thigh_thick = 2;
        int exp_arm_thick = (musc > 0.65f || mass > 0.75f) ? 3 : 2;

        TEST_ASSERT_EQ(geo.neck_w, exp_neck_w);
        TEST_ASSERT_EQ(geo.shoulder_w, exp_shoulder_w);
        TEST_ASSERT_EQ(geo.waist_w, exp_waist_w);
        TEST_ASSERT_EQ(geo.hips_w, exp_hips_w);
        TEST_ASSERT_EQ(geo.leg_gap, exp_leg_gap);
        TEST_ASSERT_EQ(geo.thigh_thick, exp_thigh_thick);
        TEST_ASSERT_EQ(geo.arm_base_thick, exp_arm_thick);

        // Also check p.body_geometry is identical
        TEST_ASSERT_EQ(p.body_geometry.shoulder_w, exp_shoulder_w);
        TEST_ASSERT_EQ(p.body_geometry.waist_w, exp_waist_w);
        TEST_ASSERT_EQ(p.body_geometry.hips_w, exp_hips_w);
        TEST_ASSERT_EQ(p.body_geometry.thigh_thick, exp_thigh_thick);
        TEST_ASSERT_EQ(p.body_geometry.neck_w, exp_neck_w);
        TEST_ASSERT_EQ(p.body_geometry.arm_base_thick, exp_arm_thick);
        TEST_ASSERT_EQ(p.body_geometry.leg_gap, exp_leg_gap);
        // Check vertical/length derived values
        int exp_total_height = 36 + static_cast<int>(std::round(p.anatomy.height * 6.0f));
        int exp_leg_len = 13 + static_cast<int>(std::round(p.anatomy.leg_ratio * 4.0f));
        int exp_torso_h = std::max(11, exp_total_height - 8 - 2 - exp_leg_len);
        int exp_arm_len = exp_torso_h + 2;

        TEST_ASSERT_EQ(p.body_geometry.total_height, exp_total_height);
        TEST_ASSERT_EQ(p.body_geometry.leg_len, exp_leg_len);
        TEST_ASSERT_EQ(p.body_geometry.torso_h, exp_torso_h);
        TEST_ASSERT_EQ(p.body_geometry.arm_len, exp_arm_len);
    }
    std::cout << "[PASS] compute_geometry matches canonical formulas identically across 100 seeds.\n";
}

void test_geometry_overrides() {
    std::cout << "Testing GeometryOverride behavior and clamping...\n";
    auto p = npc::CharacterProfile::create_default();
    auto base_geo = p.derived_geometry();

    // 0 offset must produce identical geometry to formula
    p.geometry_override = npc::GeometryOverride{};
    TEST_ASSERT(p.derived_geometry() == base_geo);

    // Explicit override deviation within bounds
    p.geometry_override.shoulder_w = 2;
    p.geometry_override.torso_h = 1;
    auto mod_geo = p.derived_geometry();
    TEST_ASSERT_EQ(mod_geo.shoulder_w, base_geo.shoulder_w + 2);
    TEST_ASSERT_EQ(mod_geo.torso_h, base_geo.torso_h + 1);

    // Clamping to canonical bounds
    p.geometry_override.shoulder_w = 999;
    auto clamped_geo = p.derived_geometry();
    TEST_ASSERT_EQ(clamped_geo.shoulder_w, npc::GeometryBounds::SHOULDER_W_MAX);

    std::cout << "[PASS] GeometryOverride zero-identity, deviations, and clamping verified.\n";
}

int main() {
    std::cout << "Running BodyGeometry unit tests (v0.3.3.1)...\n";
    test_compute_geometry_accuracy();
    test_geometry_overrides();

    if (npc::test::failure_count() > 0) {
        std::cerr << "BodyGeometry tests FAILED with " << npc::test::failure_count() << " error(s).\n";
        return 1;
    }

    std::cout << "All BodyGeometry tests passed successfully!\n";
    return 0;
}
