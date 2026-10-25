extends Control

@onready var sprite: TextureRect = $PreviewArea/TextureRect
@onready var seed_spin: SpinBox = $Controls/SeedRow/SpinBox
@onready var gender_option: OptionButton = $Controls/GenderRow/OptionButton
@onready var preset_option: OptionButton = $Controls/PresetRow/OptionButton
@onready var chest_slider: HSlider = $Controls/ChestRow/HSlider
@onready var height_slider: HSlider = $Controls/HeightRow/HSlider
@onready var torso_ratio_slider: HSlider = $Controls/TorsoRatioRow/HSlider
@onready var mass_slider: HSlider = $Controls/MassRow/HSlider
@onready var musc_slider: HSlider = $Controls/MuscRow/HSlider
@onready var post_slider: HSlider = $Controls/PostRow/HSlider
@onready var anim_option: OptionButton = $Controls/AnimRow/OptionButton
@onready var dir_option: OptionButton = $Controls/DirRow/OptionButton
@onready var amp_arm_box: CheckBox = $Controls/Amputations/LeftArm
@onready var amp_leg_box: CheckBox = $Controls/Amputations/LeftLeg

var current_profile: Dictionary = {}
var anim_time: float = 0.0
var _active_texture: ImageTexture = null

func _ready() -> void:
	_setup_ui()
	_on_seed_changed(seed_spin.value)

func _setup_ui() -> void:
	gender_option.clear()
	gender_option.add_item("Male", 0)
	gender_option.add_item("Female", 1)

	preset_option.clear()
	preset_option.add_item("Custom", 0)
	preset_option.add_item("Slim", 1)
	preset_option.add_item("Average", 2)
	preset_option.add_item("Heavy", 3)
	preset_option.add_item("Muscular", 4)

	anim_option.clear()
	anim_option.add_item("Idle", 0)
	anim_option.add_item("Walk", 1)
	anim_option.add_item("Run", 2)
	anim_option.add_item("Attack", 3)
	anim_option.add_item("Fall", 4)
	anim_option.add_item("Swim", 5)

	dir_option.clear()
	dir_option.add_item("South (Front)", 0)
	dir_option.add_item("North (Back)", 1)
	dir_option.add_item("East (Right)", 2)
	dir_option.add_item("West (Left)", 3)

	seed_spin.value_changed.connect(_on_seed_changed)
	gender_option.item_selected.connect(_on_gender_changed)
	preset_option.item_selected.connect(_on_preset_changed)

	chest_slider.value_changed.connect(func(_v): _update_params())
	height_slider.value_changed.connect(func(_v): _update_params())
	torso_ratio_slider.value_changed.connect(func(_v): _update_params())
	mass_slider.value_changed.connect(func(_v): _update_params())
	musc_slider.value_changed.connect(func(_v): _update_params())
	post_slider.value_changed.connect(func(_v): _update_params())

	anim_option.item_selected.connect(func(_i): _update_display())
	dir_option.item_selected.connect(func(_i): _update_display())
	amp_arm_box.toggled.connect(func(_b): _update_display())
	amp_leg_box.toggled.connect(func(_b): _update_display())

func _process(delta: float) -> void:
	anim_time += delta
	_update_display()

func _on_seed_changed(val: float) -> void:
	current_profile = NpcBridge.generate_profile(int(val), 1)
	if current_profile.is_empty():
		current_profile = NpcBridge.create_default_profile(1, int(val))

	if current_profile.has("anatomy"):
		var a = current_profile["anatomy"]
		height_slider.set_value_no_signal(a.get("height", 0.5))
		torso_ratio_slider.set_value_no_signal(a.get("torso_ratio", 0.5))
		mass_slider.set_value_no_signal(a.get("body_mass", 0.5))
		musc_slider.set_value_no_signal(a.get("muscularity", 0.5))
		post_slider.set_value_no_signal(a.get("posture", 0.0))

	if current_profile.has("geometry_override"):
		var ovr = current_profile["geometry_override"]
		chest_slider.set_value_no_signal(ovr.get("chest_curve", 0))
		gender_option.select(1 if ovr.get("chest_curve", 0) > 0 else 0)
	else:
		chest_slider.set_value_no_signal(0)
		gender_option.select(0)

	preset_option.select(0) # Custom
	_update_display()

func _on_gender_changed(idx: int) -> void:
	if !current_profile.has("geometry_override"):
		current_profile["geometry_override"] = {}
	var ovr = current_profile["geometry_override"]

	if idx == 1: # Female
		chest_slider.set_value_no_signal(2.0)
		ovr["chest_curve"] = 2
		ovr["shoulder_w"] = -1  # Bahu 1px lebih ramping
		ovr["waist_w"] = -1     # Pinggang masuk 1px (siluet jam pasir)
		ovr["hips_w"] = 1       # Pinggul 1px lebih lebar
	else: # Male
		chest_slider.set_value_no_signal(0.0)
		ovr["chest_curve"] = 0
		ovr["shoulder_w"] = 0
		ovr["waist_w"] = 0
		ovr["hips_w"] = 0

	_update_params()

func _on_preset_changed(idx: int) -> void:
	match idx:
		1: # Slim
			mass_slider.set_value_no_signal(0.2)
			musc_slider.set_value_no_signal(0.2)
		2: # Average
			mass_slider.set_value_no_signal(0.5)
			musc_slider.set_value_no_signal(0.5)
		3: # Heavy
			mass_slider.set_value_no_signal(0.85)
			musc_slider.set_value_no_signal(0.35)
		4: # Muscular
			mass_slider.set_value_no_signal(0.45)
			musc_slider.set_value_no_signal(0.85)
	if idx > 0:
		_update_params()

func _update_params() -> void:
	if !current_profile.has("anatomy"):
		current_profile["anatomy"] = {}
	var a = current_profile["anatomy"]
	a["height"] = height_slider.value
	a["torso_ratio"] = torso_ratio_slider.value
	a["body_mass"] = mass_slider.value
	a["muscularity"] = musc_slider.value
	a["posture"] = post_slider.value

	if !current_profile.has("geometry_override"):
		current_profile["geometry_override"] = {}
	current_profile["geometry_override"]["chest_curve"] = int(chest_slider.value)
	if chest_slider.value > 0 and gender_option.selected == 0:
		gender_option.select(1)
	elif chest_slider.value == 0 and gender_option.selected == 1:
		gender_option.select(0)

	_update_display()

func _update_display() -> void:
	if !current_profile.has("physical"):
		current_profile["physical"] = {}
	var amps := []
	if amp_arm_box.button_pressed:
		amps.append(2) # LeftArm
		amps.append(4) # LeftHand
	if amp_leg_box.button_pressed:
		amps.append(6) # LeftLeg
		amps.append(8) # LeftFoot
	current_profile["physical"]["amputations"] = amps

	var img = NpcBridge.render_profile_image(
		current_profile,
		anim_option.selected,
		dir_option.selected,
		anim_time
	)

	if _active_texture == null:
		_active_texture = ImageTexture.create_from_image(img)
		sprite.texture = _active_texture
	else:
		_active_texture.update(img)
