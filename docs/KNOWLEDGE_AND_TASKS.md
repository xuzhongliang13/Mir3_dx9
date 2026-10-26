﻿# 传奇3国服 DX9 客户端 调试知识库 + 任务分配

> 本文档由 MainAgent（Doubao）编写，提交于 Gitea 仓库 `mir3-dx9`。
> 协作规则：**所有 AI agent 以本仓库为唯一交流中枢**——修改代码 → git 提交（commit 消息注明任务编号+结论）→ push；新调查结论/新问题/进展 → 追加或更新本文件 → git 提交。拉取方先 `git pull`。
> **文件所有权隔离**：见文末"任务分配"，各任务的文件列表互不重叠，禁止越界修改他人文件；若确需修改他人文件，把补丁贴到 ISSUES.md 或 commit 消息中，由文件所有者合并。

---

## 0. 一句话现状

客户端"登录→选角→进游戏→主界面"已打通；**当前主线：主界面底栏条/8按钮平时不显示（仅悬停焦点可见）+ 魔法窗口出现双份**（T74 已部署诊断日志版，等待用户实测 LogDraw/LogWnd.txt）。

---

## 1. 环境与账号（务必先读）

| 项 | 值 |
|---|---|
| Git 仓库 | `http://192.0.2.10:8418/mir3-dev/mir3-dx9.git` |
| Git 账号（MainAgent/Doubao） | jack / xzlwy5200 |
| Git 账号（Agent B / 协作AI） | **admin / <已移除>**（与主 agent 不同，勿混用） |
| 本地镜像 | `C:\Users\Administrator\DoubaoWork\chats\2026-10-03\new-chat\Mir3China_DX9\mir3-dx9` |
| 编译产物 | `D:\Client\Mir3_dx9.exe`（部署目标） |
| 游戏服务器 | `192.0.2.10`（游戏口 Param1=7000 TCP OPEN；MSSQL 1433 不通） |
| 服务器文件 | SMB `\\192.0.2.10\mud3` = Z:（账号/密码 <已移除>） |
| 服务器端 | 只有编译好的韩版魔改二进制（**无源代码**）`D:\MUD3` / Z: |
| **已否决参考** | `D:\work\mir3-server` 是传奇2源码，用户多次申明"不算数，不要考虑" |
| 参考源码 | 国服 DX7：`D:\传奇3客户端源代码\mir3G\Mir3China\WindHorn`；韩版 DX7：`D:\传奇3客户端源代码\mir3G\Mir3Korea` |
| 正常对照端 | `D:\Client\Mir3_patched_v3_backup.exe`（官方正常运行，一切以它为基准，**严格按实测、禁止猜想**） |
| 反汇编参考 | `D:\Mir3_Compare\patched_full_disasm.txt`、`D:\Mir3_Compare\github_build_log.txt` 等 |
| 用户客户端 | Windows 11，已装 VSCode（工程已用 `code` CLI 打开） |

**游戏账号卡点（硬）**：有效游戏账号/密码未知。实测 <已移除>、test/test、123456/123456、admin/admin 均返回"此用户账号不存在或者已被加锁"；`1234567890` 是身份证号非密码。UserPic.ini（GBK）有角色名：梦想神话_Xingbarking=911130435、梦想神话_道祖/散仙/法仙。**每次要进游戏测试需请用户登录**（用户掌握账号，实测他输入账号即可进）。

---

## 2. 编译 / 部署 / 验证流程（重要）

```powershell
# 1) 杀残留进程（exe 被占用会 Copy 失败）
Get-Process | Where-Object { $_.ProcessName -like "*Mir3*" } | Stop-Process -Force -Confirm:$false
# 2) 编译（w64devkit mingw32-g++）
Set-Location "<本地镜像>\mir3-dx9"
Remove-Item Mir3_dx9.exe -Force -ErrorAction SilentlyContinue
python build_local.py
#    成功标志：`compiled 82 fail 0` + `LINK 0` + `SUCCESS: ...Mir3_dx9.exe`
#    windres "preprocessing failed" 是已知噪声，忽略
# 3) 部署
Copy-Item Mir3_dx9.exe D:\Client\Mir3_dx9.exe -Force
# 4) 清诊断日志（避免旧日志干扰判断）
Remove-Item D:\Client\LogDraw.txt, D:\Client\LogWnd.txt, D:\Client\LogWIL.txt, D:\Client\LogMain.txt -Force -ErrorAction SilentlyContinue
# 5) 请用户测试；完成后读 D:\Client\Log*.txt 分析
# 6) git add -A; git commit -m "[Txx] 说明"; git push origin HEAD
```

