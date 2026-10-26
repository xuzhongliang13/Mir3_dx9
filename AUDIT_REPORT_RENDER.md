# D:\MIR3-DX9 渲染与崩溃风险审计报告

> 审计对象：`D:\MIR3-DX9`（189 文件 / ≈10.3 万行，HEAD `0fc9137 T28`）
> 审计范围：渲染、文字、控件、魔法特效、天气特效、崩溃风险
> 方式：静态代码审计（逐点取证，未运行）；每条结论均附 文件:行号

---

## 零、架构与数据流（理解全部问题的前提）

```
RenderScene()                        ← 每帧
  ├─ Clear() / DrawWith*()           → 写 565 软缓冲 m_pSoftBackBuffer
  ├─ PutsHan() / DrawWithGDI()       → GDI 经 DIB 中转后写回同一软缓冲
  └─ DrawBillBoard() / RenderSystem()→ 直接画到 D3D backbuffer  ★问题所在
Present()                            ← 之后
  ├─ 565 → 32bit 上传 m_pd3dPresentTex
  ├─ m_pSprite->Draw(全屏纹理)        ← 用软缓冲整屏覆盖 backbuffer
  └─ 所以：凡是在 Present 之前用 D3D 直接画的东西，全部被抹掉
```

`WHDXGraphic.cpp:611-666`（CreateDXG）、`6819-6909`（Present）。

这个"两层渲染"结构本身是合理的（原版就是软渲染 + D3D 特效），
但**代码里真正走软缓冲的只有 `DrawWith*` / `PutsHan` / `DrawWithGDI` 三类**，
所有 D3D 直绘路径都被覆盖。

---

## 一、结论摘要

| 编号 | 严重度 | 问题 | 位置 |
|---|---|---|---|
| B1 | **BLOCKER** | 天气特效完全不可见：`RenderSoft()` 实际是 D3D 直绘 | `Particle.cpp:2730/2748/2778` |
| B2 | **BLOCKER** | 170 处 `DrawBillBoard` 直绘被 Present 覆盖（UI 图标/特效丢失） | 见 §五 |
| B3 | **BLOCKER** | `g_EffectLayer` 延后绘制机制是死代码，`Add()` 零调用 | `EffectLayer.h:35`、`WHDXGraphic.cpp:6902` |
| H1 | HIGH | 每次 `GetDC/ReleaseDC` 全屏拷贝 2×960KB → 每帧约 1.6GB memcpy | `WHDXGraphic.h:101-138` |
| H2 | HIGH | `LockRect` 缺 `D3DLOCK_DISCARD`，DYNAMIC 纹理逐帧全屏锁 | `WHDXGraphic.cpp:6863` |
| H3 | HIGH | `PutsHan` 删除调用方传入的 HFONT，与"每帧 CreateGameFont"叠加 → GDI 句柄风险 | `WHDXGraphic.cpp:1035-1039` |
| M1 | MED | `malloc` 分配的内存用 `delete[]` 释放 → 堆损坏 | `WHDXGraphic.h:64` vs `WHDXGraphic.cpp:645` |
| M2 | MED | `Blt` 不支持 `DDBLT_KEYSRC`，色键请求被静默忽略 | `WHDXGraphic.h:66-94` |
| M3 | MED | `Blt` 的 COLORFILL 忽略 `prcDst`，永远整屏填充 | `WHDXGraphic.h:67-69` |
| M4 | MED | `Lock()` 不填 `dwFlags`/`ddpfPixelFormat` | `WHDXGraphic.h:56-62` |
| M5 | MED | `m_pFont`(D3DXFont) 创建后从未使用，设备丢失也不重建 | `WHDXGraphic.cpp:659-661` |
| C1 | MED | `PutsHan` 未检查 `szText` 为空 | `WHDXGraphic.cpp:1031` |
| C2 | MED | `GetDC` 未检查 `CreateDIBSection`/`CreateCompatibleDC` 失败 | `WHDXGraphic.h:103-116` |
| C3 | LOW | 调试用 `static BOOL` 写 `LogMap.txt` 残留 | `WHDXGraphic.h:102,123` |

---

## 二、渲染（Rendering）

### B2 —— 二次覆盖模型没落地（影响最广）

