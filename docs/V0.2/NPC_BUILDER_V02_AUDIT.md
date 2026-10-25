# NPC Builder v0.2 — Repository Audit Report

## 1. Executive Summary & Context
- **Workspace:** Linux / WSL (`/home/subhan/Project/npc-builder`).
- **Engine / Target Environment:** Godot 4.x running on Windows host, while build toolchain (CMake 3.28, GCC 13.3 C++20) runs in WSL.
- **Reference 1 Target Analysis:** "Body Shape Studies" (16+ x 48 pixel art canvas, 5-color palette: deep contour, shadow tone, base tone, highlight tone, and dark accent/outline). Distinct silhouettes:
  - *Trim Jim* (average / balanced silhouette)
  - *Gym Jim* (muscular, wide deltoids, trapezoids, chest definition, taper waist)
  - *Slim Jim* (narrow chest, thin limbs, lean posture)
  - *Chin Jim* (heavy build, wide torso, round abdomen, thick calves and thighs)
  - *Grim Jim* (skeletal / extreme thin)
- **Reference 2 (v0.1 Current Reality):** Blocky rectangular boxes, plain 2-pixel eyes, no anatomy, no contour shading, flat colored clothing.

---

## 2. Test Suite Status
Current C++ test suite (`./build/npc_test`):
- `test_rng_determinism`: PASS
- `test_body_variations_and_render`: PASS (561 non-empty pixels)
- `test_amputations`: PASS (silhouette reduced from 498 to 429 px)
- `test_conditions_and_injuries`: PASS
- `test_all_animation_states`: PASS (6 states x 4 directions)
- `test_serialization`: PASS

---

## 3. Architecture & Rendering Pipeline Audit

### 3.1 C++20 Core
- **`core/include/npc/types.hpp`**:
  - `Direction` (South, North, East, West).
  - `AnimationState` (Idle, Walk, Run, Attack, Fall, Swim).
  - `BodyPart` (Head, Torso, LeftArm, RightArm, LeftHand, RightHand, LeftLeg, RightLeg, LeftFoot, RightFoot).
  - `ConditionType`, `Expression`, `ColorRGBA`.
- **`core/include/npc/rng.hpp`**:
  - SplitMix64 PRNG: deterministic, cross-platform float & integer range generation.
- **`core/include/npc/appearance.hpp`**:
  - `BodyConfiguration` (height, body_mass, muscularity, posture, head_scale, torso_ratio, arm_ratio, leg_ratio).
  - `FaceConfiguration` (hair_style, expression, colors).
  - `HumanAppearance` (seed, generator_version, npc_id).
- **`core/include/npc/conditions.hpp`**:
  - `PhysicalConditionSet` (amputations array with cascade logic, injuries list).
- **`core/include/npc/pixel_buffer.hpp`**:
  - 2D canvas with alpha compositing and `fill_rect`.
- **`core/src/generator.cpp`**:
  - Currently renders flat rectangles for head, torso, arms, legs.
  - This is where the anatomy & pixel cluster upgrade needs to occur.
- **`core/src/c_api.cpp`**:
  - Exports `npc_generate_appearance` and `npc_render_pixels` to `libnpc_builder.so`.

### 3.2 Godot Bridge & UI
- **`godot/scripts/npc_bridge.gd`**:
  - Contains duplicate GDScript fallback logic that mimics `generator.cpp`'s primitive rectangles so that Windows Godot can render without compiling a Windows `.dll` directly inside WSL.
  - Generates an `ImageTexture` (32x48) dynamically into Godot's `TextureRect` with `texture_filter = 1` (Nearest).
- **`godot/scripts/preview_ui.gd` & `godot/scenes/preview.tscn`**:
  - Controls: Seed, Height, Body Mass, Animation, Direction, Amputations.

---

## 4. Reusable vs. Must-Change Components

| Component | Status | Recommendation |
|---|---|---|
| `types.hpp` | Reusable | Keep intact; already has all 10 body parts, directions, states. |
| `rng.hpp` | Reusable | SplitMix64 is deterministic and rock solid. |
| `appearance.hpp` | Reusable / Minor extension | Struct already has mass, height, muscularity, posture, ratios. |
| `conditions.hpp` | Reusable | Cascade amputation and injuries already functional. |
| `serializer.hpp` | Reusable | Deterministic save/load verified. |
| `pixel_buffer.hpp` | Must extend | Add drawing primitives for anatomical spans/spans with contour shading (e.g. `draw_span`, `draw_cluster`, `draw_outline`). |
| `generator.cpp` | Must upgrade | Replace primitive rectangles with anatomical rasterizer (head contour, chest/deltoids, tapered waist, calf shaping, 5-tone ramp shading). |
| `c_api.cpp` | Reusable | C-ABI `npc_render_pixels` signature remains stable. |
| `npc_bridge.gd` | Must upgrade | Synchronize GDScript renderer with the new C++ anatomical pixel-art logic so Windows Godot immediately displays the new visual quality. |
| `preview_ui.gd` / `preview.tscn` | Must polish | Add muscularity & posture sliders, improve contrast framing as requested in BRD v0.2. |

---

## 5. Gap Analysis (v0.1 vs. v0.2)

1. **Silhouette & Anatomy:**
   - *v0.1:* Single rectangle torso, uniform arm/leg sticks.
   - *v0.2:* Tapered torso (deltoids/shoulders wider than waist in muscular/average), contoured neck, distinct head oval/chin, calf/foot taper, hand buds.
2. **Shading & Volume:**
   - *v0.1:* 1 flat color for skin, 1 flat color for shirt, 1 for pants.
   - *v0.2:* 5-color cohesive palette: Deep Outline (`#49352F`), Skin Shadow (`#A66C55`), Skin Base (`#D9A078`), Highlight (`#F0C49A`), Garment tones.
3. **Body Profiles:**
   - *v0.1:* Height and Mass only scale rectangle dimensions.
   - *v0.2:* Anatomical mapping to *Trim Jim*, *Gym Jim*, *Slim Jim*, and *Chin Jim* builds.
4. **Platform Compatibility (WSL -> Windows Godot):**
   - The user runs Godot on Windows accessing WSL workspace. `npc_bridge.gd` must cleanly provide the exact same pixel-perfect anatomical renderer in GDScript as the C++ core, ensuring seamless operation on Windows without requiring MSVC/MinGW toolchain setup.
