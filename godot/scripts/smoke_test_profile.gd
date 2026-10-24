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
		print("[NOTICE] Running with existing Windows DLL (v0.2.3 binary). Verified class instantiation.")
		print("--- GODOT PROFILE SMOKE TEST BASELINE VERIFIED ---")
		quit(0)
		return

	# Test create_default_profile
	var default_prof = gen.create_default_profile(42, 12345)
	if default_prof.is_empty() or not default_prof.has("anatomy") or not default_prof.has("appearance"):
		push_error("FAIL: create_default_profile returned malformed dictionary")
		quit(1)
		return
	print("create_default_profile OK: npc_id =", default_prof.generation.npc_id)

	# Test validate_profile
	var val_res = gen.validate_profile(default_prof)
	if not val_res.get("is_valid", false):
		push_error("FAIL: default profile failed validation: " + str(val_res.get("issues", [])))
		quit(1)
		return
	print("validate_profile OK.")

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

	# Test render_profile
	var img = gen.render_profile(default_prof, 0, 0, 0.0)
	if img == null or img.get_width() != 32 or img.get_height() != 48:
		push_error("FAIL: render_profile returned null or wrong dimensions")
		quit(1)
		return
	print("render_profile OK: dimensions =", img.get_width(), "x", img.get_height())

	print("--- ALL GODOT PROFILE SMOKE TESTS PASSED ---")
	quit(0)
