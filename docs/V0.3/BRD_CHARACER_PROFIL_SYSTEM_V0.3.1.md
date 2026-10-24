# BRD — NPC Builder v0.3.1

## Character Profile System

**Project:** NPC Builder  
**Version:** v0.3.1  
**Feature:** Character Profile System  
**Document Type:** Business Requirements Document (BRD)  
**Status:** Draft — for implementation planning  
**Primary Focus:** Character configuration architecture, profile validation, serialization, and reconstruction

---

## 1. Executive Summary

NPC Builder v0.3.1 introduces the Character Profile System as the foundational configuration layer for procedural character generation.

The system will provide a structured, validated, serializable representation of a character's configuration. It will separate anatomical configuration, visual appearance, and physical condition while preserving the existing C++ core as the single source of truth.

The purpose of this release is not to implement every planned character variation. Instead, it establishes the architecture required to add future features without repeatedly modifying the rendering pipeline or introducing tightly coupled configuration logic.

Future capabilities expected to benefit from this foundation include:

- Female anatomy and body proportion variation.
- Height, body mass, and body shape variation.
- Modular face and hair generation.
- Fantasy races and species-specific anatomy.
- Giant, miniature, and extreme body proportions.
- Clothing and equipment compatibility.
- Injuries, amputations, aging, and other physical conditions.
- Character generation at scale for the Living World project.

Version 0.3.1 must remain compatible with the existing native C++ architecture, C API, GDExtension integration, and established rendering and testing behavior.

## 2. Background and Problem Statement

### 2.1 Existing project direction

NPC Builder v0.2.3 establishes the following architectural direction:

- C++ core is the authoritative implementation of character generation.
- Godot acts as the preview and integration layer through GDExtension.
- The C API provides an integration boundary for native functionality.
- Existing appearance, physical-condition, generation, and serialization code should be reused where appropriate.
- Existing tests and golden fixtures provide regression protection.

Known repository areas relevant to this work include:

- `core/include/npc/appearance.hpp`
- `core/include/npc/conditions.hpp`
- `core/include/npc/generator.hpp`
- `core/include/npc/serializer.hpp`
- `core/include/npc/types.hpp`
- `core/src/generator.cpp`
- `core/src/c_api.cpp`
- `tests/test_generator.cpp`
- `extension/`
- `godot/scripts/npc_bridge.gd`

These paths are the initial audit targets, not a declaration that every required capability already exists in the current form.

### 2.2 Current architectural challenge

As character-generation capabilities grow, a flat collection of generation parameters becomes increasingly difficult to maintain.

Without a clear profile architecture, future features may introduce:

- Hardcoded anatomical constants inside the renderer.
- Configuration fields that overlap or conflict across systems.
- Unclear ownership between appearance and physical conditions.
- Inconsistent defaults across the C++ API and Godot interface.
- Serialization formats that cannot safely evolve.
- Repeated changes to existing APIs whenever a new character feature is introduced.
- Difficulty reproducing a character from saved configuration.
- Unintended regressions in existing anatomy, injuries, or amputations.

### 2.3 Business problem

The project needs a stable configuration contract that makes character generation extensible, reproducible, testable, and suitable for future integration into a large-scale world simulation.

The release should reduce future implementation cost rather than simply increase the number of character parameters.

## 3. Business Objectives

### BO-01 — Establish a canonical character configuration

Introduce a clear Character Profile model that represents the configuration used to generate a character.

### BO-02 — Separate configuration domains

Separate anatomical parameters, visual appearance, and physical conditions into independently understandable domains.

### BO-03 — Remove configuration hardcoding from the renderer

Move configurable character properties into explicit data structures and validated configuration paths.

This does not prohibit constants used for rendering algorithms, pixel geometry, or internal implementation. It prohibits treating user-configurable character properties as hidden renderer-specific settings.

### BO-04 — Standardize defaults and validation

Provide predictable behavior for default profiles, invalid values, missing fields, and unsupported configuration data.

### BO-05 — Support reliable serialization

Allow profiles to be serialized and reconstructed without unintended configuration loss.

### BO-06 — Preserve existing architecture

Maintain C++ as the source of truth and avoid duplicating character-generation rules in GDScript.

