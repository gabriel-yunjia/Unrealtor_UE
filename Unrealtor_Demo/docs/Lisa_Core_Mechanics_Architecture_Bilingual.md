# Unrealtor UE Core Mechanics Architecture (Bilingual)

## Document Intent / 文档目的

This document is the technical architecture map for Lisa's tutorial and PPT preparation. It explains what is already implemented in the Unreal project, how the core mechanic flows through systems, and which systems are still spec-only.

这份文档是给 Lisa 的技术架构地图，用于教程和 PPT 准备。它说明了 Unreal 项目中已经实现的系统、核心机制在系统中的流动方式，以及哪些系统目前仍停留在规范层（未实装）。

## System Ownership Map / 系统职责划分

### 1) `AUnrealtorGameMode`
- EN: Spawns second local player, caches both controllers/characters, tracks `ActivePlayerIndex`, and handles Tab-based control switching.
- 中文: 负责生成第二本地玩家、缓存两位玩家的 Controller/Character、维护 `ActivePlayerIndex`，并处理 Tab 切换控制目标。
- Files: `Source/Unrealtor_Demo/UnrealtorGameMode.h`, `Source/Unrealtor_Demo/UnrealtorGameMode.cpp`

### 2) `AUnrealtorPlayerController`
- EN: Binds Enhanced Input actions (`IA_Move`, `IA_Look`, `IA_SwitchPlayer`), routes input to active player, owns per-player alignment HUD state.
- 中文: 绑定 Enhanced Input 动作（`IA_Move`、`IA_Look`、`IA_SwitchPlayer`），将输入路由到当前激活角色，并维护每位玩家独立的对齐 HUD 状态。
- Files: `Source/Unrealtor_Demo/UnrealtorPlayerController.h`, `Source/Unrealtor_Demo/UnrealtorPlayerController.cpp`

### 3) `AUnrealtorCharacter`
- EN: First-person pawn with camera and body mesh setup, movement speed and look sensitivity control.
- 中文: 第一人称角色主体，包含相机和身体网格设置，控制移动速度与视角灵敏度。
- Files: `Source/Unrealtor_Demo/UnrealtorCharacter.h`, `Source/Unrealtor_Demo/UnrealtorCharacter.cpp`

### 4) `AMatchActor` (Core mechanic owner)
- EN: Owns left/right quads, evaluates screen-space seam alignment, computes closeness, runs auto-submit solve timer, pushes side-specific HUD visibility.
- 中文: 核心机制所有者。持有左右 Quad，执行屏幕拼缝对齐判定、计算接近度、驱动自动提交计时，并推送左右侧独立 HUD 可见性。
- Files: `Source/Unrealtor_Demo/MatchActor.h`, `Source/Unrealtor_Demo/MatchActor.cpp`

### 5) `AQuadActor`
- EN: Represents one puzzle half (left or right), caches alignment vertices, validates correct-player overlap by side, reports nearby state to `AMatchActor`.
- 中文: 表示谜题的一半（左或右），缓存对齐顶点，根据左右侧验证正确玩家触发，并向 `AMatchActor` 报告邻近状态。
- Files: `Source/Unrealtor_Demo/QuadActor.h`, `Source/Unrealtor_Demo/QuadActor.cpp`

### 6) `UAlignmentPointComponent`
- EN: Designer-authored alignment markers used as deterministic vertex references for matching.
- 中文: 由设计师放置的对齐点组件，作为匹配算法的确定性顶点参考。
- Files: `Source/Unrealtor_Demo/AlignmentPointComponent.h`, `Source/Unrealtor_Demo/AlignmentPointComponent.cpp`

## Core Mechanic Data Flow / 核心机制数据流

1. EN: Players enter left/right puzzle vicinity -> `AMatchActor` sees both sides nearby.  
   中文: 两位玩家进入左右谜题范围 -> `AMatchActor` 识别双方已接近。
2. EN: `AMatchActor` projects cached quad vertices to each player's viewport.  
   中文: `AMatchActor` 将 Quad 缓存顶点投影到各自玩家视口。
3. EN: Inner seam edges are selected (left: largest X, right: smallest X).  
   中文: 选取内侧拼缝边（左侧取最大 X，右侧取最小 X）。
4. EN: X seam error, Y edge error, and facing-angle error are evaluated against thresholds.  
   中文: 使用阈值判断 X 拼缝误差、Y 边缘误差、朝向角误差。
5. EN: Per-side alignment (`bLeftAligned`, `bRightAligned`) and full alignment (`bFullMatch`) are derived.  
   中文: 得到单侧对齐状态（`bLeftAligned`、`bRightAligned`）与完整对齐状态（`bFullMatch`）。
6. EN: Closeness and auto-submit normalized progress are sent to each player's HUD separately.  
   中文: 将接近度与自动提交进度分别发送给两位玩家各自 HUD。
