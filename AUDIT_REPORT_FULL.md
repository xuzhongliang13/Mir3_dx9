# MIR3-DX9 全量代码审计报告（上线标准）

> 审计对象：`D:\MIR3-DX9`（174 个源文件 / 约 10.4 万行）
> 审计维度：安全、渲染、特效、功能、崩溃、指针、内存、性能
> 审计方式：全树机械化扫描（正则覆盖所有 `.cpp/.h`）+ 高危点逐行人工核实
> 报告日期：2026-10-17

---

## 零、结论先行

**当前版本不具备上线标准。** 存在 **4 个阻断级问题**（Blocking）：2 个可远程/数据触发的高危缺陷、
1 类格式化字符串漏洞、1 个决定帧率上限的性能瓶颈。按本文档 §十一 的清单全部关闭后，
代码质量可达到"可上线"水平。

| 严重度 | 数量 | 说明 |
|---|---|---|
| **Blocker（阻断上线）** | 4 | 必须修复，否则有崩溃/安全问题 |
| **High（高）** | 6 | 上线前应修复 |
| **Medium（中）** | 9 | 建议修复，或明确接受风险 |
| **Low（低）** | 5 | 可排期 |

---

## 一、审计范围与方法

### 1.1 覆盖范围

| 目录 | 说明 |
|---|---|
| `Common\` | 网络、编解码、消息表、图像、字符串分割、截图 |
| `GameProcess\` | 游戏主逻辑、角色、地图、界面、魔法、粒子、NPC |
| `LoginProcess\` `SelLoginProcess\` `CharSelectProcess\` | 登录 / 选服 / 选角 |
| `WindHorn\` | 渲染窗口、软表面、DX9 呈现、D3D 工具 |
| `Sound\` `AVI\` `OrzLib\` | 声音、视频、容器库 |
| 根目录 | 命令行、全局函数、定义、资源 |

### 1.2 机械化扫描统计

| 项目 | 数量 | 风险含义 |
|---|---|---|
| `sprintf` | **963** | 无边界写入，固定缓冲溢出主因 |
| `strcpy` / `strcat` | **327** | 无长度限制拷贝 |
| `ReadFile` | **74** | **全部未检查返回值**（本报告 C2） |
| `PutsHan`（文字绘制） | **704** | 每次触发整屏 DC 拷贝（本报告 P1） |
| `CreateGameFont` | **83** | 多处每帧创建字体（本报告 P2） |
| `new[]` | 64 | 与 `delete[]` 基本配对，但有例外（M2） |
| `malloc` | 6 | 存在 1 处 `delete[]` 误配（已修，见 R3） |
| 数据文件内容作格式串 | **41** | 格式化字符串漏洞（本报告 S1） |

---

## 二、安全（Security）

### S1【Blocker】数据文件内容被当作 printf 格式串 — 41 处

`CMsg::GetMsg()` 直接返回 `CMList.Dat` 里的原始文本：

```cpp
// Common/Msg.cpp:18-36
char* CMsg::GetMsg(int nIdx)
{
    stMsgNode *pObj = FindMsg(nIdx);
    if ( pObj ) return pObj->pszMsg;      // ← 直接返回数据文件内容
    static char szNullMsg[2] = "";
    return szNullMsg;
}
```

而大量调用点把它当**格式串**使用：

| 位置 | 代码 | 风险 |
|---|---|---|
| `GameProcess\GameOverWnd.cpp:64` | `sprintf(pszTmp, g_xMsg.GetMsg(3500))` | **无参数**：消息含 `%s`/`%n` → 读/写任意地址 |
| `GameProcess\GameOverWnd.cpp:67,71` | 同上（3501/3502） | 同上 |
| `GameProcess\Interface.cpp:3107,3110,3113` | `sprintf(szInfo, g_xMsg.GetMsg(2651..2653))` | 同上 |
| `GameProcess\OptionWnd.cpp:431,435,439,443` | `sprintf(pszTmp, g_xMsg.GetMsg(2104..2107))` | 同上 |
| `GameProcess\GameProc.cpp:802,807` | `sprintf(pszDivided, g_xMsg.GetMsg(1403/1404))` | 同上 |
| `GameProcess\BeltWnd.cpp:252,488` | `sprintf(szSysMsg, g_xMsg.GetMsg(1066), nBeltInventoryNum+1)` | 格式与实参**类型不匹配** → 栈破坏 |
| `GameProcess\BeltWnd.cpp:532,545` | `GetMsg(1067)/(1068)` + 字符串实参 | 同上 |
| `CmdLineParser.cpp:157` | `sprintf(szBuf, g_xMsg.GetMsg(10000), nRefreshRate, __recommend)` | 同上 |
| `SelLoginProcess\SelLoginProc.cpp:747` | `sprintf(pszLogin, szData)` | **格式串来自 SelLogin.txt** |
| `SelLoginProcess\SelLoginProc.cpp:750` | `sprintf(pObj->szLoginSectionName, pszLogin)` | 同上，二次污染 |

**危害**：`%n` 可实现任意内存写 → 远程/本地代码执行；`%s` 误用 → 立即崩溃；
类型不匹配（给 `%s` 传 `int`）→ 把整数当地址解引用 → 崩溃。

**修复**（全局统一）
```cpp
// 无参数：一律改成 "%s"
sprintf(dst, "%s", g_xMsg.GetMsg(N));
// 或更安全：
_snprintf(dst, sizeof(dst), "%s", g_xMsg.GetMsg(N));

