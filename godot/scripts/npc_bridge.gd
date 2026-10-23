class_name NpcBridge
extends RefCounted

# Bridge to libnpc_builder.so / C++ Generator with full anatomical pixel art rasterizer
# Implements 5-color ramp shading, anatomical silhouettes (Trim, Gym, Slim, Chin Jim),
# directional facial structure and 6 animation states.

const CANVAS_WIDTH: int = 32
const CANVAS_HEIGHT: int = 48

static func generate_appearance(seed_val: int, npc_id: int = 1) -> Dictionary:
    var rng := RandomNumberGenerator.new()
    rng.seed = seed_val
    return {
        "npc_id": npc_id,
        "seed": seed_val,
        "height": rng.randf_range(0.2, 0.8),
        "body_mass": rng.randf_range(0.1, 0.9),
        "muscularity": rng.randf_range(0.1, 0.9),
        "posture": rng.randf_range(0.4, 0.9) if rng.randf() < 0.2 else 0.0,
        "head_scale": rng.randf_range(0.45, 0.55),
        "torso_ratio": rng.randf_range(0.45, 0.55),
        "arm_ratio": rng.randf_range(0.45, 0.55),
        "leg_ratio": rng.randf_range(0.45, 0.55),
        "hair_style": rng.randi_range(0, 3),
        "expression": 0,
        "skin_color": Color(0.85, 0.63, 0.47), # Base warm skin tone (Ref 1)
        "hair_color": Color(0.29, 0.21, 0.18),
        "shirt_color": Color(0.29, 0.42, 0.61),
        "pants_color": Color(0.22, 0.24, 0.29)
    }

# 5-tone ramp creation helper
static func make_ramp(base: Color, outline_tint: Color = Color(0.29, 0.21, 0.18)) -> Dictionary:
    return {
        "base": base,
        "highlight": Color(min(1.0, base.r * 1.18 + 0.08), min(1.0, base.g * 1.18 + 0.08), min(1.0, base.b * 1.18 + 0.08)),
        "shadow": Color(base.r * 0.76, base.g * 0.68, base.b * 0.66),
        "deep": Color(base.r * 0.55, base.g * 0.45, base.b * 0.42),
        "outline": outline_tint
    }

static func draw_span(img: Image, y: int, x_start: int, x_end: int, fill_c: Color, left_c: Color, right_c: Color) -> void:
    if x_end < x_start or y < 0 or y >= CANVAS_HEIGHT:
        return
    var w := img.get_width()
    if x_start == x_end:
        if x_start >= 0 and x_start < w:
            img.set_pixel(x_start, y, left_c)
        return
    if x_start >= 0 and x_start < w:
        img.set_pixel(x_start, y, left_c)
    for x in range(x_start + 1, x_end):
        if x >= 0 and x < w:
            img.set_pixel(x, y, fill_c)
    if x_end >= 0 and x_end < w:
        img.set_pixel(x_end, y, right_c)