**客户端截屏**：游戏内按 PRTSCN（VK_SNAPSHOT）→ 生成 `D:\Client\SCREENxxxx.bmp`（客户端自带功能，比外部抓屏可靠；DX9 独占表面外部 PrintWindow/BitBlt 全黑）。

---

## 3. 编码规则（踩过坑，必须遵守）

| 文件 | 编码 | 修改方式 |
|---|---|---|
| `Define.h`、`WHWilImage.cpp` | GBK(936) | 用 PowerShell `[System.Text.Encoding]::GetEncoding(936)` 行数组读写，**禁止 Edit 直改**（会乱码破坏） |
| `Interface.cpp`、`GameWnd.cpp`、`MagicWnd.cpp`、`WHDXGraphic.cpp` | UTF-8（WHDXGraphic 有 BOM） | 可直接 Edit |
| PowerShell 操作 | 避免双引号内 `\"`；脚本内用 List[string] 收集再 WriteAllLines | |

---

## 4. 关键技术知识库

### 4.1 登录协议（已修复，勿动）
`SendRunLogin` 4 字段明文：账号/密码/（2 个空字段？）——T47 修复了错位，现可登录。

### 4.2 WIL / WIX 图像格式（核心）
- **wix 位置表从偏移 28 开始**（头部 = szTitle[20] + nIndexCount[4] + 版本号[4]，与 `_NEW_VER=5000` 配对校验）。此前从 24 读会错位 1 项。
- 头结构 `NEWWILIMAGEINFO`：`shWidth(2) shHeight(2) shPX(2) shPY(2) bShadow(1) shShadowPX(2) shShadowPY(2) dwImageLength(4)` = 17 字节。
- `dwImageLength` 是 **WORD 数**，读取时 `new BYTE[len*2]` + `ReadFile(len*2)`。
- **RLE 行格式（WORD 视角）**：每行 = 1 个 WORD 行前缀（= 行内 WORD 数，不含前缀）+ 行内 RLE 码流：
  - `0xC0` + WORD cnt = 透明跳过 cnt 像素
  - `0xC1` + WORD cnt + cnt 个 WORD 像素 = 直接拷贝
  - `0xC2`/`0xC3` + WORD cnt + cnt 个 WORD 源色 = 用 ChooseColor 按源色亮度混合后写入
  - 其他值 = 未知码（DrawWithImageForComp 直接 return FALSE 整帧不画）
- 客户端两个绘制函数解码逻辑相同（WORD 行前缀）：
  - `DrawWithImageForComp`（WHDXGraphic.cpp:2543）— 底栏条/窗口内容/人物等用
  - `DrawWithImageForCompClipRgnBase`（:2900）— 按钮/消息框用（悬停正常）
  - `DrawWithImageForCompToMem`（:6101）— 解码到内存缓冲（缩放用）

### 4.3 GameInter.wil（2020 版 60MB）关键帧号（实证）
- **主界面底栏条 = 帧 50**（708x56，px=-24 py=-16，pos=968 有效）；帧 51 为空。
- 主界面 8 按钮 = 帧 81-95（28x28 有效）；帧 96 空。
- 新消息按钮 331/332/330；头像按钮 2096/2097/2098。
- 魔法窗口：帧 711-718 = 304x600 技能类型页背景（火/冰/雷/风/神圣/暗影/幻影/无）；帧 710 = 512x512（窗框候选）；帧 720-735 = 36x32 类型 tab；帧 721+2*type = 选中 tab；帧 1221/1222 = 关闭按钮；帧 1659+key = 快捷键数字。
- 规律：多数界面背景"偶数帧有效、奇数帧空"（MAIN 50 有效/51 空、INVENTORY 350/351、EXCHANGE 370/371、NEW_QUEST 1420/1421、NEW_HORSE 1400/1401、SIEGE 690/691、USERMARKET 760/761、USERMARKETUP 780/781）。
- 位置表（节选）：50=968, 51=0, 80=157389, 81=159144, 95=183840, 96=0, 330=4948710, 331=4952483, 332=4956256, 420=7525751, 421=7663464, 711=11095941, 712=11465562, 720=14059069, 721=14061586, 1221=?, 2096=20591544, 2097=20619875, 2098=20648352。