`Present()` 在 `WHDXGraphic.cpp:6888-6899` 用 `m_pSprite->Draw(m_pd3dPresentTex, 全屏, ALPHABLEND)`
把 565 软缓冲覆盖整个 backbuffer。而 `RenderScene` 期间发生的所有 D3D 直绘都在此之前，
因此被无条件抹掉。

`g_EffectLayer`（`GameProcess/EffectLayer.h`）本来是为解决这个问题设计的"延后到 Present 之后绘制"，
但：

```cpp
// EffectLayer.h:35
void Add(EF_DRAWN fn, void* data){ if(fn) m_cmds.push_back(...); }
```

**全库检索：`g_EffectLayer.Add(` 出现 0 次**；只有 `WHDXGraphic.cpp:6902` 调了 `Render()`。
所以 `m_cmds` 永远为空，`Render()` 是空转。

即便有人开始用 `Add()`，`Render()` 的调用点在 `EndScene()`（`:6900`）**之后**，
属于"D3D 场景外绘制"——DX9 调试运行时会报错，Release 下行为未定义。

**结论**：这不是"某个特效没画"，而是**整个二次覆盖架构只完成了一半**。

### H2 —— 每帧全屏上传的开销

```cpp
// WHDXGraphic.cpp:6863
if ( FAILED(m_pd3dPresentTex->LockRect(0, &lr, NULL, 0)) ) return E_FAIL;
```

- `D3DUSAGE_DYNAMIC` 纹理却传 `0` 而非 `D3DLOCK_DISCARD` → 驱动可能等待 GPU 读完、
  或退化为同步锁，帧率受严重影响。
- 6867-6879 每帧对 480,000 像素做 565→8888 展开（纯 CPU 标量循环，无 SIMD）。
  800×600 下约 192 万次位运算 + 48 万次 4 字节写。
- 可优化方向：改用 `D3DFMT_R5G6B5` 纹理直接上传 565（免转换），或 `D3DLOCK_DISCARD` + SSE2。

### M5 —— 一个纯浪费的资源

```cpp
// WHDXGraphic.cpp:659-661
if ( FAILED(D3DXCreateFontA(m_pd3dDevice, 14, 0, FW_NORMAL, 1, ..., "System", &m_pFont)) )
    m_pFont = NULL;
```

`m_pFont` 创建后**全库没有任何 `DrawText` 调用**（文字走 GDI `PutsHan`）。
设备丢失重建路径（`:6846-6855`）也只重建纹理和 Sprite，不重建字体。
建议直接删除，减少一处设备丢失隐患。

### 其他渲染细节

- `CreateDXG` 兜底用了 `D3DDEVTYPE_SW`（`:635`，纯软件光栅器）。一旦走到这条路径帧率会极低，
  且它抢在 `D3DDEVTYPE_HAL + SOFTWARE_VERTEXPROCESSING` 之后、`HAL + HARDWARE` 之前都没试过——
  顶点处理始终是软件，对 2D 精灵影响不大，但 3D 特效（粒子）会明显偏慢。
- `d3dpp` 未设置 `PresentationInterval`（`ZeroMemory` 后为 0 = `D3DPRESENT_INTERVAL_DEFAULT`，即等垂直同步），
  属合理默认，无需改。

---

## 三、文字（Text）

文字走的是**原版 GDI 路径，未改**：

```cpp
// WHDXGraphic.cpp:993-1042
if ( NULL == hFont ) hFont = m_hDefGameFont;      // 正确回退默认字体
HRESULT hr = pSurface->GetDC(&hDC);               // 取 DIB 的 DC
SetBkMode/SetTextColor; TextOut(hDC, nX, nY, ...);
pSurface->ReleaseDC(hDC);                          // 回写软缓冲
```

**优点**：与原版 DX7 行为逐像素一致，字体、字号、位置都是原版语义。

### H1 —— 性能：每画一行字就整屏拷贝两次

```cpp
// WHDXGraphic.h:101-138
GetDC:     for(y) memcpy(DIB + y*w*2, m_pBits + y*pitch, w*2);   // 960KB
ReleaseDC: for(y) memcpy(m_pBits + y*pitch, DIB + y*w*2, w*2);   // 960KB
```

