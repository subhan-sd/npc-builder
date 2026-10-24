#include "npc/generator.hpp"
#include "npc/serializer.hpp"
#include "test_framework.hpp"
#include <iostream>
#include <algorithm>

void test_rng_determinism() {
    auto app1 = npc::CharacterGenerator::generate_from_seed(42, 1);
    auto app2 = npc::CharacterGenerator::generate_from_seed(42, 1);
    TEST_ASSERT_EQ(app1.body.height, app2.body.height);
    TEST_ASSERT_EQ(app1.body.body_mass, app2.body.body_mass);
    TEST_ASSERT_EQ(app1.face.skin_color.r, app2.face.skin_color.r);
    TEST_ASSERT_EQ(app1.face.eye_color.g, app2.face.eye_color.g);

    npc::PhysicalConditionSet cond;
    npc::AnimationCommand cmd{npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f};

    auto buf1 = npc::CharacterGenerator::render(app1, cond, cmd);
    auto buf2 = npc::CharacterGenerator::render(app2, cond, cmd);

    TEST_ASSERT_EQ(buf1.pixels.size(), buf2.pixels.size());
    for (size_t i = 0; i < buf1.pixels.size(); ++i) {
        TEST_ASSERT_EQ(buf1.pixels[i].r, buf2.pixels[i].r);
        TEST_ASSERT_EQ(buf1.pixels[i].g, buf2.pixels[i].g);
        TEST_ASSERT_EQ(buf1.pixels[i].b, buf2.pixels[i].b);
        TEST_ASSERT_EQ(buf1.pixels[i].a, buf2.pixels[i].a);
    }
    std::cout << "[PASS] RNG Determinism verified.\n";
}

void test_body_variation() {
    auto app = npc::CharacterGenerator::generate_from_seed(999, 1);
    npc::PhysicalConditionSet cond;
    npc::AnimationCommand cmd{npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f};
    auto buf = npc::CharacterGenerator::render(app, cond, cmd);

    int drawn_pixels = 0;
    for (const auto& p : buf.pixels) {
        if (p.a > 0) drawn_pixels++;
    }
    TEST_ASSERT_GT(drawn_pixels, 150);
    std::cout << "[PASS] Body variation & render (" << drawn_pixels << " pixels drawn).\n";
}

void test_body_profiles_silhouette() {
    npc::PhysicalConditionSet cond;
    npc::AnimationCommand cmd{npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f};

    // Trim Jim (Baseline Average)
    auto trim = npc::CharacterGenerator::generate_from_seed(101, 1);
    trim.body.body_mass = 0.5f;
    trim.body.muscularity = 0.5f;
    auto buf_trim = npc::CharacterGenerator::render(trim, cond, cmd);

    // Gym Jim (Muscular)
    auto gym = npc::CharacterGenerator::generate_from_seed(101, 1);
    gym.body.body_mass = 0.5f;
    gym.body.muscularity = 0.95f;
    auto buf_gym = npc::CharacterGenerator::render(gym, cond, cmd);

    // Slim Jim (Slim)
    auto slim = npc::CharacterGenerator::generate_from_seed(101, 1);
    slim.body.body_mass = 0.15f;
    slim.body.muscularity = 0.15f;
    auto buf_slim = npc::CharacterGenerator::render(slim, cond, cmd);

    // Chin Jim (Heavy)
    auto chin = npc::CharacterGenerator::generate_from_seed(101, 1);
    chin.body.body_mass = 0.95f;
    chin.body.muscularity = 0.3f;
    auto buf_chin = npc::CharacterGenerator::render(chin, cond, cmd);

    int px_trim = 0, px_gym = 0, px_slim = 0, px_chin = 0;
    for (const auto& p : buf_trim.pixels) if (p.a > 0) px_trim++;
    for (const auto& p : buf_gym.pixels) if (p.a > 0) px_gym++;
    for (const auto& p : buf_slim.pixels) if (p.a > 0) px_slim++;
    for (const auto& p : buf_chin.pixels) if (p.a > 0) px_chin++;

    // Slim Jim has fewer pixels than Trim Jim; Chin Jim and Gym Jim have more
    TEST_ASSERT_LT(px_slim, px_trim);
    TEST_ASSERT_GT(px_chin, px_trim);
    TEST_ASSERT_GT(px_gym, px_slim);
    std::cout << "[PASS] Reference 1 silhouettes verified (Slim: " << px_slim
              << ", Trim: " << px_trim << ", Gym: " << px_gym << ", Chin: " << px_chin << ").\n";
}

