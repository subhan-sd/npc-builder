#include "npc/generator.hpp"
#include "npc/serializer.hpp"
#include <cassert>
#include <iostream>

void test_rng_determinism() {
    auto app1 = npc::CharacterGenerator::generate_from_seed(424242, 1);
    auto app2 = npc::CharacterGenerator::generate_from_seed(424242, 1);

    assert(app1.body.height == app2.body.height);
    assert(app1.body.body_mass == app2.body.body_mass);
    assert(app1.body.muscularity == app2.body.muscularity);
    assert(app1.face.hair_style == app2.face.hair_style);
    assert(app1.face.skin_color == app2.face.skin_color);
    std::cout << "[PASS] RNG Determinism verified.\n";
}

void test_body_variations_and_render() {
    auto app = npc::CharacterGenerator::generate_from_seed(999, 1);
    app.body.height = 1.0f;     // Extreme tall
    app.body.body_mass = 0.9f;  // Heavy (Chin Jim)
    app.body.muscularity = 0.8f;
    app.body.clamp();

    npc::PhysicalConditionSet cond;
    npc::AnimationCommand cmd{npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f};

    auto buf = npc::CharacterGenerator::render(app, cond, cmd);
    assert(buf.width == npc::CharacterGenerator::CANVAS_WIDTH);
    assert(buf.height == npc::CharacterGenerator::CANVAS_HEIGHT);

    // Verify canvas has non-zero pixels
    int non_empty_pixels = 0;
    for (const auto& p : buf.pixels) {
        if (p.a > 0) non_empty_pixels++;
    }
    assert(non_empty_pixels > 50);
    std::cout << "[PASS] Body variation & render (" << non_empty_pixels << " pixels drawn).\n";
}

void test_body_profiles_silhouette() {
    npc::PhysicalConditionSet cond;
    npc::AnimationCommand cmd{npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f};

    // Trim Jim (Average)
    auto trim = npc::CharacterGenerator::generate_from_seed(101, 1);
    trim.body.body_mass = 0.4f;
    trim.body.muscularity = 0.4f;
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
    assert(px_slim < px_trim);
    assert(px_chin > px_trim);
    assert(px_gym > px_slim);
    std::cout << "[PASS] Reference 1 silhouettes verified (Slim: " << px_slim
              << ", Trim: " << px_trim << ", Gym: " << px_gym << ", Chin: " << px_chin << ").\n";
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

    assert(count_amp < count_full);
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
    assert(found_blood_red);
    std::cout << "[PASS] Physical condition injury rendered correctly.\n";
}

void test_all_animation_states() {
    auto app = npc::CharacterGenerator::generate_from_seed(777, 3);
    npc::PhysicalConditionSet cond;

    for (int s = 0; s <= static_cast<int>(npc::AnimationState::Swim); ++s) {
        for (int d = 0; d <= static_cast<int>(npc::Direction::West); ++d) {
            npc::AnimationCommand cmd{static_cast<npc::AnimationState>(s), static_cast<npc::Direction>(d), 0.5f, 1.0f};
            auto buf = npc::CharacterGenerator::render(app, cond, cmd);
            assert(buf.width == 32 && buf.height == 48);
        }
    }
    std::cout << "[PASS] All 6 animation states x 4 directions rendered successfully.\n";
}

void test_serialization() {
    auto app1 = npc::CharacterGenerator::generate_from_seed(8888, 5);
    std::string data = npc::Serializer::serialize(app1);

    npc::HumanAppearance app2;
    bool ok = npc::Serializer::deserialize(data, app2);
    assert(ok);
    assert(app1.npc_id == app2.npc_id);
    assert(app1.appearance_seed == app2.appearance_seed);
    assert(app1.body.height == app2.body.height);
    assert(app1.face.hair_style == app2.face.hair_style);
    assert(app1.face.skin_color == app2.face.skin_color);
    std::cout << "[PASS] Serialization & deserialization verified.\n";
}

int main() {
    std::cout << "Running NPC Builder unit tests...\n";
    test_rng_determinism();
    test_body_variations_and_render();
    test_body_profiles_silhouette();
    test_amputations();
    test_conditions_and_injuries();
    test_all_animation_states();
    test_serialization();
    std::cout << "All NPC Builder core tests passed successfully!\n";
    return 0;
}
