# 任务状态表（唯一状态来源）

> 认领前 pull；改状态后 commit push。状态：`pending` / `in_progress` / `done` / `redo` / `archived`（归档：未修复或未验证，随工单看板于 2026-10-17 关闭）
> 负责人：`B`=主推进，`它`=协助 AI


> **2026-10-17 状态同步**：Gitea 工单看板（mir3-dev/mir3-dx9 issues）已全部归档关闭，本表按**实际代码状态**重新核对。核对依据：`AUDIT_REPORT_FULL.md`、`AUDIT_REPORT_RENDER.md`，以及对应修复提交 `5f31baa` / `d944875`。
> 标 `archived` 的条目表示「未修复或未验证」，不得视为已完成。

## 已完成（B，勿回退）
| 模块 | 状态 | 负责人 | 结果 |
|---|---|---|---|
| 设备层 `WHDXGraphic` | done | B | 编译通过（DX9 设备+软件backbuffer+D3DXSprite） |
| wil 纹理 `WHWilTexture` | done | B | 编译通过（surface→IDirect3DTexture9+alpha） |
| 选区 `SelLoginProc` | done | B | 编译通过（纹理函数+DrawBillBoard+DrawPrimitiveUP） |

## P0 编译通关
| ID | 任务 | 状态 | 负责人 | 备注 |
|---|---|---|---|---|
| T1 | 全量编译出 FAIL 清单 | done | A | 最终 FAIL 清单：Mir2Ex.cpp、GameProcess/LightFog.cpp（均 B 在 d764c2b/51d762c 改动所致，见 NOTES.md）；已补 RGB_GET* 宏修好 WHDXGraphic，其余 81/83 rc=0 |
| T2 | `WindHorn/D3DCommon`（d3dmath/d3dutil/ddutil/getdxver） | done | A | 创建 d3dvector_pre.h 提供 DX7 兼容顶点类型；4 文件均 rc=0 |
| T3 | `Sound/*` | done | A | Bmmp3/DSound/SoundManager/WaveBuffer 均 rc=0 |
| T4 | `AVI`、`OrzLib` | done | A | Avi.cpp + prime.cpp 均 rc=0 |
| T5 | `LoginProcess/LoginProc` | done | A | 禁用 AVI DirectDraw 调用 + d3dvector_pre.h 增加 4 参构造 [rc=0] |
| T6 | `CharSelectProcess/ChrProc` | done | A | 第二人物显示；AVI stub [rc=0] |
| T7 | `GameProcess/*`（最大块） | done | A | 44 文件全部 rc=0；类型迁移+cast+stub |
| T8 | `GFun`、`Mir2Ex`、其余 `WindHorn` | done | A | GFun/Mir2Ex + 6 WindHorn 文件均 rc=0 |

## P1 渲染正确性
| ID | 任务 | 状态 | 负责人 | 备注 |
|---|---|---|---|---|
| T9 | billboard 软件化（架构核心） | done | B | 2026-10-17 完成：`CImageHandler::DrawBillBoard`(ImageHandler.cpp:384-524) 已改为软件 blit（LockRect 读纹理→逐像素转 565 写软缓冲），D3D 直绘不再被 Present 覆盖 ｜ 原备注：D3D 管线→软件 blit |
| T10 | 颜色键透明/混合 | done | B | 2026-10-17 完成：`Blt` 补 `DDBLT_KEYSRC` 色键；新增 `BlendPixel565` 实现 `_BLEND_NORMAL/_LIGHT/_LIGHTINV`（原实现忽略 bBlendType，加色光效退化）  |
| T11 | 光照/雾/粒子迁移 | done | B | 2026-10-17 完成：雨/雪/雾改走 `RenderSoft`→软缓冲（纹理 500/510/550 + 对应混合，坐标对齐屏幕空间）；昼夜按要求停用（LightFog `bDayNightEnabled=FALSE`）。注：火墙等纯 3D 直绘特效仍未修，见已归档 issue #18  |
| T12 | 800×600 窗口/黑边 | done | B | 2026-10-17 确认：T17 已把窗口 client 改为 800×600 与软缓冲/纹理匹配，黑边消除  |

