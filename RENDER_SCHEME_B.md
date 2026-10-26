# 渲染架构主基调：方案B（两层渲染）

> 本文件是本客户端**所有特效相关改动**的统一基调。任何 AI（含后续接手者）修改特效显示，必须遵循本方案。

## 一、背景与问题
- 本客户端主渲染为**软渲染**：地图/角色/NPC/界面由 CPU 逐像素绘制到 565 软缓冲（`CDX9SoftSurface`）。
- `Present()`（`Mir2Ex.cpp:341`）把该软缓冲转成 32 位纹理、**全屏覆盖 backbuffer**，是软渲染输出的唯一通道（不能删除）。
- 原版遗留的 **D3D 特效直绘**（护盾、麻痹、剑/盔甲特效、武器发光、粒子、爆炸、闪电、`CMagic::DrawMagic`、`CHero::DrawEffect` 等）在 Present **之前**画到 backbuffer，会被 Present 覆盖而**不可见**。

## 二、方案B（本仓库唯一渲染基调，对齐 back 版）
采用**两层渲染**，与 `Mir3_patched_v3_backup.exe` 一致（软渲染主流程 + D3D 特效独立绘制、互不覆盖）：

1. **主渲染层（Present 前）**：软渲染照常，输出地图/角色/NPC/界面到软缓冲，随后 `Present()`。
2. **特效层（Present 后）**：所有 D3D 特效在 `Present()` **之后**绘制到 backbuffer，叠加在软渲染之上。

### 硬性规则
- 任何特效显示问题，一律走方案B：**保留/恢复 D3D 直绘 + 移到 Present 后绘制**。
- **禁止**再走"软件化到软缓冲"（方案A）。软件化会降性能、受 565 位深限制、无法等价 D3D 混合。

## 三、需要改造的代码（原方案A/软件化，需还原为 D3D + Present 后画）
- `GameProcess/Magic.cpp` `CMagic::DrawMagic`（约 754-930）：T99 软件加色混合 → 还原为 D3D 直绘 + Present 后画。
- `GameProcess/Actor.cpp` `CHero::DrawEffect`（约 4705-4778）：T97 软混合 → 还原为 D3D 直绘 + Present 后画。
- 火墙作用特效透明：按方案B 修复（保留 D3D 直绘 + Present 后画）。

## 四、待梳理（Present 前的 D3D 特效直绘点，需逐一分离到特效层）
- `CHero::ShowShield`（护盾，6568，3 分支）
- `CHero::ShowPahwangbo`（麻痹，6741）
- `CHero::DrawTheSwordEffect`（剑特效）
- `CHero::DrawTheAomorEffect`（盔甲特效）
- `CHero::DrawActor`（衣服特效/武器发光等）
- 粒子系统 `Particle.cpp`（散射/雪花/雨/飞行/爆炸/尾迹/烟雾/闪电/雾）
- 光照雾 `LightFog.cpp`
- 聊天背景 `ShowMessage`

## 五、实施方式（特效层）
- 主循环 `Mir2Ex.cpp:341` `Present()` 之后，增加"特效层渲染"步骤，遍历绘制所有 D3D 特效到 backbuffer。
- 特效绘制逻辑从主渲染中分离 / 延迟到 Present 后执行。
- 分阶段：先试点一个特效验证（武器发光或护盾），通过后推广到全部。

## 六、验证
- 特效在游戏内可见（不再被 Present 覆盖）。
- 层级可接受（特效叠加在场景之上，同 back 版）。
- 性能：D3D GPU 绘制，不吃 CPU。
