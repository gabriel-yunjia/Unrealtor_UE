# Unrealtor_Demo 技术设计文档

## 1. 文档目的

本文档用于描述 `Unrealtor_Demo` 当前仓库的技术实现状态，面向项目协作、答辩说明与后续扩展开发。  
本文件仅陈述“当前代码已实现内容”和“与规格文档的差异”

## 2. 范围与基线

### 2.1 实现基线

- 代码基线目录：`Source/Unrealtor_Demo`
- 运行配置基线：`Config/DefaultEngine.ini`、`Config/DefaultInput.ini`
- 规格对照基线：`Documents/Unrealtor.md`

### 2.2 当前目标覆盖

当前项目已覆盖“本地分屏 + 核心匹配判定 + HUD 状态反馈”的主链路。  
慢速模式、提示系统、过场、房间变换、门、存档、音频、完整菜单等仍属于未实现范围。

## 3. 系统架构与职责

## 3.1 `AUnrealtorGameMode`

- 文件：`Source/Unrealtor_Demo/UnrealtorGameMode.h`、`Source/Unrealtor_Demo/UnrealtorGameMode.cpp`
- 职责：
  - 创建第二本地玩家（`CreatePlayer`）。
  - 缓存双玩家 `Controller` 与 `Character` 引用。
  - 维护 `ActivePlayerIndex` 并提供切换接口。
  - 管理输入映射上下文（当前仅对 P1 添加 `IMC_Player`）。

## 3.2 `AUnrealtorPlayerController`

- 文件：`Source/Unrealtor_Demo/UnrealtorPlayerController.h`、`Source/Unrealtor_Demo/UnrealtorPlayerController.cpp`
- 职责：
  - 绑定 `IA_Move`、`IA_Look`、`IA_SwitchPlayer`。
  - 按 `GameMode.ActivePlayerIndex` 路由移动与视角输入。
  - 维护每玩家 HUD 状态（显示开关、自动提交进度、接近度）。
  - 在 `LocalPlayer` 有效后延迟创建 HUD Widget，规避分屏初始化时序问题。

## 3.3 `AUnrealtorCharacter`

- 文件：`Source/Unrealtor_Demo/UnrealtorCharacter.h`、`Source/Unrealtor_Demo/UnrealtorCharacter.cpp`
- 职责：
  - 第一人称相机与角色网格初始化。
  - 移动速度与视角灵敏度参数承载。
  - 使用 UE 标准 `CharacterMovementComponent`（无自定义重力状态机）。

## 3.4 `AMatchActor`

- 文件：`Source/Unrealtor_Demo/MatchActor.h`、`Source/Unrealtor_Demo/MatchActor.cpp`
- 职责：
  - 作为核心匹配逻辑所有者，持有 `LeftQuad` / `RightQuad`。
  - 执行屏幕空间顶点投影与阈值判定。
  - 维护对齐状态、接近度、自动提交计时和解谜完成状态。
  - 向两位玩家分别推送 UI 状态。

## 3.5 `AQuadActor` 与 `UAlignmentPointComponent`

- 文件：
  - `Source/Unrealtor_Demo/QuadActor.h`、`Source/Unrealtor_Demo/QuadActor.cpp`
  - `Source/Unrealtor_Demo/AlignmentPointComponent.h`、`Source/Unrealtor_Demo/AlignmentPointComponent.cpp`
- 职责：
  - `AQuadActor` 表示单侧谜题几何体（左/右侧）。
  - 基于 `UAlignmentPointComponent` 自动采集并排序对齐顶点（`PointIndex`）。
  - 无点位时回退使用网格 AABB 顶点。
  - 根据侧别与玩家索引进行触发合法性校验。

## 3.6 类型定义

- 文件：`Source/Unrealtor_Demo/UnrealtorTypes.h`
- 内容：`EQuadSide { Left, Right }`

## 4. 核心机制数据流（当前实现）

1. `GameMode` 在 `BeginPlay` 创建第二本地玩家，缓存双玩家引用。  
2. `PlayerController` 绑定输入动作，输入按 `ActivePlayerIndex` 路由到目标角色。  
3. 当玩家进入匹配区域，`MatchActor` 基于距离阈值确认“双方接近”状态。  
4. `MatchActor` 将左右 Quad 顶点分别投影到 P1/P2 视口，提取内侧边。  
5. 计算并判定：
   - X 方向拼缝误差；
   - Y 方向上下边差值；
   - 左右观察朝向角误差。  
6. 生成状态：
   - `bLeftAligned`、`bRightAligned`；
   - `bFullMatch`；
   - `Closeness`（0..1）；
   - `AutoSubmitNormalized`（0..1）。  
