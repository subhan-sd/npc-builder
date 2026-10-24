#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class NPCGenerator : public RefCounted {
    GDCLASS(NPCGenerator, RefCounted)

protected:
    static void _bind_methods();

public:
    NPCGenerator();
    ~NPCGenerator();

    // Legacy appearance API
    Dictionary generate_appearance(uint64_t seed, int npc_id);
    Ref<Image> render_image(
        const Dictionary& appearance,
        int anim_state,
        int direction,
        float anim_time,
        const Array& amputations,
        const Array& injuries
    );

    // v0.3.1 Character Profile API
    Dictionary create_default_profile(int npc_id, uint64_t seed);
    Dictionary generate_profile(uint64_t seed, int npc_id);
    Dictionary validate_profile(const Dictionary& profile);
    String serialize_profile(const Dictionary& profile);
    Dictionary deserialize_profile(const String& serialized_data);
    Ref<Image> render_profile(
        const Dictionary& profile,
        int anim_state,
        int direction,
        float anim_time
    );
};

} // namespace godot
