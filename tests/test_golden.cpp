#include "npc/generator.hpp"
#include "test_framework.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>

// Test verifying 7,360 golden fixture combinations against tests/golden/matriks-acuan.txt (BRD Fungsional-005)

int main(int argc, char** argv) {
    std::string golden_path = "tests/golden/matriks-acuan.txt";
    if (argc > 1) {
        golden_path = argv[1];
    }

    std::ifstream in(golden_path);
    if (!in.is_open()) {
        std::cerr << "[FAIL] Could not open golden fixture file: " << golden_path << "\n";
        return 1;
    }

    std::cout << "Verifying 7,360 combinations against " << golden_path << "...\n";

    std::string line;
    int tested_count = 0;
    int first_mismatch_line = -1;

    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);

        uint64_t seed = 0;
        int s = 0, d = 0;
        float t = 0.0f;
        std::string scenario;
        std::string expected_hash_str;

        if (!(ss >> seed >> s >> d >> t >> scenario >> expected_hash_str)) {
            continue;
        }

        auto app = npc::CharacterGenerator::generate_from_seed(seed, static_cast<uint32_t>(seed));
        npc::PhysicalConditionSet cond;

        if (scenario.rfind("amp_", 0) == 0) {
            int p = std::stoi(scenario.substr(4));
            cond.set_amputated(static_cast<npc::BodyPart>(p), true);
        } else if (scenario.rfind("inj_", 0) == 0) {
            int c = std::stoi(scenario.substr(4));
            npc::InjuryCondition inj;
            inj.condition_id = static_cast<uint32_t>(c + 1);
            inj.body_part = npc::BodyPart::Torso;
            inj.type = static_cast<npc::ConditionType>(c);
            inj.severity = 0.5f;
            inj.active = true;
            cond.injuries.push_back(inj);
        }

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
        uint64_t actual_hash = npc::test::fnv1a_64(raw_rgba.data(), raw_rgba.size());

        std::ostringstream act_ss;
        act_ss << std::hex << std::setw(16) << std::setfill('0') << actual_hash;
        std::string actual_hash_str = act_ss.str();

        if (actual_hash_str != expected_hash_str) {
            if (first_mismatch_line == -1) {
                first_mismatch_line = tested_count + 1;
                std::cerr << "[FAIL] Mismatch at combo #" << first_mismatch_line
                          << " (Seed: " << seed << ", State: " << s << ", Dir: " << d
                          << ", Time: " << t << ", Scenario: " << scenario << ")\n"
                          << "       Expected: " << expected_hash_str << "\n"
                          << "       Actual:   " << actual_hash_str << "\n";
            }
            npc::test::failure_count()++;
        }
        tested_count++;
    }

    if (tested_count != 7360) {
        std::cerr << "[FAIL] Expected 7,360 combinations, tested " << tested_count << "\n";
        return 1;
    }

    if (npc::test::failure_count() > 0) {
        std::cerr << "[FAIL] Total fixture mismatches: " << npc::test::failure_count() << "\n";
        return 1;
    }

    std::cout << "[PASS] All " << tested_count << " golden fixture hashes match perfectly!\n";
    return 0;
}
