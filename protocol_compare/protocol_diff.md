# Mir3 网络协议对比：GitHub 复刻版 vs 2004 原版（韩版泄露源码）

> 任务目标：对比 GitHub 复刻 Mir3（a939927870/Mir3）与 2004 原版泄露源码的网络协议，重点标记登录协议差异，评估对接 2005 服务端的改造工作量。
> 生成时间：2026-10-04
> 对比基准：
> - 复刻版：`D:\Mir3_Compare\Mir3`（master 78e6bd4，客户端 Mir3EIClientDev + 服务端 Mir3EIServerDev 9 模块）
> - 2004 原版服务端：`D:\work\mir3-korean-server`（韩版 GameSvr/AdminCmd/Def，49 个 cpp）
> - 2004 原版客户端：`D:\work\mir3-korean-client`（韩版客户端）
> - 注：任务指定的 D:\Mir3_2004 目录不存在；韩版服务端源码缺少 LoginSvr/LoginGate/SelGate/GameGate/DBSvr（仅 D:\MUD3\Mir3Server 有编译成品，无源码）

---

## 1. 结论速览

| 对比项 | 结果 |
|---|---|
| 客户端 Protocol.h（消息ID/包头） | **完全一致**（diff=0，140 行逐行相同） |
| 服务端 Protocol.h | **几乎一致**（492 vs 494 行，仅 2 处差异） |
| 数据包编解码（EnDecode） | **兼容**（掩码表数值不同但功能等价） |
| 登录握手流程（2001→529→104→530→100→520→103→525） | **一致** |
| ★ 进游戏认证包 SendRunLogin | **不一致（核心风险点）** |
| 对接 2005 服务端改造量 | **1 处必改（SendRunLogin），其余零改动** |

---

## 2. 消息包头结构（一致）

两套源码使用完全相同的包头结构：

| 结构 | 大小 | 字段 |
|---|---|---|
| `_TDEFAULTMESSAGE`（客户端↔登录/网关） | 12B | nRecog(4) + wIdent(2) + wParam(2) + wTag(2) + wSeries(2) |
| `_TMSGHEADER`（服务端内部，GameGate↔GameSvr 等） | 20B | nCode(4) + nSocket(4) + wUserGateIndex(2) + wIdent(2) + wUserListIndex(2) + wTemp(2) + nLength(4) |
| `TGateToSvrHeader`（网关→服务器） | 10B | szPrefix + szID + btGateIndex + nSocket + wDataLength |

> 唯一差异：复刻版服务端 EnDecode.h **启用了** `TGateToSvrHeader` 定义（韩版原版整段注释）。仅新增类型定义，不影响线上报文。
> 客户端 EnDecode.h 差异：`szEncodeData` 缓冲由 `MAX_PATH*4` 扩到 `MAX_PATH*12`（仅容量扩展）。

---

## 3. 数据包编解码（兼容）

- 算法：6-bit 编码（`fnEncode6BitBuf` / `fnDecode6BitBuf`），每 8 字节原始数据编成 6-bit 组并 +0x3c 偏移，兼容 ANSI/UNICODE 两套入口。
- 客户端 EnDecode.cpp：**韩版 = 复刻版（diff=0）**
- 服务端 EnDecode.cpp：**有 7 处差异，但功能等价**：

| 位置 | 韩版 2004 | 复刻版 | 影响 |
|---|---|---|---|
| `Decode6BitMask[5]` | `{0xff,0xfe,0xfd,0xec,0xcd}` | `{0xfc,0xf8,0xf0,0xe0,0xc0}` | **无**（nBitPos 仅取 2/4/6，掩码多保留位恒为 0，见下方推导） |
| 解码末尾 `pszDest[nDestPos]='\0'` | 注释掉 | 启用 | 复刻版更安全，无协议影响 |

**掩码等价性推导**（关键）：解码 `tmp = (ch << nBitPos) & Mask[nBitPos-2]`，nBitPos ∈ {2,4,6}：
- nBitPos=2：ch<<2 后 bit1-0 恒为 0 → `0xfc` ≡ `0xff`
- nBitPos=4：ch<<4 后 bit3-0 中 bit3=0 → `0xf8` ≡ `0xf0`
- nBitPos=6：ch<<6 后 bit5-4 恒为 0 → `0xf0` ≡ `0xc0`

结论：两套掩码表解码结果逐字节相同，**不构成协议不兼容**。

---

## 4. 消息 ID 枚举定义（一致）

### 4.1 客户端 Protocol.h

