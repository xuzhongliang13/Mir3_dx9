# D:\MIR3-DX9 审计报告（DeepSeek 修改后版本）

> 审计对象：`D:\MIR3-DX9`（DeepSeek 修改后的未提交版本）
> 审计时间：2026-10-17
> 审计范围：渲染、文字、控件、魔法特效、天气特效、崩溃风险
> 审计方式：静态代码审计（逐点取证，未运行）

---

## 一、修改概览

DeepSeek 对 10 个文件进行了修改，共 502 行新增、203 行删除：

| 文件 | 修改类型 | 主要内容 |
|---|---|---|
| Common/ImageHandler.cpp | 新增+修改 | BlendPixel565 函数（3 种混合模式），bOpa==0 淡出修复 |
| WindHorn/WHDXGraphic.h | 修改 | Release() 内存释放修复，Blt() 色键和裁剪修复 |
| WindHorn/WHDXGraphic.cpp | 修改 | LockRect 使用 D3DLOCK_DISCARD |
| GameProcess/Particle.cpp | 大幅重写 | CSnow/CRain/CMist::RenderSoft() 对齐原版 DX7 |
| GameProcess/LightFog.cpp | 新增 | ShowLightFogSoft() 软渲染光照网格 |
| GameProcess/LightFog.h | 新增 | ShowLightFogSoft() 声明 |
| GameProcess/StatusWnd.cpp | 修改 | tooltip fallback 标签 |
| GameProcess/UtilWnd.cpp | 删除 | 移除天气控制按钮和状态 |
| GameProcess/UtilWnd.h | 删除 | 移除天气控制成员变量 |
| GameProcess/GameProc.cpp | 注释 | 韩文注释编码问题（非功能性修改） |

---

## 二、修复项评估

### F1 - 窗口淡出失效修复 ✅ 正确

**位置**：`ImageHandler.cpp:459-462`

**问题**：`bOpa==0` 时原判据 `a>=bOpa` 恒真，导致淡出过程始终以全不透明绘制。

**修复**：
```cpp
if ( bOpa == 0 ) return S_OK;
```

**评估**：正确。`bOpa==0` 表示完全透明，直接返回避免无效绘制。

---

### F2 - 加色混合修复 ✅ 正确

**位置**：`ImageHandler.cpp:384-428`（BlendPixel565 函数）

**问题**：原实现完全忽略 `bBlendType`，所有 billboard 都按普通 alpha 处理，导致 `_BLEND_LIGHT`（加色，魔法/爆炸光效）退化成普通半透明。

**修复**：新增 `BlendPixel565` 函数，实现 3 种混合模式：
- `_BLEND_NORMAL` (0): SRCALPHA / INVSRCALPHA
- `_BLEND_LIGHT` (1): ONE / ONE（加色）
- `_BLEND_LIGHTINV` (2): ONE / INVSRCCOLOR

**评估**：正确。混合公式符合 DX7 `SetBlendRenderState` 语义。

**潜在问题**：
- 只实现了 3 种混合模式，但代码中引用了 6 种（`_BLEND_NORMAL` / `_BLEND_LIGHT` / `_BLEND_LIGHTINV` / `_BLEND_INVNORMAL` / `_BLEND_INVLIGHT` / `_BLEND_INVLIGHTINV`）。其余 3 种走 `default` 分支，按 `_BLEND_NORMAL` 处理。
- 如果游戏实际使用了 `_BLEND_INVNORMAL` 等模式，效果会不正确。

**建议**：确认游戏是否使用了其余 3 种混合模式，如有则补全实现。

---

### F3 - 内存释放修复 ✅ 正确

**位置**：`WHDXGraphic.h:64`

**问题**：`m_pBits` 由 `CreateDXG` 用 `malloc` 分配，原实现用 `delete[]` 释放，属未定义行为（切分辨率/退出时随机堆损坏）。

**修复**：
```cpp
void Release(){ m_pBits=NULL; delete this; }
```

