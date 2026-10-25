#include "npc/profile.hpp"
#include "npc/generator.hpp"
#include "npc/serializer.hpp"
#include "npc/c_api.h"
#include "test_framework.hpp"
#include <iostream>
#include <limits>

void test_profile_default_and_validation() {
    std::cout << "Testing default CharacterProfile construction and validation...\n";
    auto p = npc::CharacterProfile::create_default(42, 99999);

    TEST_ASSERT_EQ(p.schema_version, npc::PROFILE_SCHEMA_VERSION);
    TEST_ASSERT_EQ(p.generation.npc_id, 42u);
    TEST_ASSERT_EQ(p.generation.seed, 99999u);
    TEST_ASSERT_EQ(p.generation.generator_version, 1u);

    // Anatomy defaults
    TEST_ASSERT_EQ(p.anatomy.height, 0.5f);
    TEST_ASSERT_EQ(p.anatomy.body_mass, 0.5f);
    TEST_ASSERT_EQ(p.anatomy.muscularity, 0.5f);
    TEST_ASSERT_EQ(p.anatomy.posture, 0.0f);

    // Validate default profile
    auto res = npc::ProfileValidator::validate(p);
    TEST_ASSERT(res.is_ok());
    TEST_ASSERT(res.issues.empty());

    // Domain separation test: modifying anatomy does not touch appearance or physical
    p.anatomy.height = 0.9f;
    TEST_ASSERT_EQ(static_cast<int>(p.appearance.expression), static_cast<int>(npc::Expression::Neutral));
    TEST_ASSERT(p.physical.conditions.injuries.empty());

    // Invalid schema
    p.schema_version = 999;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());
    p.schema_version = npc::PROFILE_SCHEMA_VERSION;

    // Out of bounds anatomy
    p.anatomy.height = 1.5f;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());

    // Normalization test
    auto norm_res = npc::ProfileValidator::normalize(p);
    TEST_ASSERT(norm_res.is_ok());
    TEST_ASSERT_EQ(p.anatomy.height, 1.0f);
    TEST_ASSERT(norm_res.outcome == npc::ValidationOutcome::ValidNormalized);

    // NaN / Inf check
    p.anatomy.height = std::numeric_limits<float>::quiet_NaN();
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());
    TEST_ASSERT(!npc::ProfileValidator::normalize(p).is_ok());

    std::cout << "[PASS] Default profile creation, validation, and normalization verified.\n";
}

