#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

namespace godot {

class NPCGenerator : public RefCounted {
    GDCLASS(NPCGenerator, RefCounted)

protected:
    static void _bind_methods();

public:
    NPCGenerator();
    ~NPCGenerator();

    Dictionary generate_appearance(uint64_t seed, int npc_id);
    Ref<Image> render_image(
        const Dictionary& appearance,
        int anim_state,
        int direction,
        float anim_time,
        const Array& amputations,
        const Array& injuries
    );
};

} // namespace godot