**评估**：正确。缓冲区所有权归 `CWHDXGraphicWindow`，`CDX9SoftSurface` 不释放像素，只销毁包装对象。

---

### F4 - Blt 色键和裁剪修复 ✅ 正确

**位置**：`WHDXGraphic.h:67-113`

**问题**：
1. `DDBLT_COLORFILL` 忽略 `prcDst`，永远整屏填充
2. `DDBLT_KEYSRC` 被忽略，源图黑底被原样拷贝，形成黑框/黑边

**修复**：
1. COLORFILL 按 `prcDst` 裁剪
2. KEYSRC 按 `m_dwColorKey` 跳过色键像素

**评估**：正确。裁剪逻辑完整，色键跳过实现正确。

---

### F5 - LockRect 优化 ✅ 正确

**位置**：`WHDXGraphic.cpp:6863`

**问题**：`D3DUSAGE_DYNAMIC` 纹理使用 `LockRect(..., 0)`，驱动可能等待 GPU 读完上一帧内容，造成同步停顿。

**修复**：
```cpp
m_pd3dPresentTex->LockRect(0, &lr, NULL, D3DLOCK_DISCARD)
```

**评估**：正确。`D3DLOCK_DISCARD` 是 DYNAMIC 纹理的正确用法，告诉驱动可以丢弃旧内容。

---

### W1/W2/W4/W7 - 天气特效修复 ✅ 正确

**位置**：`Particle.cpp:2729-2958`

**问题**：
- CSnow/CRain/CMist::RenderSoft() 坐标/尺寸/混合三项都不对
- 雨丝用 NULL 纹理画纯白矩形，没有纹理
- 雾是硬边矩形，没有纹理层次

**修复**：
- CSnow: 对齐原版坐标（屏幕空间）、尺寸（m_fSize）、混合（_BLEND_LIGHT 加色）
- CRain: 雨丝用纹理 500+frame、混合 _BLEND_LIGHTINV；水花用纹理 510+frame
- CMist: 双线性采样拉伸纹理 550 + _BLEND_LIGHTINV + 缓慢漂移

**评估**：正确。代码注释详细，逻辑清晰，对齐原版 DX7 语义。

**潜在问题**：
- CMist::RenderSoft() 的双线性采样是纯 CPU 实现，每帧对 800x600=480,000 像素做采样+混合，开销较大。
- 如果纹理 550 不存在或锁定失败，会回退到均匀叠加（纯色），效果会降级但不会崩溃。

**建议**：
- 监控帧率，如果雾天帧率明显下降，考虑优化双线性采样（如 SIMD 或降采样）
- 确认纹理 550 是否始终存在

---

### L1/L4 - 光照网格软渲染修复 ✅ 正确

**位置**：`LightFog.cpp:455-574`（ShowLightFogSoft 函数）

**问题**：原 D3D 版绘制在 Present 之前，会被软缓冲整屏覆盖，导致白天黑夜完全无效。

**修复**：新增 `ShowLightFogSoft()`，把 18x19 顶点光照网格直接乘到 565 软缓冲上。使用双线性插值保证网格之间平滑。

**评估**：正确。代码逻辑清晰，双线性插值实现正确。

**潜在问题**：
- 每帧对 800x600=480,000 像素做光照计算，开销较大。
- 全白（白天）时直接跳过，优化了常见场景。

**建议**：
- 监控帧率，如果光照计算导致帧率下降，考虑优化（如降采样或 SIMD）
- 确认 `ShowLightFogSoft()` 是否在所有调用点替换了 `ShowLightFog()`

---

### T1 - Tooltip fallback 标签 ✅ 正确

**位置**：`StatusWnd.cpp:2430-2440`

**问题**：CMList.Dat 中缺少某些 id 时，`GetMsg()` 返回空字符串，导致 tooltip 框不可见。

**修复**：添加 fallback 标签（GBK 编码的中文字符串）。