### BO-07 — Enable future expansion

Provide a foundation that can accommodate additional anatomy, species, visual features, and physical conditions without redesigning the entire profile model.

## 4. Scope

### 4.1 In scope

1. Character Profile conceptual model and implementation.
2. Separation of anatomy, appearance, and physical-condition configuration.
3. Configurable profile parameters.
4. Default profile creation.
5. Validation and normalization rules.
6. Defined fallback behavior.
7. Serialization and deserialization of supported profile data.
8. Round-trip consistency tests.
9. Integration with the existing C++ generation pipeline.
10. Necessary C API and GDExtension integration.
11. Regression testing for existing generation behavior.
12. Documentation of profile fields and validation behavior.

### 4.2 Out of scope

The following are not deliverables for v0.3.1:

- Full implementation of female anatomy.
- Complete body morphology variation.
- Full modular face and hair generation.
- Full fantasy race or species system.
- Complete giant and miniature character support.
- Clothing, armor, and profession-based outfit generation.
- New animation systems or locomotion behaviors.
- Genetic inheritance and aging simulation.
- World simulation or NPC behavior AI.
- Large-scale performance optimization unrelated to profile integration.
- A complete character editor or visual profile-authoring tool.

These features may consume the profile system later, but v0.3.1 only needs to establish the necessary extensible contracts.

## 5. Stakeholders and System Responsibilities

### 5.1 Project owner

Defines the desired character-generation behavior and approves the profile model, compatibility rules, and release criteria.

### 5.2 C++ core

The authoritative layer responsible for:

- Profile data structures.
- Default profile construction.
- Validation and normalization.
- Serialization and deserialization.
- Character generation using validated configuration.
- Stable interpretation of profile parameters.

### 5.3 C API

Provides the integration boundary for operations required by external consumers.

The C API should expose profile functionality only where necessary. The release should not create a second implementation of profile validation or generation rules.

### 5.4 GDExtension

Provides the bridge between Godot and the C++ core.

It must translate requests and responses without independently reimplementing profile semantics.

### 5.5 Godot preview

Provides a way to exercise profile-related behavior and display generation results.

Godot may manage interactive controls and presentation state, but it must not become the authoritative owner of anatomy rules, defaults, or validation logic.

## 6. Functional Requirements

### FR-01 — Character Profile Model

**Priority:** Must Have

The system shall define a canonical Character Profile model representing the configuration used by the character-generation pipeline.

Requirements:

- The model must have an explicit, documented structure.
- The model must separate anatomical configuration, visual appearance configuration, and physical-condition configuration.
- The model must reuse compatible existing data types where appropriate.
- The model must avoid storing redundant values that can be derived deterministically from other fields.
- The model must support future extension without requiring every consumer to understand every possible future field.
- The model must have documented ownership and lifecycle semantics.
- The model must distinguish configuration from generated output.

The exact C++ representation must be determined after auditing the existing types. The conceptual model does not require a particular inheritance hierarchy or serialization library.

#### Conceptual composition

`CharacterProfile` represents the overall configuration and contains or references the following logical domains:

**Anatomy configuration**

- Body proportions.
- Anatomical variation parameters.
- References to applicable anatomy presets or definitions, if needed.

**Appearance configuration**

- Colors and other visual parameters already supported by the core.
- References or parameters for supported visual features.
- Extensible fields for future modular appearance systems.

**Physical-condition configuration**

- Supported injury and amputation information.
- Other existing physical-condition data where appropriate.

**Generation configuration**

- Seed or deterministic generation settings, if required by the existing generation API.
- Any explicitly defined options required to reproduce the generation request.

This composition is a logical boundary, not a requirement to place every field into one large C++ structure. Existing dedicated structures may remain separate and be composed through the profile.

### FR-02 — Separation of Configuration Domains

**Priority:** Must Have

The system shall distinguish between:

1. Anatomy configuration: how the character's body is proportioned.
2. Appearance configuration: how supported visual features and colors are selected.
3. Physical conditions: injuries, amputations, and other supported physical states.
4. Generated output: the resulting character representation or rendered image.

Requirements:

- Anatomy parameters must not be duplicated as unrelated appearance settings.
- Physical conditions must not be used as a substitute for anatomical configuration.
- Rendering output must not be treated as the authoritative source of the original profile.
- Existing condition behavior must remain compatible unless a documented migration is required.
- Changes to one configuration domain should not unintentionally overwrite unrelated domains.

For example, changing a body-proportion parameter must not silently erase injury data or replace the character's appearance configuration.

### FR-03 — Configurable Parameters

**Priority:** Must Have

Character properties intended to be configurable must be represented explicitly in the profile or in a clearly defined configuration object referenced by the profile.

Requirements:

- The renderer must consume configuration through defined interfaces.
- Configurable properties must not be hidden as scattered magic numbers.
- Each configurable field must have a documented meaning and unit, where applicable.
- Fields must have defined valid ranges or validation rules.
- Fields that are not yet implemented must not be presented as functional configuration options.
- The system must distinguish required parameters, optional parameters, and derived values.
- Configuration changes must be traceable through tests.

This requirement does not mandate making every low-level pixel coordinate configurable. Rendering implementation details may remain internal when exposing them would create unnecessary complexity.

### FR-04 — Default Profile

**Priority:** Must Have

The system shall provide a canonical way to create a valid default character profile.

Requirements:

- A default profile must satisfy all required validation rules.
- Existing baseline generation behavior should be preserved as closely as possible.
- Default values must be defined in the authoritative C++ core.
- The default profile must not depend on whether it was requested from C++, the C API, or Godot.
- Repeated creation of the same default profile must produce equivalent configuration.
- Changes to defaults must be covered by tests and documented when they affect externally observable behavior.

The default profile is not a promise that every future species or body type will share identical defaults. Future specialized profiles may override supported properties.

### FR-05 — Profile Validation

**Priority:** Must Have

The system shall validate profile data before it is used by character generation.

Validation must cover, as applicable:

- Missing required configuration.
- Unsupported enum or identifier values.
- Numeric values outside documented limits.
- Non-finite numeric values where floating-point fields are used.
- Invalid combinations of related fields.
- Unsupported profile schema versions.
- Malformed serialized input.
- Invalid references between profile components, if references are introduced.

Validation outcomes must be explicit and consistent.

The implementation should distinguish at least these outcomes:

1. **Valid:** configuration can be used as supplied.
2. **Valid with normalization:** configuration can be made valid through an explicitly documented normalization rule.
3. **Invalid:** configuration cannot safely be used and must be rejected or handled through an explicitly documented compatibility policy.

The project must not silently convert arbitrary invalid input into plausible-looking character data without making the behavior observable.

### FR-06 — Normalization and Fallback Policy

**Priority:** Must Have

The system shall define what happens when optional fields are absent, values are outside supported ranges, or an older profile does not contain newly introduced fields.

Requirements:

- Missing optional fields may receive documented defaults.
- Missing required fields must produce a defined validation outcome.
- Numeric clamping may be used only where it is semantically appropriate and documented.
- Invalid identifiers must not silently map to unrelated valid identifiers.
- Unsupported schema versions must not be interpreted as valid current profiles without an explicit compatibility rule.
- Fallback behavior must be deterministic.
- The system must provide sufficient diagnostic information to identify rejected or normalized data.

A reasonable policy is to distinguish strict validation from controlled normalization. Whether these should be separate public API operations or internal steps is an implementation decision.

### FR-07 — Serialization

**Priority:** Must Have

The system shall support serializing supported Character Profile data into a documented format compatible with the project's existing serializer.

Requirements:

- Serialization must preserve all supported configuration fields.
- The serialized representation must distinguish profile configuration from generated image data.
- Field names, types, units, and optionality must be documented.
- The format must include a schema or format version if the existing serialization contract does not already provide one.
- Serialization must use a deterministic field representation where practical.
- Unsupported or unknown data must follow an explicit compatibility policy.
- Serialization must not depend on Godot-specific object state.

The implementation must audit the existing serializer before introducing a new format or a parallel serialization mechanism.

### FR-08 — Deserialization and Reconstruction

**Priority:** Must Have

The system shall reconstruct a Character Profile from supported serialized data.

Requirements:

- Deserialization must validate input before the profile is used for generation.
- The system must distinguish malformed data from valid profiles with optional fields missing.
- Reconstruction must preserve supported profile semantics.
- Missing fields may use version-appropriate defaults where backward compatibility is supported.
- Invalid data must not cause undefined behavior, memory errors, or uncontrolled renderer access.
- The result must be usable by the same generation pipeline as an equivalent profile created directly in C++.

### FR-09 — Round-Trip Consistency

**Priority:** Must Have

For supported valid profiles, the following operation must preserve the defined profile semantics:

`Profile → Serialize → Deserialize → Reconstructed Profile`

Requirements:

- All supported persisted fields must survive the round trip.
- Default and optional field behavior must be consistent.
- Numeric values must not drift unexpectedly during serialization.
- Field ordering must not change semantic meaning.
- The reconstructed profile must pass the same applicable validation rules.
- Equivalent profiles must remain equivalent according to the project's documented comparison rules.

Exact byte-for-byte serialization equality is desirable where practical but is not a substitute for semantic round-trip equality.

### FR-10 — Integration with Character Generation

**Priority:** Must Have

The character-generation pipeline shall accept or resolve a validated profile through a documented interface.

Requirements:

- The generation pipeline must consume authoritative profile values.
- Invalid profiles must be handled before unsafe generation operations occur.
- Existing generation entry points must remain available unless a migration is explicitly approved.
- Existing appearance and condition behavior must be preserved for equivalent legacy inputs.
- Profile resolution must not create conflicting sources of configuration.
- A generated result must remain distinguishable from the profile that produced it.

The implementation may introduce an adapter between the new profile model and existing generator parameters if that minimizes disruption to the established core.

### FR-11 — C API Integration

**Priority:** Must Have when profile operations require new external entry points

The C API shall expose only the operations needed for external consumers to create, validate, serialize, deserialize, or use profiles.

Requirements:

- Public C-compatible types must not expose C++ implementation details.
- Memory ownership and buffer lifetime must be documented.
- Error reporting must follow existing API conventions where possible.
- New functions must not create a second implementation of profile logic.
- Existing C API v2 behavior must remain compatible unless a versioned breaking change is explicitly approved.
- Any new ABI requirements must be documented and tested.

The precise function signatures must be selected after auditing the current C API. This BRD does not prescribe names or signatures before that audit.

### FR-12 — GDExtension and Godot Integration

**Priority:** Must Have for end-to-end release validation

Requirements:

- Godot must be able to exercise profile functionality through the native integration layer.
- Profile validation and default resolution must remain authoritative in C++.
- Errors must be surfaced clearly to the calling layer.
- Missing or failed native functionality must not silently appear as a successful profile operation.
- Existing preview functionality must remain operational.
- Any necessary Godot controls must be limited to demonstrating and testing profile behavior in this release.

A full visual profile editor is explicitly outside the scope of v0.3.1.

### FR-13 — Diagnostics

**Priority:** Should Have

The system should provide actionable diagnostics for:

- Invalid profile values.
- Unsupported schema versions.
- Failed deserialization.
- Normalization or fallback decisions.
- Failed native integration calls.

Diagnostics must not replace structured return values or error codes where those are required by the existing API.

### FR-14 — Existing Behavior and Regression Protection

**Priority:** Must Have

The new profile system must preserve supported behavior from the existing generation pipeline.

Requirements:

- Existing appearance generation tests must continue to pass.
- Existing injury and amputation tests must continue to pass.
- Existing C API and serializer tests must remain valid.
- Existing golden fixtures must not be changed merely to make tests pass.
- Any intentional output changes must be explained and approved.
- Existing deterministic generation behavior must remain stable for equivalent inputs, subject to the project's documented compatibility guarantees.

## 7. Conceptual Data Model

The following is a logical model for discussion, not a final C++ header specification.

### 7.1 CharacterProfile

Represents the complete configuration used for a generation request.

Logical responsibilities:

- Identify the profile schema version.
- Reference or contain the supported configuration domains.
- Provide a consistent boundary for validation and serialization.
- Preserve any generation settings necessary for reproducibility.

### 7.2 AnatomyProfile

Represents configurable body structure and proportions.

Potential future fields include:

- Height or overall body scale.
- Body proportions.
- Torso proportions.
- Shoulder and hip proportions.
- Limb proportions.
- Supported body-shape parameters.

Only parameters implemented and validated in v0.3.1 should be exposed as operational features. Fields for future anatomy should be introduced when their semantics and consumers are defined, rather than added speculatively.

### 7.3 AppearanceProfile

Represents visual characteristics that are independent of the anatomical configuration.

Potential responsibilities include:

- Existing color and palette configuration.
- Supported appearance variation parameters.
- Future face and hair configuration.
- Future visual identifiers for compatible modular features.

### 7.4 PhysicalConditionProfile

Represents supported physical states, reusing the existing condition and anatomy-related structures where appropriate.

Potential responsibilities include:

- Injury configuration.
- Amputation state.
- Other supported physical conditions.

The profile must not duplicate the existing condition model without a clear reason.

### 7.5 Generated Character Output

Represents the output of the generation pipeline.

This may include existing appearance dictionaries, pixel buffers, rendered images, or other current output types.

Generated output is not the profile itself. Reconstructing a profile from an image is not a requirement of this release.

## 8. Business Rules

**BR-01 — Single source of truth**

C++ core owns profile semantics, validation rules, defaults, and generation behavior.

**BR-02 — One authoritative configuration**

When profile-based generation is introduced, conflicting values from a profile and separate legacy parameters must have a defined resolution policy.

**BR-03 — Validated input**

Only profiles that pass the required validation stage may reach operations that depend on their validity.

**BR-04 — Explicit defaults**

Default values must be defined centrally and covered by tests.

**BR-05 — No silent semantic loss**

Serialization and reconstruction must not silently discard supported profile data.

**BR-06 — Controlled schema evolution**

Changes to serialized fields must follow a documented schema compatibility policy.

**BR-07 — Deterministic reconstruction**

The same supported serialized profile, under the same schema and normalization rules, must reconstruct equivalent configuration.

**BR-08 — Preserve existing conditions**

Existing injury and amputation semantics must remain intact unless an intentional migration is documented.

**BR-09 — Configuration is not rendering**

A profile describes how to generate a character; it is not a container for all internal renderer state.

**BR-10 — Future extensibility without premature implementation**

The architecture must permit future extension, but v0.3.1 must not claim support for fields that are merely placeholders.

## 9. Non-Functional Requirements

### NFR-01 — Determinism

Profile creation, validation, normalization, serialization, and reconstruction must behave deterministically for equivalent supported inputs.

### NFR-02 — Safety and robustness

Malformed serialized data and invalid configuration must not trigger undefined behavior, out-of-bounds access, or uncontrolled allocation.

### NFR-03 — Maintainability

Profile semantics should be defined in a small number of authoritative core components rather than duplicated across the renderer, C API, extension, and GDScript.

### NFR-04 — Compatibility

Existing APIs and generation behavior must be preserved wherever possible. Any unavoidable breaking changes must be explicitly versioned and documented.

### NFR-05 — Performance

Profile validation and reconstruction should have low overhead relative to character generation. Performance optimization is not a primary goal unless profiling identifies a regression.

### NFR-06 — Testability

Profile behavior must be testable without requiring the Godot editor for every unit test. End-to-end integration must still be verified separately.

### NFR-07 — Portability

The implementation must respect the repository's supported build environments and avoid introducing platform-specific profile behavior.

### NFR-08 — Documentation

All public profile fields, validation rules, defaults, serialization behavior, and relevant error outcomes must be documented.

## 10. Acceptance Criteria

### AC-01 — Profile model exists

A canonical Character Profile representation exists in the C++ core and is integrated into the generation configuration path.

**Pass condition:** The profile is not merely a Godot-side dictionary or an unused C++ structure.

### AC-02 — Domains are separated

Anatomy, appearance, and physical conditions have clear ownership and documented boundaries.

**Pass condition:** Updating one domain does not unexpectedly erase or overwrite unrelated supported data.

### AC-03 — Default profile is valid

The default profile passes validation and can be used by the generation pipeline.

**Pass condition:** Default profile tests pass consistently across supported build configurations.

### AC-04 — Validation is deterministic

