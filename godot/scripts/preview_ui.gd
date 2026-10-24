extends Control

@onready var sprite: TextureRect = $PreviewArea/TextureRect
@onready var seed_spin: SpinBox = $Controls/SeedRow/SpinBox
@onready var height_slider: HSlider = $Controls/HeightRow/HSlider
@onready var mass_slider: HSlider = $Controls/MassRow/HSlider
@onready var musc_slider: HSlider = $Controls/MuscRow/HSlider
@onready var post_slider: HSlider = $Controls/PostRow/HSlider
@onready var anim_option: OptionButton = $Controls/AnimRow/OptionButton
@onready var dir_option: OptionButton = $Controls/DirRow/OptionButton
@onready var amp_arm_box: CheckBox = $Controls/Amputations/LeftArm
@onready var amp_leg_box: CheckBox = $Controls/Amputations/LeftLeg

var current_app: Dictionary = {}
var anim_time: float = 0.0
var _active_texture: ImageTexture = null

func _ready() -> void:
	_setup_ui()
	_on_seed_changed(seed_spin.value)

func _setup_ui() -> void:
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
	height_slider.value_changed.connect(func(_v): _update_params())
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
	current_app = NpcBridge.generate_appearance(int(val), 1)
	if current_app.has("body"):
		var b = current_app["body"]
		height_slider.set_value_no_signal(b.get("height", 0.5))
		mass_slider.set_value_no_signal(b.get("body_mass", 0.5))
		musc_slider.set_value_no_signal(b.get("muscularity", 0.5))
		post_slider.set_value_no_signal(b.get("posture", 0.0))
	_update_display()

func _update_params() -> void:
	if !current_app.has("body"):
		current_app["body"] = {}
	var b = current_app["body"]
	b["height"] = height_slider.value
	b["body_mass"] = mass_slider.value
	b["muscularity"] = musc_slider.value
	b["posture"] = post_slider.value
	_update_display()

func _update_display() -> void:
	var amputations := []
	if amp_arm_box.button_pressed:
		amputations.append(2) # LeftArm
	if amp_leg_box.button_pressed:
		amputations.append(6) # LeftLeg

	var img = NpcBridge.render_image(
		current_app,
		amputations,
		anim_option.selected,
		dir_option.selected,
		anim_time
	)

	if _active_texture == null:
		_active_texture = ImageTexture.create_from_image(img)
		sprite.texture = _active_texture
	else:
		_active_texture.update(img)
