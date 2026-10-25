#pragma once
#include "types.hpp"
#include "appearance.hpp"
#include "conditions.hpp"
#include "profile.hpp"
#include "pixel_buffer.hpp"
#include "rng.hpp"

namespace npc {

struct AnimationCommand {
    AnimationState state{AnimationState::Idle};
    Direction direction{Direction::South};
    float anim_time{0.0f}; // seconds for cycle evaluation
    float speed{1.0f};
};

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
        return render(app, profile.physical.conditions, cmd, &profile.body_geometry);
    }

    // Render character to pixel buffer based on appearance, conditions, and animation command (legacy)
    static PixelBuffer render(
        const HumanAppearance& appearance,
        const PhysicalConditionSet& conditions,
        const AnimationCommand& cmd,
        const BodyGeometry* override_geo = nullptr
    );
};

} // namespace npc