7. HUD 更新策略：
   - 单侧对齐时仅显示对应侧反馈；
   - 双侧同时对齐但未达到完整匹配时，按当前规则仅展示一侧提示；
   - 满足完整匹配并持续达到自动提交阈值后设为 `bIsSolved=true`。

## 5. 匹配判定算法说明

算法实现位置：`AMatchActor::EvaluateAlignment`

### 5.1 投影与内边提取

- 左侧：按 X 降序取前两点，形成左侧内边上下点。
- 右侧：按 X 升序取前两点，形成右侧内边上下点。

### 5.2 阈值判定

- X 阈值：`ViewW1 * XThresholdPercent / 100`
- Y 阈值：`ViewH1 * YThresholdPercent / 100`
- 朝向阈值：`FacingAngleThreshold`

### 5.3 完整匹配条件

- 左右侧均通过 X + 朝向判定；
- 顶边与底边的跨侧 Y 差值均在阈值内。

### 5.4 接近度计算

基于 X 误差、Y 误差、角度误差分别归一化后平均，得到 `Closeness`。  
该值用于 UI 渐进反馈，不直接替代最终通过条件。

### 5.5 自动提交

- 完整匹配成立时累加 `AutoSubmitTimer`；
- 不成立时按 `AutoSubmitDecayRate` 衰减；
- 达到 `AutoSubmitDuration` 后判定解谜成功。

## 6. UI/HUD 机制

### 6.1 控制器级 HUD 所有权

每个 `PlayerController` 持有自己的 Widget 引用与状态变量，避免共享 HUD 状态造成串扰。

### 6.2 数据桥接接口

- `SetAlignmentHUDState(bool, float, float)`
- `ResetAlignmentHUDState()`
- `OnAlignmentHUDStateUpdated(...)`（BlueprintImplementableEvent）

### 6.3 初始化时序控制

`TryCreateAlignmentHUDWidget()` 在 `LocalPlayer` 未就绪时按 Tick 延迟重试，避免分屏第二玩家早期阶段创建失败。

## 7. 输入与分屏现状

## 7.1 分屏配置

- 来源：`Config/DefaultEngine.ini`
- 当前值：`bUseSplitscreen=True`、`TwoPlayerSplitscreenLayout=Vertical`

## 7.2 输入模式现状

- 已实现：移动、视角、Tab 切换激活角色。
- 当前模型：输入映射主要挂在 P1，利用 `ActivePlayerIndex` 路由控制目标。
- 未实现：规格中的完整多输入模式矩阵（双键盘并行、混合输入完整路径、每玩家独立 IMC 体系）。

## 8. 规格对照与差异清单（`Documents/Unrealtor.md`）

## 8.1 已落地能力

- 本地双玩家创建与分屏运行
- 核心匹配算法（投影、阈值、朝向、接近度、自动提交）
- 侧别归属校验（Left/Right）
- 对齐点组件化工作流
- 每玩家 HUD 状态桥接

## 8.2 未落地能力

- Slow Mode / Hint 完整系统
- 独立输入管理子系统（`UUnrealtorInputManager`）
- 完整 `MatchManager` 状态机与关卡级流程编排
- 谜题完成过场、PuzzleCam、Timeline 驱动演出
- 房间变换、门系统、菜单、音频管理、存档
- 玩家状态机、重力切换、相机 Roll/Tilt 等高级移动能力

## 9. 当前技术约束与风险

1. 输入并行能力尚未达到规格终态；当前偏单激活路由模型。  
2. 分屏布局配置与规格文档中的左右描述存在口径差异，需在验收阶段统一。  
3. `MatchActor` 当前包含较多开发期调试输出，发布前应清理或改为可控日志级别。  
4. 近距离判定当前以距离驱动为主，触发器事件被降级为辅助路径。

## 10. 验证清单（PIE）

1. 启动 PIE，确认双本地玩家正确创建。  
2. 确认双方可进入匹配区域并触发匹配计算。  
3. 在非完整匹配状态观察单侧 HUD 反馈是否正确。  
4. 调整视角达到完整匹配，确认自动提交计时推进。  
5. 保持匹配至阈值，确认 `bIsSolved` 触发。  
6. 重复测试边界场景（快速进出范围、仅单侧对齐、接近后快速偏离）。

## 11. 扩展建议（按优先级）

1. 输入系统重构：从单激活路由过渡到每玩家独立输入上下文。  
2. 慢速模式与提示系统：按玩家独立后处理与材质参数通道实现。  
3. 匹配流程管理：抽离 `MatchManager`，支持多谜题统一调度与状态展示。  
4. 演出与内容层：补齐完成过场、房间变换、门联动与菜单系统。  
5. 发布前工程化：日志清理、调试开关、文档与配置口径统一。