void test_profile_enum_and_consistency_validation() {
    std::cout << "Testing ProfileValidator facial enums, amputation consistency, and injury rules...\n";
    auto p = npc::CharacterProfile::create_default(1, 12345);

    // BodyGeometry validation bounds
    p.body_geometry.shoulder_w = 99;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());
    p.body_geometry.shoulder_w = 11;

    p.body_geometry.thigh_thick = 9;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());
    p.body_geometry.thigh_thick = 3;

    // Appearance enums
    p.appearance.hair_style = 4;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());
    p.appearance.hair_style = 0;

    p.appearance.eye_shape = 3;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());
    p.appearance.eye_shape = 0;

    p.appearance.eye_spacing = 0;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());
    p.appearance.eye_spacing = 3;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());
    p.appearance.eye_spacing = 1;

    p.appearance.nose_shape = 2;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());
    p.appearance.nose_shape = 0;

    p.appearance.mouth_shape = 2;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());
    p.appearance.mouth_shape = 0;

    p.appearance.expression = static_cast<npc::Expression>(99);
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());
    p.appearance.expression = npc::Expression::Neutral;

    // Amputation consistency: Head and Torso cannot be amputated
    p.physical.conditions.amputations[static_cast<size_t>(npc::BodyPart::Head)] = true;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());
    p.physical.conditions.amputations[static_cast<size_t>(npc::BodyPart::Head)] = false;

    p.physical.conditions.amputations[static_cast<size_t>(npc::BodyPart::Torso)] = true;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());
    p.physical.conditions.amputations[static_cast<size_t>(npc::BodyPart::Torso)] = false;

    // Cascade violations: arm without hand, leg without foot
    p.physical.conditions.amputations[static_cast<size_t>(npc::BodyPart::LeftArm)] = true;
    p.physical.conditions.amputations[static_cast<size_t>(npc::BodyPart::LeftHand)] = false;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());

    p.physical.conditions.amputations[static_cast<size_t>(npc::BodyPart::LeftHand)] = true;
    TEST_ASSERT(npc::ProfileValidator::validate(p).is_ok());

    p.physical.conditions.amputations[static_cast<size_t>(npc::BodyPart::RightLeg)] = true;
    p.physical.conditions.amputations[static_cast<size_t>(npc::BodyPart::RightFoot)] = false;
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());

    p.physical.conditions.amputations[static_cast<size_t>(npc::BodyPart::RightFoot)] = true;
    TEST_ASSERT(npc::ProfileValidator::validate(p).is_ok());

    // Injury on amputated body part
    npc::InjuryCondition inj;
    inj.condition_id = 1;
    inj.body_part = npc::BodyPart::LeftArm; // LeftArm is amputated!
    inj.type = npc::ConditionType::Cut;
    inj.severity = 0.5f;
    p.physical.conditions.injuries.push_back(inj);
    TEST_ASSERT(!npc::ProfileValidator::validate(p).is_ok());

    // Move injury to valid part
    p.physical.conditions.injuries[0].body_part = npc::BodyPart::Torso;
    TEST_ASSERT(npc::ProfileValidator::validate(p).is_ok());

    // Normalization repairs cascade and removes injury on amputated part
    auto bad_p = npc::CharacterProfile::create_default(2, 54321);
    bad_p.appearance.hair_style = 9;
    bad_p.appearance.eye_spacing = 5;
    bad_p.physical.conditions.amputations[static_cast<size_t>(npc::BodyPart::Head)] = true;
    bad_p.physical.conditions.amputations[static_cast<size_t>(npc::BodyPart::RightArm)] = true; // RightHand false
    bad_p.physical.conditions.injuries.push_back({10, npc::BodyPart::RightArm, npc::ConditionType::Burn, 1.5f, 0, true, false});

    auto norm_res = npc::ProfileValidator::normalize(bad_p);
    TEST_ASSERT(norm_res.is_ok());
    TEST_ASSERT(norm_res.outcome == npc::ValidationOutcome::ValidNormalized);
    TEST_ASSERT_EQ(bad_p.appearance.hair_style, 3);
    TEST_ASSERT_EQ(bad_p.appearance.eye_spacing, 2);
    TEST_ASSERT(!bad_p.physical.conditions.is_amputated(npc::BodyPart::Head));
    TEST_ASSERT(bad_p.physical.conditions.is_amputated(npc::BodyPart::RightArm));
    TEST_ASSERT(bad_p.physical.conditions.is_amputated(npc::BodyPart::RightHand));
    TEST_ASSERT(bad_p.physical.conditions.injuries.empty()); // Removed impossible injury

    std::cout << "[PASS] Facial enums, amputation consistency, and injury rules validated.\n";
}

void test_profile_roundtrip_serialization() {
    std::cout << "Testing CharacterProfile V3 serialization round-trip for 100 seeds...\n";

    for (uint64_t seed = 1; seed <= 100; ++seed) {
        auto p1 = npc::CharacterGenerator::generate_profile_from_seed(seed, static_cast<uint32_t>(seed));
        
        // Add sample physical conditions to odd seeds
        if (seed % 2 == 1) {
            p1.physical.conditions.set_amputated(npc::BodyPart::LeftArm, true);
            npc::InjuryCondition inj;
            inj.condition_id = 101;
            inj.body_part = npc::BodyPart::Torso;
            inj.type = npc::ConditionType::Cut;
            inj.severity = 0.75f;
            inj.visual_seed = 42;
            inj.active = true;
            inj.healed = false;
            p1.physical.conditions.injuries.push_back(inj);
        }

        std::string serialized = npc::Serializer::serialize_profile(p1);
        npc::CharacterProfile p2;
        bool ok = npc::Serializer::deserialize_profile(serialized, p2);
        TEST_ASSERT(ok);

        TEST_ASSERT(p1 == p2);

        // Verify rendered pixels match identically
        npc::AnimationCommand cmd;
        cmd.direction = npc::Direction::South;
        cmd.state = npc::AnimationState::Idle;
        auto buf1 = npc::CharacterGenerator::render_profile(p1, cmd);
        auto buf2 = npc::CharacterGenerator::render_profile(p2, cmd);
        TEST_ASSERT_EQ(buf1.pixels.size(), buf2.pixels.size());
        for (size_t i = 0; i < buf1.pixels.size(); ++i) {
            TEST_ASSERT(buf1.pixels[i] == buf2.pixels[i]);
        }
    }

    std::cout << "[PASS] 100 of 100 CharacterProfiles preserved round-trip with render parity.\n";
}