void test_posture_deformation() {
    auto app_upright = npc::CharacterGenerator::generate_from_seed(101, 1);
    app_upright.body.posture = 0.0f;

    auto app_hunched = npc::CharacterGenerator::generate_from_seed(101, 1);
    app_hunched.body.posture = 1.0f;

    npc::PhysicalConditionSet cond;
    npc::AnimationCommand cmd_side{npc::AnimationState::Idle, npc::Direction::East, 0.0f, 1.0f};

    auto buf_upright = npc::CharacterGenerator::render(app_upright, cond, cmd_side);
    auto buf_hunched = npc::CharacterGenerator::render(app_hunched, cond, cmd_side);

    int min_x_upright = 32, max_x_upright = 0;
    int min_x_hunched = 32, max_x_hunched = 0;

    for (int y = 0; y < 48; ++y) {
        for (int x = 0; x < 32; ++x) {
            if (buf_upright.get_pixel(x, y).a > 0) {
                min_x_upright = std::min(min_x_upright, x);
                max_x_upright = std::max(max_x_upright, x);
            }
            if (buf_hunched.get_pixel(x, y).a > 0) {
                min_x_hunched = std::min(min_x_hunched, x);
                max_x_hunched = std::max(max_x_hunched, x);
            }
        }
    }

    TEST_ASSERT_GT(max_x_hunched, max_x_upright);
    std::cout << "[PASS] Posture deformation verified (Upright max X: " << max_x_upright
              << ", Hunched max X: " << max_x_hunched << ").\n";
}

void test_amputations() {
    auto app = npc::CharacterGenerator::generate_from_seed(123, 2);
    npc::PhysicalConditionSet cond_full;
    npc::PhysicalConditionSet cond_amp;
    cond_amp.set_amputated(npc::BodyPart::LeftArm, true);

    npc::AnimationCommand cmd{npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f};
    auto buf_full = npc::CharacterGenerator::render(app, cond_full, cmd);
    auto buf_amp = npc::CharacterGenerator::render(app, cond_amp, cmd);

    int count_full = 0, count_amp = 0;
    for (const auto& p : buf_full.pixels) if (p.a > 0) count_full++;
    for (const auto& p : buf_amp.pixels) if (p.a > 0) count_amp++;

    TEST_ASSERT_LT(count_amp, count_full);
    std::cout << "[PASS] Amputation reduces silhouette (" << count_full << " -> " << count_amp << ").\n";
}

void test_conditions_and_injuries() {
    auto app = npc::CharacterGenerator::generate_from_seed(123, 2);
    npc::PhysicalConditionSet cond;
    npc::InjuryCondition inj;
    inj.condition_id = 1;
    inj.body_part = npc::BodyPart::Torso;
    inj.type = npc::ConditionType::Cut;
    inj.severity = 0.8f;
    inj.active = true;
    cond.injuries.push_back(inj);

    npc::AnimationCommand cmd{npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f};
    auto buf = npc::CharacterGenerator::render(app, cond, cmd);

    bool found_blood_red = false;
    for (const auto& p : buf.pixels) {
        if (p.r > 150 && p.g < 50 && p.b < 50) {
            found_blood_red = true;
            break;
        }
    }
    TEST_ASSERT(found_blood_red);
    std::cout << "[PASS] Physical condition injury rendered correctly.\n";
}

void test_all_animation_states() {
    auto app = npc::CharacterGenerator::generate_from_seed(777, 3);
    npc::PhysicalConditionSet cond;

    for (int s = 0; s <= static_cast<int>(npc::AnimationState::Swim); ++s) {
        for (int d = 0; d <= static_cast<int>(npc::Direction::West); ++d) {
            npc::AnimationCommand cmd{static_cast<npc::AnimationState>(s), static_cast<npc::Direction>(d), 0.5f, 1.0f};
            auto buf = npc::CharacterGenerator::render(app, cond, cmd);
            TEST_ASSERT_EQ(buf.width, 32);
            TEST_ASSERT_EQ(buf.height, 48);
        }
    }
    std::cout << "[PASS] All 6 animation states x 4 directions rendered successfully.\n";
}

void test_serialization() {
    auto app1 = npc::CharacterGenerator::generate_from_seed(8888, 5);
    std::string data = npc::Serializer::serialize(app1);

    npc::HumanAppearance app2;
    bool ok = npc::Serializer::deserialize(data, app2);
    TEST_ASSERT(ok);
    TEST_ASSERT_EQ(app1.npc_id, app2.npc_id);
    TEST_ASSERT_EQ(app1.appearance_seed, app2.appearance_seed);
    TEST_ASSERT_EQ(app1.body.height, app2.body.height);
    TEST_ASSERT_EQ(app1.body.body_mass, app2.body.body_mass);
    TEST_ASSERT_EQ(app1.body.muscularity, app2.body.muscularity);
    TEST_ASSERT_EQ(app1.body.posture, app2.body.posture);
    std::cout << "[PASS] Serialization & deserialization verified.\n";
}

int main() {
    std::cout << "Running NPC Builder unit tests...\n";
    test_rng_determinism();
    test_body_variation();
    test_body_profiles_silhouette();
    test_posture_deformation();
    test_amputations();
    test_conditions_and_injuries();
    test_all_animation_states();
    test_serialization();

    if (npc::test::failure_count() > 0) {
        std::cerr << "Tests FAILED with " << npc::test::failure_count() << " error(s).\n";
        return 1;
    }

    std::cout << "All NPC Builder core tests passed successfully!\n";
    return 0;
}