// 有参数：先校验格式串里 % 的数量与类型
// 建议在 CMsg::GetMsg 出口处做一次白名单过滤，或提供专门的
// GetMsgFormat(N, ...) 接口，内部用 vsnprintf 并限制输出长度。
```
**工作量**：约 41 处，机械替换 0.5 天；`GetMsgFormat` 封装 0.5 天。

---

### S2【High】账号口令明文进入协议缓冲

```cpp
// Common/ClientSocket.cpp:130
wsprintf(szLoginInfo, "**%s/%s/%d/%d/%d/1/%d", szLoginID, szCharName, ...);
// Common/ClientSocket.cpp:569
wsprintf(szPacket, "%s/%s", pszID, pszPassword);            // 改密
// Common/ClientSocket.cpp:588
wsprintf(szPacket, "%s%c%s%c%s", pszID, cDivider, pszOldPassword, cDivider, pszNewPassword);
```

口令以明文拼进报文字符串，仅经自研 6-bit 编码（`fnEncode6BitBuf`）——
**这是编码不是加密**，抓包即可还原。

**说明**：这是原版协议固有设计，改动会破坏与服务端兼容。
若要满足"上线标准"，需服务端配合升级为 TLS 或挑战-响应（CHAP 类）。
**若暂时无法改协议**，必须在其他层面补偿：登录网关限速、异常登录告警、口令强度策略。

---

### S3【Medium】硬编码地址与凭据

| 位置 | 内容 |
|---|---|
| `Define.h:2589` | `#define _LOGIN_GATE_SERVER_IP "192.0.2.10"` |
| `CmdLineParser.cpp:99` | `strcpy(g_stInfoShop.pszAddr, _KORNET_IP)` |
| `LoginProcess\LoginReg.cpp:20` | 默认 `127.0.0.1` |
| `Define.h:2653/2654` | 硬编码 `account.wemade.com` 建号/改密地址 |

**修复**：内网地址不应进入发布版；全部外置到 `Mir3.ini` 并做默认值校验。

---

### S4【Medium】用户可控内容参与文件路径拼接

```cpp
// GameProcess/FaceImgUp.cpp:288
sprintf(szCryptName, "%s\\Data\\%s_%s.fac", szCryptName, g_szServerName, g_xGameProc.m_xMyHero.m_szName);
// :324  同上
// :413
sprintf(szFileName, "%s\\%s", szFileName, pstChat->pszChat);
```

`g_szServerName`（服务器下发）与 `m_szName`（角色名）进入文件名。
若其中含 `..\`、`/`、`:` → **路径穿越**，可在任意可写目录落文件。

**修复**：拼接前过滤 `\ / : * ? " < > |` 与 `..`，并校验最终路径前缀必须是 `.\Data\`。