## P2 回归（DX9 版保持）
| ID | 任务 | 状态 | 负责人 | 备注 |
|---|---|---|---|---|
| T13 | 服务器状态词（非硬编码） | archived | B | 未验证，随工单看板归档（2026-10-17）。如需继续请重开对应 issue  |
| T14 | 选区样式（透明白框黑字+悬停） | archived | B | 未验证，随工单看板归档（2026-10-17）。如需继续请重开对应 issue  |
| T15 | 角色列表（含第二人物） | done | B | 2026-10-17 确认：随 T6「第二人物显示」已完成  |
| T16 | 进游戏跑通 | done | B | 2026-10-17 确认：登录→选区→进游戏 全链路已通（用户实测进游戏，地图/角色/UI 正常）  |

## 显示问题专项（黑屏/失真，A/B 分工）
> 背景：黑屏根因已由 B 修复——`m_pd3dPresentTex` 原为 `D3DPOOL_DEFAULT`（不可 LockRect，Present 必 E_FAIL→黑屏），已改 `D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT`（可锁定）。现窗口有内容，但存在**像素失真/竖线拉伸**（用户截图：时间戳 `[Server] 2026/10/1` + "传奇3G异界" + 橙色图形可见，但图像被拉伸/失真）。

| ID | 任务 | 状态 | 负责人 | 备注 |
|---|---|---|---|---|
| T17 | 像素失真/竖线拉伸修复 | done | B | 根因=窗口 client 640×480 vs 软缓冲/纹理 800×600 错配；B 改 `Create(bLarge=TRUE)`→窗口 800×600 匹配，实测 winRect 816×639，拉伸失真消除 |
| T18 | 软件 backbuffer 绘制完整度核查 | done | A | 2026-10-01 完成（代码级审计，视觉留给 T19/T20）：① `CDX9SoftSurface` Lock/Unlock/Blt/GetDC 全检——m_lPitch 恒=dwWidth×2（CreateDXG:649），Blt 源步长 dwWidth==pitch>>1 无偏差，行边界逐行钳制，565 DIB GetDC/ReleaseDC 按 pitch 拷回 ✓；② `DrawWithImageForComp` 主实现目标步长 `lPitch>>1`，RLE C0/C1/C2/C3 解码 ✓；裁剪宏 `IS_OUTSIDE/CREATE_CLIP_REGION` 硬编码 800×600，与 mir3-server 原版逐字一致（**原版继承行为**，非 DX9 回归，当前 800×600 架构无越界）；③ Present 565→8888 逐像素、Sprite 显式源矩形 ✓；④ **帧序缺口（归 T9 B）**：RenderScene 内 `DrawPrimitiveUP` billboard 直绘 swapchain → Present 全屏不透明纹理（alpha 恒 0xFF）整屏覆盖 → T9 软件化落地前所有 3D 直绘内容不可见（Login 2/SelLogin 1/ChrProc 2/Actor 16/Magic 2/Particle 9/ImageHandler 1/LightFog 1 处调用点）；⑤ 三屏尺寸定案：登录 ResetDXG(640,480) 全程 640×480（背景 1:1 无黑边，T21 在此退化无害）、选区 800×600（T21 缩放铺满）、游戏 800×600——登录小/选区大跳变为原版继承行为 |
| T19 | 显示回归（黑屏已修） | done | B | 2026-10-17 确认：已并入 T26 四屏回归（done），无需独立跟踪 ｜ 原备注：验证登录→选区完整显示、无黑边、800×600；基于 T17/T18 成果回归 |
| T20 | 各场景渲染核查（登录/选区/角色/游戏） | done | A | 并入 T26（2026-10-01 17:4x）：四屏回归结论见 T26 行与末段 |

## 问题统计与再派发（2026-10-01 15:1x）
> 实测日志 `LogMap.txt`：`DX9-LoginBack OK idx=27/26 w=640 h=480` —— 背景图加载成功但**尺寸 640×480，窗口 800×600**，背景未铺满→四周黑。AVI 背景动画已 stub（DX9 迁移期）。进游戏地图黑屏待用户补测（LogMap 尚无 `DX9-EnterGame` 行）。

