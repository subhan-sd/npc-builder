#pragma once
#include "types.hpp"
#include "appearance.hpp"
#include "conditions.hpp"
#include "profile.hpp"
#include "pixel_buffer.hpp"
#include "rng.hpp"

namespace npc {

class CharacterGenerator {
public:
    static constexpr int CANVAS_WIDTH = 32;
    static constexpr int CANVAS_HEIGHT = 48;

    // Generate random appearance deterministically from seed (legacy)
    static HumanAppearance generate_from_seed(uint64_t seed, uint32_t npc_id = 1);

    // Generate canonical CharacterProfile deterministically from seed
    static CharacterProfile generate_profile_from_seed(uint64_t seed, uint32_t npc_id = 1) {
        auto app = generate_from_seed(seed, npc_id);
        return CharacterProfile::from_legacy(app);
    }

    // Render character using canonical CharacterProfile and AnimationCommand
    static PixelBuffer render_profile(
        const CharacterProfile& profile,
        const AnimationCommand& cmd
    ) {
        auto app = profile.to_legacy_appearance();
        auto geo = profile.derived_geometry();
        return render_internal(app, profile.physical.conditions, cmd, &geo);
    }

    // Render character to pixel buffer based on appearance, conditions, and animation command (legacy public API)
    static PixelBuffer render(
        const HumanAppearance& appearance,
        const PhysicalConditionSet& conditions,
        const AnimationCommand& cmd
    ) {
        return render_internal(appearance, conditions, cmd, nullptr);
    }

    // Internal test hook: render with explicit BodyGeometry
    static PixelBuffer render_internal(
        const HumanAppearance& appearance,
        const PhysicalConditionSet& conditions,
        const AnimationCommand& cmd,
        const BodyGeometry* override_geo
    );
};

} // namespace npc