---

### S5【Medium】`szKey[16]` 接收无长度限制的 `sscanf("%s")`

```cpp
// Common/Msg.cpp:90,120,123
CHAR  pszMsgNum[20];
sscanf(pszLine, "%s", pszMsgNum);
strcpy(pObj->szKey, pszMsgNum);      // szKey 只有 16 字节
```

消息编号超过 15 字符即**栈/堆溢出**。CMList.Dat 可被更新包替换 → 可构造触发。

**修复**：`sscanf(pszLine, "%19s", pszMsgNum)` + `strncpy(pObj->szKey, pszMsgNum, sizeof(pObj->szKey)-1)`。

---

## 三、崩溃与指针（Crash / Pointer）

### C1【Blocker】消息表缓冲区缺少 NUL 终止符 → 堆越界读

```cpp
// Common/Msg.cpp:68-95
int len = _filelength(_fileno(fp));
pszBuf = new char[len];                    // ← 只分配 len，无 '\0' 空间
fread(pszBuf, 1, len, fp);                 // ← 读满 len 字节
...
CHAR* pszLine = pszBuf + 8;
while ( pszLineNext = (char*)strchr(pszLine, '\r') )   // ← 无终止符则扫穿堆块
```

若 `CMList.Dat` 最后一个 `\r` 之后没有更多内容，`strchr` 会继续扫描**堆块之外**的内存，
直到偶然遇到 `\r` 或跨入不可读页 → **0xC0000005 随机崩溃**（且崩点与根因相隔很远，极难定位）。

**同时存在同一模式的其它位置**（均需加 `+1` 与置零）：
`Msg.cpp:164`、`GFun.cpp:217`、`FaceImgUp.cpp:237`、`Interface.cpp:7602`、`MagicWnd.cpp:85`。

**修复**
```cpp
pszBuf = new char[len + 1];
...
fread(pszBuf, 1, len, fp);
pszBuf[len] = '\0';        // ← 关键
```
**工作量**：6 处，1 小时。**这是本次审计中性价比最高的一处修复。**

---

### C2【Blocker】外部文件读取全部未做校验（74 处 `ReadFile`）

**代表位置：地图加载**

```cpp
// GameProcess/MapHandler.cpp:340-345
ReadFile(hFile, &m_stMapFileHeader, sizeof(MAPFILEHEADER), &dwReadLen, NULL);
m_pstTileInfo = new TILEINFO[(m_stMapFileHeader.shWidth*m_stMapFileHeader.shHeight)/4];   // 无上限校验
ReadFile(hFile, m_pstTileInfo, sizeof(TILEINFO)*(shWidth*shHeight)/4, &dwReadLen, NULL);  // 未检查 dwReadLen
m_pstCellInfo = new CELLINFO[m_stMapFileHeader.shWidth*m_stMapFileHeader.shHeight];       // 无上限校验
ReadFile(hFile, m_pstCellInfo, sizeof(CELLINFO)*(shWidth*shHeight), &dwReadLen, NULL);    // 未检查 dwReadLen
```

**危害链**
1. 地图文件被截断/损坏/恶意构造 → `shWidth*shHeight` 为 0 → `new [0]` → 后续按坐标索引 **越界写**
2. 数值极大 → 分配几百 MB~GB → `std::bad_alloc` **未捕获** → 进程终止
3. `ReadFile` 未读满 → 缓冲区**保留未初始化数据**，直接参与瓦片/格子渲染 → 随机崩溃
4. `dwReadLen` 完全未使用 → 无法发现损坏

**同类问题**：`Interface.cpp:7060-7063`（背包存档）、`MagicShortcutWnd.cpp:246-247`（技能快捷键存档）、
`GameProc.cpp` 多处（角色数据）。