7. EN: If full match holds long enough, puzzle marks solved.  
   中文: 若完整对齐持续达到设定时间，则谜题判定为已解。

## Input and Split-Screen Reality / 输入与分屏的当前实现现实

- EN: Split-screen is enabled via engine config (`TwoPlayerSplitscreenLayout=Vertical`), giving left/right viewport split.
- 中文: 通过引擎配置开启分屏（`TwoPlayerSplitscreenLayout=Vertical`），形成左右分屏。
- EN: Current code keeps mapping context on Player 1 and routes control by `ActivePlayerIndex` (Tab switch model).
- 中文: 当前代码将输入映射主要绑定在 Player 1，再由 `ActivePlayerIndex` 路由控制（Tab 切换模型）。
- EN: This is stable for teaching core mechanic but not yet final for fully independent simultaneous two-player control.
- 中文: 该方案足够稳定地讲解核心机制，但尚未达到“两个玩家完全独立同时控制”的最终形态。

## Implemented vs Spec-Only / 已实现与规范未实现对照

### Implemented in repo / 当前仓库已实现
- EN: Local split-screen spawn and player reference caching.
- 中文: 本地双人分屏生成与玩家引用缓存。
- EN: Basic move/look/switch input path.
- 中文: 基础移动/视角/切换输入链路。
- EN: Left/right side ownership isolation for puzzle overlap.
- 中文: 谜题左右侧归属隔离（玩家与侧别绑定）。
- EN: Screen-space seam alignment evaluation and auto-submit solve logic.
- 中文: 基于屏幕拼缝的对齐判定与自动提交解谜逻辑。
- EN: Per-player HUD state push with deferred LocalPlayer-safe widget creation.
- 中文: 每位玩家独立 HUD 状态推送，并含 LocalPlayer 安全延迟创建机制。
- EN: Alignment point component authoring workflow.
- 中文: 对齐点组件化标注工作流。

### In spec but not implemented yet / 规范中定义但尚未实装
- EN: Slow mode + hint rendering pipeline (per-player independent).
- 中文: 慢速模式与提示渲染链路（按玩家独立）。
- EN: Full input-mode matrix (TwoPlayer_KB, Mixed, full per-player IMCs).
- 中文: 完整输入模式矩阵（双键盘、混合输入、每玩家独立 IMC）。
- EN: Puzzle cinematic sequence, room transformation, door prerequisites, save/load, full menu/audio stack.
- 中文: 谜题过场、房间变换、门前置条件、存档、完整菜单与音频系统。

## Why C++ Backbone + Blueprint Frontend / 为什么采用 C++ 骨架 + 蓝图前端

- EN: For this project, the seam-math and ownership rules are correctness-critical. C++ gives deterministic behavior, strict type contracts, and safer refactors for cross-system logic.
- 中文: 对该项目而言，拼缝数学与归属隔离是正确性关键路径。C++ 提供确定性行为、严格类型契约，以及跨系统重构时更高安全性。
- EN: Blueprint remains ideal for HUD visuals, animation timing, content iteration, and designer-facing knobs.
- 中文: 蓝图依然最适合做 HUD 表现、动画时序、内容迭代和设计师可调参数。
- EN: Practical rule used here: put puzzle math/state ownership in C++, expose clean Blueprint hooks for presentation.
- 中文: 当前实践准则：核心数学与状态归属放 C++，展示层通过 Blueprint Hook 对接。

## Known Risk Areas for Tutorial Emphasis / 教学时需要重点强调的风险区

- EN: Input routing currently favors single active control path; do not claim simultaneous full two-player parity yet.
- 中文: 输入路由当前更偏向“单激活控制路径”，不要在汇报中宣称已完整支持双人同时等价控制。
- EN: Debug-heavy `MatchActor` output is useful now, but should be cleaned before final shipping build.
- 中文: `MatchActor` 目前调试输出较重，适合开发期，不适合最终发布版本。
- EN: Slow mode/hint and some systemic polish remain roadmap items, not regressions.
- 中文: 慢速模式/提示等仍属后续路线项，不是回归错误。

## Recommended Tutorial Sequence / 建议教学顺序

1. EN: Start from split-screen and ownership model.  
   中文: 先讲分屏与系统归属。
2. EN: Explain one complete puzzle cycle in `AMatchActor`.  
   中文: 再讲 `AMatchActor` 的一次完整解谜循环。
3. EN: Show HUD data bridge and why deferred widget creation matters.  
   中文: 演示 HUD 数据桥接，以及为何要延迟创建 Widget。
4. EN: Explain why alignment points were componentized.  
   中文: 说明为什么对齐点改成组件化。
5. EN: End with UE-vs-Unity decisions and current roadmap gaps.  
   中文: 最后讲 UE 相对 Unity 的决策差异与当前未完成路线。
