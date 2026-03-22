# Development Log

---

## 2026-03-22 -- Professional Chinese TDD Baseline

**Summary**: Added a professional Chinese TDD that documents the current implementation baseline of `Unrealtor_Demo` without script-style narration. The document consolidates architecture ownership, core match algorithm flow, config/runtime behavior, spec gaps, risks, and verification criteria.

**Files Changed**:
- `docs/TDD_Unrealtor_Demo_CN.md` -- new Chinese technical design baseline aligned to current code and config state.
- `docs/DEVLOG.md` -- added this development log entry.

**Behavior Changes**:
- None.

**Reasoning**: The requested deliverable was a formal Chinese TDD describing what the repository currently has, not a tutorial script. Capturing implemented scope and explicit spec deltas in one baseline document supports review and handoff accuracy.

---

## 2026-03-22 -- Chinese Module Tutorial TDD

**Summary**: Produced a module-by-module Chinese technical design teaching document tailored for Lisa, including per-module narration, architecture rationale, blueprint explanation, and PPT assembly guidance. This turns the existing long-form script into a reusable and auditable training artifact.

**Files Changed**:
- `docs/Lisa_TDD_CN_ModuleByModule.md` -- added full Chinese TDD with sectioned script modules, teaching goals, demo cues, and handoff template.
- `docs/DEVLOG.md` -- added this development log entry.

**Behavior Changes**:
- None.

**Reasoning**: The user requested a sendable Chinese TDD organized by modules and script segments. Structuring the material into discrete teaching modules reduces ambiguity and makes presentation reuse straightforward.

---

## 2026-03-22 -- Miro Teaching Diagram Pack

**Summary**: Built and published a complete Miro diagram pack for Lisa's tutorial covering architecture, core mechanics, UE-vs-Unity differences, Blueprint explanation, speaker flow, and verification sequence. This shifted deliverables to board-first visuals so the PPT can be assembled directly from diagrams.

**Files Changed**:
- `docs/Lisa_Core_Mechanics_Architecture_Bilingual.md` -- added a bilingual architecture reference mapping implemented systems, ownership boundaries, and current gaps.
- `docs/DEVLOG.md` -- added this development log entry.

**Behavior Changes**:
- None (documentation and Miro board content only).

**Reasoning**: The user requested all teaching materials to be delivered as Miro diagrams. Creating a structured diagram set provides immediate presentation assets while retaining one local architecture reference for audit and handoff.

---

## 2026-03-22 -- Full-Match UI Gating

**Summary**: Updated alignment HUD gating so both-side UI is only shown when the same condition used by full match readiness is true. Partial states now keep single-side feedback active instead of showing both frames prematurely.

**Files Changed**:
- `Source/Unrealtor_Demo/MatchActor.cpp` -- replaced direct `bLeftAligned/bRightAligned` HUD push with explicit UI gating logic tied to `bFullMatch`.
- `docs/DEVLOG.md` -- added this development log entry.

**Behavior Changes**:
- `L=N, R=N` keeps both UI sides hidden.
- Single-side alignment still shows only that side.
- When both sides align but full-match checks are not yet satisfied, only one side remains visible; both sides appear only once full-match-ready conditions are met.

**Reasoning**: The previous UI mapping exposed both aligned frames before full readiness and created false-positive feedback. Binding dual-frame visibility to the actual full-match gate removes ambiguity while preserving progressive single-side hints.

---

## 2026-03-21 -- Default Alignment Point Pair

**Summary**: Added two default `UAlignmentPointComponent` subobjects to `AQuadActor` so every new quad starts with a usable top/bottom vertex pair. Designers can delete, move, or add more as needed.

**Files Changed**:
- `Source/Unrealtor_Demo/QuadActor.h` -- added `DefaultAlignPoint0` and `DefaultAlignPoint1` member pointers.
- `Source/Unrealtor_Demo/QuadActor.cpp` -- created both default subobjects in constructor with offset positions and sequential `PointIndex` values.
- `docs/DEVLOG.md` -- added this development log entry.

**Behavior Changes**:
- New quad actors spawn with two alignment point components at +50 and -50 Z relative to root.
- Auto-collection still discovers all alignment point components including these defaults.
- Designers can delete either default or add more without code changes.

**Reasoning**: Most puzzles need at least two alignment vertices. Providing sensible defaults reduces per-actor setup from "add components manually every time" to "adjust positions if needed."

---

## 2026-03-21 -- Auto-Collected AlignmentPointComponent

**Summary**: Replaced the manual `AlignmentPoints` array with a dedicated `UAlignmentPointComponent` class. Designers add these components to quad actors and position them in the viewport; the quad auto-collects them at BeginPlay via `GetComponents`. No manual array wiring required.

**Files Changed**:
- `Source/Unrealtor_Demo/AlignmentPointComponent.h` -- new `USceneComponent` subclass with `PointIndex` for ordering and editor visualization enabled.
- `Source/Unrealtor_Demo/AlignmentPointComponent.cpp` -- constructor disabling tick and enabling editor billboard icon.
- `Source/Unrealtor_Demo/QuadActor.h` -- removed `AlignmentPoints` array property and `USceneComponent` forward declaration; added `UAlignmentPointComponent` forward declaration.
- `Source/Unrealtor_Demo/QuadActor.cpp` -- replaced manual array iteration with `GetComponents<UAlignmentPointComponent>` sorted by `PointIndex`; removed stale debug logging.
- `docs/DEVLOG.md` -- added this development log entry.

**Behavior Changes**:
- Alignment point authoring is now single-step: add component, position it, done.
- Vertex collection is automatic and sorted by `PointIndex`.
- AABB fallback remains when no alignment point components are present.
- Old `AlignmentPoints` array data on existing actors is discarded (was broken anyway).