**评估**：正确。fallback 标签是 GBK 编码，与游戏字体匹配。

**潜在问题**：
- fallback 标签是硬编码的中文，如果游戏需要多语言支持，需要修改。
- 字符串是 GBK 编码，源码文件必须是 GBK 编码才能正确显示。

**建议**：确认源码文件编码是 GBK，否则 fallback 标签会显示乱码。

---

### 天气控制移除 ⚠️ 需确认

**位置**：`UtilWnd.cpp` / `UtilWnd.h`

**修改**：移除了天气控制按钮（`mUtil_BtnWeather`）、天气状态（`mUtil_nWeatherState`）、天气按钮点击区域（`mUtil_rcWeather`）及相关逻辑。

**评估**：
- 如果天气控制功能不需要（由服务器控制），移除是合理的。
- 如果用户需要手动切换天气，移除会导致功能丢失。

**建议**：确认天气控制是否由服务器控制，如果是则移除合理；如果需要客户端手动控制，则不应移除。

---

## 三、潜在问题

### 3.1 注释编码问题 ⚠️

**位置**：`GameProc.cpp` 多处

**问题**：韩文注释被破坏，出现乱码（如 `?`）。

**影响**：不影响功能，但影响代码可读性。

**建议**：修复注释编码，或回滚到原始注释。

---

### 3.2 混合模式不完整 ⚠️

**位置**：`ImageHandler.cpp:384-428`（BlendPixel565 函数）

**问题**：只实现了 3 种混合模式（_BLEND_NORMAL / _BLEND_LIGHT / _BLEND_LIGHTINV），其余 3 种（_BLEND_INVNORMAL / _BLEND_INVLIGHT / _BLEND_INVLIGHTINV）走 `default` 分支，按 `_BLEND_NORMAL` 处理。

**影响**：如果游戏使用了这 3 种混合模式，效果会不正确。

**建议**：确认游戏是否使用了这 3 种混合模式，如有则补全实现。

---

### 3.3 性能开销 ⚠️

**位置**：
- `Particle.cpp:2829-2958`（CMist::RenderSoft 双线性采样）
- `LightFog.cpp:455-574`（ShowLightFogSoft 光照计算）

**问题**：
- CMist::RenderSoft() 每帧对 480,000 像素做双线性采样+混合
- ShowLightFogSoft() 每帧对 480,000 像素做光照计算

**影响**：雾天和光照变化时帧率可能下降。

**建议**：
- 监控帧率，如果明显下降，考虑优化（SIMD、降采样、缓存等）
- 当前是纯 CPU 实现，如果帧率不够，可以考虑用 GPU 着色器

---

### 3.4 天气控制移除 ⚠️

**位置**：`UtilWnd.cpp` / `UtilWnd.h`

**问题**：移除了天气控制按钮和状态。

**影响**：用户无法手动切换天气。

**建议**：确认天气是否由服务器控制，如果是则移除合理；如果需要客户端手动控制，则不应移除。

---

### 3.5 调试代码残留 ⚠️

**位置**：`WHDXGraphic.h:102,123`

**问题**：`GetDC()` 中有调试代码，首次调用时会写 `LogMap.txt`。

**影响**：不影响功能，但会在客户端目录留下文件。

**建议**：发布前移除调试代码。

---

## 四、崩溃风险评估

### 4.1 已修复的崩溃风险 ✅

| 编号 | 问题 | 修复状态 |
|---|---|---|
| M1 | malloc/delete[] 不匹配 | ✅ 已修复（F3） |
| M2 | Blt 不支持 DDBLT_KEYSRC | ✅ 已修复（F4） |
| M3 | Blt 的 COLORFILL 忽略 prcDst | ✅ 已修复（F4） |
| C1 | PutsHan 未检查 szText 为空 | ❌ 未修复 |
| C2 | GetDC 未检查 CreateDIBSection 失败 | ❌ 未修复 |

### 4.2 未修复的崩溃风险 ❌