韩版与复刻版 **140 行逐行相同**（`Compare-Object` diff=0），包含：
- 网关命令：GM_OPEN(1)/GM_CLOSE(2)/GM_CHECKSERVER(3)/GM_CHECKCLIENT(4)/GM_DATA(5)/GM_SERVERUSERINDEX(6)/GM_RECEIVE_OK(7)/GM_TEST(20)
- 登录应答：SM_VERSION_AVAILABLE(500)~SM_UPDATEID_FAIL(533)
- 登录命令：CM_PROTOCOL(2000)/CM_IDPASSWORD(2001)/CM_ADDNEWUSER(2002)/CM_CHANGEPASSWORD(2003)/CM_UPDATEUSER(2004)
- 选角命令：CM_QUERYCHR(100)/CM_NEWCHR(101)/CM_DELCHR(102)/CM_SELCHR(103)/CM_SELECTSERVER(104)

### 4.2 服务端 Protocol.h

韩版 492 行 vs 复刻版 494 行，**仅 2 处差异**：
1. 注释：`// Client To Server Commands]` → `// Client To Server Commands`（标点）
2. 复刻版在某个结构体内**新增** `char szStdType[10];`（字段，不影响已定义协议）

> 结论：登录/选角/网关消息号在 2004 原版与复刻版之间 **1:1 相同**。

---

## 5. 登录握手流程（一致）

客户端 ↔ 服务端（经 LoginGate/SelGate 透明转发）逐段验证：

```
客户端                            LoginGate/LoginSvr                SelGate/SelChrGate
  │──CM_IDPASSWORD(2001)──────────>│
  │   数据: encode6bit("ID/密码")  │──查 TBL_ACCOUNT（FLD_LOGINID）──>│
  │<──SM_ID_NOTFOUND(502)──────────│  或 SM_PASSWD_FAIL(503)         │
  │<──SM_PASSOK_SELECTSERVER(529)──│  （或 SM_CERTIFICATION_FAIL 501）│
  │──CM_SELECTSERVER(104)─────────>│
  │<──SM_SELECTSERVER_OK(530)──────│
  │──CM_QUERYCHR(100)─────────────>│────────────────────────────────>│
  │<──SM_QUERYCHR(520)─────────────│<────────────────────────────────│ 角色列表
  │──CM_SELCHR(103)───────────────>│────────────────────────────────>│
  │<──SM_STARTPLAY(525)────────────│<────────────────────────────────│ 含 "IP/Port"
  │──断开登录网关，连接游戏网关──────>│
  │──SendRunLogin（★见第6节）──────>│ GameGate→GameSvr DoClientCertification
```

- 客户端 `OnLogin`：`CM_IDPASSWORD` + `encode6bit("ID/密码")`，`SendPacket` 打包 `#<sync><16B编码头><6bit数据>!`
- 服务端 `LoginSvr/ThreadFuncForMsg.cpp`：`fnDecodeMessageA` 解出头 → `ProcLogin` → `fnDecode6BitBufA` 解出 "ID/密码" → SQL 查 TBL_ACCOUNT
- 网关（LoginGate/SelGate/GameGate）只做 **socket 代理 + 会话管理，报文原样转发**（复刻版实现，韩版无对应源码）
- SM_STARTPLAY 客户端处理：韩版 = 复刻版（解析 `IP/Port` → `DisconnectToServer()` → `m_nRenderState=2`）

---

## 6. ★ 进游戏认证包 SendRunLogin（核心差异，必读）

**位置**：客户端连接游戏网关后的第一个包（`CGameProcess::OnConnectToServer()` → `SendRunLogin`）。

| 版本 | 发送格式（encode6bit 前） | 数据字段 | 加密 |
|---|---|---|---|
| **韩版 2004 客户端** | `**ID/角色名/cert/ver/1` | 4 个 | 明文 |
| **韩版 2004 服务端解析**（DoClientCertification） | 按 `**uid/chr/cer/ver/1` 解析 | 4 个 | 明文 |
| **复刻版(GitHub) 客户端** | `**ID/角色名/X/Y/Z/1` | 5 个 | XOR |
| **复刻版(GitHub) 服务端解析** | 按 4 字段明文解析（同韩版） | 4 个 | 明文 |
| **用户 DX9 客户端**（mir3-dx9） | `**ID/角色名/X/Y/Z/1/UPGRADE` | 6 个 | XOR |

**XOR 常量**：
- 复刻版客户端：`cert^0x3EB2C5CC`、`cert^0x5580AF27^ver`、`ver^0x0FA0280AF`
- 用户 DX9 客户端：`(cert^0x44a031f3)^0x7a12f43f`、`(cert^ver)^0x5580af27`、`ver^0xfa0280af`、`nUpgradeLevel`

**关键结论**：
1. **复刻版客户端与其自身服务端不自洽**：客户端发 5 字段 XOR，服务端按 4 字段明文解析 → 服务端把加密值当作 cert/ver。
2. **用户 DX9 客户端（6 字段 XOR）对接韩版 2004 服务端必然认证错位**：服务端 `m_nCertification = 加密值X`、`m_nClientVersion = 加密值Y`，后续 `LoadPlayer → LoadHumanFromDB` 携带错误认证码查 DB。
3. 服务端（韩版=复刻版）`DoClientCertification` 逻辑完全一致（GameSvr/UserInfo.cpp:1081），只认 **4 字段明文**。