`PutsHan` 全库 700 处调用、`DrawWithGDI` 141 处，**每帧每次都是 1.92MB 的 memcpy**。
保守估计每帧 400+ 次 → **≈770MB/帧**。这是当前客户端最主要的速度瓶颈，
也解释了为什么需要把粒子数从 250/200 降下来（`T28: reduce snow250/rain200 particles`）。

**建议**：DIB 常驻并与软缓冲"一次性同步"（每帧只在首尾各同步一次），
或改为"文字绘制阶段"批量处理（收集所有 TextOut 指令，最后统一同步一次）。

### H3 —— 字体句柄所有权混乱

```cpp
// WHDXGraphic.cpp:1035-1039
if ( hFont != m_hDefGameFont ) { DeleteObject(hFont); hFont = NULL; }
```

`PutsHan` 会**删除调用方传入的 HFONT**。这是原版行为，但它意味着：

- 调用方每帧 `CreateGameFont(...)` 并把结果传进来（`ChrProc.cpp` 等多处如此），
  每次都在"创建→删除"，产生大量 GDI 句柄 churn。
- 若调用方在传参后**继续使用**该句柄（例如同一句柄画两次），第二次 `SelectObject` 已失效 → 文字丢失或崩溃。

**建议**：改为调用方拥有、`PutsHan` 不删除；或统一由缓存管理（按 LOGFONT 索引复用）。

### C1 —— 空指针

```cpp
TextOut(hDC, nX, nY, szText, strlen(szText));   // :1031
```
`szText` 为 NULL 时 `strlen` 立即崩溃。700 个调用点里只要有一处传了 NULL（如
`g_xMsg.GetMsg()` 返回 NULL 的情况），就会 0xC0000005。

---

## 四、控件（Controls）

控件走的是 `DrawWithGDI`（141 处）+ `DrawWithImageForComp`（315 处），
两者都写 565 软缓冲，**正确可见**。`DrawWithGDI` 是原版 GDI 实现（`:1609` 起），
`CreatePen/CreateSolidBrush/FrameRect/FillRect` 后用 `DeleteObject` 释放，句柄管理正确。

问题集中在：

- **H1 性能**：同上，每次 `DrawWithGDI` 触发两次整屏拷贝。
- **M4**：`Lock()` 不回填 `dwFlags`。若某个控件代码依赖 `ddsd.dwFlags & DDSD_LPSURFACE`
  判断锁定是否成功，会误判为失败而跳过绘制。
- **B2**：`StatusWnd.cpp` 有 **50 处 `DrawBillBoard`**、`Interface.cpp` 18 处、
  `UserStateWnd.cpp` 18 处、`UtilWnd.cpp` 14 处 —— 这些是 D3D 直绘，
  在当前架构下**全部不可见**。这是"控件显示不全"最可能的原因。

> 建议先做一个判定实验：把 `StatusWnd` 里某个 `DrawBillBoard` 改成
> `DrawWithImageForComp`，若该图标从"不显示"变为"显示"，即证实 B2。

---

## 五、魔法特效（Magic Effects）

**结论：魔法主体可见，特效层部分丢失。**

- `Magic.cpp` 的 `DrawMagic()` 系列（`754/1349/1428/1521`）通过
  `g_xMainWnd.GetBackBuffer()` 走 565 软缓冲 → **可见**。
  该文件仅 1 处 `Get3DDevice`、2 处 `GetBackBuffer`。
- 但以下走 D3D 直绘，被 Present 覆盖：
  - `ChrProc.cpp` 5 处 `DrawBillBoard`（选角/角色魔法特效）
  - `Particle.cpp` 5 处 `DrawBillBoard`（`CSnow/CRain/CMist::RenderSoft` 内部）
  - `GameProc.cpp` 10 处

`DrawBillBoard` 全库 170 处，按文件分布：

| 文件 | 次数 |
|---|---|
| StatusWnd.cpp | 50 |
| Interface.cpp | 18 |
| UserStateWnd.cpp | 18 |
| UtilWnd.cpp | 14 |
| GameProc.cpp | 10 |
| Messenger.cpp | 6 |
| Particle.cpp | 5 |
| ChrProc.cpp | 5 |

> 注意：**170 处里没有一处真正生效**（都被覆盖）。这是本审计里性价比最高的修复点 ——
> 把 `DrawBillBoard` 的实现改为"直接写 565 软缓冲"（复用 `DrawWithImageForCompClipRgnBase` 的像素循环），
> 170 处 UI/特效可一次性恢复。