void test_profile_legacy_v2_migration() {
    std::cout << "Testing backward compatibility: importing legacy V2 format into CharacterProfile...\n";
    auto legacy_app = npc::CharacterGenerator::generate_from_seed(12345, 1);
    std::string legacy_serialized = npc::Serializer::serialize(legacy_app);

    npc::CharacterProfile p;
    bool ok = npc::Serializer::deserialize_profile(legacy_serialized, p);
    TEST_ASSERT(ok);

    TEST_ASSERT_EQ(p.generation.npc_id, legacy_app.npc_id);
    TEST_ASSERT_EQ(p.generation.seed, legacy_app.appearance_seed);
    TEST_ASSERT_EQ(p.anatomy.height, legacy_app.body.height);
    TEST_ASSERT_EQ(p.appearance.eye_shape, legacy_app.face.eye_shape);
    TEST_ASSERT(p.physical.conditions.injuries.empty());

    std::cout << "[PASS] Legacy V2 string safely migrated to valid CharacterProfile.\n";

    // Test Format V3 migration into CharacterProfile V4
    std::string v3_data = "3 1 10 54321 1 0.5 0.5 0.5 0.0 0.5 0.5 0.5 0.5 0 1 0 0 0 0 225 185 150 255 40 30 20 255 50 40 30 255 60 90 140 255 45 50 60 255 0 0 0 0 0 0 0 0 0 0 0";
    npc::CharacterProfile p_v3;
    bool ok_v3 = npc::Serializer::deserialize_profile(v3_data, p_v3);
    TEST_ASSERT(ok_v3);
    TEST_ASSERT_EQ(p_v3.schema_version, 2u);
    TEST_ASSERT(p_v3.body_geometry.shoulder_w > 0);
    std::cout << "[PASS] Legacy Format V3 string safely migrated to CharacterProfile V4 with auto-computed geometry.\n";
}

void test_c_api_profile_functions() {
    std::cout << "Testing C API CharacterProfile functions...\n";
    CNpcProfile c_prof{};
    c_prof.struct_size = sizeof(CNpcProfile);

    int err = npc_profile_create_default(10, 54321, &c_prof);
    TEST_ASSERT_EQ(err, NPC_SUCCESS);
    TEST_ASSERT_EQ(c_prof.npc_id, 10u);
    TEST_ASSERT_EQ(c_prof.seed, 54321u);
    TEST_ASSERT_EQ(c_prof.api_version, 4u);
    TEST_ASSERT_EQ(c_prof.schema_version, 2u);
    TEST_ASSERT(c_prof.geo_shoulder_w > 0);
    TEST_ASSERT(c_prof.geo_waist_w > 0);

    // Old struct size rejected with NPC_ERR_VERSION_MISMATCH
    CNpcProfile old_sized_prof{};
    old_sized_prof.struct_size = sizeof(CNpcProfile) - 14; // missing 7x int16_t
    TEST_ASSERT_EQ(npc_profile_validate(&old_sized_prof), NPC_ERR_VERSION_MISMATCH);

    // Forward compatibility: future larger struct size must succeed
    CNpcProfile larger_prof = c_prof;
    larger_prof.struct_size = sizeof(CNpcProfile) + 32;
    TEST_ASSERT_EQ(npc_profile_validate(&larger_prof), NPC_SUCCESS);

    // Validate valid C profile
    TEST_ASSERT_EQ(npc_profile_validate(&c_prof), NPC_SUCCESS);

    // Corrupt and re-validate
    c_prof.height = 99.0f;
    TEST_ASSERT_EQ(npc_profile_validate(&c_prof), NPC_ERR_VALIDATION_FAIL);
    c_prof.height = 0.5f;

    // Corrupt geo_* fields with mismatching values on input
    c_prof.geo_shoulder_w = 999;
    c_prof.geo_waist_w = -50;

    // Render via C-API profile: must succeed and ignore geo_* input (body_geometry is purely derived)
    std::vector<uint8_t> buffer(32 * 48 * 4, 0);
    err = npc_profile_render(&c_prof, nullptr, 0, 0, 0, 0.0f, buffer.data(), buffer.size());
    TEST_ASSERT_EQ(err, NPC_SUCCESS);

    // Compare with direct C++ render
    auto p = npc::CharacterProfile::create_default(10, 54321);
    npc::AnimationCommand cmd;
    auto direct_buf = npc::CharacterGenerator::render_profile(p, cmd);
    for (size_t i = 0; i < direct_buf.pixels.size(); ++i) {
        TEST_ASSERT_EQ(buffer[i * 4 + 0], direct_buf.pixels[i].r);
        TEST_ASSERT_EQ(buffer[i * 4 + 1], direct_buf.pixels[i].g);
        TEST_ASSERT_EQ(buffer[i * 4 + 2], direct_buf.pixels[i].b);
        TEST_ASSERT_EQ(buffer[i * 4 + 3], direct_buf.pixels[i].a);
    }

    std::cout << "[PASS] C API CharacterProfile functions pass with pixel-exact parity and ignored geo_* input.\n";
}

