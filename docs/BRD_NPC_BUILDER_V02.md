# BRD v0.2 — Living World NPC Builder

## Procedural Pixel-Art Character Rendering, Anatomy, Appearance & Animation Upgrade

**Project:** Living World — NPC Builder  
**Version:** 0.2.0  
**Document Type:** Business Requirements Document + Visual Technical Specification + Implementation Requirements  
**Status:** Proposed — Ready for Repository Audit and Implementation Planning  
**Target Engine:** Godot 4.x  
**Simulation Language:** C++20  
**Existing Integration:** C++ NPC Core + Godot UI/Preview  
**Primary Platform:** Windows development environment with the repository and AI coding workflow currently in WSL  
**Development Principle:** Preserve existing behavior, improve visual quality incrementally, and keep C++ as the source of truth for NPC generation.

---

# 1. Executive Summary

## 1.1 Background

NPC Builder v0.1 has successfully established a functional foundation for procedural human character generation.

The current application already has a Godot preview interface with controls for:

- Seed.
- Character height.
- Body mass.
- Animation selection.
- Facing direction.
- Left-arm amputation.
- Left-leg amputation.

The current implementation can display a character and expose these parameters through the UI. However, the rendered NPC is visually primitive, consisting of rectangular body parts, simple eyes, flat colors, and minimal anatomical detail.

This is acceptable as an initial proof of concept, but it is insufficient for the intended Living World experience.

Living World requires NPCs that remain visually readable when many characters coexist in a simulated world. Each NPC should have a recognizable silhouette, distinguishable anatomy, procedural appearance, and animation that communicates its current state.

## 1.2 Business Objective

Upgrade the NPC Builder from a primitive procedural character preview into a reusable **2D procedural pixel-art character rendering system**.

The upgraded system must allow the simulation to generate visually varied human NPCs without requiring a unique manually authored sprite for every individual.

A character should be constructed from procedural anatomy, appearance parameters, a coherent pixel-art palette, and animation poses.

The same character-generation system should eventually support the broader Living World simulation, including:

- Population simulation.
- Family and lineage.
- Character aging.
- Physical injuries.
- Equipment and clothing.
- Combat and movement.
- Different human body types.
- Future nonhuman races through a separate extension of the architecture.

## 1.3 Visual Target

The first attached reference image, showing multiple body shapes in a pixel-art style, is the primary visual reference.

The second attached screenshot, showing the current Godot application, is the baseline to improve.

The implementation must preserve the current functional controls while substantially improving the character representation.

### Expected transformation

| Dimension   | Current v0.1                      | Target v0.2                                                        |
| ----------- | --------------------------------- | ------------------------------------------------------------------ |
| Body        | Primitive rectangles              | Recognizable head, torso, arms, legs, hands, and feet              |
| Silhouette  | Block-like                        | Distinct anatomical proportions                                    |
| Body shapes | Basic height and mass             | Visually distinguishable slim, muscular, average, and heavy builds |
| Face        | Simple square head and eyes       | Pixel-art facial structure, hair, eyes, nose, and mouth            |
| Color       | Flat colors                       | Coordinated skin, shadow, highlight, hair, and clothing palettes   |
| Shading     | Minimal or absent                 | Pixel clusters, contour shading, highlights, and volume            |
| Clothing    | Basic rectangular shirt           | Structured garments with readable folds and borders                |
| Animation   | Primitive or placeholder motion   | Coherent pixel-art poses with visible movement                     |
| Direction   | Existing direction selection      | Consistent directional character construction                      |
| Amputation  | Existing checkbox functionality   | Missing limb visibly reflected in the rendered character           |
| Preview     | Functional controls and basic NPC | Character studio with useful framing and visual inspection         |

The goal is not photorealism. The goal is a coherent, attractive, readable, game-ready pixel-art character.

---

# 2. Scope and Non-Goals

## 2.1 Included in v0.2

1. Audit the current C++ core, Godot scripts, scenes, and rendering pipeline.
2. Replace the primitive character renderer with a modular procedural pixel-art renderer.
3. Establish a consistent character coordinate system and pixel scale.
4. Implement anatomical components and body-shape profiles.
5. Implement procedural face, hair, skin, and clothing details.
6. Add deterministic appearance variation based on seed.
7. Integrate existing height and body-mass parameters with visible anatomy.
8. Implement or improve directional presentation.
9. Implement or improve animation presentation.
10. Preserve and visibly render amputation states.
11. Add visual states for injuries and bandages if the existing data model supports them.
12. Improve preview composition, scaling, and background presentation.
13. Add automated and manual visual regression tests.
14. Document the rendering architecture and asset-generation strategy.

## 2.2 Explicitly out of scope

Unless the repository audit demonstrates that an item already exists and can be reused without expanding scope, v0.2 does not require:

- Full Living World simulation.
- Population management or civilization AI.
- Multiplayer.
- A complete inventory or equipment system.
- A full combat system.
- Complex physics or ragdoll simulation.
- Realistic skeletal deformation.
- A complete human anatomy simulator.
- Procedural generation of every possible clothing item.
- Nonhuman races.
- A complete pixel-art editor.
- Hundreds of handcrafted animation frames.
- Automatic generation of production-quality art for every possible human appearance.

The implementation should establish foundations for future work without attempting to deliver the entire game.

## 2.3 Non-negotiable constraints

- Preserve working v0.1 functionality.
- Preserve the existing C++20 simulation/generation architecture.
- Do not move business logic into GDScript merely to simplify rendering.
- Do not replace the existing project structure without evidence that it is necessary.
- Do not rewrite the generator before understanding its existing API and tests.
- Do not make every NPC a unique manually authored sprite.
- Do not create an architecture that requires hundreds of disconnected textures to support basic body variation.
- Do not claim a feature works merely because a UI control exists.

---

# 3. User Stories

### US-001 — View a detailed NPC

As a developer, I want to see a recognizable pixel-art human character so that I can assess the visual quality of the generator.

**Acceptance:** The default NPC is anatomically readable at the preview scale and no longer resembles a stack of primitive rectangles.