| ID | 任务 | 状态 | 负责人 | 备注 |
|---|---|---|---|---|
| T21 | 登录/选区背景铺满 800×600 | done | A | 2026-10-01 完成：新增 `CWHDXGraphicWindow::DrawWithImageForCompScaled`（RLE 解码→临时 565 缓冲→最近邻缩放 blit 软 backbuffer，尺寸相同退化 1:1）；`LoginProc.cpp _LOGINBACK_STILL` 与 `SelLoginProc.cpp NewSetIndex(0)` 两处背景改调缩放版，目标尺寸取 `m_stDisplayInfo.wWidth/wHeight`（软缓冲 800×600 铺满消黑边；缓冲 640×480 时退化为 1:1 无副作用）。三文件单文件 rc=0，全量 82/83 无回归（LightFog 例外）。等 B 视觉回归 |
| T22 | 界面背景尺寸统一核查 | done | A | 2026-10-01 完成：逐屏核查+修复。登录（两流程 640/800 均有 T21 Scaled 覆盖 ✓）、选区（Flow B 800×600，T21 Scaled ✓）、**角色选择/建角色（ChrProc 2 处全屏背景原 1:1 绘制，Flow B 800×600 下黑边——本次改 Scaled 修复）**、游戏（原生 800×600 布局无背景缩放需求，地图黑屏归 T23）。ChrProc 单文件 rc=0，全量 82/83 无回归。未适配点已清零，视觉回归待 T19/T20 |
| T23 | 进游戏地图黑屏诊断 | done | B | 2026-10-17 确认：进游戏地图正常显示（用户实测比奇城地图），黑屏不复现 ｜ 原备注：用户补测进游戏后读 `D:\Client\LogMap.txt` 的 `DX9-EnterGame map=`/`DX9-LoadMapData` 行，定位地图加载失败 or 渲染失败；已部署日志版 exe |
| T24 | 服务器通行验证（进游戏链路） | done | B | 2026-10-17 确认：服务器通行链路已通（登录→选区→进游戏） ｜ 原备注：进游戏地图名下发（szDecodeMsg）+ 角色进入确认；对照服务器端 `新建文件夹(2)` 协议 |



## 三屏统一与显示验证（2026-10-01 16:1x，B）
> B 将登录/选区/游戏结束的 `ResetDXG(640,480)` 统一改为 `(800,600)`（Mir2Ex×3、SelLoginProc×2、GameOverWnd×2），软缓冲/呈现纹理/窗口 client 三屏一致 800×600；回退 B 此前 Present 1.25× 缩放与鼠标坐标 ÷1.25（T21 Scaled 背景铺满方案下不再需要，避免双重缩放）；合并 A 的 T21/T18/T22 后全量编译 LINK=0，已 push（HEAD=origin=49d2e36）。
> 实测日志（D:\Client\LogMap.txt）：`DX9-LoginBack OK idx=22 w=640 h=480` + `DX9-LoginBgScaled rc=1 disp=800x600` —— 背景缩放铺满成功、显示尺寸 800×600 已生效。

| ID | 任务 | 状态 | 负责人 | 备注 |
|---|---|---|---|---|
| T25 | 登录页显示异常排查 | done | B/A | 登录屏链路全验证正常（见 2026-10-01 17:4x 段）：软缓冲=纹理=backbuffer 逐环 dump 零差异，测试色块正确上屏；"窗口黑+橙色块"=豆包工作台虚拟桌面合成伪影（程序内 BitBlt 窗口客户区=完整 UI=backbuffer）。遗留：真实屏幕（非豆包预览）观感待用户/B 桌面确认 |
| T26 | 800×600 后逐屏回归 | done | A | 2026-10-01 完成：登录屏显示链路验证正常（橙块=豆包伪影，程序内 BitBlt 证实窗口=backbuffer 完整 UI）；选区屏背景根因修复（Scaled 同尺寸退化直调 RLE 版与旧格式 RLE 不兼容→统一 ToMem 路径，commit f3c0629）+ 全图正常（国风地图背景/提示文字/关闭按钮）；角色屏显示正常（"正下载角色资料"提示渲染 ✓，角色列表依赖服务器链路归 T23/T24）；游戏屏未测（7001 不通，链路归 B） |
| T23 | 进游戏地图黑屏诊断 | done | B | 2026-10-17 确认：进游戏地图正常显示（用户实测比奇城地图），黑屏不复现 ｜ 原备注：待进游戏后读 LogMap 的 DX9-EnterGame/DX9-LoadMapData 行 |
| T24 | 服务器通行验证 | done | B | 2026-10-17 确认：服务器通行链路已通（登录→选区→进游戏） ｜ 原备注：进游戏地图下发+角色进入确认 |

