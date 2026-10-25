# BRD v0.2.1 — Living World NPC Builder

## Visual Anatomy Refinement & Procedural Human Body System

**Project:** Living World — NPC Builder  
**Version:** 0.2.1  
**Type:** Visual Quality Upgrade / Technical Requirements  
**Status:** Ready for Repository Audit and Implementation  
**Engine:** Godot 4.x  
**Core Language:** C++20  
**Current Baseline:** NPC Builder v0.2  
**Primary Objective:** Improve anatomical quality, silhouette readability, body proportions, posture, and pixel-art rendering before introducing clothing and expanded character variation.

---

# 1. Executive Summary

## 1.1 Current situation

NPC Builder v0.2 has established a functional procedural human character renderer.

Existing capabilities reportedly include:

- C++ anatomical rasterizer.
- Five-color rendering palette.
- Procedural head, torso, shoulders, arms, legs, and feet.
- Height, body mass, muscularity, and posture controls.
- Six animation states.
- Four viewing directions.
- Amputation and physical-condition rendering.
- Deterministic generation.
- A Godot preview interface.

However, the current character still has visible anatomical limitations:

- Arms appear too straight and cylindrical.
- Shoulders and torso have insufficient anatomical transitions.
- The torso looks like a colored geometric block rather than a body with volume.
- Hands and feet lack convincing structure.
- The waist, pelvis, thighs, and calves need better transitions.
- Posture changes may not yet produce convincing whole-body deformation.
- Clothing obscures the anatomy that needs to be evaluated.
- The visual result is not yet as convincing as the supplied pixel-art reference.

The next release should concentrate on improving these weaknesses instead of adding unrelated functionality.

## 1.2 Business objective

Transform the current NPC renderer into a reliable procedural anatomy system capable of producing readable, distinct human body shapes.

The system must provide a good foundation for future:

- Male and female body profiles.
- Different ages and physical conditions.
- Clothing and equipment layers.
- Combat animations.
- Character aging.
- Injury and recovery.
- Human population simulation.
- Additional humanoid races.

The immediate goal is not to build every one of these features. It is to establish the underlying anatomical representation and visual quality that those features will depend on.

## 1.3 Core design decision

**v0.2.1 will use a neutral, unclothed anatomical study model as its default visual presentation.**

The model should resemble a simplified, nonsexual anatomical figure used for studying body proportions and movement.

It should include the head, neck, shoulders, chest, abdomen, back, pelvis, arms, hands, thighs, calves, and feet.

Do not render explicit genital detail. The visual emphasis is on anatomy, proportions, posture, and body mechanics.

This allows the developer to evaluate the actual body shape without clothing concealing important features.

---

# 2. Scope

## 2.1 In scope

- Audit and preserve the existing v0.2 implementation.
- Improve pixel-art anatomy and silhouettes.
- Remove clothing from the default anatomy preview.
- Improve head and neck transitions.
- Improve shoulder, chest, abdomen, waist, and pelvis shapes.
- Improve upper-arm, forearm, hand, thigh, calf, and foot shapes.
- Improve body shading and outline consistency.
- Improve height, body-mass, muscularity, and posture controls.
- Make posture changes visible from relevant directions.
- Preserve all current animation states.
- Preserve directional rendering.
- Preserve deterministic generation.
- Preserve amputation and existing condition rendering.
- Improve the preview composition where needed.
- Add automated tests and visual comparison fixtures.

## 2.2 Out of scope

Do not implement the following as part of this release unless a minimal change is necessary for compatibility:

- Full female anatomy implementation.
- Clothing, armor, or equipment systems.
- A large collection of hairstyles.
- Complex facial-expression systems.
- New races.
- A full skeletal animation engine.
- Realistic muscle simulation.
- High-resolution anatomical textures.
- Full-body 3D models.
- A complete rewrite of the NPC generator.
- A new Godot integration architecture.

The renderer should remain extensible so these capabilities can be introduced in later versions.

---

# 3. Reference Images and Visual Direction

