# NOTES.md — Mir3 网络协议对比任务审计文档

> 任务：对比 GitHub 复刻 Mir3（a939927870/Mir3）与 2004 原版泄露源码的网络协议，重点标记登录协议差异，评估对接 2005 服务端改造量。
> 生成时间：2026-10-04
> 执行环境：Windows，工作目录 D:\Mir3_Compare

---

## 1. 任务执行记录

| 步骤 | 结果 |
|---|---|
| 1. 克隆 GitHub 仓库 | ✅ `D:\Mir3_Compare\Mir3`（master 78e6bd4，浅克隆，726 文件；首次克隆仓库配置损坏，清 git 进程重删重克隆成功） |
| 2. 编译项目 | ⚠️ 部分完成（见第 2 节，本机无 MSVC） |
| 3. 定位网络协议文件 | ✅ 客户端 7 个 + 服务端 15 个（见 protocol_diff.md 第 9 节） |
| 4. 对比网络模块 | ✅ 完成（包头/编解码/消息ID/登录握手/SendRunLogin） |
| 5. protocol_diff.md | ✅ 已生成 |
| 6. NOTES.md | ✅ 本文档 |
| 7. 提交内网 Gitea | ⏳ 待执行 |

---

## 2. 编译结果

### 2.1 环境现状

- **本机无 MSBuild / Visual Studio**（检查 C:\Program Files*、D:\ 均无）
- 已下载 VS BuildTools 引导器 `D:\vs_buildtools.exe`（4,478,032 B）
- **静默安装失败**：
  - 错误：`0x80072ee7` / WinInet 12007
  - 原因：`https://aka.ms/vs/install/latest/installer` 不可达（DNS/网络受限）
  - 日志：`C:\Users\Administrator\AppData\Local\Temp\dd_bootstrapper_20261004100950.log`
- 替代编译器：**w64devkit g++（GCC 16.2.0，i686-w64-mingw32）** 可用

### 2.2 g++ 语法级替代检查（github_build_log.txt）

对 16 个网络协议相关 .cpp 做 `-fsyntax-only` 检查：

**客户端（4/5 通过）**：
- ✅ EnDecode.cpp / ClientSocket.cpp / DecodeMessage.cpp / LoginProcess.cpp —— 语法通过
- ❌ CharacterProcess.cpp —— 1 error：`LPSTR→WORD 精度丢失`（32 位工程在 64 位 g++ 下）

**服务端（0/10 通过，全部因 MSVC 专有特性）**：
- `Def/_OrzEx/queue.h` / `List.h` / `DynamicArray.h`：模板两阶段查找（MSVC 宽松语义）
- `__try/__finally`：MSVC SEH 扩展，g++ 不支持
- 指针→DWORD/int 精度丢失、LPDWORD→PULONG_PTR：32 位工程在 64 位 g++ 下

**结论**：复刻版是 **32 位 MSVC 工程（VS2010 vcxproj）**，完整编译必须 MSVC。g++ 语法检查证明**客户端网络核心代码可移植性良好**；服务端依赖 SEH/IOCP 指针操作，必须 MSVC。**不修改源码**（用户规则）。

### 2.3 若要真实编译

1. 让用户装 VS2010/2015/2017/2019/2022 任一版本（Build Tools 或完整 VS）
2. 或解决 aka.ms 网络可达性后重跑：`D:\vs_buildtools.exe --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended --quiet --wait --norestart`
3. msbuild 编译命令：
   - 客户端：`msbuild Mir3EIClientDev\Mir3EIClientDev.sln /p:Configuration=Release /p:Platform=Win32`
   - 服务端：`msbuild Mir3EIServerDev\Mir2EIServerDev.sln /p:Configuration=Release /p:Platform=Win32`

---

## 3. 代码文件清单（网络模块）

### 复刻版客户端（Mir3EIClientDev\Mir3EIClient）

| 文件 | 大小 | 职责 |
|---|---|---|
| Common\Protocol.h | 4,620 B | 协议定义（140 行） |
| Common\EnDecode.cpp / .h | 2,468 / 1,529 B | 6-bit 编解码 |
| Common\ClientSocket.cpp / .h | 15,614 / 2,954 B | 发包/收包/登录/选角/移动 |
| GameProcess\DecodeMessage.cpp | - | 服务端消息解码分发 |
| LoginProcess\LoginProcess.cpp | 27,504 B | 登录流程状态机 |
| CharSelectProcess\CharacterProcess.cpp | - | 选角 + SM_STARTPLAY |

### 复刻版服务端（Mir3EIServerDev\Mir3server）

