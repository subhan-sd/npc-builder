extends Node

const CANVAS_WIDTH: int = 32
const CANVAS_HEIGHT: int = 48

class ColorRamp:
	var highlight: Color
	var base: Color
	var shadow: Color
	var deep: Color
	var outline: Color

static func make_ramp(base: Color, outline_tint := Color(73.0/255.0, 53.0/255.0, 47.0/255.0, 1.0)) -> ColorRamp:
	var r := ColorRamp.new()
	r.base = base
	r.highlight = Color(
		min(1.0, base.r * 1.14 + 0.06),
		min(1.0, base.g * 1.14 + 0.06),
		min(1.0, base.b * 1.14 + 0.06),
		base.a
	)
	r.shadow = Color(base.r * 0.78, base.g * 0.70, base.b * 0.67, base.a)
	r.deep = Color(base.r * 0.58, base.g * 0.48, base.b * 0.44, base.a)
	r.outline = outline_tint
	return r

static func draw_span(img: Image, y: int, x1: int, x2: int, fill: Color, outline: Color, shadow: Color) -> void:
	if y < 0 or y >= CANVAS_HEIGHT or x1 > x2:
		return
	var left: int = max(0, x1)
	var right: int = min(CANVAS_WIDTH - 1, x2)
	for x in range(left, right + 1):
		var col: Color = fill
		if x == left or x == right:
			col = outline
		elif x == right - 1 and (right - left > 2):
			col = shadow
		img.set_pixel(x, y, col)

static func generate_appearance(seed_val: int, npc_id: int) -> Dictionary:
	var rng := RandomNumberGenerator.new()
	rng.seed = seed_val

	var app := {
		"npc_id": npc_id,
		"seed": seed_val,
		"height": rng.randf_range(0.2, 0.8),
		"body_mass": rng.randf_range(0.1, 0.9),
		"muscularity": rng.randf_range(0.1, 0.9),
		"posture": rng.randf_range(0.4, 0.9) if (rng.randf() < 0.2) else 0.0,
		"head_scale": rng.randf_range(0.45, 0.55),
		"torso_ratio": rng.randf_range(0.45, 0.55),
		"arm_ratio": rng.randf_range(0.45, 0.55),
		"leg_ratio": rng.randf_range(0.45, 0.55),
		"skin_color": Color(0.85, 0.63, 0.47),
		"hair_color": Color(0.29, 0.21, 0.18),
		"eye_color": Color(0.20, 0.14, 0.12),
		"shirt_color": Color(0.29, 0.42, 0.61),
		"pants_color": Color(0.22, 0.24, 0.29),
		"hair_style": rng.randi_range(0, 3)
	}
	return app