The AI coding agent must inspect the supplied reference images directly before implementing visual changes.

## Reference A — Original pixel-art body-shape reference

**Suggested repository path:** `docs/references/body_shape_reference.png`

Purpose:

- Establish the intended pixel-art style.
- Compare silhouette differences.
- Evaluate muscularity and body mass.
- Establish the desired level of anatomical detail.
- Guide palette and shading decisions.

The reference contains clothed characters. Use it as a guide to style, body proportions, silhouette quality, and visual clarity—not as an instruction to retain clothing in the new anatomical preview.

## Reference B — Original v0.1 Godot preview

**Suggested repository path:** `docs/references/npc_builder_v01_baseline.png`

Purpose:

- Document the original functional preview.
- Establish a historical baseline.
- Preserve the context of the existing controls.

## Reference C — Current v0.2 Godot preview

**Suggested repository path:** `docs/references/npc_builder_v02_baseline.png`

Purpose:

- Show the current character's geometry.
- Identify the areas that need improvement.
- Prevent the agent from accidentally reverting to the primitive v0.1 renderer.

## Reference D — New v0.2.1 anatomical target

**Suggested repository path:** `docs/references/npc_anatomy_target.png`

Purpose:

- Establish a specific unclothed anatomical visual target.
- Show the intended degree of anatomical definition.
- Guide the relationship between body proportions, silhouette, and shading.

If Reference D is not available, the agent must use the requirements in this BRD to establish the first target and capture a proposed implementation screenshot for review.

Do not invent or assume that Reference D already exists.

## 3.1 Visual priorities

Apply these priorities in order:

1. Recognizable human silhouette.
2. Correct anatomical proportions.
3. Natural transitions between body parts.
4. Clear distinction between different body profiles.
5. Readable pixel-art shading.
6. Consistent outlines and palette.
7. Facial and smaller anatomical details.

A character with a convincing silhouette and restrained shading is preferable to a character with many details but poor proportions.

---

# 4. Functional Requirements

## FR-001 — Preserve the existing v0.2 foundation

**Priority: P0**

Before changing code, inspect the existing repository and determine how the current implementation works.

Relevant areas reportedly include:

- `core/include/npc/pixel_buffer.hpp`
- `core/include/npc/appearance.hpp`
- `core/include/npc/conditions.hpp`
- `core/include/npc/generator.hpp`
- `core/include/npc/types.hpp`
- `core/src/generator.cpp`
- `core/src/c_api.cpp`
- `tests/test_generator.cpp`
- `godot/scripts/npc_bridge.gd`
- `godot/scripts/preview_ui.gd`
- `godot/project.godot`
- Existing Godot scenes and build configuration.

These are starting points, not a complete list. The agent must inspect the actual repository.

Requirements:

1. Run the existing test suite before making changes.
2. Record any existing failures.
3. Identify the current rasterization path.
4. Identify the current data model for height, mass, muscularity, posture, animation, direction, and conditions.
5. Identify whether C++ and GDScript use separate rasterizers.
6. Determine how their outputs are synchronized.
7. Reuse existing functionality where appropriate.

**Acceptance criteria:**

- Existing functionality is documented.
- No unrelated architecture rewrite is performed.
- Existing public interfaces remain compatible unless a change is justified.
- Tests can distinguish pre-existing failures from regressions.

## FR-002 — Default anatomical preview without clothing

**Priority: P0**

Replace the default clothed NPC presentation with a neutral anatomical model.

The default character must visibly expose:

- Head and neck.
- Shoulders.
- Chest and upper torso.
- Abdomen.
- Waist.
- Pelvis and hips.
- Upper arms.
- Forearms.
- Hands.
- Thighs.
- Knees.
- Calves.
- Ankles.
- Feet.

The model must remain nonsexual and focused on anatomical structure.

### Requirements

