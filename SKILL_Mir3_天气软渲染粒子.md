***

name: mir3-weather-softrender

description: Mir3 China DX9 客户端 (D:\MIR3-DX9) 天气系统 (雪 / 雨 / 雾) 软渲染实现规范。将 D3D billboard 粒子渲染替换为软渲染到 565 主缓冲，使粒子在地图之上、UI 之下可见且达到原版官方效果。适用于在此项目上实现 / 修复天气效果的任意 AI (本 agent 或 ADA), 以及遇到 "雪太淡 / 雨发黑 / 雾全屏盖屏 / 黑块 / 崩溃" 等天气问题时按本规范重做。



***

# Mir3 天气系统软渲染实现规范

## 1. 必须理解的架构 (决定一切)

本客户端**主渲染是软渲染**: 全部场景 / UI / 文字画到 `m_pSoftBackBuffer`(16bit 565 软缓冲),`Present()`(WHDXGraphic.cpp) 把软缓冲全屏绘制上屏。



* 地图、UI、文字都是**软渲染**, 进同一个 565 缓冲。

* 原版 DX7 的雪 / 雨 / 雾是 **D3D billboard**。在软渲染主缓冲架构下，D3D billboard 要么被软缓冲盖住 (不可见), 要么必须在 swap 前特殊时机画 ——**这条 D3D 特效层路径反复失败，已被判定为死胡同，禁止再走**。

* 正确方向:**雪 / 雨 / 雾也软渲染进&#x20;**`m_pSoftBackBuffer`, 在地图之后、UI 之前画。这样层级天然正确 (地图上、UI 下), 效果可控，达到原版效果。

## 2. 已就绪的基础设施 (不许从零重写)

`WHDXGraphic.h`(D:\MIR3-DX9\WindHorn) 已存在，直接复用:



| 函数 / 成员                                                                                                     | 作用                              |
| ----------------------------------------------------------------------------------------------------------- | ------------------------------- |
| `m_pSoftBackBuffer`                                                                                         | 565 软缓冲像素指针 (WORD\*)            |
| `Draw565Scaled(nX,nY,nSrcW,nSrcH,pwSrc,nDstW,nDstH)`                                                        | 原始 565 像素最近邻缩放绘制到软缓冲            |
| `DrawWithImageForCompScaled(nX,nY,nSrcW,nSrcH,pwSrc,nDstW,nDstH,wChooseColor1=0xFFFF,wChooseColor2=0xFFFF)` | RLE 缩放绘制，带 colorkey 透明          |
| `GetBackBuffer()`                                                                                           | 返回 CDX9SoftSurface (可访问 565 缓冲) |

**禁止**：自己写缓冲访问、自己写 blit 内核。只准用以上既有函数。

## 3. 铁律 (踩坑教训，违反即崩 / 黑块 / 盖屏)



1. **colorkey 透明**：粒子纹理必须让 "黑像素 (0x0000) 透明"。用 `DrawWithImageForCompScaled` 的 `wChooseColor1/2`，或先将像素清为 0 再做透明判断。**不透明会造成黑块 / 方块**。

2. **边界 clamp (最高优先级)**：blit 前必须把目标位置 / 尺寸 clamp 到软缓冲范围 (0\~800、0\~600)。**越界是上次软渲染崩溃、黑块的直接原因**。任何负坐标、超大尺寸必须裁剪。

3. **纹理必须转 565**：wil 图库图像在传给软 blit 前必须是 `WORD*`(16bit 565)。确认源像素格式，必要时转换。

4. **缩放必须 >0**：`nDstW/nDstH` 必须 ≥1，否则除零 / 异常。

5. **禁止改 D3D 渲染路径**：不要在 RenderSystem (D3D)、特效层 (EffectLayer)、Present () 时机上再做任何 "碰运气" 改动。粒子一律走软渲染。

6. **小步一次一个点**：一次只改一种天气 (如先雪), 编译部署实测确认后再动雨、雾。禁止连环改。

## 4. 实现步骤 (雪 / 雨 / 雾分别)

### 通用流程



1. 读 `GameProcess/Particle.cpp` 对应类的渲染函数 (现为 D3D RenderSystem / DrawMist)。