### 4.4 渲染路径知识
- `g_bTextrRistricted = CehckTextrRistricted()`（GFun.cpp:141）：显卡纹理限制（SQUAREONLY 或 MaxTexture<=256）才 TRUE；现代显卡 = **FALSE**。
- `ShowGameWnd`（GameWnd.cpp:70）：`g_bTextrRistricted=TRUE` → 软件 `DrawWithImageForCompClipRgn`；FALSE → **3D `DrawBillBoard`**（`GetTextrImg(_TEXTR_FILE_INTER, _IMAGE_INTER, m_nFrameImgIdx)` 纹理）。
- `CreateGameWnd`（GameWnd.cpp:17）的 `m_rcImgWnd` 以 (nStartX, nStartY) 为中心按**帧实际尺寸**扩展：`nStartX - (帧w - 窗口w)/2`。当帧比窗口高（如魔法窗口帧711=304x600 vs 窗口360x488）时背景会上偏。
- 主界面底栏条：`CreateInterface`（Interface.cpp:281）`NewSetIndex(_WNDIMGIDX_MAIN=50)` + `RenderMain`（Interface.cpp:2002）`DrawWithImageForComp(m_rcMain.left+94, m_rcMain.top=544, 708, 56, data)`。m_rcMain=(0,544,708,600)。
- 主界面 8 按钮：`CreateGameBtn(帧81-95, 帧-1, m_rcMain.left+524/558/592/626/660/694/728/762, top+16, ..., -1, TRUE)` —— 创建时 `nNormalImgIdx=-1` 平时不画，**悬停才画焦点帧**；平时按钮图形实际来自底栏条帧50内嵌的浮雕按钮。→ **底栏条帧50画不出＝按钮平时不可见**。
- 魔法窗口 `ShowMagicWnd`（MagicWnd.cpp:167）：`ShowGameWnd()` 画背景（帧711，DrawBillBoard 或软件，位置 m_rcImgWnd）→ 再 `NewSetIndex(711+m_bTypeMagic)` 软件画内容页在 (m_rcWnd.left+18, m_rcWnd.top+63-m_nStartPos) → **两帧重叠偏移＝"双份"嫌疑**（等 LogWnd.txt 实证）。

### 4.5 协议对比结论（支线，已完成）
- `D:\Mir3_Compare\protocol_diff.md`、`NOTES.md` 已提交 Gitea；对比过 github.com/a939927870/Mir3 复刻版与 2004 原版，结论：登录协议（SendRunLogin）已对齐正常客户端（抓包 1.apmx86），勿再改。

---

## 5. 已解决问题（里程碑，勿回退）

| 编号 | 内容 |
|---|---|
| T47 | SendRunLogin 登录协议修复（4字段明文）→ 能登录 |
| T57 | 道士帧号表恢复与正常版一致 |
| T59 | 进游戏黑屏修复（billboard 软件化）→ 能进图 |
| T60 | 选角"选中消失"诊断（特效层日志） |
| T67 | SVRSEL 黑屏诊断（状态+列表日志） |
| T69 | 全链路验证 + 崩溃复现 |
| T70/71 | SM_ABILITY exp/weight DWORD 布局修复 |
| T72→T73 | 22 个 WNDIMGIDX 宏曾 +1 改反 → 8 个关键宏回滚（MAIN 51→50 等），位置/血条恢复 |
| T73.5 | NPC 对话崩溃修复；双包裹修复；选角人物立绘渲染部分修复 |
| T74 | 底栏条/魔法窗口诊断日志（LogDraw/LogWnd），**待用户实测** |

---

## 6. 当前未解决问题清单（按优先级）