---

## 六、天气特效（Weather）

### B1 —— 天气依然不可见（本次改造未真正生效）

```cpp
// Particle.cpp:2730
VOID CSnow::RenderSoft()
{
    LPDIRECT3DDEVICE9 pDev = g_xMainWnd.Get3DDevice();
    ...
    g_xGameProc.m_xImage.DrawBillBoard(pDev, &vTrans, &vScale, &mtrl, tex, 0);  // ← D3D 直绘！
}
```

函数名叫 `RenderSoft`，注释说"软渲染"，但实际调用 `DrawBillBoard(pDev, ...)` ——
**是 D3D 设备直绘，不是软缓冲像素操作**。`CRain::RenderSoft`（2748）、`CMist::RenderSoft`（2778）同样。

调用点在 `GameProc.cpp:2726/2731/2736`，位于 `RenderScene` 内、`Present` 之前 →
**画完立刻被软缓冲整屏覆盖**。

所以 `T26 weather soft-render via DrawBillBoard (fix black flakes)` 那次提交：
- 确实解决了"黑色雪片"（换了素材/混合），
- 但**没有解决可见性**，天气现在是从"黑片"变成"完全不显示"。

### 死代码：旧的天气绘制路径

`GameProc.cpp:2341` 的 `DrawWeatherLayer()` 内含 `m_xSnow.RenderSystem(...)`（`:2372`）、
`m_xRain.RenderSystem(...)`（`:2378`），但**该函数全库从未被调用**。
`RenderSystem` 里每一帧都自己 `BeginScene/EndScene`（`Particle.cpp:545/782`），
若被调用也会与主循环场景嵌套冲突——好在它没被调用。

**建议**：删除 `DrawWeatherLayer` 与 `RenderSystem` 天气分支，避免后人误用。

### 其他天气细节

- `CRain::RenderSoft` 在 `tex == NULL` 时直接 `continue`（`Particle.cpp:2763`），
  而"白色雨线"分支传 `NULL` 纹理给 `DrawBillBoard`（`:2772`）——
  需确认 `DrawBillBoard` 对 `NULL` 纹理有兜底（否则可能画不出或崩）。
- `CSnow::RenderSoft` 每帧对每个粒子做一次 `GetTextrImg`（纹理缓存查询），
  粒子数 250 时尚可，若回升到原版数量需确认缓存命中率。

---

## 七、崩溃风险（Crash Risks）

### M1 —— malloc / delete[] 不匹配（真实堆损坏风险）

```cpp
// WHDXGraphic.cpp:645
m_pSoftBackBuffer = (WORD*)malloc(m_dwSoftBufSize);
m_pddsBackBuffer->m_pBits = m_pSoftBackBuffer;

// WHDXGraphic.h:64
void Release(){ delete[] m_pBits; m_pBits=NULL; delete this; }
```

`malloc` 分配的内存被 `delete[]` 释放 —— **未定义行为**。
在 `ResetDXG()`（切换分辨率/全屏）或析构时会触发，表现为随机的堆损坏崩溃，
且往往在崩溃点之前很久就已破坏堆，极难定位。

**修复**：`Release()` 改用 `free()`，或分配改用 `new WORD[]`。

### C2 —— GetDC 未检查创建失败

```cpp
// WHDXGraphic.h:103-116
m_hDC = CreateCompatibleDC(NULL);                 // 未检查
m_hDib = CreateDIBSection(...);                   // 未检查
SelectObject(m_hDC, m_hDib);
...
if(phDC) *phDC = m_hDC;
return DD_OK;                                     // 即使 m_hDC 为 NULL 也返回成功
```

若 `CreateDIBSection` 失败（内存不足/句柄耗尽），`PutsHan` 会拿 `NULL` HDC 去 `TextOut`。
应检查并返回失败。

### M2 / M3 —— Blt 语义不完整

```cpp
// WHDXGraphic.h:66-94
HRESULT Blt(... DWORD dwFlags ...){
    if(dwFlags & DDBLT_COLORFILL){ 整屏填充(忽略 prcDst) }
    else if(pSrc && ...){ 逐行 memcpy(无 DDBLT_KEYSRC 处理) }
}
```