| 编号 | 问题 | 位置 | 影响 |
|---|---|---|---|
| C1 | PutsHan 未检查 szText 为空 | WHDXGraphic.cpp:1031 | strlen(NULL) 立即崩溃 |
| C2 | GetDC 未检查 CreateDIBSection 失败 | WHDXGraphic.h:103-116 | 内存不足时 TextOut 崩溃 |

**建议**：
- C1：在 `PutsHan` 开头加 `if (!szText) return;`
- C2：在 `GetDC` 中检查 `CreateDIBSection` 返回值，失败时返回错误码

---

## 五、编译和部署

### 5.1 编译状态 ✅

```
compiled 85 fail 0
LINK 0
SUCCESS: D:\MIR3-DX9\Mir3_dx9.exe
```

编译成功，无错误。

### 5.2 部署状态 ✅

```
D:\MIR3-DX9\Mir3_dx9.exe - 2.5M
D:\Client\Mir3_dx9.exe - 2.5M（已更新）
```

已部署到 `D:\Client\Mir3_dx9.exe`。

---

## 六、总结

### 6.1 修复质量评估

| 修复项 | 评估 | 说明 |
|---|---|---|
| F1 窗口淡出 | ✅ 正确 | 逻辑正确，无副作用 |
| F2 加色混合 | ✅ 正确 | 混合公式正确，但只实现 3 种模式 |
| F3 内存释放 | ✅ 正确 | 修复堆损坏风险 |
| F4 Blt 色键/裁剪 | ✅ 正确 | 裁剪逻辑完整，色键跳过正确 |
| F5 LockRect | ✅ 正确 | D3DLOCK_DISCARD 是正确用法 |
| W1/W2/W4/W7 天气 | ✅ 正确 | 对齐原版 DX7 语义，逻辑清晰 |
| L1/L4 光照 | ✅ 正确 | 双线性插值正确，优化了白天场景 |
| T1 Tooltip | ✅ 正确 | fallback 标签合理 |
| 天气控制移除 | ⚠️ 需确认 | 确认是否由服务器控制 |

### 6.2 整体评估

**优点**：
1. 修复了多个关键问题（内存释放、混合模式、天气特效、光照）
2. 代码注释详细，逻辑清晰
3. 对齐原版 DX7 语义，兼容性好
4. 编译成功，无错误

**缺点**：
1. 只实现了 3 种混合模式，其余 3 种走 default 分支
2. 天气控制和光照计算的性能开销较大
3. 注释编码问题（韩文注释被破坏）
4. 未修复 C1/C2 崩溃风险
5. 调试代码残留

### 6.3 建议

**必须修复**：
1. C1：PutsHan 加 szText 空检查
2. C2：GetDC 加 CreateDIBSection 失败检查

**建议修复**：
1. 补全混合模式实现（_BLEND_INVNORMAL / _BLEND_INVLIGHT / _BLEND_INVLIGHTINV）
2. 修复注释编码问题
3. 移除调试代码（LogMap.txt）
4. 确认天气控制是否由服务器控制

**性能优化**：
1. 监控雾天和光照变化时的帧率
2. 如果帧率下降明显，考虑优化双线性采样和光照计算（SIMD、降采样等）

---

## 七、结论

DeepSeek 的修改整体质量较高，修复了多个关键问题（内存释放、混合模式、天气特效、光照），代码注释详细，逻辑清晰，对齐原版 DX7 语义。

**主要遗留问题**：
1. C1/C2 崩溃风险未修复
2. 混合模式只实现 3 种，其余 3 种走 default 分支
3. 天气控制和光照计算的性能开销较大
4. 注释编码问题（韩文注释被破坏）

**建议下一步**：
1. 修复 C1/C2 崩溃风险
2. 补全混合模式实现
3. 监控帧率，必要时优化性能
4. 修复注释编码问题
5. 移除调试代码

**部署状态**：已编译成功并部署到 `D:\Client\Mir3_dx9.exe`，可以运行测试。