**修复模板**
```cpp
if ( !ReadFile(hFile, &hdr, sizeof(hdr), &dwReadLen, NULL) || dwReadLen != sizeof(hdr) ) { /* 错误处理 */ }
if ( hdr.shWidth == 0 || hdr.shHeight == 0 ||
     hdr.shWidth > _MAX_MAP_WIDTH || hdr.shHeight > _MAX_MAP_HEIGHT ) { /* 拒绝 */ }
SIZE_T need = (SIZE_T)hdr.shWidth * hdr.shHeight;
if ( need == 0 || need > _MAX_MAP_CELLS ) { /* 拒绝 */ }
m_pstTileInfo = new (std::nothrow) TILEINFO[need / 4];
if ( !m_pstTileInfo ) { /* 内存不足处理 */ }
if ( !ReadFile(hFile, m_pstTileInfo, sizeof(TILEINFO)*(need/4), &dwReadLen, NULL)
     || dwReadLen != sizeof(TILEINFO)*(need/4) ) { /* 拒绝 */ }
```
**工作量**：地图 + 存档 + 配置三类约 20 个关键读取点，2~3 天。

---

### C3【High】`StringSplitter` 在空串时 `memcpy` 负长度 → 堆溢出

```cpp
// Common/StringSplitter.cpp:83-91
if ( sizeLen.cx >= m_nLineWidth || pBuffer[nCnt] == '\0' )
{
    if ( bTwoByteUsing == TRUE )
    {
        m_pLineArray[ m_nLineCount ] = new char[ nCnt ];        // nCnt 可能为 0
        memset( m_pLineArray[ m_nLineCount ], 0, nCnt );
        memcpy( m_pLineArray[ m_nLineCount ], szTemp, nCnt - 1 );   // ← nCnt=0 时是 (size_t)-1
        m_pLineArray[ m_nLineCount ][ nCnt - 1] = '\0';             // ← 越界写
```

`nCnt == 0` 时 `nCnt-1` 转 `size_t` 变成 `0xFFFFFFFF` → `memcpy` 试图拷贝 4GB → **立即崩溃**。
聊天窗口对空消息/特殊字符调用该分割器即可触发。

**修复**：分支入口加 `if ( nCnt <= 0 ) break;`，并把 `nCnt-1` 改为显式安全计算。

---

### C4【Medium】定长 `memcpy` 读取变长字符串 → 越界读 + 未初始化内存

```cpp
// GameProcess/Interface.cpp:1467-1471
CHAR pszShow[64];
memcpy(pszShow, pstChat->pszChat, 36);   // 源串可能不足 36 字节 → 读越界
sprintf(&pszShow[36], "...");
pszShow[63] = '\0';                      // 但 pszShow[36..62] 未初始化 → 渲染脏数据
```

**修复**
```cpp
char pszShow[64];
_snprintf(pszShow, sizeof(pszShow), "%.36s...", pstChat->pszChat);
```

---

### C5【Medium】`PutsHan` 不检查 `szText` 为空

```cpp
// WindHorn/WHDXGraphic.cpp:1031
TextOut(hDC, nX, nY, szText, strlen(szText));   // szText == NULL → 立即崩溃
```
704 个调用点，只要有一处传 NULL 就崩。**修复**：函数入口 `if ( !szText ) return;`。

---

## 四、内存（Memory）

### M1【Medium】6 处 `new char[len]` 无终止符

`Msg.cpp:69,164`、`GFun.cpp:217`、`FaceImgUp.cpp:237`、`Interface.cpp:7602`、`MagicWnd.cpp:85`
—— 与 C1 同源，修复方式一致（`+1` 并置零）。

### M2【Low】内存过度分配（2 倍）

| 位置 | 代码 | 实际需要 |
|---|---|---|
| `FaceImgUp.cpp:448` | `new WORD[sizeof(WORD) * nWidth * nHeight]` | `nWidth*nHeight` |
| `Interface.cpp:7659` | 同上 | 同上 |
| `ScreenCapture.cpp:159` | `new WORD[ddsd.dwWidth * ddsd.dwHeight * 2]` | `w*h` |

不致命，但人脸/截图在内存紧张时会放大一倍占用。

### M3【Medium】`sprintf` / `strcpy` 无边界（963 / 327 处）

固定缓冲（多为 `CHAR szBuff[MAX_PATH]`）+ 无长度限制。**触发路径**：
- 长物品名（服务器下发）、长角色名/行会名（玩家输入）
- 长聊天文本进入 `szSysMsg`、`pszTmp`
- 长服务器名进入 `FaceImgUp` 路径

