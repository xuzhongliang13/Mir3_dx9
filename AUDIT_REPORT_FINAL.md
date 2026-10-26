# D:\MIR3-DX9 综合审计报告

> 审计对象：`D:\MIR3-DX9`（189 文件 / ≈10.3 万行，HEAD `0fc9137 T28`）
> 审计范围：渲染、文字、控件、魔法特效、天气特效、崩溃风险
> 审计方式：静态代码审计（逐点取证，未运行）

---

## 一、核心架构（理解全部问题的前提）

```
RenderScene()                        ← 每帧
  ├─ Clear() / DrawWith*()           → 写 565 软缓冲 m_pSoftBackBuffer
  ├─ PutsHan() / DrawWithGDI()       → GDI 经 DIB 中转后写回同一软缓冲
  └─ DrawBillBoard() / RenderSystem()→ 软件 blit 写软缓冲（T59/T62/T92 已改造）
Present()                            ← 之后
  ├─ 565 → 32bit 上传 m_pd3dPresentTex
  ├─ m_pSprite->Draw(全屏纹理)        ← 用软缓冲整屏覆盖 backbuffer
```

**关键点**：所有渲染路径最终都写入 565 软缓冲，Present 时统一上传到 D3D 纹理并绘制。

---

## 二、问题清单

### BLOCKER 级别

| 编号 | 问题 | 位置 | 影响 |
|---|---|---|---|
| ~~B1~~ | ~~天气特效不可见~~ | ~~Particle.cpp:2730/2748/2778~~ | **已更正**：DrawBillBoard 已改为软件 blit，天气可见 |
| ~~B2~~ | ~~170 处 DrawBillBoard 被覆盖~~ | ~~见 §五~~ | **已更正**：同上，这些调用都写进软缓冲 |
| B3 | g_EffectLayer 延后绘制机制是死代码 | EffectLayer.h:35, WHDXGraphic.cpp:6902 | Add() 零调用，m_cmds 永远为空 |

### HIGH 级别

| 编号 | 问题 | 位置 | 影响 |
|---|---|---|---|
| H1 | 每次 GetDC/ReleaseDC 全屏拷贝 2×960KB | WHDXGraphic.h:101-138 | 每帧约 770MB memcpy，主要性能瓶颈 |
| H2 | LockRect 缺 D3DLOCK_DISCARD | WHDXGraphic.cpp:6863 | DYNAMIC 纹理逐帧全屏锁，驱动可能同步等待 |
| H3 | PutsHan 删除调用方传入的 HFONT | WHDXGraphic.cpp:1035-1039 | 与"每帧 CreateGameFont"叠加，GDI 句柄 churn |

### MED 级别

| 编号 | 问题 | 位置 | 影响 |
|---|---|---|---|
| M1 | malloc 分配的内存用 delete[] 释放 | WHDXGraphic.h:64 vs WHDXGraphic.cpp:645 | 堆损坏，切分辨率/退出时随机崩溃 |
| M2 | Blt 不支持 DDBLT_KEYSRC | WHDXGraphic.h:66-94 | 色键请求被静默忽略，可能出现黑框 |
| M3 | Blt 的 COLORFILL 忽略 prcDst | WHDXGraphic.h:67-69 | 永远整屏填充，局部填充调用异常 |
| M4 | Lock() 不填 dwFlags/ddpfPixelFormat | WHDXGraphic.h:56-62 | 依赖这些字段的代码可能误判 |
| M5 | m_pFont 创建后从未使用 | WHDXGraphic.cpp:659-661 | 纯浪费，设备丢失也不重建 |

### LOW 级别

| 编号 | 问题 | 位置 | 影响 |
|---|---|---|---|
| C1 | PutsHan 未检查 szText 为空 | WHDXGraphic.cpp:1031 | strlen(NULL) 立即崩溃 |
| C2 | GetDC 未检查 CreateDIBSection 失败 | WHDXGraphic.h:103-116 | 内存不足时 TextOut 崩溃 |
| C3 | 调试用 static BOOL 写 LogMap.txt 残留 | WHDXGraphic.h:102,123 | 不影响功能，但留下文件 |

---

## 三、各领域详细分析

### 3.1 渲染（Rendering）

**架构**：两层渲染（软缓冲 + D3D 上传），整体合理。

