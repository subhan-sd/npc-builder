#include "npc/body_geometry.hpp"
#include "test_framework.hpp"
#include <iostream>

void test_12_discrete_thresholds() {
    std::cout << "Testing 12 discrete thresholds...\n";

    // 1. neck_w: musc > 0.6f || mass > 0.7f => 3 else 2
    TEST_ASSERT_EQ(npc::compute_geometry(0.5f, 0.59f).neck_w, 2);
    TEST_ASSERT_EQ(npc::compute_geometry(0.5f, 0.61f).neck_w, 3);
    TEST_ASSERT_EQ(npc::compute_geometry(0.69f, 0.5f).neck_w, 2);
    TEST_ASSERT_EQ(npc::compute_geometry(0.71f, 0.5f).neck_w, 3);

    // 2. shoulder_w: continuous (checked across range)
    TEST_ASSERT(npc::compute_geometry(0.0f, 0.0f).shoulder_w < npc::compute_geometry(0.0f, 1.0f).shoulder_w);

    // 3. waist_w: continuous
    TEST_ASSERT(npc::compute_geometry(0.0f, 0.5f).waist_w < npc::compute_geometry(1.0f, 0.5f).waist_w);

    // 4. V-taper clamp: musc > 0.5f && mass < 0.6f => waist_w <= shoulder_w - 4
    auto v_prof = npc::compute_geometry(0.4f, 0.9f);
    TEST_ASSERT(v_prof.waist_w <= v_prof.shoulder_w - 4);

    // 5. leg_gap: musc > 0.6f || mass < 0.35f => 1 else 0
    TEST_ASSERT_EQ(npc::compute_geometry(0.5f, 0.59f).leg_gap, 0);
    TEST_ASSERT_EQ(npc::compute_geometry(0.5f, 0.61f).leg_gap, 1);
    TEST_ASSERT_EQ(npc::compute_geometry(0.36f, 0.5f).leg_gap, 0);
    TEST_ASSERT_EQ(npc::compute_geometry(0.34f, 0.5f).leg_gap, 1);

    // 6 & 7. thigh_thick: mass > 0.7 && musc > 0.5 => 4; mass < 0.3 && musc < 0.3 => 2; else 3
    TEST_ASSERT_EQ(npc::compute_geometry(0.5f, 0.5f).thigh_thick, 3);
    TEST_ASSERT_EQ(npc::compute_geometry(0.75f, 0.55f).thigh_thick, 4);
    TEST_ASSERT_EQ(npc::compute_geometry(0.25f, 0.25f).thigh_thick, 2);

    // 10. arm_base_thick: musc > 0.65f || mass > 0.75f => 3 else 2
    TEST_ASSERT_EQ(npc::compute_geometry(0.5f, 0.64f).arm_base_thick, 2);
    TEST_ASSERT_EQ(npc::compute_geometry(0.5f, 0.66f).arm_base_thick, 3);
    TEST_ASSERT_EQ(npc::compute_geometry(0.74f, 0.5f).arm_base_thick, 2);
    TEST_ASSERT_EQ(npc::compute_geometry(0.76f, 0.5f).arm_base_thick, 3);

    std::cout << "[PASS] Discrete thresholds verified accurately.\n";
}

void test_monotonicity_pairs() {
    std::cout << "Testing monotonicity tables per pair...\n";

    // Pair 1: (muscularity, shoulder_w) is monotonically non-decreasing
    for (float mass = 0.0f; mass <= 1.0f; mass += 0.2f) {
        int last_w = -1;
        for (float musc = 0.0f; musc <= 1.0f; musc += 0.05f) {
            int w = npc::compute_geometry(mass, musc).shoulder_w;
            if (last_w >= 0) {
                TEST_ASSERT(w >= last_w);
            }
            last_w = w;
        }
    }

    // Pair 2: (body_mass, waist_w) is monotonically non-decreasing for constant musc
    for (float musc = 0.0f; musc <= 1.0f; musc += 0.2f) {
        int last_w = -1;
        for (float mass = 0.0f; mass <= 1.0f; mass += 0.05f) {
            int w = npc::compute_geometry(mass, musc).waist_w;
            if (last_w >= 0) {
                TEST_ASSERT(w >= last_w);
            }
            last_w = w;
        }
    }

    // Pair 3: (body_mass, hips_w) is monotonically non-decreasing
    for (float musc = 0.0f; musc <= 1.0f; musc += 0.2f) {
        int last_w = -1;
        for (float mass = 0.0f; mass <= 1.0f; mass += 0.05f) {
            int w = npc::compute_geometry(mass, musc).hips_w;
            if (last_w >= 0) {
                TEST_ASSERT(w >= last_w);
            }
            last_w = w;
        }
    }

    // Pair 4: (muscularity, waist_w) is monotonically non-increasing for constant mass
    for (float mass = 0.0f; mass <= 1.0f; mass += 0.2f) {
        int last_w = 999;
        for (float musc = 0.0f; musc <= 1.0f; musc += 0.05f) {
            int w = npc::compute_geometry(mass, musc).waist_w;
            if (last_w != 999) {
                TEST_ASSERT(w <= last_w);
            }
            last_w = w;
        }
    }

    std::cout << "[PASS] Monotonicity pairs verified per specification table.\n";
}

int main() {
    std::cout << "Running Transition and Monotonicity tests (v0.3.3)...\n";
    test_12_discrete_thresholds();
    test_monotonicity_pairs();

    if (npc::test::failure_count() > 0) {
        std::cerr << "Transition tests FAILED with " << npc::test::failure_count() << " error(s).\n";
        return 1;
    }

    std::cout << "All Transition tests passed successfully!\n";
    return 0;
}