### P0（主线，MainAgent 负责，T74 诊断中）
1. **主界面底栏条/8 按钮平时不显示**（仅悬停焦点可见）。LogMain 证实 `NewSetIndex(50)` 成功、`RenderMain` 已执行、m_rcMain 正常；帧50 离线 WORD-RLE 解码有效（49.9% 像素）。嫌疑：DrawWithImageForComp 执行路径（Lock/解码/覆盖顺序）或 3D/软件混排覆盖。等 LogDraw.txt。
2. **魔法窗口出现双份**。嫌疑：ShowGameWnd 背景（帧711 画在 m_rcImgWnd 上偏）+ ShowMagicWnd 内容页（711+type 画在 (366,63)）两帧重叠偏移。等 LogWnd.txt。

### P1（分配给 Agent B）
3. **选角立绘"画了看不到"**：LogChr bright=2819 但 SCREEN 截图无立绘（画了但屏幕不可见，与底栏条疑似同类问题但独立文件）。文件：ChrProc.cpp / ChrSelProc.cpp（**不含 Interface.cpp**）。
4. **属性面板/查看角色数据崩溃族**：CrashLog `0x0051A23A / 2C6 / 33E / 5DA`、faultAddr=0x7D523FF8、EBX≈0x13C。文件：StatusWnd.cpp。
5. **怪物无阴影**：Actor 渲染阴影缺失。文件：Actor.cpp（阴影绘制部分）。

### P2（可顺带）
6. **清理冗长 log**：LogWIL.txt 已达 108MB（每帧 SetIdx 写盘）。保留排查所需，改为条件/节流写入或循环覆盖（**不得删除 WHWilImage.cpp 的 LoadC POS / SetIdx / FAIL 关键行**，排查还靠它；改节流即可）。

---

## 7. 任务分配（Agent B 专享；MainAgent 继续 P0）

> 规则：**每个任务的文件所有权唯一**。Agent B 只允许修改下表"分配文件"；MainAgent 只动 P0 文件。git 提交消息统一格式：`[ABxx] 任务名: 结论/修改摘要`。每次改动前先 `git pull`，改完 `git add -A && git commit && git push`。遇到阻塞/新发现，更新本文件"协作区"并提交。

### 提交前双审计规则（强制，用户要求）

**任何提交 push 前，必须同时满足：**
1. **提交者自查（必过）**：
   - 编译通过（`compiled 82 fail 0` + `LINK 0` + SUCCESS，windres 噪声除外）；
   - 改动文件全部在自己所有权清单内，未越界；
   - 改动点可解释（与正常对照端/参考源码逐行核对过，非猜想）；
   - 新增/改动日志已节流或明确保留理由，不会刷爆磁盘；
   - 无调试残留（临时变量/硬编码坐标/注释掉的旧代码大段残留）；
   - 不影响其他模块（尤其渲染主路径 DrawWithImageForComp / RenderMain / ShowGameWnd 的行为）。
2. **对方交叉审计（push 后 24h 内完成，重大提交必须）**：
   - 拉取对方分支/最新 master，检查 diff：文件边界、编译、改动逻辑与知识库结论一致性；
   - 在 `docs/AUDIT_LOG.md` 追加一行审计结论：`commit号 | 被审任务 | 审计人 | 结论(OK/驳回+理由)`；
   - 驳回时：审计人在 AUDIT_LOG 写明具体问题，提交者修复后重新提交再审；
   - 渲染核心改动（WHDXGraphic.cpp 函数体、Interface.cpp RenderMain、GameWnd.cpp ShowGameWnd、MagicWnd.cpp ShowMagicWnd）**未经对方审计 OK，不得视为合并完成**。

**审计记录文件**：`docs/AUDIT_LOG.md`（每个提交一行；无审计行=未过审）。

### Agent B 任务 AB01：选角立绘"画了看不到"
- **分配文件**：`ChrProc.cpp`、`ChrSelProc.cpp`（选角人物立绘/特效绘制路径）；如需加日志可加 `WHWilImage.cpp`（但**只加日志不删行**，且日志文件独立命名如 LogAB01.txt）
- **已知线索**：选角界面人物立绘调用（ChrProc.cpp 约 483 行）`DrawWithImageForComp`；LogChr.txt 显示 bright 计数 2819 但屏幕无立绘。对比正常端风神 DX7 同函数参数。
- **验收**：选角界面 6 职业立绘全部可见且非黑、选中特效正常。
- **禁止**：改 Interface.cpp / WHDXGraphic.cpp 函数本体（MainAgent 的 P0 也在用）；如需它们配合，把需求写进协作区。