Valid, normalizable, and invalid configurations have documented outcomes.

**Pass condition:** Tests cover boundary values, missing required fields, unsupported values, and malformed input.

### AC-05 — Fallback behavior is explicit

Missing optional fields and supported legacy data use documented defaults or migration rules.

**Pass condition:** No undocumented fallback silently converts invalid data into a successful result.

### AC-06 — Round-trip is correct

Valid supported profiles can be serialized and deserialized without losing supported configuration semantics.

**Pass condition:** Round-trip tests compare the defined profile fields and verify equivalent generation configuration.

### AC-07 — Generation uses the profile

The new profile is connected to the authoritative C++ generation pipeline.

**Pass condition:** Changing a supported profile parameter produces the expected observable configuration or generation change.

### AC-08 — Existing behavior is preserved

Existing relevant tests continue to pass.

**Pass condition:** Any changed golden output has an explicitly approved reason and documented expected behavior.

### AC-09 — Native integration works

The native extension loads and profile-related operations work through the supported integration path.

**Pass condition:** A Godot smoke test verifies actual native integration rather than relying only on fallback behavior.

### AC-10 — Public contracts are documented

The profile structure, defaults, validation rules, serialization schema, and compatibility policy are documented.

**Pass condition:** Another developer can create, validate, serialize, and reconstruct a profile without relying on undocumented implementation details.

## 11. Testing Strategy

### 11.1 Unit tests

Cover:

- Default profile construction.
- Profile field defaults.
- Valid and invalid profile validation.
- Numeric boundary behavior.
- Missing and unsupported values.
- Normalization and fallback rules.
- Profile equality or semantic comparison.
- Separation of anatomy, appearance, and condition data.

### 11.2 Serialization tests

Cover:

- Valid profile serialization.
- Valid profile deserialization.
- Round-trip preservation.
- Missing optional fields.
- Malformed serialized data.
- Unsupported schema versions.
- Backward compatibility for explicitly supported older formats.
- Deterministic serialization where required.

### 11.3 Generation integration tests

Cover:

- Default profile generation.
- Generation using a modified supported parameter.
- Existing injury behavior.
- Existing amputation behavior.
- Interaction between profile configuration and existing generation inputs.
- Reproducibility under equivalent inputs.

### 11.4 C API tests

If new public profile operations are introduced, cover:

- Valid requests.
- Invalid requests.
- Memory ownership and cleanup.
- Error reporting.
- Compatibility with existing API behavior.

### 11.5 GDExtension smoke tests

Cover:

- Extension loading.
- Native class availability.
- Successful profile-related invocation.
- Error propagation.
- Generation or preview output through the native path.

A fallback preview must not be accepted as evidence that the extension works.

### 11.6 Regression suite

Run the existing CTest suite and all relevant golden fixtures.

The existing fixture count should be verified against the current repository before it is quoted in release notes. The profile work must not weaken existing regression coverage.

## 12. Implementation Workstreams

### WS-01 — Repository audit

- Inspect current appearance, conditions, generator, serializer, types, and C API.
- Identify existing data that can be reused.
- Identify hardcoded character parameters in the generation path.
- Map current generation entry points and compatibility constraints.
- Record the baseline test results before modifying code.

**Deliverable:** A concise architecture audit and proposed mapping from existing structures to the Character Profile model.

### WS-02 — Profile model design

- Define conceptual and concrete data ownership.
- Decide which existing types are reused or composed.
- Define schema-version responsibilities.
- Define required versus optional fields.
- Define profile equality and reconstruction semantics.

**Deliverable:** Approved model specification and C++ implementation plan.

### WS-03 — Defaults and validation

- Implement default profile construction.
- Implement validation and normalization.
- Define structured failure behavior.
- Add boundary and invalid-input tests.

**Deliverable:** A validated default profile and documented validation contract.

### WS-04 — Serialization and reconstruction

- Integrate with the existing serializer.
- Implement or extend reconstruction logic.
- Establish schema evolution behavior.
- Add round-trip and malformed-input tests.

**Deliverable:** A tested, version-aware profile serialization path.

### WS-05 — Generator integration

- Connect the profile to the existing generator.
- Remove or centralize relevant hardcoded configurable properties.
- Preserve compatibility with supported existing inputs.
- Add generation integration tests.

