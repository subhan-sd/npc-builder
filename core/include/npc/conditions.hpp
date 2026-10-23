#pragma once
#include "types.hpp"
#include <vector>
#include <array>

namespace npc {

struct InjuryCondition {
    uint32_t condition_id{0};
    BodyPart body_part{BodyPart::Torso};
    ConditionType type{ConditionType::Cut};
    float severity{0.5f}; // 0.0 to 1.0
    uint32_t visual_seed{0};
    bool active{true};
    bool healed{false};
};

struct PhysicalConditionSet {
    std::array<bool, static_cast<size_t>(BodyPart::Count)> amputations{}; // true if missing
    std::vector<InjuryCondition> injuries;

    PhysicalConditionSet() {
        amputations.fill(false);
    }

    bool is_amputated(BodyPart part) const {
        return amputations[static_cast<size_t>(part)];
    }

    void set_amputated(BodyPart part, bool amputated) {
        amputations[static_cast<size_t>(part)] = amputated;
        // Cascade down: if arm amputated, hand is amputated
        if (part == BodyPart::LeftArm && amputated) amputations[static_cast<size_t>(BodyPart::LeftHand)] = true;
        if (part == BodyPart::RightArm && amputated) amputations[static_cast<size_t>(BodyPart::RightHand)] = true;
        if (part == BodyPart::LeftLeg && amputated) amputations[static_cast<size_t>(BodyPart::LeftFoot)] = true;
        if (part == BodyPart::RightLeg && amputated) amputations[static_cast<size_t>(BodyPart::RightFoot)] = true;
    }
};

} // namespace npc