- Do not render a shirt, trousers, shoes, or decorative garments in the default anatomical preview.
- Do not leave remnants of clothing as unexplained colored blocks.
- Do not simply remove the existing clothing colors and leave a rectangular torso.
- Reconstruct the body contours and shading so that the exposed anatomical regions look intentional.
- Keep the head and face consistent with the existing visual style.
- Use a coherent skin palette across all visible anatomical regions.
- Make the torso readable without clothing seams or garment outlines.

If the existing renderer currently uses clothing geometry to define the torso silhouette, the agent must replace that geometry with actual body geometry rather than merely disabling the clothing layer.

### Acceptance criteria

- The default preview contains no clothing.
- The torso is anatomically shaped rather than rectangular.
- The waist and pelvis are visually distinguishable.
- The shoulders transition naturally into the arms.
- The legs have distinct thigh, knee, and calf regions.
- The silhouette remains readable at the standard preview scale.

## FR-003 — Anatomical proportions

**Priority: P0**

The renderer must use coherent relationships between body regions.

The following measurements should be represented as parameters or derived geometry where practical:

- Total body height.
- Head height and width.
- Neck width and length.
- Shoulder width.
- Chest width.
- Upper-torso length.
- Waist width.
- Pelvis width.
- Upper-arm length.
- Forearm length.
- Hand size.
- Thigh length.
- Calf length.
- Foot length.

The implementation does not need to expose every measurement as a UI control.

The purpose is to allow the renderer to produce a coherent body when the primary parameters change.

### Requirements

- The head must remain proportionate to the body.
- The neck must connect the head to the torso.
- The shoulders must not look like two disconnected blocks.
- The chest must transition into the waist.
- The waist must transition into the pelvis.
- Arms must have a readable upper-arm and forearm structure.
- Legs must have a readable thigh, knee, and calf structure.
- Hands and feet must be aligned with their respective limbs.

Do not use independent arbitrary scaling for every component if it causes disconnected anatomy.

## FR-004 — Head and neck refinement

**Priority: P0**

Improve the existing head and neck without unnecessarily replacing the current facial generation system.

Requirements:

- A recognizable head contour.
- A coherent jaw and chin shape.
- Appropriate head-width variation where supported.
- A visible neck connecting the head to the torso.
- Correct alignment between the head, neck, and shoulders.
- Consistent face placement across body profiles.
- Appropriate changes when the direction changes.
- Appropriate movement when posture changes.

The head must not appear to float above the shoulders.

For a hunched character, the relationship between head, neck, and shoulders must change coherently.

## FR-005 — Shoulder and torso refinement

**Priority: P0**

The torso must no longer resemble a flat rectangular column.

### Shoulder region

Implement readable shoulder contours.

- Shoulders must connect the neck, chest, and upper arms.
- Shoulder width must reflect the body profile.
- Muscular characters should have broader, more pronounced shoulders.
- Slim characters should have narrower shoulders.
- Heavy characters should not simply have enlarged shoulders without corresponding changes elsewhere.

### Chest

The chest should have a readable shape that changes with muscularity and body mass.

- Muscularity may influence chest definition and upper-body contours.
- Body mass may influence torso volume and softness.
- Shading must communicate form rather than create random visual noise.

### Abdomen

The abdomen should be a distinct region between the chest and pelvis.

- Muscularity may produce visible abdominal definition where appropriate.
- Higher body mass may produce a fuller abdominal silhouette.
- The default profile should remain readable without excessive detail.

### Waist and pelvis

- The waist must transition naturally from the chest to the pelvis.
- Hip width must respond to the selected body profile.
- The pelvis must provide a plausible attachment point for both legs.
- The crotch and upper-leg junction must not form a large unintended gap.
- The model should remain suitable for future animation and clothing layers.

Do not require explicit genital anatomy.

## FR-006 — Arm anatomy refinement

**Priority: P0**

The arms must have a more natural silhouette than straight rectangular columns.

Required regions:

- Shoulder attachment.
- Upper arm.
- Elbow transition.
- Forearm.
- Wrist.
- Hand.

### Upper arm

- Connect naturally to the shoulder.
- Respond to muscularity.
- Remain proportionate to the torso.