- `DrawWithImage`/`DrawWithSurface` 等调用会传 `DDBLT_KEYSRC` 期望"源色键透明"，
  当前实现**完全忽略**，源图的黑底会被原样拷贝过去 → 出现黑框。
  这与历史上反复出现的"NPC 对话框黑边""聊天窗黑边"是同一类症状。
- `DDBLT_COLORFILL` 忽略 `prcDst`，永远填满整屏。`Clear()` 恰好需要整屏，所以没暴露；
  但任何"局部填充"调用都会把整屏刷掉。

### C3 —— 残留调试代码

```cpp
// WHDXGraphic.h:102, 123
static BOOL bGDC2=FALSE; if(!bGDC2){ FILE*fp=fopen("LogMap.txt","ab"); ... }
```
首次 `GetDC` 会写 `LogMap.txt`。不影响功能，但会在客户端目录留下文件、且在只读目录下 `fopen` 失败无处理。

### 其他

- `Present()` 在 `LockRect` 失败时直接 `return E_FAIL`（`:6863`），不重建纹理也不降级，
  主循环若只处理 `D3DERR_DEVICELOST` 会陷入每帧失败。
- 设备丢失重建路径（`:6830-6859`）重建了纹理和 Sprite，但**未重建 `m_pFont`**，
  且未调用 `m_pSprite->OnLostDevice()/OnResetDevice()`（D3DX9 的推荐用法）。
  目前因为 `m_pFont` 没被使用，问题暂未暴露。

---

## 八、修复优先级建议

按"收益 / 风险"排序：

1. **P0 — 让 D3D 直绘真正落在软缓冲上**
   把 `CImageHandler::DrawBillBoard`（`ImageHandler.cpp:384`）与
   `CChrProcess/CLoginProcess/CSelLoginProcess::DrawBillBoard` 的实现，
   从"D3D 设备绘制"改为"展开到 565 软缓冲"（复用 `DrawWithImageForCompClipRgnBase` 的像素循环）。
   **一次改动可同时修复 B1（天气）+ B2（170 处 UI/特效）+ 魔法特效残留**。

2. **P0 — 修 malloc/delete[] 不匹配**（`WHDXGraphic.h:64`），消除随机堆崩溃。

3. **P1 — GetDC/ReleaseDC 批量化**（`WHDXGraphic.h:101-138`），
   把"每次绘制同步整屏"改为"每帧同步一次"，预计是最直接的帧率提升。

4. **P1 — `Blt` 补 `DDBLT_KEYSRC` 与 `prcDst` 裁剪**（`WHDXGraphic.h:66-94`），修复黑框类问题。

5. **P2 — `LockRect` 加 `D3DLOCK_DISCARD`**（`WHDXGraphic.cpp:6863`）。

6. **P2 — 删除 `g_EffectLayer` 或补齐 `Add()` 调用**；
   删除 `DrawWeatherLayer` / 天气 `RenderSystem` 死代码。

7. **P2 — `PutsHan` 加 `szText` 空检查；字体所有权改由调用方持有**。

8. **P3 — 删除未使用的 `m_pFont`**；`GetDC` 补创建失败检查；清理 `LogMap.txt` 调试代码。

---

## 九、已确认"没问题"的部分（避免重复排查）

- `PutsHan` 的文字绘制逻辑 = 原版 DX7 实现，字体回退（`hFont==NULL → m_hDefGameFont`）正确。
- `DrawWithGDI` 的 GDI 句柄管理正确（`CreatePen/SelectObject/DeleteObject` 配对）。
- `DrawWithImageForComp` 等 315 处 565 合成路径写的是软缓冲 → 可见。
- `Clear()` 通过 `CDX9SoftSurface::Blt(COLORFILL)` 整屏填充，行为正确。
- 设备创建：`Windowed=TRUE` + `D3DFMT_UNKNOWN` + 显式 BackBuffer 尺寸（`WHDXGraphic.cpp:621-631`），配置正确。
- `Present` 有设备丢失检测与 `Reset` 恢复（`:6824-6860`），基本框架正确。

---

# 十、【重要更正】本报告初版的三条错误结论

在动手修复前的验证阶段，发现初版审计有 3 条判断错误，特此更正（保留原文以示过程可追溯）：

