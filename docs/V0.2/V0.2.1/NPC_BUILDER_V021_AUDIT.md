# NPC Builder v0.2.1 Audit Report

**Date:** 2026-10-24  
**Scope:** Living World Procedural Human NPC Builder — Visual Anatomy Refinement  
**Baseline:** v0.2 Commit `40734e9`

---

## 1. Executive Summary & Test Status

- **Build & Test Suite:** `./build/npc_test` runs 7 test cases (`RNG Determinism`, `Body variation & render`, `Reference 1 silhouettes`, `Amputation`, `Physical conditions`, `6 animation states x 4 directions`, `Serialization`) with 100% pass rate.
- **Visual Deficiencies Identified:** Current v0.2 generator still exhibits geometric boxiness:
  1. Default model still renders shirt (`shirt_color`) and trousers/shoes (`pants_color`) rather than an unclothed neutral anatomical study figure.
  2. The torso uses a single linear trapezoid from shoulder width to waist width, missing pectoral contour, ribcage curve, abdominal blocks, and iliac crest/pelvic taper.
  3. Arms are uniform vertical strips lacking deltoid bulb, elbow narrowing, forearm taper, and structured hands.
  4. Legs are simple vertical bars with only a 1px calf tick, missing thigh contour, knee cap definition, and angled feet.
  5. Posture slider only shifts head by ±1px horizontally and +1px vertically, without tilting torso, shoulders, spine, or pelvis.

---

## 2. Geometry & Anatomical Analysis

### 2.1 Torso & Pelvis
- **Current state:** `for (int y = 0; y < torso_h; ++y)` draws spans with linear lerp between `shoulder_w` and `waist_w`.
- **Reference 1 Study:**
  - Torso has 3 distinct zones: Upper pectorals/deltoids (wide), mid waist (tapered), and lower pelvis/hips (widening again into crotch).
  - Pectoral muscles create horizontal shadow lines.
  - Rectus abdominis creates vertical and horizontal divisions on muscular profiles (*Gym Jim*).
  - Heavy profiles (*Chin Jim*) have a pear-shaped rounded abdomen hanging slightly over the pelvis.

### 2.2 Arms & Hands
- **Current state:** Constant thickness `arm_thick` (2 or 3px).
- **Reference 1 Study:**
  - Deltoid caps outer shoulder (1px protrusion).
  - Upper arm narrows slightly at elbow joint.
  - Forearm bulges at brachioradialis then narrows at wrist.
  - Hand has distinct palm block and thumb indication.

### 2.3 Legs & Feet
- **Current state:** Constant vertical bar `leg_thick` from hip to ankle.
- **Reference 1 Study:**
  - Thigh is wider at hip socket and tapers down to knee.
  - Knee cap is a distinct 1-2px transition zone.
  - Calf bulges outward on lateral side.
  - Ankle pinches inward (narrowest point).
  - Foot extends forward 2-3px with sole contour.

### 2.4 Posture Mechanics
- **Current state:** Only `head_x += hunched_offset_x` and `head_y += 1`.
- **v0.2.1 Target:**
  - Side view (East/West): Posture shifts head forward 2-3px, tilts upper torso forward 1-2px, drops neck forward, arches spine shadow on back view, and relaxes arm angle forward.
  - Front view (South): Head drops slightly between shoulders; trapezius appears higher/tensed.
  - Back view (North): Spine curvature highlighted with deep shadow.

---

## 3. C++ Core vs GDScript Bridge Alignment

Both `core/src/generator.cpp` and `godot/scripts/npc_bridge.gd` currently share the exact same procedural formulas, dimensions, color ramp generation (`make_ramp`), and layer ordering. Any upgrade to the C++ rasterizer will be mirrored 1:1 in `npc_bridge.gd`.

---

## 4. Acceptance Criteria & Implementation Plan

### Phase 1: Unclothed Neutral Anatomical Model (FR-002)
- Remove `shirt_color` and `pants_color` from default anatomical preview; render entire body using `skin_ramp` with anatomical shading.
- Keep `shirt_color` and `pants_color` in data structure for future clothing layers (v0.3), but default preview displays pure anatomical form.

### Phase 2: Curvilinear Anatomical Rasterizer (FR-003, FR-005, FR-006, FR-007)
- Torso: Deltoid/trapezius shoulder arch, pectoral contour, abdominal shading, pelvic flare.
- Arms: Deltoid bulge, elbow pinch, forearm swell, wrist pinch, palm/hand contour.
- Legs: Thigh curve, knee joint, lateral calf swell, ankle pinch, foot heel & toe extension.

### Phase 3: Coherent Whole-Body Posture (FR-010)
- Parameter `posture` (0.0 to 1.0) produces multi-part coordinate transforms:
  - Upper torso lean `torso_lean_x = round(posture * 2.0)` in side view.
  - Head forward lean `head_lean_x = round(posture * 3.0)` in side view.
  - Shoulder and arm forward pivot.
  - Trapezius elevation in front/back views.

### Phase 4: Verification & Fixtures
- Update `tests/test_generator.cpp` with anatomical checks and posture deformation validation.
- Run tests and capture visual fixtures.