### Elbow

- Provide a readable transition between upper arm and forearm.
- Must not create an unintended sharp gap.

### Forearm

- Have a coherent taper or contour.
- Must not be uniformly cylindrical for all body profiles.

### Wrist and hand

- The wrist must be distinguishable where resolution permits.
- The hand must have a recognizable silhouette.
- Hands must remain aligned with the forearms.
- Hand shape must remain coherent during animation.

The first release may simplify individual fingers to preserve pixel readability.

## FR-007 — Leg anatomy refinement

**Priority: P0**

The legs must have more natural proportions and contours.

Required regions:

- Upper thigh.
- Knee.
- Calf.
- Ankle.
- Foot.

### Thighs

- Connect naturally to the pelvis.
- Respond to body mass and muscularity.
- Maintain plausible spacing between the legs.

### Knees

- Provide a readable transition between the thigh and calf.
- Avoid abrupt changes in width that resemble disconnected blocks.

### Calves

- Have a visible contour.
- May use subtle tapering and a wider calf region where resolution permits.
- Must remain consistent across directions.

### Ankles and feet

- Feet must connect naturally to the ankles.
- Foot length and width must remain proportionate.
- The feet must provide a stable visual base.
- The character should not appear to stand on detached rectangular blocks.

Animation may alter limb geometry or position, but it must preserve coherent attachment.

## FR-008 — Body-shape profiles

**Priority: P0**

Preserve and improve the existing profile variations.

Required profiles:

1. Slim.
2. Average or Trim.
3. Muscular or Gym.
4. Heavy or Chin.
5. Tall.
6. Short.
7. Hunched.

The implementation must allow combinations where supported by the current model.

Examples:

- Tall + slim.
- Short + heavy.
- Average height + muscular.
- Tall + hunched.
- Heavy + hunched.

### Slim

- Narrower torso.
- Narrower arms and legs.
- Reduced waist and hip width.
- Clear but restrained anatomical definition.

### Average

- Balanced shoulder, torso, pelvis, and limb proportions.
- Serves as the baseline for comparison.

### Muscular

- Wider shoulders.
- More pronounced upper arms.
- Fuller chest.
- More defined thighs and calves.
- Clear anatomical definition without relying on clothing.

### Heavy

- Fuller torso.
- Wider waist and abdomen.
- Appropriate hip and limb proportions.
- Softer transitions between body regions.

### Tall and short

- Change total height coherently.
- Avoid merely stretching or uniformly shrinking the whole image.
- Maintain sensible relationships between head, torso, and limbs.

### Hunched

- Adjust the relationship between head, neck, shoulders, torso, and pelvis.
- Produce a distinct posture rather than simply moving the head.

### Acceptance criteria

- Each profile is visually distinguishable.
- Profiles do not depend on clothing colors to appear different.
- Changing a profile does not disconnect anatomical components.
- The same profile is reproducible for the same seed and parameters.

## FR-009 — Muscularity and body-mass controls

**Priority: P0**

The existing controls must remain functional and gain more meaningful anatomical effects.

### Muscularity

Must affect the underlying body shape, including where appropriate:

- Shoulder width.
- Upper-arm volume.
- Chest definition.
- Torso contour.
- Thigh and calf volume.
- Visible muscle separation.

It must not merely increase the brightness or contrast of the entire character.

### Body mass

Must affect:

- Torso volume.
- Waist and abdominal shape.
- Hip width where appropriate.
- Upper-arm and thigh volume.
- Overall silhouette.

It must not be treated as a direct substitute for muscularity.

A muscular character and a heavy character should remain visually distinguishable even if their overall widths are similar.

### Acceptance criteria

- Adjusting muscularity produces visible anatomical changes.
- Adjusting body mass produces visible changes in volume.
- The two controls are not visually redundant.
- Extreme supported values remain within valid anatomical bounds.

## FR-010 — Posture control

**Priority: P0**

The existing Posture slider controls the degree of forward-hunched posture.