| 初版结论 | 实际情况 | 依据 |
|---|---|---|
| **B1** 天气不可见：`RenderSoft()` 走 `DrawBillBoard` 被 Present 覆盖 | **错误**。`CImageHandler::DrawBillBoard` 早在 T59/T62/T92 就被改造成**纯软件 blit**：锁纹理 `D3DLOCK_READONLY` 读出像素，逐像素转 565 写入软缓冲。因此天气**是可见的**。 | `ImageHandler.cpp:477-523` |
| **B2** 170 处 `DrawBillBoard` 全部不可见 | **错误**。同上，这些调用都写进软缓冲，可见。 | 同上 |
| **H2** 每帧纹理回读导致 GPU 停顿 | **减轻**。WIL 纹理是 `D3DFMT_A8R8G8B8` + `D3DPOOL_MANAGED`，`LockRect` 读的是系统内存副本，不是显存回读；开销主要在逐像素 32→16 转换，量级远小于初版估计。 | `ImageHandlerHelper.cpp:22`、`WHWilTexture.cpp:180` |

**教训**：本次审计初版把「函数名/签名」当成了实现，没有读函数体就下结论。
`RenderSoft` 这个名字和 `DrawBillBoard` 这个签名（`LPDIRECT3DDEVICE9` + `IDirect3DTexture9*`）
都在强烈暗示"D3D 直绘"，但函数体其实早已改成软件 blit。**审计必须以函数体为准。**

---

# 十一、本轮实际修复（已应用并构建通过）

修改脚本：`D:\MIR3-DX9\patch_render.py`（5/5 全部应用）
备份：`D:\MIR3-DX9_backup_20261017-110811`
构建：`python build_current.py` → **85 文件 / 0 失败 / LINK 0** → `Mir3_dx9.exe`
测试：`D:\Client\Mir3_fix.exe` 可启动，登录界面正常渲染（背景立绘 + LEGEND OF MIR Logo + 公告文字）

| 编号 | 位置 | 修复内容 |
|---|---|---|
| **F1** | `ImageHandler.cpp`（`bOpa` 计算后） | `bOpa == 0` 直接返回。原判据 `a >= bOpa` 在 `bOpa==0` 时恒真 → 窗口淡出永远以全不透明绘制，淡出失效 |
| **F2** | `ImageHandler.cpp` 新增 `BlendPixel565()` | 纹理 billboard 原先**完全忽略 `bBlendType`**，所有效果都按普通 alpha 处理 → `_BLEND_LIGHT`（加色，魔法/爆炸光效）退化成普通半透明，**光效不亮**。现按 DX7 `SetBlendRenderState` 语义实现 `_BLEND_NORMAL` / `_BLEND_LIGHT` / `_BLEND_LIGHTINV` |
| **F3** | `WHDXGraphic.h:64` | `Release()` 原用 `delete[]` 释放 `malloc` 分配的内存（`m_pSoftBackBuffer`）→ 未定义行为，切分辨率/退出时随机堆损坏。现改为不释放像素（所有权归窗口） |
| **F4** | `WHDXGraphic.h` `Blt` | ① 补 `DDBLT_KEYSRC` 色键：原实现忽略该标志，源图黑底被原样拷贝 → 黑框/黑边；② COLORFILL 补 `prcDst` 裁剪：原实现永远整屏填充 |
| **F5** | `WHDXGraphic.cpp:6863` | `LockRect(..., 0)` → `D3DLOCK_DISCARD`（DYNAMIC 纹理的正确用法，避免同步停顿） |

---

# 十二、仍未解决：登录公告文字重叠

**现象**（`D:\Client\shot_fix1.png`）：登录界面公告文本多行**明显重叠**。
**对照**（`D:\Client\shot_orig.png`）：未打补丁的 `Mir3_dx9.exe` 在同一区域显示为**一条黑色长条**（文字未正常渲染）。

已排除：
- 行距代码本身正确：`PutsHan(NULL, m_rcNotice.left, m_rcNotice.top + (nCnt*14) + nScrlY, ...)`（`LoginProc.cpp:375`），14px 行距合理。
- 与 F1/F2 无关：`PutsHan` 是 GDI 路径，不经过 `DrawBillBoard`。