**Deliverable:** Profile-driven generation using the C++ core.

### WS-06 — Native integration

- Expose only necessary operations through the C API and GDExtension.
- Preserve existing API contracts where possible.
- Surface errors to Godot.
- Verify actual extension loading and operation.

**Deliverable:** A working native profile integration path.

### WS-07 — Release validation and documentation

- Run Release build and CTest.
- Run applicable golden fixture tests.
- Run Godot smoke tests.
- Review API and serialization compatibility.
- Document the supported profile contract and known limitations.

**Deliverable:** Release-ready v0.3.1 implementation and changelog.

## 13. Dependencies and Risks

### Dependency D-01 — Existing data structures

The profile model depends on the current definitions of appearance, conditions, generator inputs, types, and serializer.

**Mitigation:** Audit existing structures before designing replacement types.

### Dependency D-02 — Existing C API contract

The profile system must coexist with the current public API.

**Mitigation:** Prefer additive changes or adapters over unnecessary breaking changes.

### Risk R-01 — Overengineering the profile schema

Designing for every future species, outfit, animation, and biological condition can make the initial implementation unnecessarily abstract.

**Mitigation:** Build only the shared concepts required by current functionality and a clearly defined extension boundary.

### Risk R-02 — Duplicate sources of truth

Profile defaults or validation may be implemented independently in C++, the C API, and GDScript.

**Mitigation:** Keep the authoritative rules in the C++ core.

### Risk R-03 — Breaking existing generation output

Refactoring parameters into a profile may unintentionally change rendering behavior.

**Mitigation:** Capture baseline tests and compare existing fixtures before and after the refactor.

### Risk R-04 — Ambiguous fallback behavior

Silent normalization may hide incorrect inputs and make bugs difficult to diagnose.

**Mitigation:** Document validation outcomes and test them explicitly.

### Risk R-05 — Premature public API changes

Exposing an unstable profile model too early can make future schema changes expensive.

**Mitigation:** Keep the public interface minimal until field semantics and compatibility requirements are clear.

### Risk R-06 — False confidence from preview fallback

Godot may display a fallback result when native integration is unavailable.

**Mitigation:** Add a smoke test that verifies native class availability and successful invocation.

## 14. Release Definition

Version v0.3.1 is ready for release when:

1. The Character Profile model is integrated into the C++ core.
2. Anatomy, appearance, and physical conditions have defined boundaries.
3. Default creation, validation, and fallback behavior are documented and tested.
4. Supported profiles can be serialized and reconstructed consistently.
5. The generation pipeline consumes the profile through a defined interface.
6. Existing generation and condition tests pass.
7. Required C API and GDExtension integration tests pass.
8. Release build, CTest, and Godot smoke tests succeed.
9. Public documentation describes the actual implemented contract.
10. The release notes make no claims about future features that have not been implemented.

## 15. Definition of Done Checklist

- [ ] Repository audit completed.
- [ ] Existing structures mapped to the profile model.
- [ ] Character Profile implemented in C++ core.
- [ ] Anatomy, appearance, and condition ownership documented.
- [ ] Default profile implemented and tested.
- [ ] Validation and normalization rules implemented.
- [ ] Fallback behavior documented and tested.
- [ ] Serialization and reconstruction integrated.
- [ ] Round-trip tests pass.
- [ ] Generator consumes the validated profile.
- [ ] Existing behavior and golden fixtures verified.
- [ ] C API changes tested, if applicable.
- [ ] GDExtension integration verified.
- [ ] Godot smoke test passes.
- [ ] Documentation and changelog updated.
- [ ] Release build and CTest pass.

## 16. Expected Outcome

After v0.3.1, NPC Builder should have a stable configuration foundation for character generation.

Developers should be able to define a character profile, validate it, serialize it, reconstruct it, and pass it into the existing C++ generation pipeline without duplicating character-generation rules across integration layers.

The release should make later anatomy and appearance work incremental rather than architectural rewrites.

**Next planned milestone:** v0.3.2 may use the new profile model to introduce the first concrete anatomy-profile parameters, subject to the results of the v0.3.1 architecture audit and acceptance tests.