The agent must inspect the existing parameter range and preserve it if practical. If the current slider uses a normalized range of 0.0–1.0, the following behavior applies.

### Posture = 0.0

Expected:

- Upright stance.
- Head aligned above the neck and torso.
- Shoulders in a neutral position.
- Spine and torso without intentional forward curvature.
- Normal arm and leg alignment.

### Posture = approximately 0.3

Expected:

- Mild forward lean.
- Small change in head and neck alignment.
- Slightly altered shoulder position.
- Clearly distinguishable from the fully upright pose when viewed from the side.

### Posture = approximately 0.6

Expected:

- Moderate forward lean.
- Head and upper torso visibly displaced forward in side view.
- Neck and shoulders adjusted coherently.
- Slightly altered arm hang or shoulder alignment where appropriate.

### Posture = 1.0

Expected:

- Strong but anatomically coherent hunch.
- Clearly distinguishable silhouette.
- No detached head, floating shoulders, or disconnected torso.
- Legs and pelvis remain capable of supporting a standing pose.

These values are illustrative acceptance points, not a demand to use a specific mathematical curve.

### Directional behavior

**East/West — side views**

- The forward direction must be defined consistently.
- Head and upper torso must lean forward relative to the pelvis.
- The neck must connect the head and torso.
- The shoulder line should change coherently.
- The silhouette must communicate hunching clearly.

**South — front view**

- Head may sit slightly lower between the shoulders.
- Shoulder and neck relationships should reflect the changed posture.
- The body must not simply become shorter without showing a coherent pose.

**North — back view**

- The upper back and shoulder relationship should reflect the hunched posture.
- Head placement must remain consistent with the side view.

### Technical guidance

Use a coherent body-pose transformation or derived anatomical coordinates.

Do not implement posture by changing only the head's x-coordinate.

Do not apply unrelated random offsets to individual body parts.

The implementation should keep posture deterministic and compatible with the existing animation system.

## FR-011 — Pixel-art palette and shading

**Priority: P0**

Preserve the existing five-color palette approach if it is suitable, but improve how colors are used.

The palette should distinguish:

- Highlight.
- Base skin tone.
- Shadow.
- Deep shadow.
- Outline.

The exact color values may be adjusted after visual comparison with the reference.

Requirements:

- Use consistent lighting direction.
- Use deliberate pixel clusters.
- Define contours through shape and shading.
- Avoid unnecessary isolated pixels.
- Avoid noisy textures.
- Avoid outlines that are disproportionately thick.
- Maintain readability at native sprite resolution.
- Use shading to communicate the chest, abdomen, shoulders, arms, thighs, and calves.

Not every anatomical region requires all five colors in every frame.

The agent must prioritize form and readability rather than applying all palette colors uniformly.

## FR-012 — Pixel scale and silhouette clarity

**Priority: P0**

The renderer must preserve a coherent pixel grid.

Requirements:

- Keep the internal character resolution consistent.
- Use nearest-neighbor scaling where appropriate.
- Avoid unintended texture blur.
- Avoid inconsistent pixel sizes.
- Keep the character centered and correctly framed.
- Avoid cropping the head, hands, or feet.
- Ensure the silhouette remains readable when zoomed out.

The agent must inspect the result at the actual Godot preview size, not only in enlarged screenshots.

## FR-013 — Animation compatibility

**Priority: P0**

Preserve the existing animation categories:

- Idle.
- Walk.
- Run.
- Attack.
- Fall.
- Swim.

The anatomy refinement must not improve the idle frame at the expense of breaking the other animations.

Requirements:

- Body parts remain connected across frames.
- The feet do not randomly jump.
- Arm and leg movement remains coherent.
- The torso and head respond appropriately to the animation.
- The posture parameter remains compatible with the pose system.
- The character maintains a consistent visual identity across frames.
- The character does not revert to a primitive shape during animation.

At minimum, inspect the idle, walk, and run animations after the anatomical changes. Then verify the remaining animation states before declaring the release complete.

## FR-014 — Direction compatibility