待查方向（按可能性）：
1. **重复绘制**：公告被画了两遍（一次在 `RenderLogin`，一次在别处），两次起点略有差异 → 视觉重叠。
2. `nScrlY`（滚动偏移）与 `nCnt*14` 叠加后被错误重置，导致多行落在同一 y。
3. 字体过大：`m_hDefGameFont` 的 `lfHeight` 比 14 大，行高不足 → 行间挤压。
4. 公告区域底色由 `DrawBillBoard(NULL纹理)` 绘制（`LoginProc.cpp:333/340`），F1 改变其行为后文字与底板层次变化。

**下一步**：在 `LoginProc.cpp:375` 的循环内打印 `nCnt / nScrlY / nY / strlen(szText)`，
一次即可判断是"位置计算错"还是"重复绘制"。

---

# 十三、天气与昼夜光照修复（对齐原版 DX7）

## 根因

| 模块 | 原版 DX7 实现 | DX9 版原状 | 为什么看不到 |
|---|---|---|---|
| 雪 `CSnow::RenderSystem` | 中心 `(x-400, -y+300)`、尺寸 `m_fSize`、`ONE/INVSRCALPHA` 加色、纹理 `500+frame` | `RenderSoft` 用 `(x,y)`、`m_fSize*4`、普通 alpha | 坐标/尺寸/混合三项全错 |
| 雨 `CRain::RenderSystem` | 雨丝 `1.5 x m_fSize`、水花 `32x32` 纹理 `510+frame`、`_BLEND_LIGHTINV` | 用 `NULL` 纹理画纯白矩形 | 没有纹理，形态不对 |
| 雾 `CMist::DrawMist` | 纹理 `550`、缩放 `(5,2)`、`_BLEND_LIGHTINV`、`m_vTrans` 往复漂移 | 全屏纯色 + 普通 alpha | 没有纹理、没有漂移 |
| 昼夜 `CLightFog::ShowLightFog` | 18x19 顶点网格，`ZERO/SRCCOLOR` 乘法变暗 | `ShowLightFog` 用 D3D 绘制 | **画在 Present 之前，被软缓冲整屏覆盖 → 完全无效果** |

即：前三项是"软渲染化时参数照抄错"，第四项是"根本没软渲染化"。

## 已修复

| 编号 | 文件 | 内容 |
|---|---|---|
| **W1** | `Particle.cpp` `CSnow::RenderSoft` | 恢复 `(x-400,-y+300)` 坐标、`m_fSize` 尺寸、`_BLEND_LIGHT` 加色、纹理 `500+frame` |
| **W2** | `Particle.cpp` `CRain::RenderSoft` | 雨丝 `2 x m_fSize` + 纹理 `500+frame`；落地水花 `32x32` + 纹理 `510+frame`；均用 `_BLEND_LIGHTINV` |
| **W3** | `Particle.cpp` `CMist::RenderSoft` | 纹理 `550` + `_BLEND_LIGHTINV` + 大四边形缓慢往复漂移（`m_vTrans` 语义） |
| **L1** | `LightFog.cpp` 新增 `ShowLightFogSoft()` | 把 18x19 光照网格按 tile 平均色**乘法**到 565 软缓冲；全白时直接跳过（白天零开销） |
| **L2** | `GameProc.cpp:2739` | 改调 `ShowLightFogSoft()` |

**网格到屏幕的映射**（与 `ShowLightFog` 的 `vTrans=(-400,+300)` + y 轴向上一致）：
`screen_x = nTx*48 - 8`，`screen_y = nTy*32 - 29`。

**颜色格式安全性**：`dwFogColor = RGB(m_fDarkRate, m_fDarkRate, m_fDarkRate)` 是灰度
（夜晚 `_FOGSTATE_NIGHT` 取 15，白天 255），R/G/B 等值，故 `RGB()` 与 `D3DCOLOR` 通道序差异无影响。

## 构建与验证状态

- 构建：`python build_current.py` → **85 文件 / 0 失败 / LINK 0**
- 部署：`D:\Client\Mir3_weather.exe`（2.5MB）
- 已验证：进程 66MB 稳定、登录界面正常渲染、无崩溃
- **未验证**：雨/雪/雾的实际观感、昼夜开关效果 —— 需要在游戏内（有账号登录后）才能观察

## 需要你在游戏内确认的点

1. 天气命令（服务端 `@weather` 或对应 GM 指令）切换雨/雪/雾，看是否：
   - 雪：白色发亮小片，**加色**效果（不是灰块）
   - 雨：细长雨丝 + 落地水花
   - 雾：淡青色纹理整体缓慢漂移（不是纯灰蒙版）