**修复**：优先处理"数据来源不可信"的调用点（约 80 处），统一改为
`_snprintf(dst, sizeof(dst), ...)` / `strncpy_s` 语义封装。**工作量**：2~3 天。

### M4【Low】`SAFE_DELETEARR` / `delete[]` 配对基本正确

抽样 24 处 `delete`（非数组）与 54 处 `delete[]`，未发现新的误配
（此前已修的 `CDX9SoftSurface::Release()` 用 `delete[]` 释放 `malloc` 内存的问题已关闭，见 R3）。

---

## 五、渲染（Rendering）

### R1【Medium】`m_pFont`（D3DXFont）创建后从未使用

```cpp
// WindHorn/WHDXGraphic.cpp:659-661
if ( FAILED(D3DXCreateFontA(m_pd3dDevice, 14, 0, FW_NORMAL, 1, FALSE, ..., "System", &m_pFont)) )
    m_pFont = NULL;
```
全库无一处 `DrawText` 调用 —— 文字实际走 GDI（`PutsHan`）。
设备丢失重建路径（`:6846-6855`）也不重建该字体。**建议删除**。

### R2【Medium】`Present` 每帧全屏 565→32bit 软件转换

```cpp
// WHDXGraphic.cpp:6863-6879
m_pd3dPresentTex->LockRect(0, &lr, NULL, D3DLOCK_DISCARD);   // 已修：原为 0
for (y) for (x) { 565 -> 8888 展开 }                          // 48 万像素纯标量循环
```
可优化：改用 `D3DFMT_R5G6B5` 纹理直接上传（免转换），或 SSE2 批量展开。

### R3【已修复】软表面释放与 Blt 语义

已在本轮前序修复：
- `CDX9SoftSurface::Release()` 不再对 `malloc` 内存用 `delete[]`
- `Blt` 补 `DDBLT_KEYSRC` 色键与 COLORFILL 的 `prcDst` 裁剪
- `Present` 的 `LockRect` 加 `D3DLOCK_DISCARD`
- `GetBackBuffer()` 返回真实 DDraw 表面，不再把 `CSoftSurface*` 当 COM 接口
- `CehckTextrRistricted()` 补空设备判断

### R4【Low】绘制调用规模

`DrawWithImageForComp` 315 处 + `DrawWithGDI` 141 处 + `PutsHan` 704 处，
全部 565 软件合成。**这是本项目架构的既定成本**，无法通过局部优化消除，
只能靠减少调用次数（字体缓存 P2）与整屏拷贝（P1）来改善。

---

## 六、特效（Effects）

### E1【Low】`DrawBillBoard` 每次锁定纹理

```cpp
// Common/ImageHandler.cpp:477-481
lpddsTextr->GetLevelDesc(0, &sd);
lpddsTextr->LockRect(0, &lr, NULL, D3DLOCK_READONLY);
```
WIL 纹理是 `D3DPOOL_MANAGED`，读的是系统内存副本，**不是显存回读**，成本可接受。
但可加一层 CPU 像素缓存（按纹理指针 + 尺寸做 key），把重复锁降到一次。

### E2【Low】粒子数量与混合

雪 250 / 雨 200 的规模下，`DrawBillBoard` 的逐像素 32→16 转换是主要成本。
混合模式 `_BLEND_LIGHT` / `_BLEND_LIGHTINV` 已正确实现（本轮前序修复）。

### E3【Info】天气与昼夜现状

- 雨/雪/雾：走软缓冲，可见，参数已对齐原版 DX7
- 昼夜：按要求**已停用**（`LightFog.cpp` 中 `bDayNightEnabled = FALSE`）

---

## 七、功能（Functionality）

