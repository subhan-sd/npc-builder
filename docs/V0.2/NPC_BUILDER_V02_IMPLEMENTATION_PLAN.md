# NPC Builder v0.2 — Implementation Plan

## Phase Plan & Vertical Slice Strategy

### Vertical Slice (Phase 1 & 2): Default Anatomical Pixel Art Human
Goal: Render one high-quality human character matching Reference 1 (anatomical contours, head with jaw/neck, shoulders/deltoids, tapered waist, contoured legs with calves and feet) with 5-tone shading ramp.

---

### Step 1: Pixel Buffer & Drawing Primitives (`core/include/npc/pixel_buffer.hpp`)
- Add span drawing and outlined primitive helpers:
  - `draw_hline(x, y, len, color)`
  - `draw_shaded_rect(x, y, w, h, base_color, shadow_color, highlight_color, outline_color)`
- Ensure boundary safety & alpha composite.

### Step 2: C++ Anatomical Renderer (`core/src/generator.cpp`)
- Implement 5-color palette generation per skin tone (Highlight, Base, Shadow, Outline/Deep shadow).
- Define anatomical body spans:
  - **Head & Face:** 7x9 contoured head with rounded crown, jaw taper, eye sockets, nose tip, directional ear & chin profile.
  - **Torso & Shoulders:** Shoulder width derived from `muscularity` and `body_mass`. Trapezoid/chest contours with upper-left light source.
  - **Limbs:** Deltoid bulges, forearm taper, hand pixels. Thigh curve, calf bulge, foot base.
  - **Garments:** Structured shirt layered over torso with neckline and sleeve borders; trousers with waistband and knee crease.
- Keep all existing 6 animation states & 4 directions cleanly adapted to the new anatomical anchor points.

### Step 3: Test Suite Expansion (`tests/test_generator.cpp`)
- Verify anatomical pixel count and non-rectangular silhouette variations across Trim Jim, Gym Jim, Slim Jim, and Chin Jim.
- Validate determinism and amputation integrity with the new renderer.

### Step 4: Godot Bridge & Preview Sync (`godot/scripts/npc_bridge.gd`, `preview_ui.gd`, `preview.tscn`)
- Port the exact anatomical drawing logic to `npc_bridge.gd` so Windows Godot runs it natively out of the box with zero native compilation friction.
- Add `Muscularity` slider to the preview UI to showcase the Gym Jim vs. Slim Jim profiles directly.
- Polish preview frame: neutral dark studio backdrop with crisp nearest-neighbor display.

---

## Risks & Mitigations

| Risk | Mitigation |
|---|---|
| Platform mismatch: WSL `.so` cannot be loaded by Windows Godot | Maintain mirrored pixel-level rasterizer in `npc_bridge.gd` so preview works immediately on Windows without requiring cross-compilation toolchains. |
| Over-detailing causes pixel noise | Strictly follow the 5-color palette ramp and cluster rules from Reference 1 (large readable clusters, no single-pixel noise). |
| Animations break anatomical attachments | Use parameterized joint offsets (`torso_y`, `hips_y`, `shoulder_x`) rather than absolute screen coordinates. |
