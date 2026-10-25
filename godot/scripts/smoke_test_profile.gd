extends SceneTree

func _init() -> void:
	print("--- Running Godot Profile System Smoke Test ---")
	if not ClassDB.class_exists("NPCGenerator"):
		push_error("FAIL: NPCGenerator class not found in ClassDB!")
		quit(1)
		return

	var gen = ClassDB.instantiate("NPCGenerator")
	print("NPCGenerator instantiated successfully.")

	if not gen.has_method("create_default_profile"):
		push_error("FAIL: Loaded extension binary does not contain create_default_profile! Outdated or missing v0.3.x binary.")
		quit(1)
		return

	# Test create_default_profile
	var default_prof = gen.create_default_profile(42, 12345)
	if default_prof.is_empty() or not default_prof.has("anatomy") or not default_prof.has("appearance") or not default_prof.has("body_geometry"):
		push_error("FAIL: create_default_profile returned malformed dictionary (missing body_geometry)")
		quit(1)
		return
	if default_prof["body_geometry"]["shoulder_w"] <= 0:
		push_error("FAIL: body_geometry.shoulder_w invalid")
		quit(1)
		return
	print("create_default_profile OK: npc_id =", default_prof.generation.npc_id, "shoulder_w =", default_prof.body_geometry.shoulder_w)

	# Test validate_profile
	var val_res = gen.validate_profile(default_prof)
	if not val_res.get("is_valid", false):
		push_error("FAIL: default profile failed validation: " + str(val_res.get("issues", [])))
		quit(1)
		return
	print("validate_profile OK for valid profile.")

	# Test validate_profile rejection of invalid profile
	var invalid_prof = default_prof.duplicate(true)
	invalid_prof["appearance"]["hair_style"] = 99
	var invalid_val = gen.validate_profile(invalid_prof)
	if invalid_val.get("is_valid", true):
		push_error("FAIL: validate_profile accepted invalid hair_style 99")
		quit(1)
		return
	print("validate_profile correctly rejected invalid hair_style.")

	# Test serialize & deserialize profile
	var serialized = gen.serialize_profile(default_prof)
	if serialized.is_empty():
		push_error("FAIL: serialize_profile returned empty string")
		quit(1)
		return
	var deser_res = gen.deserialize_profile(serialized)
	if not deser_res.get("success", false):
		push_error("FAIL: deserialize_profile failed")
		quit(1)
		return
	print("serialize_profile and deserialize_profile roundtrip OK.")

	# Test render_profile valid
	var img = gen.render_profile(default_prof, 0, 0, 0.0)
	if img == null or img.get_width() != 32 or img.get_height() != 48:
		push_error("FAIL: render_profile returned null or wrong dimensions")
		quit(1)
		return

	# Verify that the rendered sprite is not a blank/transparent texture
	var non_transparent_pixels = 0
	for y in range(img.get_height()):
		for x in range(img.get_width()):
			if img.get_pixel(x, y).a > 0.0:
				non_transparent_pixels += 1
	if non_transparent_pixels == 0:
		push_error("FAIL: render_profile returned a completely transparent/blank image!")
		quit(1)
		return
	print("render_profile OK: dimensions =", img.get_width(), "x", img.get_height(), "visible_pixels =", non_transparent_pixels)

	# Test render_profile rejects invalid profile
	var rejected_img = gen.render_profile(invalid_prof, 0, 0, 0.0)
	if rejected_img != null:
		push_error("FAIL: render_profile rendered invalid profile instead of returning null")
		quit(1)
		return
	print("render_profile correctly rejected invalid profile before rendering.")

	print("--- ALL GODOT PROFILE SMOKE TESTS PASSED ---")
	quit(0)