| 编号 | 严重度 | 问题 | 状态 |
|---|---|---|---|
| F1 | Medium | 属性窗口悬停提示：`for(INT nI)` 遮蔽导致 `switch(nI)` 恒为 0，只有"等级"能显示 | **已修复** |
| F2 | Low | 登录界面公告文字多行重叠 | 未解决 |
| F3 | Low | 属性提示后 4 项为内置推断文字（`CMList.Dat` 缺对应消息） | 需补数据 |
| F4 | Low | `LoginReg.cpp` 的 `DoCreateAccountDialog` 已无调用（死代码） | 可删 |
| F5 | Info | 登录界面"卡密充值"已删除，"创建账号/密码修改"恢复为打开网页 | **已完成** |

---

## 八、性能（Performance）

### P1【Blocker／性能上限】`GetDC` / `ReleaseDC` 每次整屏拷贝

```cpp
// WindHorn/WHDXGraphic.h:101-138（CDX9SoftSurface）
HRESULT GetDC(HDC* phDC){
    ...
    for ( LONG y=0; y<m_dwHeight; y++ )
        memcpy( dibRow, srcRow, m_dwWidth*2 );      // 整屏 800x600 = 960KB
}
HRESULT ReleaseDC(HDC hDC){
    for ( LONG y=0; y<m_dwHeight; y++ )
        memcpy( srcRow, dibRow, m_dwWidth*2 );      // 再拷一次 960KB
}
```

**每调用一次 `PutsHan` 或 `DrawWithGDI`，就是 1.92MB 的 memcpy。**
全库 `PutsHan` 704 处、`DrawWithGDI` 141 处，保守估计每帧 400+ 次
→ **约 770 MB/帧 的内存带宽**。这是当前客户端帧率的硬上限，
也是为什么需要把粒子数从 250/200 降下来。

**修复方案（按收益排序）**
1. **字体/文字批处理**：把一帧内所有 `TextOut` 收集起来，DC 只在帧首尾各同步一次
   （`GetDC` 一次 → 全部 `TextOut` → `ReleaseDC` 一次）。收益：10~50 倍。
2. **脏矩形**：只同步被 GDI 修改的矩形区域，而非整屏。
3. **取消 DIB 中转**：为 `PutsHan` 实现自绘字形（字形位图缓存），完全不依赖 GDI/DC。

**工作量**：方案 1 约 3~4 天（改动集中在 `CDX9SoftSurface` + `PutsHan` 调用包装），
预计帧率提升 5~10 倍。

### P2【High】每帧创建 GDI 字体（83 处 `CreateGameFont`）

多处直接内联在 `PutsHan` 参数里：

```cpp
// GameProcess/Actor.cpp:5450 / 6398 / 10857
g_xMainWnd.PutsHan(NULL, rcHPText, RGB(250,250,250), RGB(0,0,0), szHP,
                   g_xMainWnd.CreateGameFont(g_xMsg.GetMsg(1074), 7, 0, FW_NORMAL));
// CharSelectProcess/ChrProc.cpp:921,922,923,927,1114,1117,1120,1123 同类
```

而 `PutsHan` 内部会 `DeleteObject(hFont)`（`WHDXGraphic.cpp:1035-1039`）
→ **每个角色每帧创建并销毁若干 GDI 字体**。GDI 句柄抖动、字体枚举、DC 选入/选出，
成本远高于文字绘制本身。

**修复**：按 `(字号, 粗体, 斜体, 字符集)` 建一个 `HFONT` 缓存表（进程级，
最多几十个），`CreateGameFont` 改为查表；`PutsHan` 不再删除非默认字体。
**工作量**：1 天。**收益：显著**（同时消除 GDI 句柄耗尽风险）。

### P3【Medium】描边文字 = 4 倍成本

多处用"4 次偏移绘制"实现文字描边（`Interface.cpp:1462-1463` 等），
每次都是一次完整 `PutsHan`（含整屏 DC 拷贝）。修完 P1/P2 后此项自然缓解。

---

## 九、指针与返回值

| 检查项 | 结果 |
|---|---|
| `Get3DDevice()->` 链式解引用 | 0 处（此前已修 `CehckTextrRistricted`） |
| `new` 后判空 | 57 处（不足；`new` 失败抛异常而非返回 NULL，实际作用有限） |
| `malloc` 后判空 | 3 / 6 处 |
| `ReadFile` 返回值检查 | **0 / 74 处**（见 C2） |
| `Lock`/`LockRect` 返回值检查 | 大部分已检查 |
| 接口 C 风格强转 | 30 处（`(LPDIRECTDRAW*)` 等），需逐个确认非跨类型强转 |