**Priority: P0**

Preserve the four existing viewing directions:

- South/front.
- North/back.
- East/side.
- West/side.

Requirements:

- The body must use direction-appropriate contours.
- Facial details must be hidden or adjusted where appropriate.
- Side views must make posture differences easy to inspect.
- Front and back views must preserve coherent body width and attachment points.
- Left/right asymmetries must not be accidentally reversed.
- The same appearance parameters must be used across all directions.

A front-view model must not simply be reused unchanged for every direction.

## FR-015 — Amputation and condition compatibility

**Priority: P0**

Preserve the current physical-condition behavior.

Existing conditions reportedly include amputations, cuts, blood, bandages, scars, and bruises.

Requirements:

- Missing limbs must remain absent.
- The anatomy beneath a condition must not be rendered as an intact limb if it has been amputated.
- Conditions must remain attached to the correct anatomical region.
- Wounds and bandages must follow their body part during animation.
- Conditions must remain correctly placed across supported directions.
- Body-shape changes must not place conditions at arbitrary unrelated coordinates.

The new anatomical renderer must not break existing condition rendering.

Do not expand the condition feature set in this release unless necessary to preserve existing behavior.

## FR-016 — Godot preview improvements

**Priority: P0**

The existing NPC Builder Studio must remain functional.

Required controls:

- Seed.
- Height.
- Body mass.
- Muscularity.
- Posture.
- Animation.
- Direction.
- Existing amputation toggles.

The preview should make anatomy the primary focus.

Requirements:

- The character should be sufficiently large to inspect.
- The background should provide clear contrast.
- Controls must not overlap the character.
- The character should remain centered.
- The entire body should remain visible.
- Slider changes should update the preview.
- Existing controls should not disappear during UI refinement.

Optional: provide a zoom control or a comparison mode if it can be added without disrupting the existing layout.

Do not add new UI controls solely for appearance if they do not help evaluate the anatomy.

## FR-017 — Determinism

**Priority: P0**

Preserve the current seed-based generation behavior.

Requirements:

- The same seed and the same parameters must reproduce the same character.
- Changing posture must not randomly change unrelated appearance parameters.
- Changing muscularity must not randomly change hair or skin color.
- Rendering the same animation frame repeatedly must produce the same output.
- Visual changes caused by a parameter must be attributable to that parameter or documented dependencies.

If the current implementation uses separate random streams or stable derived seeds, reuse them.

## FR-018 — Performance and memory safety

**Priority: P0**

The revised renderer must remain suitable for interactive preview.

Requirements:

- Avoid unnecessary full regeneration during every idle frame.
- Cache generated geometry or textures where appropriate.
- Keep pixel-buffer accesses within valid bounds.
- Validate dimensions and parameters.
- Avoid introducing unbounded allocations.
- Maintain responsive UI interaction.
- Avoid duplicating the entire generator in multiple languages without a documented reason.

Record any significant performance regression and explain its cause.

---

# 5. Technical Design Principles

## 5.1 Procedural anatomy, not a collection of unrelated sprites

The renderer should generate anatomy from a shared representation.

A body profile should define proportions and shapes. The pose system should define how body parts are positioned. The rasterizer should convert those shapes into pixels.

The system may use a combination of:

- Parameterized geometry.
- Pixel masks.
- Anatomical templates.
- Reusable contour functions.
- Shading functions.
- Layer composition.

The BRD does not require a particular mathematical representation.

The agent should choose the simplest approach that works with the current C++ pixel-buffer architecture.

## 5.2 Shared source of truth

The existing project reportedly contains both C++ rendering code and a GDScript rasterizer.

The agent must determine whether the two implementations are currently synchronized and whether both are still necessary.

Avoid maintaining two independent copies of complex anatomical geometry if a simpler shared implementation is feasible.

However, do not remove the Windows-compatible rendering path before proving that the replacement can actually run in the target Godot environment.

Any change to the bridge must include build and runtime verification.

## 5.3 Future extensibility