void test_c_api_pre_render_validation_and_errors() {
    std::cout << "Testing C API pre-render validation hardening (FR-03)...\n";
    CNpcProfile c_prof{};
    c_prof.struct_size = sizeof(CNpcProfile);
    int err = npc_profile_create_default(1, 12345, &c_prof);
    TEST_ASSERT_EQ(err, NPC_SUCCESS);

    std::vector<uint8_t> buffer(32 * 48 * 4, 0xAA); // Fill sentinel
    const std::vector<uint8_t> sentinel(32 * 48 * 4, 0xAA);

    // 1. Invalid anatomy (NaN) -> rejected before render, buffer untouched
    c_prof.height = std::numeric_limits<float>::quiet_NaN();
    err = npc_profile_render(&c_prof, nullptr, 0, 0, 0, 0.0f, buffer.data(), buffer.size());
    TEST_ASSERT_EQ(err, NPC_ERR_VALIDATION_FAIL);
    TEST_ASSERT(buffer == sentinel);
    c_prof.height = 0.5f;

    // 2. Invalid facial enum (hair_style = 9) -> rejected before render
    c_prof.hair_style = 9;
    err = npc_profile_render(&c_prof, nullptr, 0, 0, 0, 0.0f, buffer.data(), buffer.size());
    TEST_ASSERT_EQ(err, NPC_ERR_VALIDATION_FAIL);
    TEST_ASSERT(buffer == sentinel);
    c_prof.hair_style = 0;

    // 3. Invalid amputation (Head amputated) -> rejected before render
    c_prof.amputations_mask[static_cast<size_t>(npc::BodyPart::Head)] = 1;
    err = npc_profile_render(&c_prof, nullptr, 0, 0, 0, 0.0f, buffer.data(), buffer.size());
    TEST_ASSERT_EQ(err, NPC_ERR_VALIDATION_FAIL);
    TEST_ASSERT(buffer == sentinel);
    c_prof.amputations_mask[static_cast<size_t>(npc::BodyPart::Head)] = 0;

    // 4. Invalid amputation cascade (LeftArm amputated, LeftHand not) -> rejected
    c_prof.amputations_mask[static_cast<size_t>(npc::BodyPart::LeftArm)] = 1;
    c_prof.amputations_mask[static_cast<size_t>(npc::BodyPart::LeftHand)] = 0;
    err = npc_profile_render(&c_prof, nullptr, 0, 0, 0, 0.0f, buffer.data(), buffer.size());
    TEST_ASSERT_EQ(err, NPC_ERR_VALIDATION_FAIL);
    TEST_ASSERT(buffer == sentinel);

    // Fix cascade
    c_prof.amputations_mask[static_cast<size_t>(npc::BodyPart::LeftHand)] = 1;

    // 5. Injury on amputated limb -> rejected
    CNpcInjury inj{};
    inj.condition_id = 99;
    inj.body_part = static_cast<uint8_t>(npc::BodyPart::LeftArm);
    inj.type = static_cast<uint8_t>(npc::ConditionType::Cut);
    inj.severity = 0.5f;
    inj.active = 1;
    err = npc_profile_render(&c_prof, &inj, 1, 0, 0, 0.0f, buffer.data(), buffer.size());
    TEST_ASSERT_EQ(err, NPC_ERR_VALIDATION_FAIL);
    TEST_ASSERT(buffer == sentinel);

    // 6. Null injuries pointer with count > 0 -> NPC_ERR_INVALID_ARGUMENT
    err = npc_profile_render(&c_prof, nullptr, 5, 0, 0, 0.0f, buffer.data(), buffer.size());
    TEST_ASSERT_EQ(err, NPC_ERR_INVALID_ARGUMENT);

    // 7. Move injury to non-amputated Torso -> success, buffer modified
    inj.body_part = static_cast<uint8_t>(npc::BodyPart::Torso);
    err = npc_profile_render(&c_prof, &inj, 1, 0, 0, 0.0f, buffer.data(), buffer.size());
    TEST_ASSERT_EQ(err, NPC_SUCCESS);
    TEST_ASSERT(buffer != sentinel);

    std::cout << "[PASS] C API pre-render validation thoroughly verified.\n";
}

int main() {
    std::cout << "Running CharacterProfile unit tests (v0.3.2)...\n";
    test_profile_default_and_validation();
    test_profile_enum_and_consistency_validation();
    test_profile_roundtrip_serialization();
    test_profile_legacy_v2_migration();
    test_c_api_profile_functions();
    test_c_api_pre_render_validation_and_errors();

    if (npc::test::failure_count() > 0) {
        std::cerr << "Profile tests FAILED with " << npc::test::failure_count() << " error(s).\n";
        return 1;
    }

    std::cout << "All CharacterProfile tests passed successfully!\n";
    return 0;
}
