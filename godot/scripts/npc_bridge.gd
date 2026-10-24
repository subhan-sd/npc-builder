extends Node

## NpcBridge adapter connecting Godot script layer directly to GDExtension NPCGenerator native class.
## Single source of truth is C++20 core via GDExtension.

var _generator = null

func _init() -> void:
	if ClassDB.class_exists("NPCGenerator"):
		_generator = ClassDB.instantiate("NPCGenerator")
		print("[NpcBridge] Successfully instantiated native GDExtension NPCGenerator.")
	else:
		push_warning("[NpcBridge] GDExtension NPCGenerator class not found. Ensure extension library is compiled and present in res://bin/.")

func generate_appearance(seed_val: int, npc_id: int) -> Dictionary:
	if _generator != null:
		return _generator.generate_appearance(seed_val, npc_id)
	return {
		"npc_id": npc_id,
		"appearance_seed": seed_val,
		"height": 0.5,
		"body_mass": 0.5,
		"muscularity": 0.5,
		"posture": 0.0
	}

func render_image(
	app: Dictionary,
	amputations: Array,
	anim_state: int,
	direction: int,
	anim_time: float,
	injuries: Array = []
) -> Image:
	if _generator != null:
		return _generator.render_image(app, anim_state, direction, anim_time, amputations, injuries)
	return Image.create(32, 48, false, Image.FORMAT_RGBA8)

func render_texture(
	app: Dictionary,
	amputations: Array,
	anim_state: int,
	direction: int,
	anim_time: float,
	injuries: Array = []
) -> ImageTexture:
	var img = render_image(app, amputations, anim_state, direction, anim_time, injuries)
	return ImageTexture.create_from_image(img)
