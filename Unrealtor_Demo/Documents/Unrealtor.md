UNREALTOR — UE5 RECREATION SPECIFICATION
=========================================

This document is a complete technical specification for recreating the game
"Unrealtor" in Unreal Engine 5 using C++ and Blueprint. The original was
built in Unity (C#/Cinemachine/URP). This spec captures every gameplay
system, the exact algorithms used, and the intended architecture for
a clean UE5 implementation.

The game is a local split-screen co-op first-person puzzle game.
Two players share one screen, split left/right. Each player controls
a first-person character. The core mechanic: objects ("quads") are
placed in each player's world space. When both players position
themselves so their respective objects visually align at the vertical
screen-center seam, a "match" is detected and the puzzle solves.

Think of it as: each player sees half of a jigsaw piece on their side
of the screen. They must physically walk and look until those halves
line up perfectly across the center divide.

=====================================
SECTION 1: PROJECT SETUP
=====================================

Engine: Unreal Engine 5.4+ (or latest stable)
Template: First Person (C++)
Render Pipeline: Default forward/deferred (no custom pipeline needed)
Plugins required:
  - Enhanced Input (built-in)
  - Common UI (optional, for menus)

Project name: Unrealtor

Create the following module structure:
  Source/Unrealtor/         (primary game module)
    Player/                 (player controller, character, state machine)
    Puzzle/                 (match system, quad, puzzle base)
    Managers/               (game manager, match manager, input manager)
    Camera/                 (split-screen camera management)
    UI/                     (HUD, match UI, menus)
    Audio/                  (audio manager — MetaSounds or simple)
    Save/                   (save/load data)

=====================================
SECTION 2: INPUT SYSTEM
=====================================

Use Enhanced Input exclusively. Create one Input Mapping Context (IMC)
per player, not one shared context.

--- Input Actions (IA) ---
IA_Move          (Vector2D, WASD / Left Stick)
IA_Look          (Vector2D, Mouse Delta / Right Stick)
IA_Jump          (Digital, Space / South Button)
IA_Interact      (Digital, E / West Button)
IA_SlowMode      (Digital, Left Shift / Left Trigger)
IA_ShowHint      (Digital, Z / Right Trigger)
IA_TiltLeft      (Digital, Q / Left Bumper)
IA_TiltRight     (Digital, E / Right Bumper)
IA_ResetRoll     (Digital, R / Right Stick Press)
IA_SwitchPlayer  (Digital, Tab / Select Button) — single-player only
IA_Pause         (Digital, Escape / Start Button)
IA_Submit        (Digital, F / North Button) — reserved for manual submit

--- Input Mapping Contexts ---
IMC_Player1_Keyboard: WASD + mouse look (or WASD only in split-keyboard)
IMC_Player2_Keyboard: Arrow keys + numpad (or Arrow keys only)
IMC_Player_Gamepad:   Standard gamepad layout

--- Controller Modes ---
The game supports three configurations, chosen from the main menu:
  1. SinglePlayer:     One player controls both characters, Tab to switch.
                       Only the active character receives input.
  2. TwoPlayer_KB:     Player 1 uses WASD, Player 2 uses Arrow keys.
                       Mouse can be assigned to either (or Player 2).
  3. TwoPlayer_Mixed:  Player 1 keyboard+mouse, Player 2 gamepad
                       (or two gamepads).

Implementation in C++:
  - UUnrealtorInputManager (UGameInstanceSubsystem or spawned actor)
  - Holds references to both AUnrealtorPlayerController instances
  - On game start, reads config and assigns IMCs to each local player
  - For single-player mode, only one controller is "active" at a time;
    Tab swaps which PC gets the IMC bound

=====================================
SECTION 3: PLAYER CHARACTER
=====================================

--- AUnrealtorCharacter (C++ : ACharacter) ---
This is the pawn each player controls. Two instances exist in the level.

Properties (UPROPERTY, exposed to Blueprint):
  - PlayerIndex (int32, 0 or 1)
  - MoveSpeed (float, default 700 cm/s)
  - JumpHeight (float, default 150 cm)
  - GravityScale (float, default 1.0)
  - SensitivityMultiplier (float, 0..2, default 1.0)
  - CurrentGravityDirection (FVector, default (0,0,-1))
  - bEnableAutoStep (bool, default true)
  - MaxStepHeight (float, default 40 cm)
  - bEnableFallPrevention (bool, default true)
  - MaxAllowedDropHeight (float, default 200 cm)

Key Components:
  - UCameraComponent (first person camera, attached to capsule)
  - USpringArmComponent (minimal, mostly for roll control)
  - USkeletalMeshComponent (third-person model visible to OTHER player)
  - UInputComponent via Enhanced Input

--- Movement ---
Movement is camera-relative on the plane perpendicular to gravity.
Project camera forward onto the anti-gravity plane, then build a
right vector orthogonal to it. Input X maps to right, Input Y to forward.

In the Unity version, movement uses Rigidbody.MovePosition (kinematic).
In UE5, use the CharacterMovementComponent with custom gravity support:
  - Override GetGravityDirection() to return CurrentGravityDirection
  - Movement is grounded when a sphere trace along gravity finds a surface
  - Air control remains standard UE defaults

--- State Machine ---
Implement a simple hierarchical FSM. No need for a plugin.

Top-level states: Grounded, Jumping, Falling
Grounded sub-states: Idle, Walking, Running

Transitions:
  Grounded -> Jumping:  Jump input triggered AND IsGrounded
  Grounded -> Falling:  !IsGrounded (walked off edge)
  Jumping  -> Falling:  Velocity dot GravityDirection >= 0 (apex reached)
  Jumping  -> Grounded: IsGrounded AND falling
  Falling  -> Grounded: IsGrounded

Sub-state transitions (within Grounded):
  Idle    -> Walking:  Input magnitude > 0.1
  Walking -> Idle:     Input magnitude <= 0.1
  Walking -> Running:  Input magnitude > 0.8
  Running -> Walking:  Input magnitude <= 0.8
  Running -> Idle:     Input magnitude <= 0.1

Each state drives an animator blend parameter "Speed":
  Idle = 0, Walk = 1, Run = 2

--- Gravity Switching ---
The character can have its gravity direction changed at runtime.
When switched:
  1. Slerp the character's rotation from current to new orientation
     over 0.5 seconds (new Up = -NewGravityDirection)
  2. Update movement plane accordingly
  3. Ground checks use the new gravity direction

This is triggered by GravitySwitchZone volumes placed in the level.

--- Auto-Step ---
When grounded and moving into a low obstacle:
  1. Forward sphere cast at foot level detects obstacle
  2. If obstacle normal is too vertical (wall), attempt step:
     a. Sweep upward by MaxStepHeight — if blocked, abort
     b. From raised position, sweep forward — if still a wall, abort
     c. Move character to raised+forward position
  This handles stairs and small ledges without jumping.

--- Fall Prevention ---
When grounded and moving toward a ledge:
  1. Cast a probe forward+down from the intended move position
  2. If no ground found, or drop exceeds MaxAllowedDropHeight:
     Block the horizontal component of movement in that direction
  3. This prevents walking off tall drops but allows small step-downs

--- Camera Roll/Tilt ---
Players can tilt the camera (roll) using Q/E or bumpers.
  - Roll speed: 90 deg/s
  - Return speed: 180 deg/s (auto-centers when released)
  - Clamp to [-60, +60] degrees
  - During SlowMode, roll speed is multiplied by slowRollRatio (0.3)

Implementation: Apply roll to the camera component's relative rotation.

=====================================
SECTION 4: SPLIT-SCREEN CAMERA
=====================================

Use UE5's built-in local multiplayer split-screen:
  - UGameViewportClient configured for 2-player horizontal split
  - Player 1: left half (Rect 0,0 to 0.5,1)
  - Player 2: right half (Rect 0.5,0 to 1.0,1)

Each player has their own camera. The camera is a standard first-person
POV camera attached to the character. Sensitivity is per-player and
configurable in settings.

For match-completion cutscenes, a dedicated "PuzzleCamera" actor is
activated per-player, overriding the normal camera via priority or
SetViewTargetWithBlend.

--- Puzzle Camera Sequence ---
When a match completes:
  1. Disable player input
  2. Blend both cameras to a shared "puzzle cam" looking at the match
  3. Hold for PuzzleCamDuration (2 sec)
  4. Optionally play a Sequencer timeline
  5. Blend back to player cameras
  6. Re-enable input

=====================================
SECTION 5: MATCH PUZZLE SYSTEM (CORE MECHANIC)
=====================================

This is the heart of the game. Pay close attention to this section.

--- Concept ---
Each puzzle ("Match") consists of:
  - A LeftQuad:  a mesh (typically a flat quad/plane) placed in
                 Player 1's play area
  - A RightQuad: a corresponding mesh placed in Player 2's play area
  - Both quads have the same silhouette shape

The quads are placed such that when Player 1 looks at their quad
from the correct angle and distance, AND Player 2 does the same
with their quad, the two halves visually align at the screen's
vertical center line — forming a complete shape.

--- Data Model ---

UPuzzleBase (C++ : AActor)
  - bIsSolved (bool)

UMatchBase (C++ : UPuzzleBase)
  - MatchType (enum: Matchbox, Extinguisher, Thermostat, Mirror,
    Key, Lamp, Painting, Doorknob, Plunger, BubbleWand, Clock,
    Drain, Ring, ExitSign, None)
  - LeftQuad (AQuadBase*)
  - RightQuad (AQuadBase*)
  - XThresholdPercent (float, 0.75 — percentage of screen width)
  - YThresholdPercent (float, 1.2 — percentage of screen height)
  - FacingAngleThreshold (float, 20 degrees)
  - bHasExternalCondition (bool)
  - ExternalCondition (UPuzzleBase*)
  - Post-completion data:
    - CompleteItem (AActor* — the merged object to show)
    - HideObjects (TArray<AActor*>)
    - ShowObjects (TArray<AActor*>)
    - MatchText (FString)
    - EffectText (FString)
    - PlayableTimeline (ULevelSequence*)
    - TransformRoomType (enum, for room transitions)
    - bDeactivateAfterSolve (bool)

AQuadBase (C++ : AActor)
  - QuadType (enum: Left, Right)
  - MeshFilter (UStaticMeshComponent — the visible quad shape)
  - HitCollider (UBoxComponent — for raycast validation)
  - MatchRangeCollider (USphereComponent — trigger zone for proximity)
  - Vertices (TArray<FVector> — world-space mesh vertices, cached on Begin)
  - OwnerMatch (UMatchBase*)

On BeginPlay, AQuadBase caches the world-space positions of its
mesh vertices (minimum 4 for a quad).

When a player enters MatchRangeCollider:
  - Check if the entering player matches the expected side
    (Player 0 for LeftQuad, Player 1 for RightQuad)
  - If so, register this match as "nearby" with the MatchManager

When both players exit, unregister the match.

--- Match Detection Algorithm (CRITICAL) ---

This runs every frame in the MatchManager for each "nearby" match.
Reimplemented from the Unity C# exactly:

1. OBSTRUCTION CHECK (optional, currently disabled in Unity):
   Raycast from each quad toward its respective player.
   If something other than the quad's own colliders blocks the ray,
   the match is obstructed.

2. SCREEN-SPACE VERTEX PROJECTION:
   For the LeftQuad:
     Project all vertices through Player 1's camera (WorldToScreen).
     Find the two vertices with the LARGEST screen X values.
     Of those two, the one with larger Y is "topLeft", smaller is "bottomLeft".

   For the RightQuad:
     Project all vertices through Player 2's camera (WorldToScreen).
     Find the two vertices with the SMALLEST screen X values.
     Of those two, the one with larger Y is "topRight", smaller is "bottomRight".

   In other words: we find the inner edge of each quad
   (the edge closest to the screen center).

3. ALIGNMENT CHECK:
   screenMiddleX = ScreenWidth / 2

   topLeftDiffFromMiddle    = screenMiddleX - topLeftVertex.X
   bottomLeftDiffFromMiddle = screenMiddleX - bottomLeftVertex.X
   topRightDiffFromMiddle   = screenMiddleX - topRightVertex.X
   bottomRightDiffFromMiddle= screenMiddleX - bottomRightVertex.X

   topQuadDifference   = |topLeftVertex.Y - topRightVertex.Y|
   bottomQuadDifference= |bottomLeftVertex.Y - bottomRightVertex.Y|

   xThreshold = ScreenWidth * XThresholdPercent / 100
   yThreshold = ScreenHeight * YThresholdPercent / 100

4. FACING CHECK:
   For each quad, compute the angle between:
     (Player position - Quad position).Normalized
     and
     Quad's forward vector
   If this angle exceeds FacingAngleThreshold, that side is not aligned.

5. PER-SIDE ALIGNMENT:
   isLeftMatched =
     |topLeftDiffFromMiddle| <= xThreshold AND
     |bottomLeftDiffFromMiddle| <= xThreshold AND
     leftFacingAngle <= FacingAngleThreshold AND
     match is in NearbyList

   isRightMatched = (same logic for right side)

6. FULL MATCH:
   isPassing =
     isLeftMatched AND isRightMatched AND
     topQuadDifference <= yThreshold AND
     bottomQuadDifference <= yThreshold

   If bHasExternalCondition: isPassing = ExternalCondition->bIsSolved

7. CLOSENESS METRIC (for UI feedback):
   xError = average of all four |diffFromMiddle| values
   yError = average of top and bottom quad differences
   angleError = average of left and right facing angles

   xFactor = 1 - clamp(xError / (xThreshold * 3))
   yFactor = 1 - clamp(yError / (yThreshold * 3))
   angleFactor = 1 - clamp(angleError / (FacingAngleThreshold * 3))
   closeness = (xFactor + yFactor + angleFactor) / 3    [0..1]

--- Match Manager States ---
enum EMatchState:
  NoMatches, LeftAlignedHint, RightAlignedHint, BothAlignedHint,
  MatchHold, MatchComplete, MatchEffect, AllMatchesComplete

State logic per frame:
  - If any match isPassing -> MatchHold, start auto-submit timer
  - If auto-submit timer >= AutoSubmitDuration (0.5s) -> MatchComplete
  - If only left aligned -> LeftAlignedHint
  - If only right aligned -> RightAlignedHint
  - If both aligned but not passing Y -> BothAlignedHint
  - If none -> NoMatches

--- Match Completion Sequence ---
When MatchComplete fires:
  1. Disable all player input
  2. Show match text on screen
  3. Play "blink" animation (black bars close from top/bottom)
  4. Switch cameras to puzzle cam, wait for blend
  5. Wait PuzzleCamDuration
  6. Activate CompleteItem, show ShowObjects
  7. If PlayableTimeline exists, play it and wait for finish
  8. Else if CompleteItem exists, zoom completion camera toward it
  9. Hide HideObjects, trigger room transition events
  10. Play "blink open" animation
  11. Restore player cameras and input
  12. After delay, clear UI text and optionally trigger dialogue

=====================================
SECTION 6: SLOW MODE / HINT SYSTEM
=====================================

When a player holds the SlowMode button:
  - Time scale lerps toward 0.3 over fadeInTime (1 sec)
  - That player's camera desaturates (saturation -> -100)
  - Puzzle hint objects near the player become highlighted
  - Movement and roll speeds are reduced by slowMovementRatio (0.2)

When released:
  - Time scale returns to 1.0
  - Saturation returns to 0
  - Hints fade out

Each player's slow mode is independent. If only Player 1 holds it,
only Player 1's screen desaturates and shows hints.

--- Hint Rendering ---
Puzzle-related objects have a "hint" material or post-process effect.
When slow mode is active for a player:
  - For each hint object on that player's side:
    - Calculate distance from player to object
    - Intensity = 1 - (distance / maxDistance), clamped, eased (power 2)
    - Apply intensity to material parameter or post-process

In UE5, implement this with:
  - A custom post-process material for desaturation (per-player camera)
  - Material parameter collection for hint intensity
  - Or: simply use a MPC that each hint-tagged actor reads in its material

=====================================
SECTION 7: ROOM / ENVIRONMENT TRANSITIONS
=====================================

The game progresses through acts. Completing certain matches triggers
room transformations:

TransformRoom actor:
  - OldObjects (TArray<AActor*>) — actors to hide
  - NewObjects (TArray<AActor*>) — actors to show
  - NewPlayerTransforms (FTransform per player) — teleport destinations
  - Associated gravity direction changes

When triggered:
  1. Enable all NewObjects
  2. Disable all OldObjects
  3. Teleport players to new positions
  4. Switch gravity if needed

--- Doors ---
Doors have a list of prerequisite matches (doorKeyList).
When all prerequisites are solved, the door opens (rotate halves
via timeline or lerp, disable collision).

=====================================
SECTION 8: UI
=====================================

--- Match HUD (always visible during gameplay) ---
  - Left alignment border:  visible when isLeftMatched
  - Right alignment border: visible when isRightMatched
  - Complete border:        visible when all matches done
  - Match text:             shown during completion sequence
  - Submit fill bars:       fill based on auto-submit timer progress
  - Middle divider line:    the split-screen seam indicator

--- Blink Effect ---
Two black bars (top and bottom of full screen, covering both viewports).
On close: tween from offscreen to covering the center.
On open: tween back out.
Used for cinematic transitions during match completion.

--- Letterbox Bars ---
Cinematic aspect ratio bars (2.39:1 target).
Overlay canvas that covers both split-screen viewports.

--- Silhouette Hints ---
UI images on each player's HUD showing the silhouette of nearby puzzles.
Fades in based on distance when slow mode is held.

--- Menus ---
  - Start Menu: 1-Player / 2-Player / Quit
  - Pause Menu: Resume / Settings / Main Menu / Quit
  - Settings: Sensitivity sliders per player, volume controls

=====================================
SECTION 9: AUDIO
=====================================

Implement a simple audio manager (AActor singleton or GameInstanceSubsystem).

Sound categories: Music, SFX, Ambience, VO
Each sound entry has: SoundID (enum), SoundWave/Cue, bLooping,
DefaultVolume, Pitch, Category, Priority.

Key sounds:
  - FootstepConcrete (played on timer while moving, interval 0.5s)
  - JumpConcrete
  - MatchCorrectPosition
  - MatchSuccess
  - Failure
  - Ambience (looping)

Volume settings saved to SaveGame or GameUserSettings:
  - MasterVolume, MusicVolume, SFXVolume, VOVolume

If FMOD is desired, integrate the FMOD UE plugin. Otherwise, native
USoundCue/MetaSounds is sufficient.

=====================================
SECTION 10: SAVE / LOAD
=====================================

Implement IDataPersistence interface (or UE equivalent: SaveGame object).

Per-player saved data:
  - Position (FVector)
  - Rotation (FRotator)
  - GravityDirection (FVector)

Global saved data:
  - Which matches are solved
  - Current game state / act
  - Objective index

=====================================
SECTION 11: GAME STATE / PROGRESSION
=====================================

enum EGameState: Tutorial, Act1_Renovated, Act2_Gentrified, Act3_Escheristic

enum ECutsceneState: Gameplay, Cutscene

The GameManager tracks current act. Each act has a list of required
matches. When all matches for an act are solved, advance to the next act.

The CutsceneState gates input: during Cutscene, player input is disabled.

=====================================
SECTION 12: KNOWN BUGS IN UNITY VERSION (DO NOT RECREATE)
=====================================

These bugs exist in the Unity build. The UE5 version must avoid them:

1. Player 1's slow mode / hint buttons affect Player 2's state.
   FIX: Each player's input actions must be completely isolated.
   Never share an InputAction instance between players.

2. Player 2's slow mode and hint buttons are non-functional.
   FIX: Ensure Player 2's input mapping context is properly bound
   and the SlowManager reads from the correct player's actions.

3. Cross-contamination of camera effects between viewports.
   FIX: Use per-player post-process volumes with correct layer masking.
   Each camera should only be affected by its own volume.

4. Single-player Tab switching sometimes leaves the previous
   player's camera enabled.
   FIX: On switch, explicitly disable the previous player's camera
   input provider before enabling the new one.

=====================================
SECTION 13: IMPLEMENTATION ORDER
=====================================

Phase 1 — Foundation (Week 1):
  1. Project setup, module structure, build verification
  2. Enhanced Input: create all IAs and IMCs
  3. AUnrealtorCharacter with basic movement (no gravity switching yet)
  4. Split-screen viewport setup (two local players)
  5. Basic first-person camera per player

Phase 2 — Core Mechanic (Week 1-2):
  6. AQuadBase actor with mesh, trigger zone, vertex caching
  7. UMatchBase with all match data properties
  8. AMatchManager with proximity tracking and EvaluateProximity algorithm
  9. Screen-space projection and alignment detection
  10. Auto-submit timer and match completion trigger
  11. Basic Match HUD (alignment borders, text)

Phase 3 — Polish & Systems (Week 2-3):
  12. Match completion sequence (camera blend, blink, object swap)
  13. Player state machine (idle/walk/run/jump/fall)
  14. Gravity switching system
  15. Auto-step and fall prevention
  16. Slow mode with desaturation and hint rendering
  17. Camera roll/tilt

Phase 4 — Content & UI (Week 3-4):
  18. Room transition system (TransformRoom)
  19. Door system with prerequisites
  20. Full menu system (start, pause, settings)
  21. Audio manager and sound integration
  22. Save/load system
  23. Tutorial system
  24. Silhouette hint UI

=====================================
SECTION 14: FILE MANIFEST (EXPECTED)
=====================================

C++ Headers + Source:
  UnrealtorCharacter.h/.cpp
  UnrealtorPlayerController.h/.cpp
  UnrealtorPlayerState.h/.cpp         (FSM states)
  UnrealtorInputManager.h/.cpp
  QuadBase.h/.cpp
  MatchBase.h/.cpp
  MatchManager.h/.cpp
  PuzzleBase.h/.cpp
  DoorBase.h/.cpp
  TransformRoom.h/.cpp
  SlowModeComponent.h/.cpp
  AudioManagerSubsystem.h/.cpp
  UnrealtorGameManager.h/.cpp
  UnrealtorSaveGame.h/.cpp
  UnrealtorGameInstance.h/.cpp
  UnrealtorHUD.h/.cpp

Blueprints (derived from C++ base classes):
  BP_UnrealtorCharacter
  BP_QuadBase (and per-puzzle variants)
  BP_MatchBase (and per-puzzle variants)
  BP_DoorBase
  BP_TransformRoom
  WBP_MatchHUD
  WBP_MainMenu
  WBP_PauseMenu
  WBP_SettingsMenu

Maps:
  L_MainMenu
  L_Gameplay (main level with all acts)

=====================================
END OF SPECIFICATION
=====================================