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
    }
    std::cout << "[PASS] compute_geometry matches canonical formulas identically across 100 seeds.\n";
}

int main() {
    std::cout << "Running BodyGeometry unit tests (v0.3.3)...\n";
    test_compute_geometry_accuracy();

    if (npc::test::failure_count() > 0) {
        std::cerr << "BodyGeometry tests FAILED with " << npc::test::failure_count() << " error(s).\n";
        return 1;
    }

    std::cout << "All BodyGeometry tests passed successfully!\n";
    return 0;
}