> ⚠️ 这可能正是"选角进游戏后黑屏/无数据渲染"的深层原因之一：客户端发了服务端解析不了的认证包，服务端不回复任何游戏数据（与用户观察"进游戏后渲染日志一行都没有"吻合）。旧主线定位的 CMList.dat 崩溃（GetMsg NULL）是另一独立问题，两处需分别修复。

---

## 7. 其余模块差异清单（非协议）

| 文件 | 差异 | 性质 |
|---|---|---|
| 客户端 LoginProcess.cpp | 全局套接字改名 `g_xClientSocket`→`g_xLoginSocket`、UI/进度状态机调整 | 非协议 |
| 客户端 ClientSocket.cpp | 新增 SendRunMsg/SendWalkMsg 等移动包（韩版无）、错误 MessageBox 增加 | 非协议 |
| 服务端 EnDecode.h | 启用 TGateToSvrHeader 结构 | 新增定义 |
| 客户端 EnDecode.h | 编码缓冲 4x→12x | 容量 |

---

## 8. 对接 2005 服务端改造量评估

> 目标服务端 = 韩版 2004 服务端（D:\work\mir3-korean-server 同源，D:\MUD3 为编译成品）。"2005 服务端"具体版本待用户确认，但协议层以韩版为基准。

### 必改（1 处）

**客户端 `SendRunLogin` 进游戏认证包**，改回韩版明文 4 字段格式：
```c
// 原（用户 DX9 当前）：**ID/CharName/(XOR)/1/(XOR)/1/UPGRADE
// 改为：
wsprintf(szLoginInfo, "**%s/%s/%d/%d/1", szLoginID, szCharName, nCertification, VersionNum);
```
- 影响文件：`Common/ClientSocket.cpp`（一个 wsprintf）
- 若需保留 XOR 防作弊，可同步改服务端 `GameSvr/UserInfo.cpp DoClientCertification` 为解 XOR + 5/6 字段（服务端源码在 D:\work\mir3-korean-server 可编译）

### 无需改（验证通过）

- 包头结构、消息 ID 枚举、6-bit 编解码、登录握手（2001→529→104→530→100→520→103→525）、选角流程、SM_STARTPLAY 处理
- 网关转发架构（客户端↔服务端报文格式一致）

### 评估

| 项 | 工作量 | 说明 |
|---|---|---|
| 协议号/包头/编解码 | 0 | 完全兼容 |
| 登录握手 | 0 | 一致 |
| SendRunLogin | 小（1 函数） | 必改 |
| 若服务端为 D:\MUD3 成品（无源码） | 只能改客户端 | 客户端适配明文格式 |
| 数据库（TBL_ACCOUNT 等） | 未知 | 需核对 D:\MUD3 数据库结构 |

---

## 9. 附：关键文件清单

| 复刻版路径 | 作用 |
|---|---|
| `Mir3EIClientDev/Mir3EIClient/Common/Protocol.h` | 客户端协议定义（140 行） |
| `Mir3EIClientDev/Mir3EIClient/Common/EnDecode.cpp/.h` | 客户端编解码 |
| `Mir3EIClientDev/Mir3EIClient/Common/ClientSocket.cpp` | 客户端发包/收包/登录/选角 |
| `Mir3EIClientDev/Mir3EIClient/LoginProcess/LoginProcess.cpp` | 登录流程 |
| `Mir3EIClientDev/Mir3EIClient/CharSelectProcess/CharacterProcess.cpp` | 选角 + SM_STARTPLAY 处理 |
| `Mir3EIServerDev/Mir3server/Def/Protocol.h` | 服务端协议定义（494 行） |
| `Mir3EIServerDev/Mir3server/Def/EnDecode.cpp/.h` | 服务端编解码 |
| `Mir3EIServerDev/Mir3server/LoginSvr/ThreadFuncForMsg.cpp` | 登录服务器收包分发 |
| `Mir3EIServerDev/Mir3server/LoginSvr/GateInfo.cpp` | ProcLogin 账号验证 |
| `Mir3EIServerDev/Mir3server/GameSvr/UserInfo.cpp` | DoClientCertification 进游戏认证 |
| `Mir3EIServerDev/Mir3server/GameSvr/SockMsg_GateComm.cpp` | GameSvr 收网关消息分发 |
| `Mir3EIServerDev/Mir3server/GameGate/ServerSockMsg.cpp` | GameGate 客户端会话（IOCP） |

韩版基准路径：`D:\work\mir3-korean-server\Def\*`、`D:\work\mir3-korean-client\Common\*`、`D:\work\mir3-korean-client\LoginProcess\*`、`D:\work\mir3-korean-client\CharSelectProcess\*`。
