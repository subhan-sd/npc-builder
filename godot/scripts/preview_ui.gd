extends Control

@onready var sprite: TextureRect = $PreviewArea/TextureRect
@onready var seed_spin: SpinBox = $Controls/SeedRow/SpinBox
@onready var height_slider: HSlider = $Controls/HeightRow/HSlider
@onready var mass_slider: HSlider = $Controls/MassRow/HSlider
@onready var anim_option: OptionButton = $Controls/AnimRow/OptionButton
@onready var dir_option: OptionButton = $Controls/DirRow/OptionButton
@onready var amp_arm_box: CheckBox = $Controls/Amputations/LeftArm
@onready var amp_leg_box: CheckBox = $Controls/Amputations/LeftLeg

var current_app: Dictionary = {}
var anim_time: float = 0.0

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
    anim_option.item_selected.connect(func(_i): _update_display())
    dir_option.item_selected.connect(func(_i): _update_display())
    amp_arm_box.toggled.connect(func(_b): _update_display())
    amp_leg_box.toggled.connect(func(_b): _update_display())

func _process(delta: float) -> void:
    anim_time += delta
    _update_display()

func _on_seed_changed(val: float) -> void:
    current_app = NpcBridge.generate_appearance(int(val), 1)
    height_slider.set_value_no_signal(current_app.get("height", 0.5))
    mass_slider.set_value_no_signal(current_app.get("body_mass", 0.5))
    _update_display()

func _update_params() -> void:
    current_app["height"] = height_slider.value
    current_app["body_mass"] = mass_slider.value
    _update_display()

func _update_display() -> void:
    var amputations := [false, false, false, false, false, false, false, false, false, false]
    amputations[2] = amp_arm_box.button_pressed # Left Arm
    amputations[6] = amp_leg_box.button_pressed # Left Leg

    var tex = NpcBridge.render_texture(
        current_app,
        amputations,
        anim_option.selected,
        dir_option.selected,
        anim_time
    )
    sprite.texture = tex