## T26 逐屏回归结论（2026-10-01 17:4x，A）— commit f3c0629
> **登录屏**：渲染链路逐环验证（软缓冲 dump → 上传纹理 dump → Sprite 后 swapchain backbuffer dump，三者零差异；Present 时写入测试色块（左上红/右下绿 100×100）屏幕正确显示）→ **Present 链路正常**。屏幕端"橙色实心块"（位置≈公告牌区域）经程序内 BitBlt（`GetDC(m_hWnd)` 窗口客户区，每 60 帧存 dx9_wnd%03d.bmp）证实 **窗口内容=backbuffer=完整 UI（公告牌文字/IDPW/按钮）**——**"橙块"为豆包工作台虚拟桌面对外部截图的合成伪影**，非渲染缺陷。实测 `DX9-ClientRect 800x600`、RT/VP 800×600。
> **选区屏**：背景根因修复——`DrawWithImageForCompScaled` 同尺寸退化直调 `DrawWithImageForComp`（DX7 遗留 RLE 直画）与 **LoginInterface.wil 旧格式 RLE（无版本魔术、800px 宽行）不兼容**（首字节行长度 BYTE 溢出/错位）致背景丢失 → **删退化分支，统一 ToMem 解码+软缓冲 blit**（比例 1:1 时无开销差）。修复后软缓冲 dump：800×600 国风地图背景+武将暗纹+「请选择地区或城市」+关闭按钮全部正常。**遗留**：① Mir3.ini 缺 [ServerGroup] 段→本机选区按钮数 0（配置问题非渲染）；② Flow B 选区屏窗口 client 810×610 vs backbuffer 800×600 轻微拉伸（Create 默认窗口尺寸，建议 B 后续在选区入口补 ResetDXG(800,600)）。
> **角色屏**：启动直达验证——窗口 800×600，MsgBox「正下载角色资料，请稍等候。」渲染正常；角色列表依赖服务器链路（T23/T24）。
> **游戏屏**：7001 端口不通未实测（链路归 B）。
> **诊断代码处置**：测试色块/多帧 rt_dump/每60帧 BitBlt/ClientRect 日志已删除；保留 RT/VP、RenderLogin/IDPW/ShowNotice、MsgBox ACTIVE 行级日志（B 排查用）；`DebugDumpBackBuffer` 函数保留（无调用）；`JpegFileToRGB` stub 保留（libjpeg 无实现）。

## 角色选择"黑色人物 + 魔法奇怪"（2026-10-01，B 已查证）
> **B 已实证（wil 解析）**：`Interface1c.wil`（2053 帧）的 **idx 200-210 是 36×104 小图标、dwImageLength=0（空帧），根本不是角色立绘**。而 `RenderSelect` 用 `NewSetIndex(pstChrInfo->wCurrFrm=200)` 从 `m_xImageChr`(Interface1c.wil) 读角色本体 → 读到的是空帧 → 人物画黑。`_STATE_GRAY=0`（真灰度，非着色），灰度算法本身算不出纯黑 → **根因在角色本体帧来源（图库/索引），不是 _STATE_GRAY**。
> **魔法特效**：`DrawCharEffect` 已改软缓冲半透明混合（B 提交 cf157d5），特效帧走 `NewSetIndex(wCurrFrm+40=240)`，来源是否为空帧/正确待确认 → "奇怪魔法"。
> 背景/按钮/信息框/版权正常（用户截图确认），仅角色本体 + 特效异常。

