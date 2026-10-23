class_name NpcBridge
extends RefCounted

# Bridge to libnpc_builder.so via Godot 4 GDExtension / FFI or pure GDScript fallback
# Uses packed byte arrays and Image / ImageTexture generation

const CANVAS_WIDTH: int = 32
const CANVAS_HEIGHT: int = 48

# GDScript native fallback generator matching the C++ core exactly
# Ensures preview runs out of the box even without external shared library binding
static func generate_appearance(seed_val: int, npc_id: int = 1) -> Dictionary:
    var rng := RandomNumberGenerator.new()
    rng.seed = seed_val
    return {
        "npc_id": npc_id,
        "seed": seed_val,
        "height": rng.randf_range(0.1, 0.9),
        "body_mass": rng.randf_range(0.1, 0.9),
        "muscularity": rng.randf_range(0.1, 0.9),
        "posture": rng.randf_range(0.4, 0.9) if rng.randf() < 0.2 else 0.0,
        "head_scale": rng.randf_range(0.4, 0.6),
        "torso_ratio": rng.randf_range(0.4, 0.6),
        "arm_ratio": rng.randf_range(0.4, 0.6),
        "leg_ratio": rng.randf_range(0.4, 0.6),
        "hair_style": rng.randi_range(0, 3),
        "expression": 0,
        "skin_color": Color(0.88, 0.72, 0.58),
        "hair_color": Color(0.2, 0.15, 0.12),
        "shirt_color": Color(0.23, 0.35, 0.55),
        "pants_color": Color(0.17, 0.2, 0.23)
    }