### US-002 — Change body shape

As a developer, I want to generate different body builds so that NPCs have visually distinct silhouettes.

**Acceptance:** Slim, average, muscular, and heavy characters have distinguishable torso, limb, and overall body proportions.

### US-003 — Change character height

As a developer, I want to adjust character height so that the NPC becomes visibly taller or shorter without breaking anatomical proportions.

### US-004 — Change appearance using a seed

As a developer, I want a seed to determine appearance consistently so that I can reproduce specific NPCs during debugging.

### US-005 — Inspect a character from different directions

As a developer, I want to view front, back, and side-facing representations so that I can evaluate directional consistency.

### US-006 — Preview animations

As a developer, I want to switch between idle, walk, run, attack, fall, and swim so that I can inspect movement and pose readability.

### US-007 — Inspect physical conditions

As a developer, I want to preview amputations, wounds, bandages, and scars so that physical conditions have a visible representation.

### US-008 — Reuse the character system

As a developer, I want the renderer to be reusable by the future Living World simulation rather than being tightly coupled to the preview screen.

---

# 4. Functional Requirements

Priority definitions:

- **P0:** Mandatory for the v0.2 release.
- **P1:** Important; implement if the existing architecture supports it within the release scope.
- **P2:** Extension point or optional enhancement.

## FR-001 — Repository and architecture audit (P0)

Before modifying code, the AI coding agent must inspect:

- Root `CMakeLists.txt`.
- Existing C++ headers and implementation files.
- NPC types and appearance structures.
- Generator and random-number generation logic.
- Pixel-buffer implementation.
- Serializer and C API.
- Existing tests.
- `godot/project.godot`.
- `godot/scripts/npc_bridge.gd`.
- `godot/scripts/preview_ui.gd`.
- Relevant Godot scenes and project resources.
- Existing build scripts and native-library integration.

The agent must document:

1. How the NPC is currently generated.
2. Which fields already represent appearance, height, body mass, direction, animation, and physical conditions.
3. How C++ output reaches Godot.
4. Whether the current renderer draws rectangles, pixels, textures, or a combination.
5. How the project is built and executed on the current development platform.
6. Which features are genuinely implemented versus placeholders.

**Acceptance criteria:**

- Existing code paths and APIs are identified.
- The implementation plan names the actual files to modify.
- No broad rewrite occurs before the audit is complete.
- Existing tests pass before changes, or existing failures are recorded.

## FR-002 — Pixel-art rendering architecture (P0)

The renderer must support a modular character assembled from visual components.

Recommended conceptual layers:

1. Rear hair and rear accessories.
2. Rear arm.
3. Torso and clothing.
4. Neck.
5. Head and facial features.
6. Front arm.
7. Legs.
8. Feet and footwear.
9. Optional equipment and condition overlays.

The precise draw order must depend on the selected direction and pose.

Not every component must be stored as a separate image file. Components may be generated procedurally and drawn into a shared pixel buffer.

The implementation must support:

- Per-pixel drawing.
- Filled regions.
- Outlines.
- Pixel clusters for shading.
- Layer composition.
- Alpha transparency where needed.
- A consistent coordinate system.
- Reproducible rendering from character parameters.

**Architecture recommendation:** Reuse the existing C++ pixel-buffer and appearance modules wherever appropriate. Introduce a dedicated renderer or anatomy builder only if it fits the existing architecture.

Do not assume that a Godot `Sprite2D`, `ImageTexture`, `TextureRect`, or other specific rendering class is already in use. Determine the current integration method first.

## FR-003 — Pixel-art resolution and scaling (P0)

The character must be designed at a deliberately low internal resolution and scaled up cleanly.

Initial reference dimensions:

- Target character canvas: approximately 32 × 48 pixels.
- Alternative canvas for more detailed characters: approximately 48 × 64 pixels.
- Background and preview canvas dimensions are independent of the character's internal resolution.

These values are starting points, not hard requirements. The agent must choose a resolution based on the existing implementation and the target visual.

Requirements:

- Pixel-aligned drawing.
- Nearest-neighbor scaling for the character texture.
- No unintended bilinear blur.
- No fractional scaling that causes inconsistent pixel widths.
- Consistent pixel density across all body components.
- Stable alignment between frames.

If Godot displays the generated image at a larger size, it must preserve sharp pixel edges.

The preview should remain visually crisp when resized.

## FR-004 — Anatomical structure (P0)

The human character must be represented using distinct anatomical regions:

- Head.
- Neck.
- Torso.
- Shoulder region.
- Upper and lower arms, where appropriate to the selected resolution.
- Hands.
- Upper and lower legs, where appropriate.
- Feet.

Each region must have defined geometry, proportions, palette, and attachment positions.

The renderer should make the silhouette readable without relying on facial details or clothing.

Requirements:

- The head must connect naturally to the neck and torso.
- Arms must attach at plausible shoulder positions.
- Hands must be visible when the selected resolution permits.
- Legs must connect naturally to the pelvis or lower torso.
- Feet must be aligned with the legs.
- Body parts must not create unintended gaps.
- Components must not overlap in visually confusing ways.
- The character must remain recognizable when rendered without clothing.

Anatomy may be represented as parameterized polygons, pixel masks, generated clusters, or other suitable procedural structures.

Do not require a full 3D skeleton.

## FR-005 — Silhouette quality (P0)

Silhouette quality is the primary visual acceptance criterion.

The body must be readable when rendered as a single dark shape against a contrasting background.

The system must support distinguishable silhouettes for:

- Slim.
- Average.
- Muscular.
- Heavy.
- Tall.
- Short.
- Hunched.

A character may combine multiple characteristics when the data model allows it.

For example:

- Tall + slim.
- Short + heavy.
- Average height + muscular.
- Tall + hunched.

The system must avoid simply stretching one rectangular body to represent all builds.

Body width, torso shape, shoulder width, limb thickness, head placement, and leg proportions should be controlled independently where appropriate.

## FR-006 — Body-shape profiles (P0)