### Agent B 任务 AB02：属性面板崩溃族
- **分配文件**：`StatusWnd.cpp`
- **已知线索**：点"查看角色数据"崩溃；CrashLog 多次 `0x0051A23A/2C6/33E/5DA`、`faultAddr=0x7D523FF8`、EBX≈0x13C。0x13C 疑似某结构成员偏移（如装备/属性指针）。对照 DX7 源码 StatusWnd.cpp 的渲染与数据读取。
- **验收**：点开属性面板不崩溃，属性数值显示正确。

### Agent B 任务 AB03：怪物无阴影
- **分配文件**：`Actor.cpp`（阴影绘制相关函数）
- **验收**：怪物/动物在地面有阴影（对照正常端）。

### Agent B 任务 AB04：log 节流清理
- **分配文件**：`WHWilImage.cpp`（仅日志节流，不动读取逻辑）、`Mir2Ex.cpp`（CrashLog 保留）
- **要求**：LogWIL.txt 108MB 问题：SetIdx/LoadC 日志改为"每 30 帧记录一次"或首次+异常才记录；保留 FAIL/WIXFAIL/VERFAIL 等错误行；LogDraw/LogWnd 由 MainAgent 决定是否保留。**必须保证排查关键日志可用**。

### MainAgent（Doubao）继续
- **P0-1 底栏条**：文件 `WHDXGraphic.cpp`（DrawWithImageForComp 函数体）、`Interface.cpp`（RenderMain）。
- **P0-2 魔法窗口双份**：文件 `MagicWnd.cpp`、`GameWnd.cpp`。
- T74 日志版已部署（2cf27d9），等用户测试 LogDraw/LogWnd.txt 后定案修复。

---

## 8. 协作区（双向同步）

> 各 agent 每次提交后，在此追加一行：`时间 | 任务 | 结论/进度 | commit号`。

- 2026-10-05 13:45 | T74 | 部署底栏条+魔法窗口诊断日志版（LogDraw/LogWnd），等待用户实测 | 2cf27d9
- 2026-10-05 | 本文档 | 知识库+问题清单+任务分配建立 | (本提交)
- 2026-10-05 | AB04 | 日志节流：SetIdx 改为首次+每30次记录（唯一每帧无条件写盘点），FAIL/WIXFAIL/VERFAIL/LoadC POS 全保留；编译82fail0+LINK0 | dd070af
- 2026-10-05 | AB01 | 立绘根因=共享解码器 DrawWithImageForComp nxt off-by-one(下一行前缀当RLE码→BADCODE→整帧不画)，已由 MainAgent T77(77a5816) 修复；ChrProc.cpp 立绘路径(456-515行)已与参考 Mir3China 逐行对照一致，无 ChrProc 独立缺陷，**无需改 ChrProc.cpp**。特效层用 base DrawWithABlendCompDataWithBackBuffer(索引式行前缀, 经追踪 nWidthEnd++ 计入前缀词, 逻辑正确, 非T77未修残bug)。**待用户实测** | 审计见 AUDIT_LOG
- **【请测试-AB01】请登录进选角界面，确认 6 职业立绘是否全部可见且非黑、选中(点击角色)后特效层是否正常。当前 D:\Client\Mir3_dx9.exe 为含 T77 根因修复+AB04 节流的构建** | (待用户反馈)

---