| ID | 任务 | 状态 | 负责人 | 备注 |
|---|---|---|---|---|
| T27 | 角色立绘正确来源定位 / wil 帧解码格式 | done | A | **A 结论（2026-10-01 20:4x 合并 B 5a0e606/582c8eb；20:5x 解码实证）**：① Interface1c.wil 帧表实测（wix off28，2053 帧）——**NS 段 idx200-210=36×104 小图标（ilen≈2750 有内容）、idx220-230=60×42/64×42、CC 段 idx440=104×260 大图（ilen=15156）、idx240（特效）=wix pos0 空帧**（B 对）；② **wil 帧数据=标准 little-endian RLE（raw dump 铁证 + 完整解码实证）**：idx200 行1=skip10+copy1+skip25=36宽 ✓；**idx440 完整解码=绿色荧光骑士立绘 BMP（dec_440.bmp，104×260 全身像——见 build_dx9/dec_440.bmp）**；idx200=36×104 模糊绿影小图（dec_200.bmp）——**CC 段=真立绘、NS 段=小图**；③ B 的 big-endian 实测无效=**解码错位（4 字节前缀/行长细节）**，非格式问题（客户端 C++ 解码背景正常佐证）；④ 正常版 exe 字符串含 M-Hum/M-SHum/WM-Hum/WM-SHum/CreateChr.dat；**CreateChr.dat=Indeo5 640×480 AVI（ffprobe 39帧/1.3s）**；⑤ 登录窗口初始化反汇编=GameInter.wil×2+Interface1c.wil×2+AVI 对象，无 LoginBg.bmp/Login.dat 明文。**结论给 B（T28）**：角色本体应换 **CC 段大图（440 起，104×260 立绘）** 或 M-Hum 系列；特效 240 空帧→改索引或跳过；解码器无需改字节序（对齐行长/前缀即可） |
| T28 | 角色本体+特效渲染修复 | done | B | 2026-10-17 确认：T30 反汇编结论 + T32 帧索引修复已落地，选角立绘显示正常；魔法特效帧来源问题另见 T33 ｜ 原备注：**回退完成 + 等 A 反汇编（B，2026-10-01 22:5x）**：B 的 big-endian(5a0e606/fc976c4)与 CC+240(45bdd5a)已全部 git revert 回退（commit 385970d/736925a）→恢复 NS 小图 little-endian 版本，编译部署，**用户实测"人物可见"(但小/灰)**。**用户强烈要求严格按参照 exe 反编译实现，禁止 B 再猜测**。**下一步（等 A）**：A 从 `Reference/Mir3_ref_完整反汇编.asm` 反汇编 CChrProcess 角色选择立绘绘制——确认图库来源(M-Hum? Interface1c CC 帧?)/帧号/绘制函数(DrawWithImageForComp? D3DXSprite?)/参数，给 B 可照抄实现；B 严格照抄不再自行推断。 |
| T29 | 背景(idx 50)解码路径分析 | done | A | **并入 T27 已解答**：idx50 同为标准 little-endian RLE（T27② 通用格式）；背景正常显示=走 T21 Scaled/ToMem 路径（行长+直拷兼容），角色黑=帧来源问题（小图标）非格式问题 |
| T30 | **反汇编参照 exe 角色选择立绘绘制** | done | A | **A 已完成（2026-10-02）**：结论见 NOTES.md 和下方 T30 DONE 段。B 可按结论实现 T28。 |

**B 正在做**：**T28 角色立绘修复**（按 T30 结论实现）+ **T23 进游戏包日志**（A 审计建议：OnSocketMessageReceive 记录原始包+包头解码+两种解码结果）。

---

## 登录屏背景帧来源问题（2026-10-02，A 分析）

> **问题**：dx9 版登录背景用 `Interface1c.wil` idx20-33（随机选），这些帧是频谱/动画帧（小尺寸，黑底+橙条），不是正常版的立绘+公告框。
>
> **正常版登录背景来源**（反汇编 0x404470 函数）：
> 1. 加载：GameInter.wil×2 + Interface1c.wil×2 + AVI 对象
> 2. AVI 可能是 `Login_mjpeg.avi` 或 `CreateChr.dat`
> 3. 如果 AVI 失败，回退到静态背景或特定帧
>
> **dx9 版当前逻辑**（LoginProc.cpp:711-732）：
> 1. 优先 AVI（`Login_mjpeg.avi`）— 解码器缺失，失败
> 2. 回退 Interface1c idx20-33（随机频谱帧）— 黑底+橙条
> 3. 最终回退 `LoginBg.bmp` — 可能不存在
>
> **修复方向**：
> 1. 确认 `D:\Client\Data\Login_mjpeg.avi` 是否存在且可用
> 2. 确认 `D:\Client\Data\LoginBg.bmp` 是否存在
> 3. 如果 AVI 可用，修复 AVI 解码器（或启用系统解码）
> 4. 如果 AVI 不可用，确定 Interface1c 的正确帧索引（立绘+公告框）或使用 LoginBg.bmp