The system must define a body-shape parameter model.

Recommended conceptual parameters:

| Parameter           | Purpose                            |
| ------------------- | ---------------------------------- |
| `height_scale`      | Overall vertical proportion        |
| `head_scale`        | Head size relative to the body     |
| `shoulder_width`    | Upper-body width                   |
| `torso_width`       | Main torso width                   |
| `torso_length`      | Torso height                       |
| `waist_width`       | Waist contour                      |
| `hip_width`         | Pelvis and hip width               |
| `arm_thickness`     | Arm silhouette thickness           |
| `leg_thickness`     | Leg silhouette thickness           |
| `leg_length_ratio`  | Leg length relative to body height |
| `posture`           | Upright or hunched body posture    |
| `muscle_definition` | Visible muscular contours          |
| `body_fat`          | Softness and body volume           |

These are conceptual requirements, not a demand to create every field with exactly these names.

The agent must map them to existing types or propose the smallest necessary model extension.

### Slim profile

- Narrow torso.
- Narrower limbs.
- Reduced waist and hip width.
- Readable but not skeletal anatomy.

### Average profile

- Balanced shoulder, torso, hip, and limb proportions.
- Serves as the default baseline.

### Muscular profile

- Wider shoulders.
- More defined upper arms.
- More pronounced chest and upper torso.
- Stronger thigh and calf silhouettes where the resolution permits.

### Heavy profile

- Wider torso.
- Fuller abdomen and waist.
- Wider hips.
- Thicker limbs.
- Weight distribution that remains visually coherent.

### Tall profile

- Increased overall height.
- Appropriate adjustments to limb and torso proportions.
- No disproportionate enlargement of the head.

### Short profile

- Reduced overall height.
- Consistent anatomical relationships.
- Avoid merely shrinking the complete character uniformly when body proportions should also change.

### Hunched profile

- Forward-leaning head and torso.
- Adjusted shoulder position.
- Coherent arm attachment.
- A visibly different silhouette from the upright profile.

## FR-007 — Height and body-mass integration (P0)

Existing UI controls must continue to work.

The renderer must translate their values into visible changes in anatomy.

Requirements:

- Height changes must alter the visible body height.
- Body mass must affect body volume, not only the width of the entire sprite.
- The controls must have defined minimum and maximum values.
- Values must be clamped to valid ranges.
- Extreme values must not create disconnected body parts or broken proportions.
- The same input values must produce the same base anatomy when the other parameters remain unchanged.

If body mass is currently represented as a scalar, preserve the public interface unless an extension is necessary.

The agent must define how body mass maps to the available body-shape profiles.

## FR-008 — Head and face construction (P0)

The head must be a distinct anatomical component rather than a plain rectangle.

Required visual features:

- Head contour.
- Skin base color.
- Shadow regions.
- Highlight regions.
- Hair or a bald state.
- Eyes.
- Nose or a readable nose representation.
- Mouth.
- Ear representation where resolution and direction permit.

The face must be recognizable at the chosen sprite resolution.

### Facial detail requirements

**Eyes**

- Must be aligned consistently.
- Must vary appropriately by direction.
- Must not appear through the back of the head.
- Must remain readable at the intended scale.

**Nose**

- May use one or more pixels, a shadow cluster, or a contour variation.
- Must be direction-aware.
- Must not require a highly detailed nose at a resolution where it would reduce readability.

**Mouth**

- Must be positioned consistently.
- May support neutral, smiling, angry, or hurt expressions as the architecture permits.
- Must not be drawn in an anatomically implausible position.

**Hair**

- Must support multiple silhouettes, such as short hair, messy hair, long hair, and bald.
- Hair must have a base color and, where appropriate, shadow and highlight colors.
- Hair must follow the head contour.
- Hair must be rendered behind or in front of facial features according to the selected style.

## FR-009 — Skin palette and shading (P0)

The renderer must use a coherent palette rather than assigning unrelated flat colors to anatomical regions.

Minimum recommended skin palette:

1. Base skin tone.
2. Shadow tone.
3. Deep shadow or outline tone.
4. Highlight tone.

Requirements:

- Shadows should communicate form.
- Highlights should be used selectively.
- Pixel clusters should follow anatomical structure.
- Shading must remain readable at the intended display scale.
- The outline color should complement the palette rather than always using pure black.
- The palette must remain consistent across the head, torso, arms, and legs.

Support multiple skin-tone presets.

Variation should be controlled by the appearance parameters or deterministic seed, not by uncontrolled per-frame randomness.

The agent must ensure that skin-tone variation does not change the anatomy or break the visual palette.

## FR-010 — Clothing system (P0)

The default human NPC must have a recognizable clothing configuration.

At minimum, the default character should include:

- An upper-body garment.
- Lower-body clothing.
- Appropriate footwear or a clearly intentional barefoot state.

Clothing must be represented as a visual layer over the body, not as a replacement for all anatomical structure.

### Upper-body garment

Must support:

- Clear torso boundaries.
- Shoulder coverage.
- A visible garment color.
- Appropriate layering relative to the arms and neck.
- Optional collar, sleeve, seam, or fold details.

### Lower-body clothing

Must support:

- A visible separation between the torso garment and legs.
- A coherent waistline.
- Shorts or trousers as the initial options, if feasible.

### Footwear

Must align with the feet and remain consistent across poses.

### Clothing colors

At minimum, the appearance system should support a primary garment color and a secondary accent or shadow color.

The initial implementation may use a limited set of predefined palettes.

Randomized colors must remain visually harmonious.

Clothing variation must not be implemented by selecting completely unrelated full-body images for each character.

## FR-011 — Appearance variation (P0)

The generator must be able to produce visibly different NPCs from a shared anatomical system.

Variation dimensions should include:

- Body shape.
- Height.
- Body mass.
- Skin tone.
- Hair style.
- Hair color.
- Clothing palette.
- Facial characteristics.
- Posture, where supported.

Variation must remain bounded by the chosen visual style.

A seed should determine the procedural choices unless a parameter is explicitly overridden by the user.