**Reasoning**: A typed component class eliminates the entire class of "forgot to wire the array" bugs, provides editor-visible icons at each point, and scales cleanly across quad variants. `GetComponents` discovery is the standard UE pattern for this kind of designer-authored spatial markup.

---

## 2026-03-21 -- Stable Component Array Assignment

**Summary**: Replaced `AlignmentPoints` with direct scene-component instance pointers to eliminate Details panel array assignment stomping. This makes per-index alignment marker assignment stable for actor instances.

**Files Changed**:
- `Source/Unrealtor_Demo/QuadActor.h` -- changed `AlignmentPoints` to `TArray<TObjectPtr<USceneComponent>>`.
- `Source/Unrealtor_Demo/QuadActor.cpp` -- updated alignment-point iteration and logging to consume direct component pointers.
- `docs/DEVLOG.md` -- added this development log entry.

**Behavior Changes**:
- Assigning one alignment point in the array no longer clears previously assigned entries.
- Cached vertex extraction now reads directly from assigned scene-component instances.

**Reasoning**: `FComponentReference` array editing can be unstable for same-actor component assignment in this workflow. Direct object references provide deterministic editor behavior with simpler runtime access.

---

## 2026-03-21 -- Component Picker Alignment References

**Summary**: Switched quad alignment point authoring to component references so instance-level marker components can be selected directly in Details and resolved safely at runtime. This removes class-only selection behavior and aligns with editor-side point assignment workflow.

**Files Changed**:
- `Source/Unrealtor_Demo/QuadActor.h` -- added `AlignmentPoints` as `TArray<FComponentReference>` with component picker metadata.
- `Source/Unrealtor_Demo/QuadActor.cpp` -- resolved each component reference via `GetComponent(this)` and cached world locations from valid scene components.
- `docs/DEVLOG.md` -- added this development log entry.

**Behavior Changes**:
- `Alignment Points` on placed quad actors now supports selecting concrete components (for example `AlignPoint_0..3`) instead of class-only style references.
- Vertex caching now consumes resolved component references and falls back to mesh bounds when none are valid.

**Reasoning**: Component instance assignment in UE Details panels is most reliable through `FComponentReference` for actor-owned components. This keeps runtime lookup explicit and robust while preserving existing fallback behavior.

---

## 2026-03-21 -- Defer HUD Creation Until LocalPlayer

**Summary**: Updated player-controller HUD initialization to defer widget creation until a valid `LocalPlayer` is attached. This prevents split-screen Player 2 from hitting `CreateWidget` with no player owner during early lifecycle.

**Files Changed**:
- `Source/Unrealtor_Demo/UnrealtorPlayerController.h` -- added internal retry state and helper declaration for deferred HUD creation.
- `Source/Unrealtor_Demo/UnrealtorPlayerController.cpp` -- replaced direct `BeginPlay` widget spawn with guarded retry-based creation using next-tick deferral.
- `docs/DEVLOG.md` -- added this development log entry.

**Behavior Changes**:
- Player controllers now wait for a valid `LocalPlayer` before creating alignment HUD widgets.
- The `CreateWidget cannot be used on Player Controller with no attached player` runtime error is prevented for delayed-attached local players.

**Reasoning**: In split-screen startup, secondary local controllers can exist briefly before their `LocalPlayer` is fully attached. Deferring creation on a short retry loop keeps ownership correct without moving HUD responsibility out of the controller.

---

## 2026-03-21 -- Fix UMG Module Linkage

**Summary**: Added the missing `UMG` module dependency so `UUserWidget` symbols resolve at link time for the player controller HUD widget usage. This unblocks editor DLL linking without changing gameplay logic.

**Files Changed**:
- `Source/Unrealtor_Demo/Unrealtor_Demo.Build.cs` -- added `UMG` to `PublicDependencyModuleNames`.
- `docs/DEVLOG.md` -- added this development log entry.

**Behavior Changes**:
- None.

**Reasoning**: `UUserWidget` APIs are implemented in the `UMG` module, so any module referencing those symbols must explicitly link against it. This is the minimal, correct fix for the reported unresolved externals.

---

## 2026-03-21 -- Alignment HUD Data Bridge

**Summary**: Added a per-player alignment HUD data bridge so split-screen controllers can own their own widget instance and receive frame/progress/closeness updates from the match evaluator. This enables Blueprint UI authoring without changing match math behavior.

**Files Changed**:
- `Source/Unrealtor_Demo/UnrealtorPlayerController.h` -- added UI widget class/state properties, update/reset functions, and Blueprint event contract.
- `Source/Unrealtor_Demo/UnrealtorPlayerController.cpp` -- created per-player widget in `BeginPlay` and implemented state update/reset logic.
- `Source/Unrealtor_Demo/MatchActor.h` -- added helper declaration to push alignment UI state to both players.
- `Source/Unrealtor_Demo/MatchActor.cpp` -- pushed/reset UI state through alignment lifecycle and exposed normalized submit progress.
- `docs/DEVLOG.md` -- added this development log entry.

**Behavior Changes**:
- Each local player controller can now spawn its own alignment HUD widget via `AlignmentHUDWidgetClass`.
- Alignment frame visibility is now side-specific (`LeftAligned` drives P1, `RightAligned` drives P2).
- Auto-submit progress and closeness are pushed to both players every evaluation tick and reset on non-valid paths.

**Reasoning**: The controller-owned widget approach keeps split-screen ownership explicit and avoids global HUD contention. Using a narrow C++ data contract with Blueprint events lets UI iteration stay in WBP while preserving current gameplay logic.