static func render_texture(app: Dictionary, amputations: Array, anim_state: int, direction: int, anim_time: float) -> ImageTexture:
    var img := Image.create(CANVAS_WIDTH, CANVAS_HEIGHT, false, Image.FORMAT_RGBA8)
    img.fill(Color(0, 0, 0, 0))

    var h_scale: float = app.get("height", 0.5)
    var mass: float = app.get("body_mass", 0.5)
    var musc: float = app.get("muscularity", 0.5)
    var post: float = app.get("posture", 0.0)

    var total_h: int = 32 + int(round(h_scale * 10.0))
    var torso_w: int = 6 + int(round(mass * 4.0 + musc * 2.0))
    var limb_thick: int = 3 if (musc > 0.6 or mass > 0.7) else 2
    var head_sz: int = 7 + int(round(app.get("head_scale", 0.5) * 2.0))
    var leg_l: int = 8 + int(round(app.get("leg_ratio", 0.5) * 4.0))
    var torso_l: int = max(8, total_h - head_sz - leg_l)
    var arm_l: int = torso_l + 1

    var bob_y: int = 0
    var leg_l_dy: int = 0
    var leg_r_dy: int = 0
    var arm_l_dy: int = 0
    var arm_r_dy: int = 0

    if anim_state == 0: # Idle
        bob_y = int(sin(anim_time * 3.0) * 0.8)
    elif anim_state == 1: # Walk
        var c := sin(anim_time * 8.0)
        leg_l_dy = int(c * 2.0)
        leg_r_dy = -leg_l_dy
        arm_l_dy = -leg_l_dy
        arm_r_dy = leg_l_dy
        bob_y = int(abs(c) * 1.0)
    elif anim_state == 2: # Run
        var c := sin(anim_time * 14.0)
        leg_l_dy = int(c * 4.0)
        leg_r_dy = -leg_l_dy
        arm_l_dy = -leg_l_dy
        arm_r_dy = leg_l_dy
        bob_y = int(abs(c) * 2.0)
    elif anim_state == 3: # Attack
        arm_r_dy = -int(abs(sin(anim_time * 10.0)) * 5.0)
    elif anim_state == 4: # Fall
        torso_l = 5
        leg_l = 4
        head_sz = 6
    elif anim_state == 5: # Swim
        bob_y = int(sin(anim_time * 5.0) * 1.5)
        leg_l_dy = int(sin(anim_time * 6.0) * 1.5)
        leg_r_dy = -leg_l_dy
        arm_l_dy = int(cos(anim_time * 6.0) * 2.0)
        arm_r_dy = -arm_l_dy

    var cx: int = CANVAS_WIDTH / 2
    var ground_y: int = CANVAS_HEIGHT - 3
    var base_y: int = ground_y - total_h - bob_y

    var hunched_x: int = 1 if (post > 0.3 and direction == 2) else (-1 if (post > 0.3 and direction == 3) else 0)
    var hx: int = cx - head_sz / 2 + hunched_x
    var hy: int = base_y
    var tx: int = cx - torso_w / 2
    var ty: int = hy + head_sz - 1
    var hips_y: int = ty + torso_l

    var leg_lx: int = tx + 1
    var leg_rx: int = tx + torso_w - limb_thick - 1
    var arm_lx: int = tx - limb_thick
    var arm_rx: int = tx + torso_w
    var arm_y: int = ty + 1

    var skin_col: Color = app.get("skin_color", Color(0.88, 0.72, 0.58))
    var shirt_col: Color = app.get("shirt_color", Color(0.23, 0.35, 0.55))
    var pants_col: Color = app.get("pants_color", Color(0.17, 0.2, 0.23))
    var hair_col: Color = app.get("hair_color", Color(0.2, 0.15, 0.12))

    # Legs
    if not (amputations.size() > 6 and amputations[6]):
        fill_rect(img, leg_lx, hips_y, limb_thick, leg_l + leg_l_dy, pants_col)
    if not (amputations.size() > 7 and amputations[7]):
        fill_rect(img, leg_rx, hips_y, limb_thick, leg_l + leg_r_dy, pants_col)

    # Torso
    fill_rect(img, tx, ty, torso_w, torso_l, shirt_col)

    # Arms
    if not (amputations.size() > 2 and amputations[2]):
        fill_rect(img, arm_lx, arm_y + arm_l_dy, limb_thick, arm_l - 2, shirt_col)
        fill_rect(img, arm_lx, arm_y + arm_l_dy + arm_l - 2, limb_thick, 2, skin_col)
    if not (amputations.size() > 3 and amputations[3]):
        fill_rect(img, arm_rx, arm_y + arm_r_dy, limb_thick, arm_l - 2, shirt_col)
        fill_rect(img, arm_rx, arm_y + arm_r_dy + arm_l - 2, limb_thick, 2, skin_col)

    # Head & Hair
    fill_rect(img, hx, hy, head_sz, head_sz, skin_col)
    var hair_st: int = app.get("hair_style", 1)
    if hair_st > 0:
        fill_rect(img, hx - 1, hy - 1, head_sz + 2, 2, hair_col)
        if hair_st >= 2:
            fill_rect(img, hx - 1, hy, 1, head_sz - 2, hair_col)
            fill_rect(img, hx + head_sz, hy, 1, head_sz - 2, hair_col)
        if direction == 1: # Back view
            fill_rect(img, hx, hy, head_sz, head_sz - 2, hair_col)

    # Face eyes (if not back)
    if direction != 1:
        var eye_col := Color(0.15, 0.12, 0.1)
        var ey: int = hy + head_sz / 2 - 1
        if direction == 0:
            img.set_pixel(hx + 1, ey, eye_col)
            img.set_pixel(hx + head_sz - 2, ey, eye_col)
        elif direction == 2:
            img.set_pixel(hx + head_sz - 2, ey, eye_col)
        elif direction == 3:
            img.set_pixel(hx + 1, ey, eye_col)

    return ImageTexture.create_from_image(img)

static func fill_rect(img: Image, x: int, y: int, w: int, h: int, color: Color) -> void:
    var cw := img.get_width()
    var ch := img.get_height()
    for dy in range(h):
        for dx in range(w):
            var px := x + dx
            var py := y + dy
            if px >= 0 and px < cw and py >= 0 and py < ch:
                img.set_pixel(px, py, color)