Changing the seed may alter generated appearance.

Keeping the seed and all explicit parameters constant must reproduce the same appearance.

Changing the height slider should not unexpectedly randomize the hair color or clothing.

Changing the body-mass slider should not regenerate the entire character's identity.

**Implementation principle:** Separate deterministic identity/appearance generation from the rendering process and from individual UI interactions.

## FR-012 — Seed determinism (P0)

The existing seed functionality must be preserved.

The system must distinguish:

- The master character seed.
- Explicit user-selected parameters.
- Derived appearance choices.
- Animation state.
- Current animation frame.

Requirements:

- The same seed and explicit parameters produce the same generated character.
- Animation playback must not change permanent appearance.
- Rendering the same frame repeatedly must not change the image.
- UI interactions must not consume random values in a way that unpredictably changes unrelated appearance properties.
- Separate random streams or stable derived seeds may be used where useful.

If the existing C++ random-number generator already satisfies these requirements, reuse it.

## FR-013 — Directional rendering (P0)

The existing direction selector must remain functional.

Required directions:

- South/front.
- North/back.
- East/side.
- West/side.

The renderer must provide direction-aware composition.

### Front

- Facial features visible.
- Body proportions balanced around the character's centerline.
- Arms and legs readable independently.

### Back

- Facial features hidden.
- Rear hair and clothing details visible as appropriate.
- Shoulders and body contours remain coherent.

### East and west

- Side-facing silhouette.
- Reduced facial detail.
- Nose and face orientation appropriate to the direction.
- Limbs layered to communicate depth.

East and west may reuse mirrored geometry only when doing so produces a coherent result.

The system must not mirror every visual element indiscriminately if the result would reverse asymmetric features, accessories, or injuries.

All directions must use the same character identity and underlying body parameters.

## FR-014 — Animation system (P0)

The current animation selector must continue to work.

The renderer must support the following animation categories:

| Animation | Required visual behavior                              |
| --------- | ----------------------------------------------------- |
| Idle      | Stable standing pose with optional subtle breathing   |
| Walk      | Alternating leg movement and coordinated arm swing    |
| Run       | Larger stride and more dynamic arm movement           |
| Attack    | A readable attack pose with coordinated body movement |
| Fall      | A transition into a fallen or collapsing pose         |
| Swim      | A pose and limb movement appropriate to swimming      |

The animation system must not change the character's permanent appearance.

The first v0.2 implementation should prioritize coherent movement over an excessive number of frames.

Recommended starting point:

- Idle: 1–2 frames or a small controlled breathing cycle.
- Walk: 4–6 frames.
- Run: 4–6 frames.
- Attack: 3–6 frames.
- Fall: a small sequence of distinct poses.
- Swim: 4–6 frames.

These frame counts are recommendations and may be adjusted to the selected resolution.

### Animation quality requirements

- Feet must not jump unpredictably between frames.
- The character's anchor point must remain stable.
- The head must remain connected to the body.
- Limbs must remain attached unless an explicit physical condition removes them.
- Frame transitions must not create unintended flashes or color changes.
- The selected animation must actually affect the rendered pose.
- Animation playback should be deterministic for a given frame index.

If the existing implementation already supports animation states, preserve their names and integration contracts wherever practical.

## FR-015 — Animation timing and preview controls (P1)

If feasible within the current preview architecture, add:

- Play/pause.
- Animation speed.
- Frame stepping.
- Current frame indicator.
- Restart animation.
- A stable preview anchor.

These controls are intended to make visual debugging easier.

They must not be allowed to mutate permanent NPC appearance.

If the current preview is too tightly coupled to the animation implementation, document the limitation and defer nonessential controls.

## FR-016 — Amputation rendering (P0)

Existing amputation controls must remain functional.

Required initial states:

- No amputation.
- Left-arm amputation.
- Left-leg amputation.

When a limb is amputated:

- The affected limb must not be drawn as an intact limb.
- The remaining anatomy must remain coherent.
- The limb must not reappear in an animation frame.
- The character must not have accidental gaps caused by incorrect layer ordering.
- The condition must be visible from relevant viewing directions.
- The renderer must distinguish a missing limb from a limb merely hidden behind the torso.

If the existing data model supports additional missing limbs, reuse those states where feasible without expanding the release beyond its scope.

A stump, bandage, or injury mark may be shown when the appearance and condition models support it.

Do not add graphic gore as a requirement for this feature.

## FR-017 — Injury, wounds, bandages, and scars (P1)

If the existing physical-condition model supports these states, integrate them into the renderer.

Initial visual condition types:

- Minor wound.
- Visible scar.
- Bandage on an arm.
- Bandage on a leg.
- Bandage on the head.
- General injury mark.

Requirements:

- Conditions must be layered correctly over the affected anatomy.
- The renderer must not place a wound on a nonexistent or amputated limb.
- Scars must remain attached to the correct body region across directions and poses.
- Bandages must not float independently of the body.
- Condition rendering must not alter the permanent anatomy unless the condition explicitly represents a structural change.

The first release should prioritize clear, restrained visual indicators.

If the existing data model cannot represent these conditions reliably, document the required extension instead of inventing disconnected UI-only state.

## FR-018 — Character preview composition (P0)

The preview must be redesigned to showcase the generated character.

Requirements:

- A clearly visible character preview area.
- A contrasting background.
- Appropriate character scale.
- Consistent vertical alignment.
- Sufficient separation between the controls and the character.
- No accidental cropping of the head, feet, or limbs.
- No stretching caused by an incorrect texture aspect ratio.
- A clear indication of the currently selected animation and direction.

The preview should make it easy to judge body proportions and pixel detail.

A neutral dark background is acceptable if it provides sufficient contrast.

An optional checkerboard or plain neutral background may be provided for transparency inspection.

## FR-019 — UI layout and usability (P0)

Preserve existing controls.

The interface should organize controls into logical groups:

1. Identity and seed.
2. Body shape.
3. Appearance.
4. Animation.
5. Direction.
6. Physical conditions.

