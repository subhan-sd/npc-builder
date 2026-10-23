#include "npc/generator.hpp"
#include <fstream>
#include <vector>
#include <iostream>

void save_ppm(const std::string& path, const npc::PixelBuffer& buf, int scale = 6) {
    std::ofstream out(path, std::ios::binary);
    int sw = buf.width * scale;
    int sh = buf.height * scale;
    out << "P6\n" << sw << " " << sh << "\n255\n";

    // Checkerboard backdrop for transparency
    for (int y = 0; y < sh; ++y) {
        int src_y = y / scale;
        for (int x = 0; x < sw; ++x) {
            int src_x = x / scale;
            auto p = buf.get_pixel(src_x, src_y);
            uint8_t r = 60, g = 64, b = 75; // Theme background
            if (((x / 12) + (y / 12)) % 2 == 1) {
                r = 50; g = 54; b = 65;
            }
            if (p.a > 0) {
                float alpha = p.a / 255.0f;
                r = static_cast<uint8_t>(p.r * alpha + r * (1.0f - alpha));
                g = static_cast<uint8_t>(p.g * alpha + g * (1.0f - alpha));
                b = static_cast<uint8_t>(p.b * alpha + b * (1.0f - alpha));
            }
            out.put(r);
            out.put(g);
            out.put(b);
        }
    }
}

int main() {
    npc::PhysicalConditionSet cond;

    // 1. Trim Jim (Average)
    auto trim = npc::CharacterGenerator::generate_from_seed(101, 1);
    trim.body.body_mass = 0.5f;
    trim.body.muscularity = 0.5f;
    auto buf_trim = npc::CharacterGenerator::render(trim, cond, {npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f});
    save_ppm("docs/references/fixture_trim_south.ppm", buf_trim);

    // 2. Gym Jim (Muscular)
    auto gym = npc::CharacterGenerator::generate_from_seed(101, 1);
    gym.body.body_mass = 0.5f;
    gym.body.muscularity = 0.95f;
    auto buf_gym = npc::CharacterGenerator::render(gym, cond, {npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f});
    save_ppm("docs/references/fixture_gym_south.ppm", buf_gym);

    // 3. Slim Jim (Slim)
    auto slim = npc::CharacterGenerator::generate_from_seed(101, 1);
    slim.body.body_mass = 0.15f;
    slim.body.muscularity = 0.15f;
    auto buf_slim = npc::CharacterGenerator::render(slim, cond, {npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f});
    save_ppm("docs/references/fixture_slim_south.ppm", buf_slim);

    // 4. Chin Jim (Heavy)
    auto chin = npc::CharacterGenerator::generate_from_seed(101, 1);
    chin.body.body_mass = 0.95f;
    chin.body.muscularity = 0.3f;
    auto buf_chin = npc::CharacterGenerator::render(chin, cond, {npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f});
    save_ppm("docs/references/fixture_chin_south.ppm", buf_chin);

    // 5. Posture East side comparison: Upright vs Hunched
    auto post_upright = trim;
    post_upright.body.posture = 0.0f;
    auto buf_upright = npc::CharacterGenerator::render(post_upright, cond, {npc::AnimationState::Idle, npc::Direction::East, 0.0f, 1.0f});
    save_ppm("docs/references/fixture_posture_upright_east.ppm", buf_upright);

    auto post_hunched = trim;
    post_hunched.body.posture = 1.0f;
    auto buf_hunched = npc::CharacterGenerator::render(post_hunched, cond, {npc::AnimationState::Idle, npc::Direction::East, 0.0f, 1.0f});
    save_ppm("docs/references/fixture_posture_hunched_east.ppm", buf_hunched);

    std::cout << "Visual PPM fixtures generated successfully.\n";
    return 0;
}