Although female anatomy and clothing are out of scope for this release, the architecture must not assume that every character has the same proportions.

The model should permit future profile variation without requiring a completely separate generator for each character type.

The anatomy renderer should also be separable from:

- Permanent appearance.
- Animation state.
- Physical conditions.
- Clothing.
- Equipment.

This separation will make later extensions easier.

---

# 6. Implementation Plan

## Phase 0 — Audit and baseline

Tasks:

1. Read the existing v0.2 implementation.
2. Run existing tests.
3. Capture the current preview.
4. Inspect the current rasterizer and pixel buffer.
5. Identify the source of the rectangular torso and cylindrical limbs.
6. Identify how body-mass, muscularity, and posture affect geometry.
7. Inspect all four directions and six animation states.
8. Identify whether C++ and GDScript produce identical or equivalent outputs.
9. Write a file-by-file implementation plan.

Deliverable:

`docs/NPC_BUILDER_V021_AUDIT.md`

Exit criteria:

The agent can explain the current geometry-generation process and identify specific weaknesses before changing the code.

## Phase 1 — Default anatomical character

Tasks:

1. Remove clothing from the default anatomical presentation.
2. Replace clothing-dependent torso geometry with anatomical geometry.
3. Refine the shoulder-to-arm transition.
4. Refine chest, abdomen, waist, and pelvis.
5. Refine thigh, knee, calf, ankle, and foot contours.
6. Improve shading and silhouette readability.
7. Validate the default character in Godot.

Deliverable:

One convincing neutral human anatomical model.

Exit criteria:

The character no longer resembles a rectangular torso with cylindrical arms and legs.

## Phase 2 — Body-shape refinement

Tasks:

1. Refine slim.
2. Refine average.
3. Refine muscular.
4. Refine heavy.
5. Refine tall.
6. Refine short.
7. Refine hunched.
8. Verify interactions between height, mass, muscularity, and posture.

Deliverable:

A coherent collection of body profiles produced by the same rendering system.

Exit criteria:

Profiles remain visibly distinct without relying on clothing.

## Phase 3 — Posture and directional consistency

Tasks:

1. Define the posture transformation.
2. Improve side-view hunching.
3. Improve front-view posture.
4. Improve back-view posture.
5. Validate head and neck attachment.
6. Validate shoulder and arm alignment.
7. Validate posture compatibility with animation.

Deliverable:

A posture system with predictable and coherent visual results.

Exit criteria:

The posture slider changes the body in a way that can be clearly inspected.

## Phase 4 — Animation and condition regression

Tasks:

1. Test idle.
2. Test walk.
3. Test run.
4. Test attack.
5. Test fall.
6. Test swim.
7. Test amputation states.
8. Test injuries and bandages.
9. Test all four directions.
10. Fix visual defects caused by the anatomical changes.

Deliverable:

A stable renderer compatible with existing animation and condition functionality.

## Phase 5 — Visual validation and polish

Tasks:

1. Compare the new model with the reference images.
2. Inspect the character at native preview size.
3. Inspect an enlarged pixel-art view.
4. Capture body-profile comparison screenshots.
5. Capture posture comparison screenshots.
6. Run automated tests.
7. Launch the actual Godot project.
8. Document remaining limitations.

Deliverable:

A tested v0.2.1 preview and implementation report.

---

# 7. Testing and Acceptance Criteria

## 7.1 Visual fixture matrix

The agent must produce a comparison set containing:

| Fixture   | Required observation                       |
| --------- | ------------------------------------------ |
| Average   | Balanced default anatomy                   |
| Slim      | Narrow torso and limbs                     |
| Muscular  | Wider shoulders and defined muscle groups  |
| Heavy     | Fuller torso, waist, and limbs             |
| Tall      | Increased height with coherent proportions |
| Short     | Reduced height with coherent proportions   |
| Hunched   | Clear forward-leaning posture              |
| Front     | Readable frontal anatomy                   |
| Back      | Coherent rear anatomy                      |
| Side      | Clear body contour and posture             |
| Walk      | Attached limbs and readable movement       |
| Run       | Dynamic but coherent pose                  |
| Amputated | Missing limb correctly represented         |
| Injured   | Condition overlays remain attached         |