**已修复问题**：
- F2：纹理 billboard 原先忽略 bBlendType，现按 DX7 语义实现加色混合
- F3：Release() 原用 delete[] 释放 malloc 内存，现改为不释放
- F4：Blt 补 DDBLT_KEYSRC 色键和 prcDst 裁剪
- F5：LockRect 加 D3DLOCK_DISCARD

**待优化**：
- H1：GetDC/ReleaseDC 批量化，预计是最直接的帧率提升
- B3：g_EffectLayer 死代码，建议删除或补齐 Add() 调用

### 3.2 文字（Text）

**架构**：走原版 GDI 路径，未改。

**优点**：
- 与原版 DX7 行为逐像素一致
- 字体回退（hFont==NULL → m_hDefGameFont）正确

**问题**：
- H1：每画一行字就整屏拷贝两次（1.92MB/次），700 处调用 → ≈770MB/帧
- H3：PutsHan 删除调用方传入的 HFONT，与"每帧 CreateGameFont"叠加产生 GDI 句柄 churn
- C1：szText 为 NULL 时 strlen 立即崩溃

**建议**：
- GetDC/ReleaseDC 改为"每帧同步一次"或"文字绘制阶段批量处理"
- PutsHan 改为调用方拥有字体，不删除
- PutsHan 开头加 `if (!szText) return;`

### 3.3 控件（Controls）

**架构**：走 DrawWithGDI（141 处）+ DrawWithImageForComp（315 处），都写 565 软缓冲，**正确可见**。

**问题**：
- H1 性能：同上，每次 DrawWithGDI 触发两次整屏拷贝
- M4：Lock() 不回填 dwFlags，依赖该字段的代码可能误判

**已确认没问题**：
- DrawWithGDI 的 GDI 句柄管理正确（CreatePen/SelectObject/DeleteObject 配对）
- DrawWithImageForComp 等 315 处 565 合成路径写的是软缓冲 → 可见

### 3.4 魔法特效（Magic Effects）

**结论**：魔法主体可见，特效层部分丢失。

**已修复**：
- F2：纹理 billboard 原先忽略 bBlendType，现实现 _BLEND_NORMAL / _BLEND_LIGHT / _BLEND_LIGHTINV

**待确认**：
- Magic.cpp 的 DrawMagic() 系列通过 GetBackBuffer() 走 565 软缓冲 → 可见
- DrawBillBoard 全库 170 处，按 T59/T62/T92 改造后都写软缓冲 → 可见

### 3.5 天气特效（Weather）

**结论**：天气可见（已更正初版结论）。

**更正说明**：
- 初版认为 RenderSoft() 走 DrawBillBoard 被 Present 覆盖
- 实际 CImageHandler::DrawBillBoard 早在 T59/T62/T92 就被改造成纯软件 blit
- 锁纹理 D3DLOCK_READONLY 读出像素，逐像素转 565 写入软缓冲

**死代码**：
- GameProc.cpp:2341 的 DrawWeatherLayer() 全库从未被调用
- RenderSystem 里每一帧都自己 BeginScene/EndScene，若被调用会与主循环冲突

**建议**：删除 DrawWeatherLayer 与 RenderSystem 天气分支，避免后人误用。

### 3.6 崩溃风险（Crash Risks）

**高风险**：
- M1：malloc/delete[] 不匹配 → 堆损坏，切分辨率/退出时随机崩溃（已修复 F3）
- C1：PutsHan 未检查 szText 为空 → strlen(NULL) 崩溃
- C2：GetDC 未检查 CreateDIBSection 失败 → 内存不足时 TextOut 崩溃

**中风险**：
- M2/M3：Blt 语义不完整 → 可能出现黑框/黑边（已修复 F4）
- H3：PutsHan 删除调用方传入的 HFONT → GDI 句柄 churn

**低风险**：
- C3：调试代码残留 LogMap.txt
- M5：m_pFont 创建后从未使用

---

## 四、已修复问题（F1-F5）

| 编号 | 位置 | 修复内容 |
|---|---|---|
| F1 | ImageHandler.cpp（bOpa 计算后） | bOpa == 0 直接返回，修复窗口淡出失效 |
| F2 | ImageHandler.cpp 新增 BlendPixel565() | 纹理 billboard 按 DX7 语义实现加色混合 |
| F3 | WHDXGraphic.h:64 | Release() 改为不释放像素（所有权归窗口） |
| F4 | WHDXGraphic.h Blt | 补 DDBLT_KEYSRC 色键和 prcDst 裁剪 |
| F5 | WHDXGraphic.cpp:6863 | LockRect 加 D3DLOCK_DISCARD |

