#include "npc/c_api.h"
#include "npc/generator.hpp"
#include "test_framework.hpp"
#include <iostream>
#include <vector>

void test_c_api_version() {
    uint32_t ver = npc_get_api_version();
    TEST_ASSERT_EQ(ver, 2u);
    std::cout << "[PASS] npc_get_api_version() returned 2.\n";
}

void test_c_api_parity_100_seeds() {
    std::cout << "Testing 100 seeds for 100% exact RGBA byte match between C++ core and C API...\n";
    int matches = 0;

    for (uint64_t seed = 1; seed <= 100; ++seed) {
        // Direct C++ core call
        auto app_core = npc::CharacterGenerator::generate_from_seed(seed, static_cast<uint32_t>(seed));
        npc::PhysicalConditionSet cond_core;
        npc::AnimationCommand cmd_core{npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f};
        auto buf_core = npc::CharacterGenerator::render(app_core, cond_core, cmd_core);

        // C API V2 call
        CNpcAppearance c_app{};
        c_app.struct_size = sizeof(CNpcAppearance);
        int gen_res = npc_generate_appearance_v2(seed, static_cast<uint32_t>(seed), &c_app);
        TEST_ASSERT_EQ(gen_res, NPC_SUCCESS);

        CNpcRenderConfig cfg{};
        cfg.struct_size = sizeof(CNpcRenderConfig);
        cfg.api_version = NPC_API_VERSION;
        cfg.anim_state = static_cast<uint8_t>(npc::AnimationState::Idle);
        cfg.direction = static_cast<uint8_t>(npc::Direction::South);
        cfg.anim_time = 0.0f;
        cfg.speed = 1.0f;

        std::vector<uint8_t> out_rgba(32 * 48 * 4, 0);
        int render_res = npc_render_pixels_v2(&c_app, &cfg, out_rgba.data(), out_rgba.size());
        TEST_ASSERT_EQ(render_res, NPC_SUCCESS);

        // Byte-by-byte comparison
        bool identical = true;
        for (size_t i = 0; i < buf_core.pixels.size(); ++i) {
            if (out_rgba[i * 4 + 0] != buf_core.pixels[i].r ||
                out_rgba[i * 4 + 1] != buf_core.pixels[i].g ||
                out_rgba[i * 4 + 2] != buf_core.pixels[i].b ||
                out_rgba[i * 4 + 3] != buf_core.pixels[i].a) {
                identical = false;
                break;
            }
        }
        TEST_ASSERT(identical);
        if (identical) matches++;
    }

    TEST_ASSERT_EQ(matches, 100);
    std::cout << "[PASS] 100 of 100 seeds rendered identical RGBA bytes between C++ core and C API!\n";
}

void test_c_api_validation_errors() {
    CNpcAppearance c_app{};
    c_app.struct_size = sizeof(CNpcAppearance);
    npc_generate_appearance_v2(42, 1, &c_app);

    CNpcRenderConfig cfg{};
    cfg.struct_size = sizeof(CNpcRenderConfig);
    cfg.api_version = NPC_API_VERSION;
    cfg.anim_state = 0;
    cfg.direction = 0;

    std::vector<uint8_t> buf(32 * 48 * 4, 0);

    // Null pointer checks
    TEST_ASSERT_EQ(npc_render_pixels_v2(nullptr, &cfg, buf.data(), buf.size()), NPC_ERR_INVALID_ARGUMENT);
    TEST_ASSERT_EQ(npc_render_pixels_v2(&c_app, nullptr, buf.data(), buf.size()), NPC_ERR_INVALID_ARGUMENT);
    TEST_ASSERT_EQ(npc_render_pixels_v2(&c_app, &cfg, nullptr, buf.size()), NPC_ERR_INVALID_ARGUMENT);

    // Buffer too small
    TEST_ASSERT_EQ(npc_render_pixels_v2(&c_app, &cfg, buf.data(), 100), NPC_ERR_BUFFER_TOO_SMALL);

    // Invalid enum
    cfg.anim_state = 99;
    TEST_ASSERT_EQ(npc_render_pixels_v2(&c_app, &cfg, buf.data(), buf.size()), NPC_ERR_INVALID_ENUM);
    cfg.anim_state = 0;

    // Struct size mismatch
    c_app.struct_size = sizeof(CNpcAppearance) - 4;
    TEST_ASSERT_EQ(npc_render_pixels_v2(&c_app, &cfg, buf.data(), buf.size()), NPC_ERR_VERSION_MISMATCH);

    std::cout << "[PASS] C API error handling and input validation verified.\n";
}

int main() {
    std::cout << "Running C API verification tests...\n";
    test_c_api_version();
    test_c_api_parity_100_seeds();
    test_c_api_validation_errors();

    if (npc::test::failure_count() > 0) {
        std::cerr << "C API tests FAILED with " << npc::test::failure_count() << " error(s).\n";
        return 1;
    }

    std::cout << "All C API tests passed successfully!\n";
    return 0;
}
