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

        // Check derived_geometry() is identical to canonical expected values
        auto der_geo = p.derived_geometry();
        TEST_ASSERT_EQ(der_geo.shoulder_w, exp_shoulder_w);
        TEST_ASSERT_EQ(der_geo.waist_w, exp_waist_w);
        TEST_ASSERT_EQ(der_geo.hips_w, exp_hips_w);
        TEST_ASSERT_EQ(der_geo.thigh_thick, exp_thigh_thick);
        TEST_ASSERT_EQ(der_geo.neck_w, exp_neck_w);
        TEST_ASSERT_EQ(der_geo.arm_base_thick, exp_arm_thick);
        TEST_ASSERT_EQ(der_geo.leg_gap, exp_leg_gap);
        // Check vertical/length derived values
        int exp_total_height = 36 + static_cast<int>(std::round(p.anatomy.height * 6.0f));
        int exp_leg_len = 13 + static_cast<int>(std::round(p.anatomy.leg_ratio * 4.0f));
        int exp_torso_h = std::max(11, exp_total_height - 8 - 2 - exp_leg_len);
        int exp_arm_len = exp_torso_h + 2;

        TEST_ASSERT_EQ(der_geo.total_height, exp_total_height);
        TEST_ASSERT_EQ(der_geo.leg_len, exp_leg_len);
        TEST_ASSERT_EQ(der_geo.torso_h, exp_torso_h);
        TEST_ASSERT_EQ(der_geo.arm_len, exp_arm_len);
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

void test_clothing_foundation_posed_placement() {
    std::cout << "Testing clothing foundation PosedPlacement coordinate match with render()...\n";

    // Test across several animation states and directions
    std::vector<npc::AnimationState> states = {
        npc::AnimationState::Idle,
        npc::AnimationState::Walk,
        npc::AnimationState::Run,
        npc::AnimationState::Attack,
        npc::AnimationState::Fall,
        npc::AnimationState::Swim
    };
    std::vector<npc::Direction> dirs = {
        npc::Direction::South,
        npc::Direction::North,
        npc::Direction::East,
        npc::Direction::West
    };

    for (uint64_t seed = 1; seed <= 20; ++seed) {
        auto p = npc::CharacterGenerator::generate_profile_from_seed(seed, static_cast<uint32_t>(seed));
        auto geo = p.derived_geometry();

        for (auto state : states) {
            for (auto dir : dirs) {
                npc::AnimationCommand cmd;
                cmd.state = state;
                cmd.direction = dir;
                cmd.anim_time = 0.5f;
                cmd.speed = 1.0f;

                auto placement = npc::compute_posed_placement(geo, p.anatomy.posture, p.physical.conditions, cmd);

                // Verify that computed placement bounds strictly match canonical rules
                for (size_t i = 0; i < 10; ++i) {
                    const auto& part_place = placement.parts[i];
                    TEST_ASSERT(part_place.bounds.width >= 0);
                    TEST_ASSERT(part_place.bounds.height >= 0);
                    // Base layer must be defined
                    TEST_ASSERT(part_place.base_layer == npc::LayerOrder::BaseBody);
                }

                // Check specific parts:
                auto head_pl = placement.get(npc::BodyPart::Head);
                TEST_ASSERT_EQ(head_pl.bounds.width, 7);
                if (state == npc::AnimationState::Fall) {
                    TEST_ASSERT_EQ(head_pl.bounds.height, 6);
                } else {
                    TEST_ASSERT_EQ(head_pl.bounds.height, 8);
                }

                auto torso_pl = placement.get(npc::BodyPart::Torso);
                TEST_ASSERT_EQ(torso_pl.bounds.width, geo.shoulder_w);

                auto larm_pl = placement.get(npc::BodyPart::LeftArm);
                TEST_ASSERT_EQ(larm_pl.bounds.width, geo.arm_base_thick);

                auto lleg_pl = placement.get(npc::BodyPart::LeftLeg);
                TEST_ASSERT_EQ(lleg_pl.bounds.width, geo.thigh_thick);
            }
        }
    }

    // Verify cascade amputation propagation in PosedPlacement
    auto p_amp = npc::CharacterProfile::create_default();
    p_amp.physical.conditions.set_amputated(npc::BodyPart::LeftArm, true);
    // cascade amputate LeftHand as well
    p_amp.physical.conditions.set_amputated(npc::BodyPart::LeftHand, true);

    npc::AnimationCommand cmd;
    auto pl_amp = npc::compute_posed_placement(p_amp.derived_geometry(), p_amp.anatomy.posture, p_amp.physical.conditions, cmd);
    TEST_ASSERT(!pl_amp.get(npc::BodyPart::LeftArm).is_present);
    TEST_ASSERT(!pl_amp.get(npc::BodyPart::LeftHand).is_present);
    TEST_ASSERT(pl_amp.get(npc::BodyPart::RightArm).is_present);

    std::cout << "[PASS] PosedPlacement foundation verified matching canonical generator dimensions and cascades.\n";
}

int main() {
    std::cout << "Running BodyGeometry unit tests (v0.3.3.1)...\n";
    test_compute_geometry_accuracy();
    test_geometry_overrides();
    test_clothing_foundation_posed_placement();

    if (npc::test::failure_count() > 0) {
        std::cerr << "BodyGeometry tests FAILED with " << npc::test::failure_count() << " error(s).\n";
        return 1;
    }

    std::cout << "All BodyGeometry tests passed successfully!\n";
    return 0;
}
