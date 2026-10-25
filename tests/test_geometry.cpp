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
    p.geometry_override.chest_curve = 2;
    auto mod_geo = p.derived_geometry();
    TEST_ASSERT_EQ(mod_geo.shoulder_w, base_geo.shoulder_w + 2);
    TEST_ASSERT_EQ(mod_geo.torso_h, base_geo.torso_h + 1);
    TEST_ASSERT_EQ(mod_geo.chest_curve, 2);

    // Clamping to canonical bounds
    p.geometry_override.shoulder_w = 999;
    p.geometry_override.chest_curve = 999;
    auto clamped_geo = p.derived_geometry();
    TEST_ASSERT_EQ(clamped_geo.shoulder_w, npc::GeometryBounds::SHOULDER_W_MAX);
    TEST_ASSERT_EQ(clamped_geo.chest_curve, npc::GeometryBounds::CHEST_CURVE_MAX);

    // Stale body_geometry cache ignored: derived_geometry() always recalculates
    p.geometry_override.chest_curve = 3;
    p.body_geometry.chest_curve = 0; // Stale cached value
    TEST_ASSERT_EQ(p.derived_geometry().chest_curve, 3);

    // Reset override
    p.geometry_override = npc::GeometryOverride{};

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

void test_posed_placement_differential_against_render() {
    std::cout << "Testing differential pixel verification: PosedPlacement vs render()...\n";

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
    std::vector<float> anim_times = {0.0f, 0.1f, 0.35f, 0.5f};

    // Profiles: seeds 1..20 plus boundary profiles (extreme fat/muscular/tall/hunched)
    std::vector<npc::CharacterProfile> test_profiles;
    test_profiles.reserve(25);
    for (uint64_t seed = 1; seed <= 20; ++seed) {
        test_profiles.push_back(npc::CharacterGenerator::generate_profile_from_seed(seed, static_cast<uint32_t>(seed)));
    }
    // Boundary 1: Max bulk & height
    {
        auto p = npc::CharacterProfile::create_default(101, 101);
        p.anatomy.height = 1.0f;
        p.anatomy.body_mass = 1.0f;
        p.anatomy.muscularity = 1.0f;
        p.anatomy.posture = 1.0f;
        p.anatomy.leg_ratio = 1.0f;
        p.body_geometry = p.derived_geometry();
        test_profiles.push_back(p);
    }
    // Boundary 2: Min bulk & height
    {
        auto p = npc::CharacterProfile::create_default(102, 102);
        p.anatomy.height = 0.0f;
        p.anatomy.body_mass = 0.0f;
        p.anatomy.muscularity = 0.0f;
        p.anatomy.posture = 0.0f;
        p.anatomy.leg_ratio = 0.0f;
        p.body_geometry = p.derived_geometry();
        test_profiles.push_back(p);
    }

    int mismatch_count = 0;
    int tested_combinations = 0;

    for (const auto& base_profile : test_profiles) {
        auto geo = base_profile.derived_geometry();

        for (auto state : states) {
            for (auto dir : dirs) {
                for (float anim_time : anim_times) {
                    npc::AnimationCommand cmd;
                    cmd.state = state;
                    cmd.direction = dir;
                    cmd.anim_time = anim_time;
                    cmd.speed = 1.0f;

                    // Baseline render without injuries
                    auto baseline_buf = npc::CharacterGenerator::render_profile(base_profile, cmd);
                    auto placement = npc::compute_posed_placement(geo, base_profile.anatomy.posture, base_profile.physical.conditions, cmd);

                    for (size_t part_idx = 0; part_idx < static_cast<size_t>(npc::BodyPart::Count); ++part_idx) {
                        auto part = static_cast<npc::BodyPart>(part_idx);
                        const auto& part_placement = placement.get(part);

                        // 1. Single injury test on this body part (severity 0.3f so no width expansion beyond 1px)
                        auto inj_profile = base_profile;
                        npc::InjuryCondition inj;
                        inj.condition_id = 777;
                        inj.body_part = part;
                        inj.type = npc::ConditionType::Cut;
                        inj.severity = 0.3f;
                        inj.visual_seed = 12345;
                        inj.active = true;
                        inj.healed = false;
                        inj_profile.physical.conditions.injuries.push_back(inj);

                        auto inj_buf = npc::CharacterGenerator::render_profile(inj_profile, cmd);
                        tested_combinations++;

                        // Find all pixels that changed from baseline
                        for (int y = 0; y < npc::CharacterGenerator::CANVAS_HEIGHT; ++y) {
                            for (int x = 0; x < npc::CharacterGenerator::CANVAS_WIDTH; ++x) {
                                auto p_base = baseline_buf.get_pixel(x, y);
                                auto p_inj = inj_buf.get_pixel(x, y);
                                if (p_base != p_inj) {
                                    // Verify changed pixel is within PosedPlacement bounds (x and y)
                                    int min_x = part_placement.bounds.x;
                                    int max_x = part_placement.bounds.x + part_placement.bounds.width - 1;
                                    int min_y = part_placement.bounds.y;
                                    int max_y = part_placement.bounds.y + part_placement.bounds.height - 1;

                                    if (x < min_x || x > max_x || y < min_y || y > max_y) {
                                        mismatch_count++;
                                        std::cerr << "[MISMATCH] Part " << part_idx
                                                  << " state " << static_cast<int>(state)
                                                  << " dir " << static_cast<int>(dir)
                                                  << " t " << anim_time
                                                  << " pixel (" << x << "," << y << ") out of bounds ["
                                                  << min_x << ".." << max_x << ", "
                                                  << min_y << ".." << max_y << "]\n";
                                    }
                                }
                            }
                        }

                        // 2. Amputation test: if part is amputated, is_present must be false and injury produces NO pixels
                        auto amp_profile = base_profile;
                        if (part != npc::BodyPart::Head && part != npc::BodyPart::Torso) {
                            amp_profile.physical.conditions.set_amputated(part, true);
                            if (part == npc::BodyPart::LeftArm) amp_profile.physical.conditions.set_amputated(npc::BodyPart::LeftHand, true);
                            if (part == npc::BodyPart::RightArm) amp_profile.physical.conditions.set_amputated(npc::BodyPart::RightHand, true);
                            if (part == npc::BodyPart::LeftLeg) amp_profile.physical.conditions.set_amputated(npc::BodyPart::LeftFoot, true);
                            if (part == npc::BodyPart::RightLeg) amp_profile.physical.conditions.set_amputated(npc::BodyPart::RightFoot, true);

                            auto amp_placement = npc::compute_posed_placement(geo, amp_profile.anatomy.posture, amp_profile.physical.conditions, cmd);
                            TEST_ASSERT(!amp_placement.get(part).is_present);

                            // Render amputated with injury on amputated part
                            auto amp_base_buf = npc::CharacterGenerator::render_profile(amp_profile, cmd);
                            amp_profile.physical.conditions.injuries.push_back(inj);
                            auto amp_inj_buf = npc::CharacterGenerator::render_profile(amp_profile, cmd);

                            // Invariant: zero pixels can differ when injury is on an amputated part
                            TEST_ASSERT(amp_base_buf.pixels == amp_inj_buf.pixels);
                        }
                    }
                }
            }
        }
    }

    TEST_ASSERT_EQ(mismatch_count, 0);
    std::cout << "[PASS] Differential testing verified 100% of injury pixels inside PosedPlacement bounds ("
              << tested_combinations << " combinations checked).\n";
}

