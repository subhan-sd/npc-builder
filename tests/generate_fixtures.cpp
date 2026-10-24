#include "npc/generator.hpp"
#include "test_framework.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>

// Generates 7,360 reference combinations as specified in BRD Section 7.1.1:
// 1. Base group: seeds 1..100, 6 animation states, 4 directions, 3 animation times (0.0, 0.25, 0.5) = 7,200
// 2. Condition group: seeds 1..10, Idle, South, 0.0s, 16 scenarios per seed (10 amputations + 6 injuries) = 160
// Total = 7,360 combinations

int main(int argc, char** argv) {
    std::string out_path = "tests/golden/matriks-acuan.txt";
    if (argc > 1) {
        out_path = argv[1];
    }

    std::ofstream out(out_path);
    if (!out.is_open()) {
        std::cerr << "Failed to open output file: " << out_path << "\n";
        return 1;
    }

    std::cout << "Generating 7,360 golden fixture hashes into " << out_path << "...\n";
    int count = 0;

    // 1. Base group (7,200)
    for (uint64_t seed = 1; seed <= 100; ++seed) {
        auto app = npc::CharacterGenerator::generate_from_seed(seed, static_cast<uint32_t>(seed));
        npc::PhysicalConditionSet cond;

        for (int s = 0; s <= static_cast<int>(npc::AnimationState::Swim); ++s) {
            for (int d = 0; d <= static_cast<int>(npc::Direction::West); ++d) {
                float times[3] = {0.0f, 0.25f, 0.5f};
                for (float t : times) {
                    npc::AnimationCommand cmd{
                        static_cast<npc::AnimationState>(s),
                        static_cast<npc::Direction>(d),
                        t,
                        1.0f
                    };
                    auto buf = npc::CharacterGenerator::render(app, cond, cmd);

                    std::vector<uint8_t> raw_rgba(buf.pixels.size() * 4);
                    for (size_t i = 0; i < buf.pixels.size(); ++i) {
                        raw_rgba[i * 4 + 0] = buf.pixels[i].r;
                        raw_rgba[i * 4 + 1] = buf.pixels[i].g;
                        raw_rgba[i * 4 + 2] = buf.pixels[i].b;
                        raw_rgba[i * 4 + 3] = buf.pixels[i].a;
                    }
                    uint64_t hash = npc::test::fnv1a_64(raw_rgba.data(), raw_rgba.size());

                    out << seed << " "
                        << s << " "
                        << d << " "
                        << std::fixed << std::setprecision(2) << t << " "
                        << "base "
                        << std::hex << std::setw(16) << std::setfill('0') << hash << std::dec << "\n";
                    count++;
                }
            }
        }
    }

    // 2. Condition group (160)
    for (uint64_t seed = 1; seed <= 10; ++seed) {
        auto app = npc::CharacterGenerator::generate_from_seed(seed, static_cast<uint32_t>(seed));
        npc::AnimationCommand cmd{npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f};

        // 10 amputations
        for (int p = 0; p < static_cast<int>(npc::BodyPart::Count); ++p) {
            npc::PhysicalConditionSet cond;
            cond.set_amputated(static_cast<npc::BodyPart>(p), true);

            auto buf = npc::CharacterGenerator::render(app, cond, cmd);
            std::vector<uint8_t> raw_rgba(buf.pixels.size() * 4);
            for (size_t i = 0; i < buf.pixels.size(); ++i) {
                raw_rgba[i * 4 + 0] = buf.pixels[i].r;
                raw_rgba[i * 4 + 1] = buf.pixels[i].g;
                raw_rgba[i * 4 + 2] = buf.pixels[i].b;
                raw_rgba[i * 4 + 3] = buf.pixels[i].a;
            }
            uint64_t hash = npc::test::fnv1a_64(raw_rgba.data(), raw_rgba.size());

            out << seed << " 0 0 0.00 amp_" << p << " "
                << std::hex << std::setw(16) << std::setfill('0') << hash << std::dec << "\n";
            count++;
        }

        // 6 injuries on Torso
        for (int c = 0; c <= static_cast<int>(npc::ConditionType::BloodStain); ++c) {
            npc::PhysicalConditionSet cond;
            npc::InjuryCondition inj;
            inj.condition_id = static_cast<uint32_t>(c + 1);
            inj.body_part = npc::BodyPart::Torso;
            inj.type = static_cast<npc::ConditionType>(c);
            inj.severity = 0.5f;
            inj.active = true;
            cond.injuries.push_back(inj);

            auto buf = npc::CharacterGenerator::render(app, cond, cmd);
            std::vector<uint8_t> raw_rgba(buf.pixels.size() * 4);
            for (size_t i = 0; i < buf.pixels.size(); ++i) {
                raw_rgba[i * 4 + 0] = buf.pixels[i].r;
                raw_rgba[i * 4 + 1] = buf.pixels[i].g;
                raw_rgba[i * 4 + 2] = buf.pixels[i].b;
                raw_rgba[i * 4 + 3] = buf.pixels[i].a;
            }
            uint64_t hash = npc::test::fnv1a_64(raw_rgba.data(), raw_rgba.size());

            out << seed << " 0 0 0.00 inj_" << c << " "
                << std::hex << std::setw(16) << std::setfill('0') << hash << std::dec << "\n";
            count++;
        }
    }

    std::cout << "Done! Generated " << count << " reference combinations.\n";
    return 0;
}
