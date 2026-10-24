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
}

void test_c_api_profile_functions() {
    std::cout << "Testing C API CharacterProfile functions...\n";
    CNpcProfile c_prof{};
    c_prof.struct_size = sizeof(CNpcProfile);

    int err = npc_profile_create_default(10, 54321, &c_prof);
    TEST_ASSERT_EQ(err, NPC_SUCCESS);
    TEST_ASSERT_EQ(c_prof.npc_id, 10u);
    TEST_ASSERT_EQ(c_prof.seed, 54321u);

    // Validate valid C profile
    TEST_ASSERT_EQ(npc_profile_validate(&c_prof), NPC_SUCCESS);

    // Corrupt and re-validate
    c_prof.height = 99.0f;
    TEST_ASSERT_EQ(npc_profile_validate(&c_prof), NPC_ERR_VALIDATION_FAIL);
    c_prof.height = 0.5f;

    // Render via C-API profile
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

    std::cout << "[PASS] C API CharacterProfile functions pass with pixel-exact parity.\n";
}

int main() {
    std::cout << "Running CharacterProfile unit tests (v0.3.1)...\n";
    test_profile_default_and_validation();
    test_profile_roundtrip_serialization();
    test_profile_legacy_v2_migration();
    test_c_api_profile_functions();

    if (npc::test::failure_count() > 0) {
        std::cerr << "Profile tests FAILED with " << npc::test::failure_count() << " error(s).\n";
        return 1;
    }

    std::cout << "All CharacterProfile tests passed successfully!\n";
    return 0;
}