The UI must remain usable at the current project window size and should not depend on a single hardcoded screen resolution.

Recommended improvements:

- Group related sliders.
- Use meaningful labels.
- Display current numeric values where useful.
- Separate character preview from configuration controls.
- Keep the preview sufficiently large to inspect pixel details.
- Ensure that all relevant controls remain accessible without clipping.

The agent must not remove a working control simply to make the UI look cleaner.

## FR-020 — Rendering and preview performance (P0)

The renderer must be efficient enough for interactive use.

Requirements:

- Re-render only when relevant inputs change or when the animation frame changes.
- Avoid regenerating permanent appearance on every rendered frame.
- Cache generated textures or intermediate pixel buffers when appropriate.
- Avoid repeated expensive allocations in the animation loop.
- Keep texture dimensions bounded.
- Ensure that changing one parameter does not trigger unnecessary regeneration of unrelated state.

The agent should record baseline and post-change preview performance where practical.

A single-character preview should remain responsive during ordinary interaction.

The architecture should permit later rendering of many NPCs without requiring a unique heavyweight scene or texture-generation operation for every frame.

## FR-021 — Error handling (P0)

The application must handle invalid input safely.

Examples:

- Negative height.
- Body mass outside the supported range.
- Invalid seed values.
- Unsupported animation identifiers.
- Invalid direction identifiers.
- Invalid physical-condition combinations.

The implementation must use explicit defaults or clamp values as appropriate.

An invalid parameter must not cause an application crash or corrupt the pixel buffer.

Errors should be visible in the development output or returned through the existing integration interface.

---

# 5. Visual Design Specification

## 5.1 Reference interpretation

The first reference image demonstrates the following visual qualities:

- Low-resolution pixel art.
- Strong and readable body silhouettes.
- Warm skin palette.
- Distinct shadow and highlight clusters.
- Visible muscle and body-mass variation.
- Recognizable head and facial structure.
- A consistent outline style.
- Characters displayed at a comparable scale.
- A limited, coherent color palette.

These qualities are the intended direction.

The implementation is not required to reproduce the exact reference characters, typography, layout, watermark, or composition.

The reference is for visual language and character quality.

## 5.2 Visual style rules

### Pixel cluster discipline

Pixels must be arranged in intentional clusters.

Avoid:

- Randomly scattered isolated pixels.
- Noise used as a substitute for shading.
- Excessively detailed textures at tiny resolutions.
- Inconsistent pixel sizes.
- Blurred edges.
- Uncontrolled color transitions.

### Outline discipline

Use a consistent outline strategy.

- External contours should define the silhouette.
- Internal contours should communicate anatomy.
- Clothing boundaries should be distinguishable from skin.
- Deep shadows may share the outline palette.
- The outline should not overpower small facial details.

### Lighting discipline

Use a consistent assumed light direction across all body parts.

For example, lighting may come from the upper-left.

The choice is flexible, but the direction must remain consistent between the head, torso, arms, and legs.

### Readability at multiple scales

The character must be readable both at native preview scale and when scaled up.

The design should prioritize:

1. Silhouette.
2. Body proportions.
3. Clothing boundaries.
4. Facial readability.
5. Shading and decorative detail.

Do not add micro-details before the silhouette and proportions are correct.

## 5.3 Recommended initial palette

The following palette is illustrative, not mandatory.

| Role               | Example   |
| ------------------ | --------- |
| Deep outline       | `#49352F` |
| Skin shadow        | `#A66C55` |
| Skin base          | `#D9A078` |
| Skin highlight     | `#F0C49A` |
| Hair shadow        | `#49352F` |
| Hair base          | `#77513B` |
| Hair highlight     | `#A87952` |
| Clothing shadow    | `#30486A` |
| Clothing base      | `#496A9B` |
| Clothing highlight | `#7190BC` |

The agent may choose better colors to match the reference.

Palette values should be centralized in the appearance or rendering system rather than scattered across unrelated functions.

---

# 6. Technical Architecture Requirements

## 6.1 Separation of responsibilities

The architecture should distinguish four responsibilities.

### A. NPC data and generation — C++20

Responsible for:

- Seed handling.
- Body parameters.
- Appearance selection.
- Physical-condition state.
- Deterministic generation.
- Validation of generated parameters.

This layer should remain independent of Godot rendering APIs wherever practical.

### B. Anatomy and pose representation

Responsible for:

- Converting body parameters into anatomical dimensions.
- Resolving body-part attachment points.
- Producing a pose for the selected direction and animation frame.
- Applying structural conditions such as missing limbs.

This may live in the existing core or in a dedicated rendering-oriented C++ module, depending on the repository.

### C. Pixel-art renderer

Responsible for:

- Drawing body components.
- Applying palettes.
- Producing outlines and shading.
- Compositing layers.
- Producing the final pixel buffer or texture data.

It must not own unrelated world-simulation logic.

### D. Godot preview and UI

Responsible for:

- User controls.
- Passing parameters to the existing bridge.
- Displaying the generated result.
- Controlling preview playback.
- Showing errors and debugging information.

GDScript may coordinate Godot UI and rendering. It must not become the authoritative NPC generator.

## 6.2 Reuse existing implementation

The current repository reportedly includes modules related to appearance, conditions, generation, pixel buffers, random-number generation, serialization, and a C API.

The AI agent must inspect those modules before creating alternatives.

The agent should reuse the existing pixel buffer if it already supports the necessary operations.

The agent should reuse the existing C API if it provides a stable bridge.

The agent must not introduce a second generator that disagrees with the existing generator.

## 6.3 Godot integration

The current Godot preview is already running, so v0.2 must improve it without unnecessarily replacing the entire integration.

The agent must determine whether the current bridge transfers:

- Raw pixel-buffer data.
- Serialized appearance parameters.
- A generated image.
- A native-library object or handle.
- Another representation.

The renderer should integrate with the current bridge or introduce a minimal, documented extension.

The final application must work in the actual target environment.

If the Godot editor runs on Windows while the C++ core is compiled in WSL for Linux, the native-library platform mismatch must be handled explicitly.