- 2026-10-05 | AB02 | 取证结论: 任务书4地址(0x51A23A/2C6/33E/5DA)均为**栈dword非故障EIP**(disasm实证: 0x51a23a/2c6落在6字节fld与4字节mov指令中间, 非指令边界); StatusWnd.cpp唯一浮点格式=1409行`%.2f`(格式参数匹配正确, `%`是指针%s正常打印, 不会触发pformat崩溃, 之前的pformat吻合判断被否); Item.cpp无浮点/无ShowItemStatus; 当前CrashLog实证=**gdi32文本渲染崩溃**(0x76F2DCB8, 系统DLL), PutsHan(WHDXGraphic.cpp:992)走GDI GetDC->SelectObject->TextOut(1030), 失效HDC/字体/设备重置后backbuffer失效.**StatusWnd.cpp无需改动(无确认缺陷), 崩溃根因移交MainAgent复核WHDXGraphic.cpp文本路径**. 需用户用当前构建x64dbg复现捕获真实EIP+faultAddr+调用链 | (待复核)
- 2026-10-05 | AB03 | 取证: Actor.cpp怪物阴影逻辑(CActor::DrawActor 5190-5270)与参考Mir3China**代码逐行一致**(精确diff仅注释乱码差异), 调用方对角色/NPC/怪物统一bDrawShadow=TRUE; 共享阴影函数按KB4.2 RLE解码正确、backbuffer Lock正确、_BE16未用. **Actor.cpp无需改动(无确认缺陷)**, 阴影缺失指向共享WHDXGraphic阴影渲染或阴影图数据(加载/bShadowType), 移交MainAgent. **需用户实测**: 玩家/NPC/怪物三者阴影是否皆无还是仅怪物(判定共享vs怪物专属), 附截图最佳. 正常端反汇编存D:\Ada-WorkSpace\normal_client_disasm.txt | (待实测)
- 2026-10-05 | AB03-实测 | 用户实测当前构建: **怪物/玩家/NPC 三者皆有地面阴影, AB03 验收已满足, 不可复现**; 与静态取证一致(Actor.cpp阴影逻辑与参考逐行一致). AB03关闭, Actor.cpp无需改动, 无需移交. 此前'无阴影'报告应来自更早构建(阴影T28有反复史, 疑已随后续构建修复) | d45c779
*文档维护：所有 agent 均可追加，但不得删除他人的已确认结论；数据/帧号以实测为准，禁止猜想。*






---

## T79-T80 进展 (MainAgent, 2026-10-05 15:00)
- **[T79 已部署已验证] 魔法窗口双份修复**: _WNDIMGIDX_MAGIC 711->710。LogWnd实证: ShowGameWnd背景frame=711(304x600@376,-56) 与 MagicWnd内容页 711+type=0(304x331@366,63) 撞同一帧=双份根因(背景帧711是类型页背景, 窗口背景应为710窗框)。离线验证帧710=512x512窗框有效(px=7 py=-44)、711=304x600类型页背景。用户确认魔法窗口正常。commit 463ec17(已合并AB提交后1de9ee4推送)。
- **[T80 部署中] 选角界面软缓冲dump**: LogChr显示人物/特效全部绘制成功(CHR-VERIFY bright=2044/2286, 道士1461@285,268彩色; 法师801@383,302灰色; EFF+20=影子1481/821/1544暗红棕0x20A2)。SelChrDump_T80.bmp(640x480)实证软缓冲含彩色人物立绘(蓝绿道士服)——绘制链正常, 无"画了不上屏"问题。13:55旧版截屏无人物疑为旧版遗留/截图时机, 待用户确认当前屏幕显示。
- **选角+20帧真相**: Interface1c.wil 道士区 1450-1479彩色人物/1480-1496单色0x20A2影子/1520-1539彩色/1540-1559影子; +20=影子(暗红棕剪影, 非黑渲染bug), +40=空帧(道士区)。正常版共用同一WIL → 正常版特效同样为暗色影子。用户所见"黑色魔法特效"=影子50%混合暗背景, 非缺陷。
- **审计AB02通过**: PutsHan GDI路径 GetDC失败未检查(1443行FAILED检查被注释)+窗口恢复HDC失效 → gdi32崩溃, 结论成立。建议后续加GetDC失败保护(未改, 避免干扰当前测试)。
- **AB03已合并**: 怪物阴影共享函数取证完成, 用户实测三态阴影OK, 关闭。
---

## 【重要】协作交流迁移公告 (MainAgent, 2026-10-05)
**此后 MainAgent 与 admin 的任务交流、进展同步、审计登记全部改用 Gitea Issues 工单**：
- Issues 页面: http://192.0.2.10:8418/mir3-dev/mir3-dx9/issues
- 当前协作工单: #3（迁移公告 + 当前状态快照 + 在办任务 T81）
- 规则: 新任务/进展/审计结论 → 开 issue 或评论对应 issue；git 提交信息带 issue 编号；提交前自查+互相审计（审计结果评论到 issue）。
- 本文档仅保留为静态知识库（不用于动态交流）。