2. 昼夜：切换夜晚后整体画面应明显变暗（乘 15/255 ≈ 6% 亮度），白天恢复。
3. 若某个效果仍不对，把该模块名字告诉我即可 —— 参数已按原版逐项对齐，剩余差异只可能在
   `DrawBillBoard` 的 `_BLEND_LIGHT` / `_BLEND_LIGHTINV` 分支强度上，可单点微调。

## 十三.1 修正：雨雪方向与半屏问题（FIX-W4）

**我引入的错误**：把原版 DX7 的 `(x-400, -y+300)` 直接照搬到屏幕空间。

原版那是"中心原点 + Y 轴向上"的 **3D 世界坐标**，经正交投影后：
- `screen_x = (x-400) + 400 = x`
- `screen_y = 300 - (-y+300) = y`

即投影后**正好还原成 `(x, y)`**。软渲染本身在屏幕空间作图，再套一次该变换导致：

| 症状 | 原因 |
|---|---|
| 雨雪从下往上飞 | Y 翻转：粒子 y 增大本应向下，`-y+300` 后变向上 |
| 只显示左半边 | x 左移 400：粒子 x∈[0,800] 映射到 [-400,400]，负值被裁掉，只剩 400..800 → 0..400 |

**已修**：`Particle.cpp` 的 `CSnow::RenderSoft` / `CRain::RenderSoft` 改用
`cx = p->m_vecPos.x; cy = p->m_vecPos.y;`（共 2 处，`patch_weather4.py`）。

**构建**：85 文件 / 0 失败 / LINK 0
**部署**：`D:\Client\Mir3_weather.exe`（2.5MB，12:19:08，哈希一致）
**已验证**：进程 66MB 稳定运行

**雾（CMist）未改动**：仍是 800x600 全屏纹理 + `_BLEND_LIGHTINV` + 漂移。
注意其漂移幅值是按原版世界坐标的 -1.9/帧 搬来的（800 帧累计 -1520px），
在屏幕空间下偏大，可能漂出画面 —— 若你测到雾会"飘走"，把 `sfDriftX -= 1.9f` 改成
`-= 0.125f`（800 帧累计 -100px）即可。

## 十三.2 修正：雾（FIX-W5）

**症状**（用户截图）：左侧出现一块**硬边矩形亮块**，不是铺满屏幕的雾。

**原因**：`CImageHandler::DrawBillBoard` 是**从纹理 1:1 拷贝**，并且会裁剪到纹理尺寸：
```cpp
if ( nW > texW - sx ) nW = texW - sx;
if ( nH > texH - sy ) nH = texH - sy;
```
我传 `vScale(800,600)`，但雾纹理只有几百像素 → 只画出与纹理等大的一块矩形，
既没有缩放也没有覆盖屏幕，边缘自然成为硬边。

原版 `CMist::DrawMist` 是"**一个大四边形 + 纹理缩放**"（`SetScaleMatrix(5,2)`），
本质是纹理拉伸，`DrawBillBoard` 做不到。

**修复**（`Particle.cpp` `CMist::RenderSoft` 重写）：
- 直接 `LockRect(D3DLOCK_READONLY)` 读雾纹理，**逐像素最近邻拉伸**铺满 800x600
- 混合沿用原版 `_BLEND_LIGHTINV`：`dst = src + dst*(1 - srcColor)`
- 纹理坐标用 16.16 固定点增量 + 回绕，循环内无除法，全屏约 48 万像素
- 漂移改为 0.45 / 0.12 像素每帧（原版世界坐标 1.9/帧 换算到屏幕会漂出画面）

**构建**：85 文件 / 0 失败 / LINK 0
**部署**：`D:\Client\Mir3_weather.exe`（2.5MB，12:25:06）

**若仍有问题**，可单点调整：
- 雾太浓/太淡 → 改 `_BLEND_LIGHTINV` 算式里的 src 权重，或在读纹理后乘一个强度系数
- 漂移太快/太慢 → `sfDriftX += 0.45f; sfDriftY += 0.12f;`
- 若纹理拉伸后显得模糊/块的形状不自然 → 说明纹理 550 不是整屏雾图，应改为**平铺(tile)** 采样