A Linux shared library cannot simply be loaded by a Windows Godot process. Build and validate the appropriate target binary or use a verified cross-platform integration approach.

Do not treat successful C++ compilation in WSL as proof that Windows Godot can load the result.

## 6.4 Proposed conceptual data flow

`Character Seed + User Parameters`

↓

`Deterministic NPC Appearance Generation`

↓

`Body Profile + Facial Features + Clothing + Conditions`

↓

`Pose Construction for Direction and Animation Frame`

↓

`Anatomical Layer Composition`

↓

`Pixel Buffer / Texture Output`

↓

`Godot Preview`

This diagram represents the intended conceptual flow. The agent must adapt it to the repository's actual interfaces.

## 6.5 Suggested conceptual models

The following are design concepts, not instructions to create identically named structs.

### Character appearance

- Skin palette identifier.
- Hair style identifier.
- Hair color identifier.
- Clothing configuration.
- Body-shape profile.
- Optional facial-feature parameters.

### Body profile

- Height.
- Body mass or body-fat parameter.
- Shoulder width.
- Torso width.
- Waist width.
- Hip width.
- Arm thickness.
- Leg thickness.
- Posture.

### Pose

- Facing direction.
- Animation identifier.
- Animation frame.
- Body-part offsets or pose parameters.
- Optional posture adjustments.

### Physical conditions

- Missing-limb states.
- Injury locations.
- Scar locations.
- Bandage locations.

### Render configuration

- Internal resolution.
- Palette.
- Outline settings.
- Pixel scale.
- Layer-order configuration.

The agent should minimize unnecessary duplication of existing fields.

---

# 7. Detailed Implementation Phases

## Phase 0 — Baseline audit and safety

**Priority: P0**

Tasks:

1. Inspect the complete relevant source tree.
2. Identify existing public APIs and data models.
3. Run existing tests.
4. Run the existing Godot preview.
5. Capture a baseline screenshot.
6. Record current functionality and known defects.
7. Determine how to build the C++ core for the target runtime.
8. Identify the current rendering path.
9. Produce a file-by-file implementation plan.

Deliverables:

- `docs/NPC_BUILDER_V02_AUDIT.md`
- `docs/NPC_BUILDER_V02_IMPLEMENTATION_PLAN.md`
- Baseline test results.

Exit criteria:

- The agent can explain how the current NPC appears in Godot.
- The agent knows which modules can be reused.
- The agent has not broken the working application.

## Phase 1 — Rendering foundation

**Priority: P0**

Tasks:

1. Establish the internal character resolution.
2. Define pixel-coordinate conventions.
3. Define the character anchor point.
4. Define a consistent palette structure.
5. Implement or extend the anatomical layer system.
6. Implement silhouette rendering.
7. Integrate the new renderer with the existing bridge.
8. Display the result in the existing Godot preview.

Deliverable:

A recognizable human silhouette with separate head, torso, arms, legs, and feet.

Exit criteria:

- The default NPC is no longer rendered as a stack of flat rectangles.
- The texture has crisp pixel edges.
- The renderer produces a stable image.
- Existing controls still load.

## Phase 2 — Body shapes and proportions

**Priority: P0**

Tasks:

1. Implement slim, average, muscular, and heavy profiles.
2. Implement tall and short variations.
3. Implement a hunched posture.
4. Map existing height and body-mass controls to anatomy.
5. Validate anatomical attachments.
6. Add silhouette comparison tests.

Deliverable:

A set of visibly distinct body types generated by a shared system.

Exit criteria:

- The profiles are distinguishable at preview scale.
- Height and mass controls have meaningful visual effects.
- Extreme supported values do not break the body.
- Repeated generation remains deterministic.

## Phase 3 — Face, skin, hair, and clothing

**Priority: P0**

Tasks:

1. Implement the head contour.
2. Add eyes, nose, and mouth.
3. Add hair styles and hair colors.
4. Add skin palettes.
5. Add consistent shading and highlights.
6. Implement a basic upper-body garment.
7. Implement lower-body clothing.
8. Implement feet or footwear.
9. Add deterministic appearance variation.

Deliverable:

A clothed pixel-art human with a readable face and a coherent color palette.

Exit criteria:

- The default NPC resembles a deliberately designed pixel-art character.
- The face is readable at the selected scale.
- Clothing is distinct from skin.
- Palette consistency is maintained.
- Changing the seed creates controlled variation.

## Phase 4 — Directions and animations

**Priority: P0**

Tasks:

1. Implement front-facing composition.
2. Implement back-facing composition.
3. Implement east-facing composition.
4. Implement west-facing composition.
5. Implement idle poses.
6. Implement walking.
7. Implement running.
8. Implement attacking.
9. Implement falling.
10. Implement swimming.
11. Validate attachment points and animation alignment.

Deliverable:

A character that can be inspected in all required directions and animation categories.

Exit criteria:

- Every required animation produces a visibly appropriate pose.
- Direction selection works.
- The same appearance is preserved across directions and frames.
- No body parts randomly disappear or detach.

## Phase 5 — Physical conditions

**Priority: P0/P1**

Tasks:

1. Integrate left-arm amputation.
2. Integrate left-leg amputation.
3. Validate all supported animation combinations.
4. Validate all supported directions.
5. Add bandages and injury overlays where the current data model permits.
6. Add scars if feasible without undermining core requirements.

Deliverable:

Visible, consistent physical-condition rendering.

Exit criteria:

- Existing amputation controls work.
- Missing limbs remain absent in every relevant animation frame.
- Conditions do not float or appear on the wrong body part.
- Existing condition data remains consistent with the rendered state.

## Phase 6 — Preview UI and visual polish

**Priority: P0**

Tasks:

1. Improve preview framing.
2. Improve control grouping.
3. Add clear parameter labels.
4. Validate pixel scaling.
5. Ensure the character is not cropped.
6. Improve background contrast.
7. Add optional animation playback controls if feasible.
8. Capture comparison screenshots.

Deliverable:

A usable character studio rather than a bare technical test screen.