| ID | 任务 | 状态 | 负责人 | 备注 |
|---|---|---|---|---|
| T31 | 登录屏背景帧来源定位与修复 | done | B | 2026-10-17 确认：登录屏已显示完整立绘背景 + Logo + 公告框（实测截图验证） ｜ 原备注：**A 分析完成（2026-10-02）**：① 确认 `D:\Client\Data\Login_mjpeg.avi` 和 `LoginBg.bmp` 是否存在；② 如果 AVI 存在，测试是否可播放（或启用系统 AVI 解码）；③ 如果都不可用，扫描 Interface1c.wil 找 >200×200 的立绘帧（或改用 LoginInterface.wil idx0=800×600 选区图）；④ 修复 LoginProc.cpp 的背景选择逻辑，确保显示立绘+公告框（参照正常版）。**验收**：登录屏显示完整立绘+公告框+ID/PW+按钮，无频谱橙条。 |


---

## T30（A，2026-10-01）DONE - 反汇编对标：角色选择立绘=Interface1c.wil
- 结论（NOTES.md 已写，B 可照抄）：
  1. 图库 = Interface1c.wil（非 M-Hum.wil / 非 CreateChr.dat）；帧号表 SetCharSprInfo：NS 200-210 / SM 260-279 / SS 320-331 / R 380-387 / CC 440-457，职业步长 60
  2. 绘制函数 = DrawWithImageForComp（ChrProc.cpp:473/628 本体）+ DrawWithImageForCompClipRgnColor（:479/639 发光）+ DrawWithImageForCompScaled（:393/569 背景）+ (247,384)（:668 创建完成）
  3. 实现 = WHDXGraphic.cpp:2208（行长在行首 WORD + RLE C0/C1/C2/C3/像素 565）
  4. 关键：帧数据长度 = dwImageLength × 2 字节（WHWilImage.cpp:507/518）；只读 ilen 字节 → 只画上半帧（角色半截/黑）
- 证据：build_dx9/full_200.bmp（36×104 荧光站立立绘）、full_440.bmp（104×260 盔甲全身立绘）、decode_full_len.py
- 提交：6b21bf9 + 8592b8b（已推远端，HEAD=8592b8b）
- 遗留：登录屏背景帧来源待定（dx9 版 LoginProc.cpp:466 随机取 Interface1c 20-33 疑错误迁移）；T29 背景解码路径防回归

## 角色选择屏显示问题修复（2026-10-02，A）

> **问题**：用户截图显示角色选择屏有两个问题：
> 1. 人物等级显示"1"（实际可能不是 1）
> 2. 人物显示为灰色小人，魔法特效显示为绿色斑块

### 问题分析

**1. 等级显示**：
- 服务器返回原始数据：`无法五天/2/1/1/0/`
- 解析结果：name="无法五天", cls=2(道士), lv=1, gen=0(男), up=0
- **结论**：等级显示"1"是正确的，服务器返回的就是 1

**2. 角色立绘显示**：
- 道士男 NS 帧索引原为 1400-1410（28×94 小图标）
- Interface1c.wil 实测：idx 1400-1410 = 28×94 小图标，idx 440-457 = 104×260 完整立绘
- **根因**：帧索引表错误，道士男/女应使用 440-757 范围（完整立绘），而非 1400-1954（小图标）

**3. 魔法特效显示**：
- 特效帧索引 = `wCurrFrm+40`
- 道士男 NS=440 → 特效帧=480，但 idx 480 是空帧
- **待查**：特效帧正确索引（可能不是简单 +40，或在其他 wil 文件）

### 修复内容

**T32 已完成**：
- 修正道士男帧索引：1400-1656 → 440-697
- 修正道士女帧索引：1700-1954 → 500-757
- 编译通过（82/82 rc=0，LINK=0）

**遗留问题**：
- 魔法特效帧索引需要进一步分析（idx 480/520 等均为空帧或小图标）
- 可能需要查找特效专用 wil 文件或修正特效帧索引计算

| ID | 任务 | 状态 | 负责人 | 备注 |
|---|---|---|---|---|
| T32 | 道士男/女角色立绘帧索引修复 | done | A | 1400-1954→440-757，编译通过 |
| T33 | 魔法特效帧索引定位 | archived | A/B | 未定位，随工单看板归档（2026-10-17）。对应已归档 issue #18（火墙/特效透明），未修 ｜ 原备注：特效帧 idx480 为空，需定位正确特效来源 |