Each fixture should use known parameter values and a recorded seed.

## 7.2 Posture regression tests

Test posture at:

- 0.0.
- Approximately 0.3.
- Approximately 0.6.
- 1.0.

For each value, inspect front, back, and side views.

Acceptance criteria:

- Posture changes are visible.
- Side-view forward lean is clear.
- Head and neck remain connected.
- The torso changes coherently.
- Front and back views do not become malformed.
- The standing pose remains recognizable.

## 7.3 Automated tests

Add or extend tests for:

- Body-profile parameter validity.
- Height and body-mass bounds.
- Muscularity behavior.
- Posture behavior.
- Deterministic output.
- Pixel-buffer bounds.
- Amputation compatibility.
- Animation compatibility.
- Condition placement where practical.

A test that merely checks whether the output image is nonempty is insufficient to prove anatomical quality.

Tests should verify meaningful properties such as output dimensions, silhouette differences, valid parameter ranges, and deterministic results.

## 7.4 Visual acceptance

The implementation is accepted only when:

- [ ] The default character has no clothing.
- [ ] The torso is anatomically shaped.
- [ ] Shoulders transition naturally into the arms.
- [ ] The waist and pelvis are distinguishable.
- [ ] Arms have readable upper-arm and forearm shapes.
- [ ] Legs have readable thigh, knee, and calf shapes.
- [ ] Hands and feet look intentional.
- [ ] Body profiles are distinguishable.
- [ ] Muscularity and body mass have different effects.
- [ ] Posture produces a coherent body transformation.
- [ ] Pixel edges remain crisp.
- [ ] Existing animations still work.
- [ ] Existing physical conditions still work.
- [ ] The Godot preview remains functional.
- [ ] Tests pass, or pre-existing failures are documented.
- [ ] A final screenshot demonstrates the improvements.

---

# 8. Instructions to the AI Coding Agent

Read this BRD and inspect the supplied image references before implementation.

## Mandatory workflow

1. Audit the existing repository.
2. Run existing tests.
3. Record the baseline.
4. Produce an implementation plan.
5. Implement the default anatomical character first.
6. Verify it in the actual Godot runtime.
7. Refine body profiles.
8. Refine posture.
9. Verify animations and conditions.
10. Run tests and produce comparison screenshots.

## Restrictions

- Do not rewrite the entire generator.
- Do not remove existing working controls.
- Do not implement female anatomy as a substitute for fixing the current baseline.
- Do not add clothing to the default anatomical preview.
- Do not create separate unrelated renderers for each body profile.
- Do not rely on palette changes alone to create anatomical differences.
- Do not hardcode posture as a head-only offset.
- Do not break the existing C++/Godot bridge.
- Do not claim success based only on C++ unit tests.
- Do not declare the release complete without launching the actual Godot project.
- Do not claim visual quality based only on silhouette pixel counts.

## Expected implementation report

At completion, report:

1. Files modified.
2. Architectural changes.
3. Anatomical improvements.
4. Behavior of the Posture slider.
5. Body-profile comparison results.
6. Animation and condition regression results.
7. Build and test commands.
8. Final Godot screenshot.
9. Known limitations.
10. Recommended next steps.

The implementation must distinguish completed, partial, and unimplemented requirements.

---

# 9. Definition of Done

NPC Builder v0.2.1 is complete when the application displays a convincing, unclothed procedural human anatomical model whose proportions can be inspected and adjusted through the existing controls.

The model must support the existing body profiles, posture adjustment, directional presentation, animations, deterministic generation, and physical-condition rendering.

The project must remain buildable and runnable in the actual target environment.

The result should provide a stable visual foundation for future female body profiles, clothing, equipment, and more advanced Living World character systems.

**Release principle:** Fix the anatomy first. Expand the character content only after the underlying body representation is convincing and reusable.