Exit criteria:

- The character is easy to inspect.
- All existing controls remain available.
- The interface works at the current project resolution.
- The result matches the intended visual direction.

## Phase 7 — Regression, performance, and documentation

**Priority: P0**

Tasks:

1. Run all existing C++ tests.
2. Add tests for new rendering behavior.
3. Validate seed determinism.
4. Validate parameter boundaries.
5. Validate the Godot bridge.
6. Validate the target operating-system build.
7. Compare visual output with the reference.
8. Document architecture and limitations.
9. Record remaining v0.3 opportunities.

Deliverable:

A reproducible v0.2 build with documented behavior and known limitations.

---

# 8. Acceptance Criteria and Definition of Done

The release is complete only when the following conditions are satisfied.

## 8.1 Visual acceptance

- [ ] Character silhouette is recognizable as a human.
- [ ] Head, torso, arms, legs, and feet are visually distinguishable.
- [ ] Slim, average, muscular, and heavy profiles have distinct silhouettes.
- [ ] Tall, short, and hunched variants are visually distinguishable.
- [ ] The head includes readable facial features.
- [ ] Hair is distinguishable from skin.
- [ ] Clothing is distinguishable from skin and anatomy.
- [ ] Skin and clothing use coherent shading palettes.
- [ ] The character has crisp pixel edges when scaled.
- [ ] The character is not unintentionally cropped.
- [ ] The preview is visually clearer and more detailed than the v0.1 baseline.

## 8.2 Functional acceptance

- [ ] Seed input remains functional.
- [ ] Height input remains functional.
- [ ] Body-mass input remains functional.
- [ ] Animation selector remains functional.
- [ ] Direction selector remains functional.
- [ ] Left-arm amputation remains functional.
- [ ] Left-leg amputation remains functional.
- [ ] Appearance changes are deterministic.
- [ ] Permanent appearance does not change during animation.
- [ ] The UI and C++ generator remain correctly integrated.
- [ ] Invalid inputs are handled safely.

## 8.3 Animation acceptance

- [ ] Idle produces a stable pose.
- [ ] Walk produces a readable walking cycle.
- [ ] Run produces a distinguishable running cycle.
- [ ] Attack produces a readable attack pose.
- [ ] Fall produces a readable falling sequence.
- [ ] Swim produces a distinguishable swimming pose.
- [ ] Character alignment remains stable between frames.
- [ ] Missing limbs do not reappear during animation.

## 8.4 Technical acceptance

- [ ] Existing tests pass, or pre-existing failures are explicitly documented.
- [ ] New tests cover deterministic appearance and critical parameter boundaries.
- [ ] The project builds for the actual target environment.
- [ ] Godot loads the integration successfully.
- [ ] No unnecessary duplicate NPC generator has been introduced.
- [ ] Documentation identifies the new rendering architecture.
- [ ] No unrelated simulation behavior has been silently removed.
- [ ] The final build has been launched and manually inspected.

A screenshot of the final result must be included in the implementation report.

---

# 9. Testing Strategy

## 9.1 Unit tests

Add tests for:

- Body-profile parameter ranges.
- Height and mass mapping.
- Appearance generation determinism.
- Palette selection.
- Invalid input handling.
- Pose parameter validity.
- Missing-limb state propagation.
- Pixel-buffer dimensions.
- Pixel-buffer bounds safety.
- Layer-order consistency where testable.

## 9.2 Determinism tests

Test cases:

**Case A:** Same seed and same parameters.

Expected: identical generated appearance and identical output for the same pose/frame.

**Case B:** Different seed, otherwise equivalent inputs.

Expected: at least one supported randomized appearance property changes for selected test seeds; the system must not guarantee that every pair of seeds differs in every property.

**Case C:** Same seed, different height.

Expected: height-related anatomy changes while unrelated identity attributes remain stable.

**Case D:** Same seed, different animation frame.

Expected: pose changes, but permanent appearance remains stable.

**Case E:** Same seed and parameters after restarting the application.

Expected: the same character appearance.

## 9.3 Visual regression tests

Where practical, save representative render outputs for comparison.

Suggested fixture set:

- Default average character.
- Slim character.
- Muscular character.
- Heavy character.
- Tall character.
- Short character.
- Hunched character.
- Front view.
- Back view.
- Side view.
- Walk frame.
- Run frame.
- Attack frame.
- Fall pose.
- Swim frame.
- Left-arm amputation.
- Left-leg amputation.

Exact pixel-by-pixel image matching may be too brittle if rendering changes are intentional. Use a combination of image comparisons, structural tests, and manual inspection.

## 9.4 Manual visual inspection

Inspect each fixture at:

- Native internal resolution.
- Normal preview scale.
- Enlarged pixel-art scale.

Check for:

- Broken silhouettes.
- Disconnected body parts.
- Inconsistent outline thickness.
- Unintended color noise.
- Misaligned eyes.
- Hair overlapping facial features incorrectly.
- Feet shifting between animation frames.
- Missing limbs being drawn accidentally.
- Clothing or injury overlays floating outside the body.

---

# 10. Risks and Mitigations

## Risk 1 — Procedural art looks artificial

**Cause:** The generator uses simple geometric primitives without intentional pixel clusters.

**Mitigation:** Prioritize silhouettes, anatomical contours, palette consistency, and controlled shading before adding detail.

## Risk 2 — Every body type requires separate hand-authored artwork

**Cause:** The system relies on unrelated complete sprites.

**Mitigation:** Use parameterized anatomy and reusable visual layers. Permit small hand-authored masks or templates where they improve quality, but keep body variation procedural.

## Risk 3 — Pixel-art detail becomes unreadable

**Cause:** The internal resolution is too small for the amount of detail.

**Mitigation:** Choose resolution based on silhouette readability and test at the final display scale. Prefer fewer intentional details over noise.

## Risk 4 — Animations break anatomy

**Cause:** Each animation is implemented independently without shared attachment points.

**Mitigation:** Use a shared pose model or consistent body-part anchors across animations.