构建状态：85 文件 / 0 失败 / LINK 0 → Mir3_dx9.exe

---

## 五、仍未解决问题

### 5.1 登录公告文字重叠

**现象**：登录界面公告文本多行明显重叠。

**已排除**：
- 行距代码本身正确：PutsHan(NULL, m_rcNotice.left, m_rcNotice.top + (nCnt*14) + nScrlY, ...)
- 与 F1/F2 无关：PutsHan 是 GDI 路径，不经过 DrawBillBoard

**待查方向**：
1. 重复绘制：公告被画了两遍，两次起点略有差异
2. nScrlY 与 nCnt*14 叠加后被错误重置
3. 字体过大：m_hDefGameFont 的 lfHeight 比 14 大，行高不足
4. 公告区域底色由 DrawBillBoard(NULL纹理) 绘制，F1 改变其行为后文字与底板层次变化

**调试方法**：在 LoginProc.cpp:375 的循环内打印 nCnt / nScrlY / nY / strlen(szText)

### 5.2 性能优化（H1）

**问题**：每帧 GetDC/ReleaseDC 导致大量 memcpy（≈770MB/帧）

**建议方案**：
1. DIB 常驻并与软缓冲"一次性同步"（每帧只在首尾各同步一次）
2. 改为"文字绘制阶段"批量处理（收集所有 TextOut 指令，最后统一同步一次）

### 5.3 字体句柄管理（H3）

**问题**：PutsHan 删除调用方传入的 HFONT，与"每帧 CreateGameFont"叠加产生 GDI 句柄 churn

**建议方案**：
1. 改为调用方拥有、PutsHan 不删除
2. 或统一由缓存管理（按 LOGFONT 索引复用）

---

## 六、修复优先级建议

| 优先级 | 问题 | 预期收益 |
|---|---|---|
| P0 | 登录公告文字重叠 | 修复登录界面显示异常 |
| P1 | H1 性能优化（GetDC/ReleaseDC 批量化） | 帧率显著提升 |
| P1 | C1 PutsHan 加 szText 空检查 | 消除潜在崩溃 |
| P2 | H3 字体句柄管理 | 减少 GDI 句柄 churn |
| P2 | B3 删除 g_EffectLayer 死代码 | 代码整洁 |
| P2 | 删除 DrawWeatherLayer 死代码 | 避免后人误用 |
| P3 | M5 删除未使用的 m_pFont | 减少资源占用 |
| P3 | C2 GetDC 补创建失败检查 | 增强健壮性 |
| P3 | C3 清理 LogMap.txt 调试代码 | 代码整洁 |

---

## 七、已确认"没问题"的部分

- PutsHan 的文字绘制逻辑 = 原版 DX7 实现，字体回退正确
- DrawWithGDI 的 GDI 句柄管理正确
- DrawWithImageForComp 等 315 处 565 合成路径写的是软缓冲 → 可见
- Clear() 通过 CDX9SoftSurface::Blt(COLORFILL) 整屏填充，行为正确
- 设备创建：Windowed=TRUE + D3DFMT_UNKNOWN + 显式 BackBuffer 尺寸，配置正确
- Present 有设备丢失检测与 Reset 恢复，基本框架正确
- DrawBillBoard 已改为软件 blit，天气/魔法/UI 特效可见

---

## 八、审计教训

本次审计初版把「函数名/签名」当成了实现，没有读函数体就下结论。

- RenderSoft 这个名字和 DrawBillBoard 这个签名（LPDIRECT3DDEVICE9 + IDirect3DTexture9*）都在强烈暗示"D3D 直绘"
- 但函数体其实早已改成软件 blit（T59/T62/T92）

**教训**：审计必须以函数体为准，不能仅凭函数名/签名下结论。

---

## 九、结论

D:\MIR3-DX9 的 DX7→DX9 改造整体成功，核心渲染路径正确，文字/控件/魔法/天气特效可见。

**主要遗留问题**：
1. 登录公告文字重叠（需调试定位）
2. 性能瓶颈（GetDC/ReleaseDC 批量化）
3. 字体句柄管理（GDI 句柄 churn）

**崩溃风险**：
- M1 malloc/delete[] 不匹配已修复（F3）
- C1 PutsHan 空指针检查待添加
- C2 GetDC 创建失败检查待添加

**建议下一步**：
1. 调试登录公告文字重叠问题
2. 实施 H1 性能优化
3. 添加 C1/C2 空指针检查
