#pragma once
#include "types.hpp"
#include "appearance.hpp"
#include "conditions.hpp"
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

    // Generate random appearance deterministically from seed
    static HumanAppearance generate_from_seed(uint64_t seed, uint32_t npc_id = 1);

    // Render character to pixel buffer based on appearance, conditions, and animation command
    static PixelBuffer render(
        const HumanAppearance& appearance,
        const PhysicalConditionSet& conditions,
        const AnimationCommand& cmd
    );
};

} // namespace npc