## Risk 5 — Existing features regress

**Cause:** The rendering rewrite replaces the current generator or changes bridge contracts.

**Mitigation:** Audit first, preserve public interfaces, run regression tests, and implement changes in small phases.

## Risk 6 — Platform-specific native-library failure

**Cause:** The core is built in WSL for Linux while the Godot editor runs on Windows.

**Mitigation:** Validate the native integration for the actual target OS and architecture.

## Risk 7 — Scope becomes too large

**Cause:** Attempting complete anatomy, many outfits, complex facial expressions, and every animation simultaneously.

**Mitigation:** Complete the phases in order. Treat high-detail expressions and expanded clothing collections as extensions after the core visual system works.

---

# 11. Recommended File and Module Strategy

This is a suggested organization, not a mandate to create every file.

The AI coding agent must first inspect the repository and adapt these concepts to the actual codebase.

Potential responsibilities:

- `appearance`: stable character appearance and body-profile parameters.
- `conditions`: injuries, amputations, scars, and bandage state.
- `generator`: deterministic generation of character properties.
- `pixel_buffer`: pixel-level image construction.
- `anatomy`: anatomical proportions and component geometry, if not already represented.
- `pose`: directional and animation-frame transformations, if not already represented.
- `renderer`: pixel-art composition, shading, and final buffer generation, if a separate module is warranted.
- `c_api`: existing or extended native integration.
- `npc_bridge.gd`: Godot-side bridge and texture updates.
- `preview_ui.gd`: preview controls and presentation.
- Godot scenes: character studio layout and visual components.
- Tests: generator, anatomy, renderer, determinism, and integration coverage.

The implementation should favor a small number of clear modules over a large collection of tiny files with overlapping responsibilities.

Do not create a second renderer if the existing pixel-buffer implementation already provides an appropriate place to implement these requirements.

---

# 12. Instructions for the AI Coding Agent

The following instructions are mandatory for the implementation workflow.

## 12.1 Before writing code

1. Inspect the existing repository.
2. Read the current NPC Builder implementation.
3. Run the existing tests.
4. Identify the current rendering pipeline.
5. Identify the bridge between C++ and Godot.
6. Read this BRD alongside both attached reference images.
7. Produce a concise implementation plan with exact files to change.
8. Identify any ambiguity that would materially affect the architecture.

Do not start with a wholesale rewrite.

## 12.2 During implementation

- Keep C++20 as the authoritative source of NPC generation.
- Keep Godot as the visual preview and rendering host.
- Reuse existing modules where appropriate.
- Preserve current UI controls.
- Preserve deterministic seed behavior.
- Implement and test one vertical slice at a time.
- Build and run the actual project after meaningful changes.
- Fix errors rather than hiding them.
- Avoid introducing unnecessary dependencies.
- Keep code modular and testable.
- Do not substitute mock output for real procedural generation without explicitly documenting it.
- Do not claim a native bridge works without testing it in Godot.
- Do not use a full-body placeholder image as a substitute for the required procedural body system.

## 12.3 Definition of a vertical slice

The first implementation slice must deliver one complete, high-quality default NPC:

1. Generated through the existing C++ architecture.
2. Composed of a head, torso, arms, legs, and feet.
3. Rendered with a coherent pixel-art palette.
4. Equipped with a face, hair, and basic clothing.
5. Displayed in the current Godot preview.
6. Responsive to the existing height and body-mass controls.
7. Deterministic for a given seed.
8. Compatible with the current integration.

Only after this slice works should the agent expand the body-shape collection and animation coverage.

## 12.4 After each phase

Report:

- Files created.
- Files modified.
- Existing features preserved.
- New features implemented.
- Build and test commands executed.
- Test results.
- Screenshot or visual evidence.
- Known limitations.
- Next recommended phase.

The report must distinguish between implemented, partially implemented, and planned features.

## 12.5 Completion report

At the end of v0.2, provide:

- A summary of the architecture.
- A list of all implemented requirements.
- A list of incomplete requirements and reasons.
- Build instructions for the actual target environment.
- Test results.
- A final Godot screenshot.
- A comparison with the supplied visual reference.
- Recommended v0.3 improvements.

---

# 13. Proposed v0.3 Roadmap

The following features are future opportunities, not v0.2 release requirements.

- Additional hairstyles and facial variations.
- More clothing styles.
- Equipment and accessories.
- Expanded expressions.
- More detailed injuries and scars.
- Additional animation states.
- Better directional animation consistency.
- Character aging and age-related proportions.
- Gender presentation and body variation, where supported by the game design.
- A reusable NPC renderer for population-scale world simulation.
- Multiple race-specific anatomy profiles.
- Optimization for rendering large numbers of NPCs.
- A character comparison and batch-generation mode.
- Exporting generated character previews for art review.

---

# 14. Final Product Vision

NPC Builder should become a reusable character-generation tool for Living World rather than a one-off demo.

The system should generate humans with recognizable body proportions, distinct appearance, consistent pixel-art styling, and meaningful physical conditions.

The underlying simulation should own character identity and state. The renderer should translate that state into a readable visual representation.

The intended progression is:

**v0.1 — Functional foundation:** Primitive NPC preview and working controls.

**v0.2 — Visual foundation:** Modular procedural pixel-art anatomy, appearance, directional presentation, and coherent animation.

**v0.3 — Content expansion:** More hairstyles, clothing, conditions, poses, and richer visual variation.

**Future versions — Simulation integration:** NPCs rendered consistently throughout the Living World simulation, including movement, combat, aging, family relationships, and population growth.

The most important engineering principle is to build a renderer that produces coherent characters from reusable parts, not a collection of disconnected pictures that only work for predefined examples.

The most important visual principle is to prioritize silhouette, proportions, and pixel-cluster quality before adding complexity.

The most important delivery principle is to preserve the working v0.1 foundation and prove each new capability through an actual build, a running Godot preview, and tests.

**Target outcome:** A deterministic, modular, extensible procedural human pixel-art system that can grow with Living World without requiring every NPC to be drawn manually.