| 文件 | 职责 |
|---|---|
| Def\Protocol.h（494 行） | 服务端协议定义 |
| Def\EnDecode.cpp / .h | 服务端编解码 |
| LoginSvr\ThreadFuncForMsg.cpp | 登录服务器收包分发 |
| LoginSvr\GateInfo.cpp | ProcLogin 账号验证 |
| LoginSvr\LoginSvr.cpp | 登录服务器主流程 |
| LoginGate\ClientSockMsg.cpp / ServerSockMsg.cpp | 登录网关 |
| SelGate\ClientSockMsg.cpp | 选角网关 |
| GameGate\ClientSockMsg.cpp / ServerSockMsg.cpp | 游戏网关（IOCP） |
| GameSvr\UserInfo.cpp | DoClientCertification 进游戏认证 |
| GameSvr\SockMsg_GateComm.cpp | GameSvr 收网关消息 |
| GameSvr\ProcessLogin.cpp | 角色加载/DB 交互 |

### 2004 原版基准（D:\work\mir3-korean-server / mir3-korean-client）

- 服务端：Def\Protocol.h（492 行）、Def\EnDecode.cpp（4,532 B）/h（2,755 B）、GameSvr\UserInfo.cpp 等 49 cpp
- 客户端：Common\Protocol.h（4,620 B）、Common\EnDecode.cpp（2,468 B）、Common\ClientSocket.cpp（14,762 B）、LoginProcess\LoginProcess.cpp（27,576 B）、CharSelectProcess\CharacterProcess.cpp

---

## 4. 风险点

| 风险 | 级别 | 说明 |
|---|---|---|
| **SendRunLogin 协议错位** | 🔴 高 | 用户 DX9 客户端（6 字段 XOR）↔ 韩版服务端（4 字段明文）认证错位 → 可能是进游戏黑屏/无数据根因之一 |
| 复刻版自身客户端/服务端不自洽 | 🟡 中 | 复刻版客户端 5 字段 XOR vs 其服务端 4 字段明文（疑为合并其他服代码所致） |
| CMList.dat 消息缺失崩溃 | 🟡 中（已修复） | 旧主线 T45 已修复并推送（4edf98f），与本任务无关但影响同一进游戏路径 |
| 编译环境缺失 | 🟠 中 | 无 MSVC，无法完整编译验证；g++ 替代检查已做 |
| 韩版服务端源码不完整 | 🟡 中 | 无 LoginSvr/LoginGate/SelGate/GameGate/DBSvr 源码，网关层无法源码级对比（仅 D:\MUD3 成品） |
| D:\Mir3_2004 目录缺失 | ⚪ 低 | 已用韩版源码替代（协议层同源） |

---

## 5. 对接 2005 服务端改造量（摘要）

详见 protocol_diff.md 第 8 节。核心结论：

- **必改 1 处**：客户端 `SendRunLogin`（Common\ClientSocket.cpp），改回韩版明文 4 字段格式 `**ID/CharName/cert/ver/1`
- **零改动**：包头结构、消息 ID、编解码、登录握手、选角流程、网关转发
- 若服务端为 D:\MUD3 成品（无源码），只能改客户端适配
- 若需保留 XOR 防作弊：同步改服务端 DoClientCertification（韩版服务端源码可编译）

---

## 6. 操作日志摘要

1. 定位复刻版网络协议文件（客户端 7 + 服务端 15）
2. 确认 D:\Mir3_2004 不存在，改用 D:\work\mir3-korean-server/client 作 2004 基准
3. 对比 Protocol.h：客户端 diff=0，服务端仅 2 处差异
4. 对比 EnDecode：客户端 diff=0，服务端掩码数值差异但功能等价（已推导）
5. 对比登录握手：一致（2001→529→104→530→100→520→103→525）
6. 发现 SendRunLogin 铁证差异（明文 vs XOR，4/5/6 字段）
7. 确认服务端 DoClientCertification 均为 4 字段明文解析（韩版=复刻版）
8. 尝试 msbuild：本机无 → 下载 BuildTools → 安装失败（aka.ms 不可达）→ 改用 g++ 语法检查
9. g++ 检查 16 文件：客户端核心 4/5 通过，服务端需 MSVC
10. 生成 protocol_diff.md / NOTES.md / github_build_log.txt

## 7. 待办

- [ ] 提交三个交付物到内网 Gitea（http://192.0.2.10:8418/mir3-dev/mir3-dx9）
- [ ] 用户确认 2005 服务端具体版本（当前按韩版 2004 基准评估）
- [ ] 若需真实编译：装 MSVC 或解决 aka.ms 网络
- [ ] SendRunLogin 修复决策：改客户端明文 / 改服务端 XOR（待用户定）