static func render_texture(app: Dictionary, amputations: Array, anim_state: int, direction: int, anim_time: float) -> ImageTexture:
    var img := Image.create(CANVAS_WIDTH, CANVAS_HEIGHT, false, Image.FORMAT_RGBA8)
    img.fill(Color(0, 0, 0, 0))

    var h: float = app.get("height", 0.5)
    var mass: float = app.get("body_mass", 0.5)
    var musc: float = app.get("muscularity", 0.5)
    var post: float = app.get("posture", 0.0)

    var total_h: int = 34 + int(round(h * 8.0))
    var shoulder_w: int = 8 + int(round(musc * 3.0 + mass * 1.5))
    var waist_w: int = 6 + int(round(mass * 4.0 + (1.0 - musc) * 1.0))
    if musc > 0.6 and mass < 0.6:
        waist_w = min(waist_w, shoulder_w - 2)

    var head_w: int = 7
    var head_h: int = 8
    var neck_w: int = 3 if (musc > 0.6 or mass > 0.7) else 2
    var leg_l: int = 10 + int(round(app.get("leg_ratio", 0.5) * 4.0))
    var torso_h: int = max(9, total_h - head_h - 2 - leg_l)
    var arm_l: int = torso_h + 1

    var bob_y: int = 0
    var leg_l_dy: int = 0
    var leg_r_dy: int = 0
    var arm_l_dy: int = 0
    var arm_r_dy: int = 0

    if anim_state == 0: # Idle
        bob_y = int(sin(anim_time * 3.0) * 0.8)
    elif anim_state == 1: # Walk
        var cycle := sin(anim_time * 8.0)
        leg_l_dy = int(cycle * 2.5)
        leg_r_dy = -leg_l_dy
        arm_l_dy = -leg_l_dy
        arm_r_dy = leg_l_dy
        bob_y = int(abs(cycle) * 1.0)
    elif anim_state == 2: # Run
        var cycle := sin(anim_time * 14.0)
        leg_l_dy = int(cycle * 4.0)
        leg_r_dy = -leg_l_dy
        arm_l_dy = -leg_l_dy
        arm_r_dy = leg_l_dy
        bob_y = int(abs(cycle) * 2.0)
    elif anim_state == 3: # Attack
        arm_r_dy = -int(abs(sin(anim_time * 10.0)) * 6.0)
    elif anim_state == 4: # Fall
        total_h = 14
        head_h = 6
        torso_h = 5
        leg_l = 3
    elif anim_state == 5: # Swim
        bob_y = int(sin(anim_time * 5.0) * 1.5)
        leg_l_dy = int(sin(anim_time * 6.0) * 2.0)
        leg_r_dy = -leg_l_dy
        arm_l_dy = int(cos(anim_time * 6.0) * 3.0)
        arm_r_dy = -arm_l_dy

    var cx: int = CANVAS_WIDTH / 2
    var ground_y: int = CANVAS_HEIGHT - 3
    var base_y: int = ground_y - total_h - bob_y

    var hunched_offset_x: int = (1 if direction == 2 else (-1 if direction == 3 else 0)) if post > 0.3 else 0
    var head_x: int = cx - head_w / 2 + hunched_offset_x
    var head_y: int = base_y + (1 if post > 0.3 else 0)
    var neck_y: int = head_y + head_h - 1
    var torso_y: int = neck_y + 1
    var hips_y: int = torso_y + torso_h

    var skin_ramp := make_ramp(app.get("skin_color", Color(0.85, 0.63, 0.47)))
    var shirt_ramp := make_ramp(app.get("shirt_color", Color(0.29, 0.42, 0.61)))
    var pants_ramp := make_ramp(app.get("pants_color", Color(0.22, 0.24, 0.29)))
    var hair_ramp := make_ramp(app.get("hair_color", Color(0.29, 0.21, 0.18)))

    # 1. LEGS (with calf shaping)
    var leg_thick: int = 3 if (mass > 0.6 or musc > 0.7) else 2
    var leg_l_x: int = cx - waist_w / 2
    var leg_r_x: int = cx + waist_w / 2 - leg_thick

    if not (amputations.size() > 6 and amputations[6]): # Left Leg
        var cur_h: int = leg_l + leg_l_dy
        for y in range(cur_h):
            var py: int = hips_y + y
            var lw: int = leg_thick + (1 if (y > cur_h / 2 and y < cur_h - 2 and (musc > 0.5 or mass > 0.5)) else 0)
            draw_span(img, py, leg_l_x, leg_l_x + lw - 1, pants_ramp.base, pants_ramp.outline, pants_ramp.shadow)
        if not (amputations.size() > 8 and amputations[8]): # Left Foot
            var fy: int = hips_y + cur_h - 1
            var fx: int = leg_l_x + (-1 if direction == 3 else 0)
            draw_span(img, fy, fx, fx + leg_thick, pants_ramp.deep, pants_ramp.outline, pants_ramp.outline)

    if not (amputations.size() > 7 and amputations[7]): # Right Leg
        var cur_h: int = leg_l + leg_r_dy
        for y in range(cur_h):
            var py: int = hips_y + y
            var rw: int = leg_thick + (1 if (y > cur_h / 2 and y < cur_h - 2 and (musc > 0.5 or mass > 0.5)) else 0)
            draw_span(img, py, leg_r_x, leg_r_x + rw - 1, pants_ramp.base, pants_ramp.outline, pants_ramp.shadow)
        if not (amputations.size() > 9 and amputations[9]): # Right Foot
            var fy: int = hips_y + cur_h - 1
            var fx: int = leg_r_x + (1 if direction == 2 else 0)
            draw_span(img, fy, fx, fx + leg_thick, pants_ramp.deep, pants_ramp.outline, pants_ramp.outline)

    # 2. TORSO (anatomical taper shoulder to waist)
    for y in range(torso_h):
        var py: int = torso_y + y
        var prog: float = float(y) / float(torso_h)
        var cur_w: int = int(round(shoulder_w * (1.0 - prog) + waist_w * prog))
        var tx: int = cx - cur_w / 2
        var fill: Color = shirt_ramp.highlight if (prog < 0.6 and y > 1) else shirt_ramp.base
        draw_span(img, py, tx, tx + cur_w - 1, fill, shirt_ramp.outline, shirt_ramp.shadow)
    draw_span(img, hips_y - 1, cx - waist_w / 2, cx - waist_w / 2 + waist_w - 1, pants_ramp.deep, pants_ramp.outline, pants_ramp.outline)

    # 3. NECK
    draw_span(img, neck_y, cx - neck_w / 2, cx - neck_w / 2 + neck_w - 1, skin_ramp.shadow, skin_ramp.outline, skin_ramp.shadow)

    # 4. ARMS & HANDS (with Deltoid & Forearm contour)
    var arm_thick: int = 3 if (musc > 0.6 or mass > 0.7) else 2
    var arm_l_x: int = cx - shoulder_w / 2 - arm_thick
    var arm_r_x: int = cx + shoulder_w / 2
    var arm_y: int = torso_y

    if not (amputations.size() > 2 and amputations[2]): # Left Arm
        var ay: int = arm_y + arm_l_dy
        for y in range(arm_l - 2):
            var py: int = ay + y
            var fill: Color = shirt_ramp.base if y < 2 else skin_ramp.base
            draw_span(img, py, arm_l_x, arm_l_x + arm_thick - 1, fill, skin_ramp.outline, skin_ramp.shadow)
        if not (amputations.size() > 4 and amputations[4]): # Left Hand
            var hy: int = ay + arm_l - 2
            draw_span(img, hy, arm_l_x, arm_l_x + arm_thick - 1, skin_ramp.base, skin_ramp.outline, skin_ramp.shadow)
            img.set_pixel(arm_l_x + arm_thick / 2, hy + 1, skin_ramp.deep)

    if not (amputations.size() > 3 and amputations[3]): # Right Arm
        var ay: int = arm_y + arm_r_dy
        for y in range(arm_l - 2):
            var py: int = ay + y
            var fill: Color = shirt_ramp.base if y < 2 else skin_ramp.base
            draw_span(img, py, arm_r_x, arm_r_x + arm_thick - 1, fill, skin_ramp.outline, skin_ramp.shadow)
        if not (amputations.size() > 5 and amputations[5]): # Right Hand
            var hy: int = ay + arm_l - 2
            draw_span(img, hy, arm_r_x, arm_r_x + arm_thick - 1, skin_ramp.base, skin_ramp.outline, skin_ramp.shadow)
            img.set_pixel(arm_r_x + arm_thick / 2, hy + 1, skin_ramp.deep)

    # 5. HEAD & FACIAL FEATURES (Skull contour, jaw, chin, hair)
    draw_span(img, head_y, head_x + 1, head_x + head_w - 2, skin_ramp.base, skin_ramp.outline, skin_ramp.shadow)
    for y in range(1, 6):
        draw_span(img, head_y + y, head_x, head_x + head_w - 1, skin_ramp.highlight if y <= 2 else skin_ramp.base, skin_ramp.outline, skin_ramp.shadow)
    draw_span(img, head_y + 6, head_x + 1, head_x + head_w - 2, skin_ramp.base, skin_ramp.outline, skin_ramp.shadow)
    draw_span(img, head_y + 7, head_x + 2, head_x + head_w - 3, skin_ramp.shadow, skin_ramp.outline, skin_ramp.deep)

    var hair_st: int = app.get("hair_style", 1)
    if hair_st > 0:
        draw_span(img, head_y - 1, head_x + 1, head_x + head_w - 2, hair_ramp.highlight, hair_ramp.outline, hair_ramp.outline)
        draw_span(img, head_y, head_x, head_x + head_w - 1, hair_ramp.base, hair_ramp.outline, hair_ramp.shadow)
        img.set_pixel(head_x + 1, head_y + 1, hair_ramp.base)
        img.set_pixel(head_x + head_w - 2, head_y + 1, hair_ramp.shadow)
        if hair_st >= 2:
            img.set_pixel(head_x, head_y + 2, hair_ramp.outline)
            img.set_pixel(head_x + head_w - 1, head_y + 2, hair_ramp.shadow)
        if hair_st == 3:
            draw_span(img, head_y + 5, head_x - 1, head_x, hair_ramp.shadow, hair_ramp.outline, hair_ramp.deep)
            draw_span(img, head_y + 5, head_x + head_w - 1, head_x + head_w, hair_ramp.deep, hair_ramp.deep, hair_ramp.outline)
        if direction == 1: # Back view
            for y in range(6):
                draw_span(img, head_y + y, head_x, head_x + head_w - 1, hair_ramp.base, hair_ramp.outline, hair_ramp.shadow)

    if direction != 1:
        var eye_y: int = head_y + 3
        var eye_col: Color = app.get("eye_color", Color(0.2, 0.15, 0.12))
        if direction == 0:
            img.set_pixel(head_x + 1, eye_y, eye_col)
            img.set_pixel(head_x + head_w - 2, eye_y, eye_col)
            img.set_pixel(head_x + head_w / 2, eye_y + 1, skin_ramp.shadow) # Nose
            img.set_pixel(head_x + head_w / 2, head_y + 5, skin_ramp.deep)    # Mouth
        elif direction == 2:
            img.set_pixel(head_x + head_w - 2, eye_y, eye_col)
            img.set_pixel(head_x + head_w, eye_y + 1, skin_ramp.shadow)
            img.set_pixel(head_x + 1, eye_y + 1, skin_ramp.shadow)
        elif direction == 3:
            img.set_pixel(head_x + 1, eye_y, eye_col)
            img.set_pixel(head_x - 1, eye_y + 1, skin_ramp.shadow)
            img.set_pixel(head_x + head_w - 2, eye_y + 1, skin_ramp.shadow)

    return ImageTexture.create_from_image(img)