---

## 十、上线达标清单（Gating Checklist）

**必须全部关闭才能上线：**

- [ ] **S1** 41 处格式化字符串 → 改为 `"%s"` 或封装 `GetMsgFormat`
- [ ] **C1** 6 处 `new char[len]` → `+1` 并置 `'\0'`
- [ ] **C2** 74 处 `ReadFile` 返回值检查 + 尺寸/上限校验（至少覆盖地图、存档、配置）
- [ ] **C3** `StringSplitter` 空串负长度拷贝
- [ ] **P1** `GetDC/ReleaseDC` 批量化（否则帧率不达标）
- [ ] **P2** `HFONT` 缓存（消除每帧建字体）

**强烈建议关闭：**

- [ ] **S2** 口令明文（需服务端配合；否则补充网关限速与告警）
- [ ] **S3** 移除内网硬编码地址
- [ ] **S4** 文件路径输入过滤
- [ ] **S5** `sszKey` 拷贝加长度
- [ ] **C4** `memcpy(...,36)` 改安全格式化
- [ ] **C5** `PutsHan` 空指针检查
- [ ] **M2** 三处 2 倍过度分配
- [ ] **M3** 不可信数据源的 `sprintf/strcpy`（约 80 处）

**可排期：**

- [ ] **R1** 删除未使用的 `m_pFont`
- [ ] **R2** Present 改 `R5G6B5` 直传 / SSE2
- [ ] **F2** 公告文字重叠
- [ ] **F3/F4** 补充消息数据、删除死代码
- [ ] **E1** 纹理 CPU 像素缓存

---

## 十一、修复优先级与工期估算

| 阶段 | 内容 | 工期 | 收益 |
|---|---|---|---|
| **P0（阻断）** | C1 + S1 + C3 + C5 | 1.5 天 | 消除可触发的崩溃与格式串漏洞 |
| **P0（性能）** | P2 字体缓存 + P1 DC 批量化 | 4~5 天 | 帧率提升 5~10 倍 |
| **P1（健壮）** | C2 文件读取校验 + M1 + M3（不可信源） | 4 天 | 消除损坏文件/恶意数据导致的崩溃 |
| **P1（安全）** | S3 + S4 + S5 + C4 | 1.5 天 | 补齐输入校验 |
| **P2（清理）** | R1 + R2 + M2 + F2/F4 | 2 天 | 资源与可维护性 |
| **协议升级** | S2（需服务端） | 另议 | 真正的传输安全 |

**合计约 13~14 人日**（不含协议升级）可达"可上线"标准。

---

## 十二、附录：扫描数据摘要

```
扫描文件           174 个（全部 .cpp/.h，排除 build_dx9）
总行数             ≈ 104,000
sprintf            963
strcpy / strcat    327
ReadFile            74（0 处检查返回值）
PutsHan            704
CreateGameFont      83
new[]               64
malloc               6
数据作格式串        41
硬编码地址           7
文件操作            10（DeleteFile 等）
socket 收发          6
```

---

## 十三、方法论说明（重要）

本报告对每条 Blocker/High 结论都做了**源码逐行核实**，并主动排除了 2 条扫描误报：

| 扫描命中 | 初判 | 核实结果 |
|---|---|---|
| `EnDecode.cpp:317` `unsigned char btBuffer[32]` + `memcpy(sizeof(_TDEFAULTMESSAGE))` | 疑似栈溢出 | **误报排除**：`_TDEFAULTMESSAGE` 实为 12 字节 |
| `LoginReg.cpp:57` `CreateCompatibleDC` | 疑似 DC 泄漏 | **误报排除**：第 61 行有 `DeleteDC` |

审计中"只看函数签名就下结论"曾导致误判（详见 `AUDIT_REPORT_RENDER.md` 第十节），
本报告所有结论均以函数体为准。

---

*报告结束*