static func render_texture(
	app: Dictionary,
	amputations: Array,
	anim_state: int,
	direction: int,
	anim_time: float
) -> ImageTexture:
	var img := Image.create(CANVAS_WIDTH, CANVAS_HEIGHT, false, Image.FORMAT_RGBA8)
	img.fill(Color(0, 0, 0, 0))

	var h: float = app.get("height", 0.5)
	var mass: float = app.get("body_mass", 0.5)
	var musc: float = app.get("muscularity", 0.5)
	var post: float = app.get("posture", 0.0)

	var total_h: int = 36 + int(round(h * 6.0))
	var head_w: int = 7
	var head_h: int = 8
	var neck_w: int = 3 if (musc > 0.6 or mass > 0.7) else 2
	var leg_l: int = 13 + int(round(app.get("leg_ratio", 0.5) * 4.0))
	var torso_h: int = max(11, total_h - head_h - 2 - leg_l)
	var arm_l: int = torso_h + 2

	var bob_y: int = 0
	var leg_l_dy: int = 0
	var leg_r_dy: int = 0
	var arm_l_dy: int = 0
	var arm_r_dy: int = 0

	if anim_state == 0:
		bob_y = int(sin(anim_time * 3.0) * 0.8)
	elif anim_state == 1:
		var cycle := sin(anim_time * 8.0)
		leg_l_dy = int(cycle * 2.5)
		leg_r_dy = -leg_l_dy
		arm_l_dy = -leg_l_dy
		arm_r_dy = leg_l_dy
		bob_y = int(abs(cycle) * 1.0)
	elif anim_state == 2:
		var cycle := sin(anim_time * 14.0)
		leg_l_dy = int(cycle * 4.0)
		leg_r_dy = -leg_l_dy
		arm_l_dy = -leg_l_dy
		arm_r_dy = leg_l_dy
		bob_y = int(abs(cycle) * 2.0)
	elif anim_state == 3:
		arm_r_dy = -int(abs(sin(anim_time * 10.0)) * 6.0)
	elif anim_state == 4:
		total_h = 14
		head_h = 6
		torso_h = 5
		leg_l = 3
	elif anim_state == 5:
		bob_y = int(sin(anim_time * 5.0) * 1.5)
		leg_l_dy = int(sin(anim_time * 6.0) * 2.0)
		leg_r_dy = -leg_l_dy
		arm_l_dy = int(cos(anim_time * 6.0) * 3.0)
		arm_r_dy = -arm_l_dy

	var cx: int = CANVAS_WIDTH / 2
	var ground_y: int = CANVAS_HEIGHT - 3
	var base_y: int = ground_y - total_h - bob_y

	var dir_sign: int = 0
	if direction == 2:
		dir_sign = 1
	elif direction == 3:
		dir_sign = -1

	var head_lean_x: int = int(round(post * 3.0)) * dir_sign
	var torso_lean_x: int = int(round(post * 1.5)) * dir_sign
	var head_drop_y: int = int(round(post * 1.5)) if post > 0.2 else 0

	var head_x: int = cx - head_w / 2 + head_lean_x
	var head_y: int = base_y + head_drop_y
	var neck_y: int = head_y + head_h - 1
	var torso_y: int = neck_y + 1
	var hips_y: int = torso_y + torso_h

	var skin_ramp := make_ramp(app.get("skin_color", Color(0.85, 0.63, 0.47)))
	var hair_ramp := make_ramp(app.get("hair_color", Color(0.29, 0.21, 0.18)))

	var shoulder_w: int = 9 + int(round(musc * 4.0 + mass * 1.5))
	var waist_w: int = 6 + int(round(mass * 4.5 + (1.0 - musc) * 1.0))
	if musc > 0.5 and mass < 0.6:
		waist_w = min(waist_w, shoulder_w - 4)
	var hips_w: int = max(waist_w, 7 + int(round(mass * 3.5)))

	# 1. LEGS & FEET
	var leg_gap: int = 1 if (musc > 0.6 or mass < 0.35) else 0
	var thigh_thick: int = 3
	if mass > 0.7 and musc > 0.5:
		thigh_thick = 4
	elif mass < 0.3 and musc < 0.3:
		thigh_thick = 2

	var leg_l_center: int = cx - 2 - leg_gap
	var leg_r_center: int = cx + 2 + leg_gap

	if not (amputations.size() > 6 and amputations[6]): # Left Leg
		var cur_h: int = leg_l + leg_l_dy
		var knee_y: int = cur_h * 4 / 10
		var calf_y: int = cur_h * 7 / 10
		for y in range(cur_h):
			var py: int = hips_y + y
			var lw: int = thigh_thick
			var lx: int = leg_l_center - lw / 2
			if y < knee_y:
				lw = thigh_thick
			elif y == knee_y:
				lw = max(2, thigh_thick - 1)
			elif y <= calf_y:
				lw = thigh_thick + (1 if (musc > 0.4 or mass > 0.4) else 0)
				lx -= 1
			else:
				lw = 2
			draw_span(img, py, lx, lx + lw - 1, skin_ramp.base, skin_ramp.outline, skin_ramp.shadow)
		if not (amputations.size() > 8 and amputations[8]): # Left Foot
			var fy: int = hips_y + cur_h - 1
			var fx: int = leg_l_center - thigh_thick / 2 - (2 if direction == 3 else 1)
			var fw: int = thigh_thick + 2
			draw_span(img, fy, fx, fx + fw - 1, skin_ramp.shadow, skin_ramp.outline, skin_ramp.deep)

	if not (amputations.size() > 7 and amputations[7]): # Right Leg
		var cur_h: int = leg_l + leg_r_dy
		var knee_y: int = cur_h * 4 / 10
		var calf_y: int = cur_h * 7 / 10
		for y in range(cur_h):
			var py: int = hips_y + y
			var rw: int = thigh_thick
			var rx: int = leg_r_center - rw / 2
			if y < knee_y:
				rw = thigh_thick
			elif y == knee_y:
				rw = max(2, thigh_thick - 1)
			elif y <= calf_y:
				rw = thigh_thick + (1 if (musc > 0.4 or mass > 0.4) else 0)
			else:
				rw = 2
			draw_span(img, py, rx, rx + rw - 1, skin_ramp.base, skin_ramp.outline, skin_ramp.shadow)
		if not (amputations.size() > 9 and amputations[9]): # Right Foot
			var fy: int = hips_y + cur_h - 1
			var fx: int = leg_r_center - thigh_thick / 2
			var fw: int = thigh_thick + 2
			draw_span(img, fy, fx, fx + fw - 1, skin_ramp.shadow, skin_ramp.outline, skin_ramp.deep)

	# 2. TORSO RENDERING (Organic sloped shoulder transitions)
	for y in range(torso_h):
		var py: int = torso_y + y
		var prog: float = float(y) / float(torso_h)
		var cur_w: int = 0
		if y == 0:
			cur_w = shoulder_w - 2
		elif prog < 0.45:
			var chest_p := (prog - 0.1) / 0.35
			cur_w = int(round(shoulder_w * (1.0 - max(0.0, chest_p) * 0.15)))
		elif prog < 0.78:
			var waist_p := (prog - 0.45) / 0.33
			var chest_bot := int(round(shoulder_w * 0.85))
			cur_w = int(round(chest_bot * (1.0 - waist_p) + waist_w * waist_p))
			if mass > 0.65:
				cur_w += int(round((1.0 - abs(waist_p - 0.5) * 2.0) * 2.5))
		else:
			var hip_p := (prog - 0.78) / 0.22
			cur_w = int(round(waist_w * (1.0 - hip_p) + hips_w * hip_p))

		var spine_offset := int(round(torso_lean_x * (1.0 - prog)))
		var tx: int = cx - cur_w / 2 + spine_offset

		var mid_fill: Color = skin_ramp.base
		if direction == 1:
			mid_fill = skin_ramp.base if prog < 0.4 else skin_ramp.shadow
		elif direction == 2 or direction == 3:
			mid_fill = skin_ramp.shadow if (post > 0.3 and prog < 0.55) else skin_ramp.base

		draw_span(img, py, tx, tx + cur_w - 1, mid_fill, skin_ramp.outline, skin_ramp.shadow)

		if direction == 0:
			if y == int(torso_h * 38 / 100) and musc > 0.45 and cur_w >= 8:
				img.set_pixel(cx - 2 + spine_offset, py, skin_ramp.shadow)
				img.set_pixel(cx + 2 + spine_offset, py, skin_ramp.shadow)
			if y >= int(torso_h * 48 / 100) and y <= int(torso_h * 72 / 100) and musc > 0.55:
				img.set_pixel(cx + spine_offset, py, skin_ramp.shadow)
				if y % 3 == 0 and cur_w >= 7:
					img.set_pixel(cx - 1 + spine_offset, py, skin_ramp.highlight)
			if mass > 0.65 and y == int(torso_h * 65 / 100):
				img.set_pixel(cx + spine_offset, py, skin_ramp.deep)

	draw_span(img, hips_y - 1, cx - hips_w / 2, cx - hips_w / 2 + hips_w - 1, skin_ramp.shadow, skin_ramp.outline, skin_ramp.deep)

	# 3. NECK
	var neck_offset := int(round(torso_lean_x * 0.9))
	draw_span(img, neck_y, cx - neck_w / 2 + neck_offset, cx - neck_w / 2 + neck_w - 1 + neck_offset, skin_ramp.shadow, skin_ramp.outline, skin_ramp.shadow)

	# 4. ARMS & HANDS (Seamless shoulder slope connection)
	var arm_base_thick: int = 3 if (musc > 0.65 or mass > 0.75) else 2
	var arm_y: int = torso_y + 1
	var arm_l_x: int = cx - shoulder_w / 2 - arm_base_thick + torso_lean_x
	var arm_r_x: int = cx + shoulder_w / 2 + torso_lean_x

	if not (amputations.size() > 2 and amputations[2]): # Left Arm
		var ay: int = arm_y + arm_l_dy
		for y in range(arm_l - 2):
			var py: int = ay + y
			var cur_w: int = arm_base_thick
			var ax: int = arm_l_x
			if y == 0 or y == 1:
				cur_w = arm_base_thick + (1 if musc > 0.4 else 0)
				ax -= (1 if musc > 0.4 else 0)
			elif y == arm_l / 2:
				cur_w = max(2, arm_base_thick - 1)
			elif y == arm_l / 2 + 1 and musc > 0.4:
				cur_w = arm_base_thick
			draw_span(img, py, ax, ax + cur_w - 1, skin_ramp.base, skin_ramp.outline, skin_ramp.shadow)
		if not (amputations.size() > 4 and amputations[4]): # Left Hand
			var hy: int = ay + arm_l - 2
			draw_span(img, hy, arm_l_x, arm_l_x + arm_base_thick, skin_ramp.base, skin_ramp.outline, skin_ramp.shadow)
			img.set_pixel(arm_l_x - (1 if direction == 3 else 0), hy, skin_ramp.shadow)
			img.set_pixel(arm_l_x + arm_base_thick / 2, hy + 1, skin_ramp.deep)

	if not (amputations.size() > 3 and amputations[3]): # Right Arm
		var ay: int = arm_y + arm_r_dy
		for y in range(arm_l - 2):
			var py: int = ay + y
			var cur_w: int = arm_base_thick
			var ax: int = arm_r_x
			if y == 0 or y == 1:
				cur_w = arm_base_thick + (1 if musc > 0.4 else 0)
			elif y == arm_l / 2:
				cur_w = max(2, arm_base_thick - 1)
			elif y == arm_l / 2 + 1 and musc > 0.4:
				cur_w = arm_base_thick
			draw_span(img, py, ax, ax + cur_w - 1, skin_ramp.base, skin_ramp.outline, skin_ramp.shadow)
		if not (amputations.size() > 5 and amputations[5]): # Right Hand
			var hy: int = ay + arm_l - 2
			draw_span(img, hy, arm_r_x, arm_r_x + arm_base_thick, skin_ramp.base, skin_ramp.outline, skin_ramp.shadow)
			img.set_pixel(arm_r_x + arm_base_thick + (0 if direction == 2 else -1), hy, skin_ramp.shadow)
			img.set_pixel(arm_r_x + arm_base_thick / 2, hy + 1, skin_ramp.deep)

	# 5. HEAD & FACIAL FEATURES
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
		if direction == 1:
			for y in range(6):
				draw_span(img, head_y + y, head_x, head_x + head_w - 1, hair_ramp.base, hair_ramp.outline, hair_ramp.shadow)

	if direction != 1:
		var eye_y: int = head_y + 3
		var eye_col: Color = app.get("eye_color", Color(0.2, 0.15, 0.12))
		if direction == 0:
			img.set_pixel(head_x + 1, eye_y, eye_col)
			img.set_pixel(head_x + head_w - 2, eye_y, eye_col)
			img.set_pixel(head_x + head_w / 2, eye_y + 1, skin_ramp.shadow)
			img.set_pixel(head_x + head_w / 2, head_y + 5, skin_ramp.deep)
		elif direction == 2:
			img.set_pixel(head_x + head_w - 2, eye_y, eye_col)
			img.set_pixel(head_x + head_w, eye_y + 1, skin_ramp.shadow)
			img.set_pixel(head_x + 1, eye_y + 1, skin_ramp.shadow)
		elif direction == 3:
			img.set_pixel(head_x + 1, eye_y, eye_col)
			img.set_pixel(head_x - 1, eye_y + 1, skin_ramp.shadow)
			img.set_pixel(head_x + head_w - 2, eye_y + 1, skin_ramp.shadow)

	return ImageTexture.create_from_image(img)