static void test_proportions_torso_and_arm_ratio() {
    std::cout << "Testing torso_ratio and arm_ratio influences (v0.3.4)...\n";

    // 1. Long torso vs short torso at identical total height
    auto geo_baseline = npc::compute_full_geometry(0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f);
    auto geo_long_torso = npc::compute_full_geometry(0.5f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f);
    auto geo_short_torso = npc::compute_full_geometry(0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 0.5f);

    TEST_ASSERT_EQ(geo_long_torso.total_height, geo_baseline.total_height);
    TEST_ASSERT_EQ(geo_short_torso.total_height, geo_baseline.total_height);

    TEST_ASSERT(geo_long_torso.torso_h >= geo_baseline.torso_h);
    TEST_ASSERT(geo_long_torso.leg_len <= geo_baseline.leg_len);

    TEST_ASSERT(geo_short_torso.torso_h <= geo_baseline.torso_h);
    TEST_ASSERT(geo_short_torso.leg_len >= geo_baseline.leg_len);

    // 2. Arm ratio influence
    auto geo_long_arm = npc::compute_full_geometry(0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 1.0f);
    auto geo_short_arm = npc::compute_full_geometry(0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f);

    TEST_ASSERT(geo_long_arm.arm_len >= geo_baseline.arm_len);
    TEST_ASSERT(geo_short_arm.arm_len <= geo_baseline.arm_len);

    std::cout << "[PASS] Torso and arm ratios verified.\n";
}

int main() {
    std::cout << "Running BodyGeometry unit tests (v0.3.3.1)...\n";
    test_compute_geometry_accuracy();
    test_geometry_overrides();
    test_proportions_torso_and_arm_ratio();
    test_clothing_foundation_posed_placement();
    test_posed_placement_differential_against_render();

    if (npc::test::failure_count() > 0) {
        std::cerr << "BodyGeometry tests FAILED with " << npc::test::failure_count() << " error(s).\n";
        return 1;
    }

    std::cout << "All BodyGeometry tests passed successfully!\n";
    return 0;
}