2. 新增 / 改写为软渲染版本：遍历存活粒子 (`m_pxParticle[nCnt]`, 跳过 `m_bIsDead`)。

3. 取纹理：wil 图库 `GetTextrImg(_TEXTR_FILE_PARTICLE, _IMAGE_PROGUSE, 500+frame)` 等段。

4. 用 `Draw565Scaled` 或 `DrawWithImageForCompScaled` blit 到软缓冲，先 clamp 边界。

5. 保留粒子位置 / 帧 / 生命逻辑不动，只换 "怎么画"。

### 雪 (纹理段 500)



* 白色雪花：纹理 500+`m_nCurrFrame`。

* 缩放：`m_fSize`(建议保持原版 2\~6 但可用), 目标 `nDstW/nDstH = nSrcW*fSize`。

* 位置：`(INT)m_vecPos.x`, `(INT)m_vecPos.y` 转换到屏幕系。

* 落地 / 淡出：保留原生命淡出逻辑 (颜色递减) 或直接画，按原版语义。

### 雨 (纹理段 500 空中雨线 / 510-514 落地水花)



* 空中雨线：纹理 500+frame, 细长条 (nDstW 窄、nDstH = m\_fSize 高)。

* **落地水花 (原版有、之前一直缺)**: 当粒子 `m_nCurrLife-m_nLife ∈ (0,510)` 或 `pos.y>=ground` 时，画纹理段 **510+frame**(32×32 水花)。必须实现，否则雨 "没有落地水花"。

* 颜色白 (不黑),colorkey 透明。

### 雾 (纹理段 550)



* 雾是全屏效果：软渲染下**不要**用 D3D 的全屏四边形 + alpha。

* 软渲染雾 = 把纹理 550 的低密度 / 低亮度形式画到软缓冲，或按原版 `DrawMist` 的移动四边形平移逻辑，用软 blit 画，保证**半透明感**且**不盖屏**。

* 若软 blit 不支持 alpha, 用低亮度色值或稀疏绘制表达 "雾", 禁止画成整屏实色盖住场景。

## 5. 验证与提交 (必须)



1. **改前备份**目标文件 (独立 .bak), 脚本全程断言，任一不匹配立即停不写盘。

2. 改后 `git diff` 逐行核对，再编译。

3. 编译:cd D:\MIR3-DX9; python build\_current.py, 成功标志 `=== compiled 85 fail 0` + `LINK 0` + `SUCCESS`。

4. 部署：D:\MIR3-DX9\Mir3\_dx9.exe → D:\Client\Mir3.exe, 先备份旧 exe。

5. 部署后**必须要求用户彻底退出游戏重启**再实测 (旧进程不加载新 exe, 会导致 "还是一样")。

6. 实测看：雪白、雨白 + 落地水花、雾半透明不盖屏。任一不对，只调对应一个参数，禁止连环改。

7. 结论写入 `D:\MIR3-DX9\NOTES.md` 并 git 提交推送。git 提交注意 add 具体文件 (未跟踪的 `nul` 文件会导致 `git add -A` 失败)。

## 6. 对 AI 的行为约束 (严禁违反)



* **禁止瞎编**：所有结论必须来自读到的代码 / 编译 / 实测，不确定就明说 "不确定", 不编造行号、函数、行为。

* **禁止重复思考打转**：同一问题最多换两个方向验证，两次无进展必须停下来报告，不得反复试。

* **先对标原版**：D:\Mir3China\GameProcess\Particle.cpp (DX7 原版) 同段为准，差异要说明。

* **源码 GBK**：任何中文字面量改动用 Python encode ('gbk') 或纯字节替换，禁止编辑工具写 UTF-8 (曾踩乱码坑)。

* **每次核查 / 实测 / 修复结果必须写入 NOTES.md 并提交推送**(用户硬性惯例)。

## 7. 完成定义 (Definition of Done)



* 雪：白色雪花在地图上可见飘落，UI 之下。

* 雨：白色雨线可见，且落地有水花。

* 雾：半透明雾感，不盖屏、不成方块。

* 无黑块、无崩溃、无越界。

* 已编译 85 fail 0、已部署、已写 NOTES.md 并 git 提交推送。

* 用户实测确认。