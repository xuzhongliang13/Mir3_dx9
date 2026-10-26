# 传奇3服务端协议逆向 - 调试笔记

## 项目信息
- **目标**: 逆向传奇3服务端网络通信协议
- **服务端路径**: D:\MUD3
- **客户端路径**: D:\Client\Mir3_patched_v3_backup.exe
- **开始时间**: 2026-10-03 14:03

---

## 任务1：环境准备与动态调试

### 1.1 服务端架构分析

**服务端目录结构 (D:\MUD3)**:
```
MUD3/
├── Mir3Server/          # 核心服务端组件
│   ├── DBSvr/          # 数据库服务器
│   ├── LogGate/        # 日志网关
│   ├── LoginDB/        # 登录数据库
│   ├── RunGate/        # 游戏运行网关
│   └── SelChrGate/     # 角色选择网关
├── RunServer/          # 启动脚本和配置
│   ├── 1-LoginSvr.exe.lnk
│   ├── 2-DBSvr.exe.lnk
│   ├── 3-Server_JOB_ItemGen.exe.lnk
│   ├── 4-rlogin.exe.lnk
│   ├── 5-SelChrGate.exe.lnk
│   └── 6-RunGate.exe.lnk
├── Envir/              # 环境配置
├── Map/                # 地图数据
└── ...
```

**服务端组件说明**:
1. **LoginSvr.exe** - 登录服务器，处理账号验证
2. **DBSvr.exe** - 数据库服务器，管理角色数据
3. **Server_JOB_ItemGen.exe** - 物品生成服务
4. **rlogin.exe** - 登录网关（可能）
5. **SelChrGate.exe** - 角色选择网关
6. **RunGate.exe** - 游戏运行网关，处理游戏内通信

### 1.2 服务端进程状态（已确认运行）

| 进程 | PID | 监听端口 | 功能 | 连接目标 |
|------|-----|----------|------|----------|
| LoginSvr.exe | 10704 | 3000/5500/5600 | 登录服务器 | - |
| DBSvr.exe | 9144 | 5100/6000 | 数据库服务器 | - |
| rlogin.exe | 13168 | 7000 | 登录网关 | LoginSvr(5500) |
| SelChrGate.exe | 13096 | 7100 | 角色选择网关 | DBSvr(5100) |
| RunGate.exe | 376 | 7200 | 游戏运行网关 | DBSvr(5010) |

### 1.3 端口映射关系

```
客户端连接流程:
1. 客户端 → LoginGate/rlogin.exe (7000) → LoginSvr.exe (5500) : 账号登录
2. 登录成功 → 跳转到 SelChrGate.exe (7100) → DBSvr.exe (5100) : 选角
3. 选角完成 → 跳转到 RunGate.exe (7200) → DBSvr.exe (5010) : 进入游戏
```

### 1.4 客户端配置

**Mir3.ini**:
```
ServerAddr=192.0.2.10  # 需要修改为127.0.0.1
Param1=7000                # LoginGate端口
```

**sellogin.ini** (Unicode):
```
ServerAddr=127.0.0.1
Param1=7000
```

### 1.5 待确认事项
- [ ] 确认客户端IP配置是否正确
- [ ] 启动客户端连接服务端
- [ ] 开始抓包

---

## 调试记录

### [2026-10-03 14:03] 开始任务1
- 确认服务端路径: D:\MUD3
- 确认客户端路径: D:\Client\Mir3_patched_v3_backup.exe
- 发现客户端登录界面截图，说明客户端可运行

### [2026-10-03 14:10] 服务端架构分析完成
- 所有服务端进程已运行
- 端口映射关系已确认
- 客户端配置需要检查IP地址

### [2026-10-03 14:15] 客户端配置修正
- 原配置: ServerAddr=192.0.2.10 (错误)
- 修改为: ServerAddr=127.0.0.1 (本地服务端)
- 端口: Param1=7000 (LoginGate)

### [2026-10-03 14:16] 准备抓包环境
- 编写了Python代理服务器脚本: mir3_packet_capture.py
- 编写了x32dbg调试脚本: x32dbg_packet_hook.txt
- 代理端口映射: 17000->7000, 17100->7100, 17200->7200

### [2026-10-03 14:17] 服务端内部连接确认
已确认的服务端内部TCP连接:
- LoginSvr(5500) <-> rlogin/LoginGate
- DBSvr(5100) <-> SelChrGate
- DBSvr(5010) <-> RunGate
- LoginSvr(5600) <-> 其他组件

客户端尚未启动，需要启动客户端开始抓包

### [2026-10-03 14:25] 客户端启动与连接确认
- 客户端进程: Mir3_dx9.exe (PID 12152)
- 连接状态: 127.0.0.1:1935 -> 127.0.0.1:7000 (ESTABLISHED)
- 已成功连接到LoginGate

### [2026-10-03 14:30] 抓包环境准备完成
- Npcap已安装，支持Loopback抓包
- scapy抓包脚本已启动: mir3_scapy_capture.py
- 捕获端口: 7000, 7100, 7200
- 日志目录: packets_log_scapy/

### [2026-10-03 14:31] 等待用户操作
- 抓包脚本运行中
- 需要用户在客户端执行登录操作
- 同时准备分析服务端可执行文件

### [2026-10-03 14:40] 数据包捕获分析

**已捕获的数据包特征**:

1. **客户端心跳/握手包 (C->S)**:
   - 格式: `#` + 序列号 + `<<<<<` + 数据 + `!`
   - 长度: 38-43字节
   - 示例: `#2<<<<<I@C<<<<<<<<UBQ]Wc=\`XnxmH_HpIOT!`
   - 序列号递增: 2,3,4,5,6,7,8,9,1...

2. **服务端响应包 (S->C)**:
   - 格式: `#` + 消息类型 + `<<<<<<t><<<<<<<<!`
   - 长度: 18字节（固定）
   - 示例: `#<<<<<<t><<<<<<<<!`

3. **数据包结构初步分析**:
   - 起始符: `#` (0x23)
   - 结束符: `!` (0x21)
   - 第2字节: 消息类型/序列号
   - 数据经过加密/混淆

**服务端可执行文件**:
- LoginSvr.exe: D:/MUD3/Mir3Server/LoginDB/LoginSvr.exe (143KB)
- rlogin.exe: D:/MUD3/Mir3Server/LogGate/rlogin.exe (登录网关)
- SelChrGate.exe: D:/MUD3/Mir3Server/SelChrGate/SelChrGate.exe
- RunGate.exe: D:/MUD3/Mir3Server/RunGate/RunGate.exe
- DBSvr.exe: D:/MUD3/Mir3Server/DBSvr/DBSvr.exe

**待完成**:
- [ ] 等待用户登录，捕获登录数据包
- [ ] 使用x32dbg动态调试服务端
- [ ] 分析加密算法
- [ ] 解析完整协议结构

### [2026-10-03 14:45] 数据包分析完成

**分析结果**:

1. **C->S 心跳包结构**:
   - 格式: `#` + 序列号(1-9循环) + `<<<<<` + 加密数据 + 数据
   - 长度: 32字节（不含结束符）或 38-43字节（含结束符）
   - 序列号: 1,2,3,4,5,6,7,8,9 循环
   - 示例: `#2<<<<<I@C<<<<<<<<UBQ]Wc=\`XnxmH_`

2. **S->C 响应包结构**:
   - 格式: `#<<<<<<t><<<<<<<<!`
   - 长度: 18字节（固定）
   - 消息类型: `<` (0x3C)

3. **协议特征**:
   - 起始符: `#` (0x23)
   - 结束符: `!` (0x21)
   - 数据经过加密/混淆
   - 客户端定期发送心跳包（约1秒间隔）

**当前状态**:
- 抓包脚本运行中
- 已捕获54个数据包（27个C->S，27个S->C）
- 等待用户输入账号密码进行登录

**下一步**:
1. 用户在客户端输入账号密码登录
2. 捕获登录数据包
3. 分析加密算法
4. 使用x32dbg动态调试服务端

### [2026-10-03 14:50] 登录数据包捕获

**捕获到登录数据包**:

1. **登录请求包 (C->S)**:
   - HEX: `23323C3C3C3C3C4940433C3C3C3C3C3C3C3C5A43656859736071485F3C6B5A43656859736071485F3C21`
   - TEXT: `#2<<<<<I@C<<<<<<<<ZCehYs\`qH_<kZCehYs\`qH_<!`
   - 长度: 42字节
   - 特征: 消息类型 '2'，包含加密的账号密码数据

2. **服务端响应包 (S->C)**:
   - HEX: `233C3C3C3C3C3D403E3C3C3C3C3C3C3C3C6C78774B7678636D6A773C64484E607721`
   - TEXT: `#<<<<<=@><<<<<<<<lxwKvxcmjw<dHN\`w!`
   - 长度: 34字节
   - 特征: 响应码 '=@>'，包含服务端返回的数据

3. **第二个登录包 (C->S)**:
   - HEX: `23333C3C3C3C3C425C3C3C3C3C3C3C3C3C3C7566466275644A6F7576525A76467A5921`
   - TEXT: `#3<<<<<B\\<<<<<<<<<ufFbudJouvRZvFzY!`
   - 长度: 35字节
   - 特征: 消息类型 '3'，可能是密码确认

**协议结构分析**:
- 起始符: `#` (0x23)
- 消息类型: 第2字节 ('2', '3', '4'...)
- 固定填充: `<<<<<` (5个小于号)
- 标识数据: 如 'I@C', 'B\\'
- 加密数据: 账号密码经过加密
- 结束符: `!` (0x21)

**加密特征**:
- 所有字节在ASCII可打印范围内 (0x21-0x7E)
- 数据包1中有重复数据段: `ZCehYs\`qH_` 出现两次
- 可能使用了简单的替换加密或XOR加密

### [2026-10-03 14:55] 代理抓包成功

**捕获到的登录数据包**:

1. **登录请求包 (C->S)**:
   - HEX: `23323C3C3C3C3C4940433C3C3C3C3C3C3C3C5A43656859736071485F3C6B5A43656859736071485F3C21`
   - TEXT: `#2<<<<<I@C<<<<<<<<ZCehYs\`qH_<kZCehYs\`qH_<!`
   - 长度: 42字节
   - 特征: 消息类型 '2'，包含加密的账号密码数据

2. **服务端响应包 (S->C)**:
   - HEX: `233C3C3C3C3C3D403E3C3C3C3C3C3C3C3C6C78774B7678636D6A773C64484E607721`
   - TEXT: `#<<<<<=@><<<<<<<<lxwKvxcmjw<dHN\`w!`
   - 长度: 34字节
   - 特征: 响应码 '=@>'，包含服务端返回的数据

3. **心跳包 (C->S)**:
   - 序列号递增: 3, 4, 5, 6, 7, 8...
   - 示例: `#3<<<<<B\\<<<<<<<<<wwzywwzywwzywwzywwzywwzy!`

**协议结构分析**:
- 起始符: `#` (0x23)
- 消息类型: 第2字节 ('2', '3', '4'...)
- 固定填充: `<<<<<` (5个小于号)
- 标识数据: 如 'I@C', 'B\\'
- 加密数据: 账号密码经过加密
- 结束符: `!` (0x21)

**加密特征**:
- 所有字节在ASCII可打印范围内 (0x21-0x7E)
- 数据包1中有重复数据段: `ZCehYs\`qH_` 出现两次
- 可能使用了简单的替换加密或XOR加密

**下一步**:
1. 分析加密算法
2. 使用x32dbg动态调试服务端
3. 定位加密函数

### [2026-10-03 15:00] 任务1完成，开始任务2

**任务1完成**:
- ✅ 服务端架构分析完成
- ✅ 数据包捕获成功（14个文件）
- ✅ 协议结构初步分析完成
- ✅ 客户端成功登录并进入游戏

**开始任务2：逆向加解密算法**
- 分析服务端可执行文件
- 定位加密函数
- 还原加密算法伪代码

### [2026-10-03 15:05] 任务2进度

**字符串分析结果**:
- 服务端(rlogin.exe): Delphi编写，包含 `DecodeTimer`, `SendTimer`, `TUserInfo4`
- 客户端(Mir3_patched_v3_backup.exe): C++编写，包含 `CLoginProcess`, `CStreamBuffer`

**协议结构分析**:
```
数据包通用格式:
  起始符: # (0x23)
  消息类型: 第2字节
  固定填充: <<<<< (5个小于号)
  数据: 经过加密/混淆
  结束符: ! (0x21)

消息类型:
  '2' (0x32) - 登录请求
  '3'-'9' (0x33-0x39) - 心跳包（序列号递增）
  '<' (0x3C) - 服务端响应
```

**加密特征**:
1. 所有字节在ASCII可打印范围内 (0x21-0x7E)
2. 心跳包数据段完全相同: `wwzywwzywwzywwzywwzywwzy`
3. 登录包中有重复数据段: `ZCehYs\`qH_` 出现两次

**下一步**:
1. 使用x32dbg动态调试客户端/服务端
2. 定位加密函数
3. 分析加密算法的具体实现

### [2026-10-03 16:20] API Monitor 成功捕获数据包

**捕获结果**:
- 总计: 85 个数据包
- C->S: 15 个包
- S->C: 48 个包

**C->S 消息类型统计**:
| 消息类型 | 十六进制 | 次数 | 说明 |
|----------|----------|------|------|
| '2' | 0x32 | 4次 | 登录请求 |
| '3' | 0x33 | 4次 | 心跳包 |
| '4' | 0x34 | 2次 | 心跳包 |
| '5' | 0x35 | 2次 | 心跳包 |
| '6' | 0x36 | 1次 | 心跳包 |
| '7' | 0x37 | 1次 | 心跳包 |
| '9' | 0x39 | 1次 | 心跳包 |

**S->C 消息类型统计**:
| 消息类型 | 十六进制 | 次数 | 说明 |
|----------|----------|------|------|
| '<' | 0x3C | 4次 | 登录响应 |
| 'b' | 0x62 | 20次 | 地图/角色数据 |
| 'c' | 0x63 | 7次 | 游戏数据 |
| 'd' | 0x64 | 2次 | 游戏数据 |
| 'e' | 0x65 | 3次 | 游戏数据 |
| 'i' | 0x69 | 1次 | 游戏数据 |
| 'j' | 0x6A | 4次 | 游戏数据 |
| 'k' | 0x6B | 1次 | 游戏数据 |
| 'q' | 0x71 | 1次 | 游戏数据 |
| 'v' | 0x76 | 1次 | 游戏数据 |
| 'y' | 0x79 | 4次 | 游戏数据 |

**关键数据包**:

1. **登录请求包** (C->S):
   - TEXT: `#2<<<<<I@C<<<<<<<<ZCehYs\`qH_<kZCehYs\`qH_<!`
   - 长度: 42字节
   - 特征: 消息类型 '2'，包含加密的账号密码

2. **服务端登录响应** (S->C):
   - TEXT: `#<<<<<=@><<<<<<<<lxwKvxcmjw<dHN\`w!`
   - 长度: 34字节
   - 特征: 响应码 '=@>'

3. **心跳包** (C->S):
   - TEXT: `#3<<<<<B\<<<<<<<<<lxwKvxcmjw<!`
   - 长度: 30字节
   - 特征: 序列号递增

**完整数据已保存到**: `D:/Ada-WorkSpace/packets_extracted.txt`

### [2026-10-03 16:30] 源码编译成功

**编译结果**:
- ✅ 82个源文件全部编译通过
- ✅ 生成 Mir3_dx9.exe (2.5MB)
- ✅ 客户端成功连接到服务器 (127.0.0.1:7000)
- ✅ 客户端可以正常运行

**编译环境**:
- 编译器: MinGW g++ (i686-w64-mingw32-g++.exe)
- JPEG库: D:\jpeg\jpeg-9f\build\libjpeg.a
- 源码路径: D:\Ada-WorkSpace\mir3-dx9

**运行方式**:
- 将编译后的 exe 复制到 D:\Client 目录
- 或直接在该目录运行（需要资源文件）

### [2026-10-03 16:35] 加密算法分析完成

**源码位置**: `Common/EnDecode.cpp`

**加密函数列表**:

| 函数名 | 用途 | 算法 |
|--------|------|------|
| `fnEncode6BitBuf` | 基础6位编码 | 8位→6位，+0x3C |
| `fnEncode6BitBufGameMessage` | 游戏消息编码 | XOR 24, 6位编码, +0x4A |
| `fnEncode6BitBufGameString` | 游戏字符串编码 | XOR 37, 6位编码, +0x3D |
| `fnDecode6BitBufOriginal` | 原始解码 | -0x3C, 6位→8位 |
| `fnDecode6BitBufGameMessage` | 游戏消息解码 | -0x4D, 6位→8位, XOR 134 |

**加密算法详解**:

1. **fnEncode6BitBuf** (基础版本):
   ```cpp
   // 将8位数据转换为6位编码
   chMade = ((chRest | (pszSrc[i] >> (2 + nRestCount))) & 0x3f);
   chRest = (((pszSrc[i] << (8 - (2 + nRestCount))) >> 2) & 0x3f);
   pszDest[nDestPos++] = chMade + 0x3c;
   ```

2. **fnEncode6BitBufGameMessage**:
   ```cpp
   // 先 XOR 24，然后6位编码
   chMade = ((chRest | ((pszSrc[i]^24) >> (2 + nRestCount))) & 0x3f);
   pszDest[nDestPos++] = chMade + 0x4a;
   ```

3. **fnDecode6BitBufGameMessage**:
   ```cpp
   // 减0x4D，6位解码，XOR 134
   ch = pszSrc[i] - 0x4d;
   ch = ch & 0x3f;
   chCode = (tmp | ((ch & 0x3f) >> (6 - nBitPos)));
   chCode = chCode ^ 134;
   ```

**数据包结构**:
```
# + 消息类型 + 编码数据 + !
```

**消息类型分类**:
- 登录消息 (2000-2004): 使用 `fnEncode6BitBuf`
- 选角消息 (100-104): 使用 `fnEncode6BitBuf`
- 游戏消息: 使用 `fnEncode6BitBufGameMessage`

**下一步**: 任务3 - 解析消息结构体

### [2026-10-03 16:45] 任务5完成 - 单元测试代码

**测试代码**: `D:\Ada-WorkSpace\mir3_test_client.cpp`

**功能**:
- ✅ 连接登录网关 (127.0.0.1:7000)
- ✅ 发送协议握手包 (CM_PROTOCOL = 2000)
- ✅ 发送登录请求包 (CM_IDPASSWORD = 2001)
- ✅ 编码/解码验证

**编译命令**:
```bash
g++ -m32 -o mir3_test_client.exe mir3_test_client.cpp -lws2_32
```

**测试结果**:
- 成功连接到服务器
- 成功发送数据包
- 编码/解码功能正常

**待完善**:
- 服务器响应解析
- 完整的登录流程测试
- 选角/进入游戏流程测试

### [2026-10-03 14:58] 登录数据包捕获完成

**捕获到的完整登录流程**:

1. **登录请求包 (C->S)**:
   - HEX: `23323C3C3C3C3C4940433C3C3C3C3C3C3C3C5A43656859736071485F3C6B5A43656859736071485F3C21`
   - TEXT: `#2<<<<<I@C<<<<<<<<ZCehYs\`qH_<kZCehYs\`qH_<!`
   - 长度: 42字节
   - 特征: 消息类型 '2'，包含加密的账号密码数据

2. **服务端响应包 (S->C)**:
   - HEX: `233C3C3C3C3C3D403E3C3C3C3C3C3C3C3C6C78774B7678636D6A773C64484E607721`
   - TEXT: `#<<<<<=@><<<<<<<<lxwKvxcmjw<dHN\`w!`
   - 长度: 34字节
   - 特征: 响应码 '=@>'，包含服务端返回的数据

3. **心跳包 (C->S)**:
   - 序列号递增: 3, 4, 5, 6, 7, 8...
   - 示例: `#3<<<<<B\\<<<<<<<<<wwzywwzywwzywwzywwzywwzy!`

**协议结构分析**:
- 起始符: `#` (0x23)
- 消息类型: 第2字节
- 固定填充: `<<<<<`
- 加密数据: 账号密码经过加密
- 结束符: `!` (0x21)

**加密特征**:
- 所有字节在ASCII可打印范围内 (0x21-0x7E)
- 数据包1中有重复数据段: `ZCehYs\`qH_` 出现两次
- 可能使用了简单的替换加密或XOR加密

**当前状态**:
- 客户端已连接到RunGate (7200)
- 登录流程完成
- 准备进行下一步：分析加密算法

### [2026-10-03 15:00] 任务1完成，开始任务2

**任务1完成**:
- ✅ 服务端架构分析完成
- ✅ 数据包捕获成功（14个文件）
- ✅ 协议结构初步分析完成
- ✅ 客户端成功登录并进入游戏

**开始任务2：逆向加解密算法**
- 分析服务端可执行文件
- 定位加密函数
- 还原加密算法伪代码

### [2026-10-03 15:05] 任务2进度

**字符串分析结果**:
- 服务端(rlogin.exe): Delphi编写，包含 `DecodeTimer`, `SendTimer`, `TUserInfo4`
- 客户端(Mir3_patched_v3_backup.exe): C++编写，包含 `CLoginProcess`, `CStreamBuffer`

**协议结构分析**:
```
数据包通用格式:
  起始符: # (0x23)
  消息类型: 第2字节
  固定填充: <<<<< (5个小于号)
  数据: 经过加密/混淆
  结束符: ! (0x21)

消息类型:
  '2' (0x32) - 登录请求
  '3'-'9' (0x33-0x39) - 心跳包（序列号递增）
  '<' (0x3C) - 服务端响应
```

**加密特征**:
1. 所有字节在ASCII可打印范围内 (0x21-0x7E)
2. 心跳包数据段完全相同: `wwzywwzywwzywwzywwzywwzy`
3. 登录包中有重复数据段: `ZCehYs\`qH_` 出现两次

**下一步**:
1. 使用x32dbg动态调试客户端/服务端
2. 定位加密函数
3. 分析加密算法的具体实现

---

*持续更新中...*

---

## [2026-10-05] AB04 日志节流（Agent B）

**问题**：LogWIL.txt 膨胀到 108MB。逐行审计 WHWilImage.cpp 的 LogWIL.txt 写入点，确定膨胀源：
- NewSetIndex（每帧无条件调用）内的 SetIdx %u mode=%d count=%u 为**唯一每帧无条件写盘**的行 → 主膨胀源。
- LoadC WIXFAIL/WILFAIL/VERFAIL/POS 仅在 WIL 文件加载/异常时触发（一次性），非每帧，不膨胀。
- Cache idx FAIL / Load idx FAIL 仅命中空帧(pos=0)时写，属错误行，按要求保留。

**修改（WHWilImage.cpp 第189-190行，GBK936 编辑）**：SetIdx 日志改为节流——static DWORD g_nSetIdxLogCnt 首次 + 每30次记录一次；FAIL/WIXFAIL/VERFAIL 错误行全部保留未动；LoadC POS 诊断保留。

**验证**：编译 compiled 82 fail 0 + LINK 0 + SUCCESS。SetIdx 写盘量降至约 1/30。不影响渲染主路径（仅动诊断日志写盘）。

---

## [2026-10-05] AB01 选角立绘（Agent B）

**根因定位**：选角立绘'画了看不到'根因不在 ChrProc.cpp，而在共享解码器 DrawWithImageForComp(WHDXGraphic.cpp:2594)。其 RLE 行前缀逻辑 
xt = src + *src++ + 1 多算 1 个 WORD → 把下一行行前缀当 RLE 码 → BADCODE → 整帧不画（底栏条/立绘/窗口内容全消失）。由 MainAgent T77(77a5816) 修复为 
xt = src + *src++，逻辑正确（按知识库4.2：每行=[前缀W][数据W字]）。

**ChrProc.cpp 核查**：立绘主路径(471-515行) NewSetIndex(wCurrFrm) → DrawWithImageForComp；特效层(456行) rm+20 → DrawWithABlendCompDataWithBackBuffer(base 3340, 索引式行前缀)。与参考 D:\Ada-WorkSpace\Mir3China\CharSelectProcess\ChrProc.cpp 逐行对照一致，帧号/位置/尺寸/数据指针均正确，无 ChrProc 独立缺陷。base ABlend 的 nWidthEnd++ 是计入前缀词，逻辑正确非残bug。

**结论**：T77 修复即解决 AB01 根因，ChrProc.cpp 无需改动。已部署含 T77 构建到 D:\Client\Mir3_dx9.exe，写'请测试'请求待用户进选角界面验证。审计 T74/T75/T76/T77 均 OK 已登记 AUDIT_LOG。


---

## [2026-10-05] AB02 属性面板崩溃族（Agent B 取证）

**任务**：点"查看角色数据"崩溃，CrashLog 多次 `0x0051A23A/2C6/33E/5DA`、`faultAddr=0x7D523FF8`、EBX≈0x13C。分配文件 StatusWnd.cpp。验收：点开属性面板不崩溃、数值正确。

### 结论（证据链）
1. **任务书 4 地址均为"栈 dword"，非故障 EIP**。disasm_full.txt 精确地址核查：0x51a237 是 6 字节 `fld DWORD PTR ds:0x5931c0`（0x51a237-0x51a23c），0x51a2c5 是 4 字节 `mov DWORD PTR [esp+0x1c],eax`。**0x51a23a / 0x51a2c6 均落在指令中间，不是指令边界** → 不可能是返回地址，只能是崩溃时栈上的数据值（恰落在 CStatusWnd 代码区，被任务书误当调用链）。0x51a33e/0x51a5da 同理（0x51a33e 是 ShowEquipItemState 的 `mov [ebp-0x10],ecx` 存 this，写合法栈不可故障）。故真正的故障 EIP 不在任务书 4 地址中。

2. **StatusWnd.cpp 全文件唯一浮点格式 = 第1409行** `sprintf(szBuff,"%.2f%s",dExpRate*100,"%")`，且**格式与参数完全匹配**：`%.2f`←dExpRate*100(double, x86 cdecl 栈上8字节)、`%s`←"%"字面量(char* 4字节)。dExpRate 由 dlngExp/dlngMaxExp 计算、有 dlngMaxExp!=0 守卫。任务书担心的"第二个%单独传参误解析"不成立：`"%"` 是指针，`%s` 正常打印字面百分号。**此行不会单独触发 pformat 崩溃**，之前"与反汇编 pformat_emit_xfloat 高度吻合"的判断被否。

3. **装备悬停路径无浮点**：Item.cpp 全文无 `%f/%lf/%g` 等浮点格式、无 ShowItemStatus → 崩溃栈上 ShowEquipItemState/CheckEquipItem（0x51A33E/5DA）所在路径非 pformat 源。

4. **当前 CrashLog 实证**：D:\Client\CrashLog.txt 两条相同 `code=0xC0000005 addr=0x76F2DCB8`（moduleBase=0x76EA0000, offset=0x0008DCB8, 系统 DLL, EBX=0xE6）→ **GDI 文本渲染崩溃**。PutsHan(WHDXGraphic.cpp:992) 走 GDI：`pSurface->GetDC(&hDC)`(1012)→`SelectObject(hDC,hFont)`(1026)→`TextOut(hDC,...,strlen(szText))`(1030)→`ReleaseDC`(1040)。gdi32 崩溃 = 失效 HDC / 失效字体 / 设备重置后 backbuffer 失效。**根因在 WHDXGraphic.cpp（MainAgent 归属），非 StatusWnd.cpp**。

5. **StatusWnd.cpp 字体/纹理用法安全**：全部为"inline CreateGameFont→PutsHan 内 DeleteObject"单次创建/删除，无复用删除双删；成名徽章 DrawBillBoard 对 NULL 纹理做纯色填充(ImageHandler.cpp:429)，NewSetIndex 守卫帧有效性。无迁移引入的新空指针。

### 移交与下一步
- StatusWnd.cpp **无需改动**（无确认缺陷）。崩溃根因在 WHDXGraphic.cpp PutsHan 的 GDI 文本路径（HDC/字体/backbuffer 失效，疑似设备重置后未重建）。
- 请 MainAgent 复核 WHDXGraphic.cpp PutsHan/GetDC 的设备重置处理；请用户用当前构建 D:\Client\Mir3_dx9.exe 在 x64dbg 下复现（Access Violation 断点），捕获**真实故障 EIP + faultAddr + 调用链**（现有 CrashLog 只给 EIP/寄存器，无模块名栈回溯）。

---

---

## [2026-10-05] AB03 怪物无阴影（Agent B 取证）

**任务**：怪物/动物在地面有阴影（对照正常端）。分配文件 Actor.cpp（阴影绘制部分）。验收：怪物/动物有地面阴影。

### 结论（证据链）
1. **Actor.cpp 怪物阴影逻辑与参考 Mir3China 代码逐行一致**（精确 diff CActor::DrawActor 5190-5270 阴影块：81 行中仅 6 处注释编码乱码差异，全部代码/参数/调用一致）。怪物路径：CActor::DrawActor（Actor.cpp:5110）→ 阴影图(pxSahdowImage=m_xImageList[m_bActorImgIdx+_MAX_MONSTER_IMAGE])帧有效则直接画(DrawWithImageForCompClipRgn:5233 / DrawWithABlendCompDataWithBackBufferEx:5225)，无效则回退 DrawWithShadowABlend(5253/5262) 剪影阴影。
2. **调用方不区分**：GameProc.cpp RenderActorList(:3276) 对 角色/NPC/怪物 统一 `bDrawShadow=TRUE`，同循环同参数。
3. **共享阴影函数按知识库 4.2 RLE 格式解码正确**：DrawWithShadowABlend(WHDXGraphic.cpp:3872) 行前缀+0xC0跳过/0xC1拷贝/0xC2/0xC3混合，未知码 return FALSE；DrawWithImageForCompClipRgnBase(:2917) 索引式前缀(nWidthEnd++计入前缀词)正确；DrawWithABlendCompDataWithBackBufferEx(:3718) `nxt=src+*src++`(修复后写法,无off-by-one)。
4. **backbuffer 兼容正确**：m_pddsBackBuffer=CDX9SoftSurface*(WHDXGraphic.h:158)，Lock(:56)正确填 lpSurface=m_pBits/lPitch，DrawWithShadowABlend 能写软缓冲。_BE16 宏当前未使用(92dcbcd 已回退 big-endian RLE)。
5. **阴影系统有 T28 回退史**：fc976c4(改 big-endian RLE 修"阴影层画黑")→92dcbcd(回退)、45bdd5a/736925a("去黑阴影/怪特效")——共享 WHDXGraphic 阴影相关曾反复改动，是主 agent 排查重点。

### 未决与移交
- **关键未决问题**：无法静态确认——(a) 是否所有角色(玩家/NPC/怪物)都无阴影，还是仅怪物？(a) 判定根因在共享 DrawWithShadowABlend(影响所有) 还是怪物专属路径(阴影图未加载/monster bShadowType)。只能实机观察。
- Actor.cpp **无需改动**（无确认缺陷，与参考一致）。阴影缺失根因指向共享 WHDXGraphic 阴影渲染或阴影图数据(加载/bShadowType)，移交 MainAgent。
- **请用户实测**：当前构建 D:\Client\Mir3_dx9.exe 进图观察——玩家(自己)、NPC、怪物 三者地面阴影是否存在？若三者皆无=共享函数问题；仅怪物无=怪物阴影图/数据问题。附截图最佳。正常端反汇编已存 D:\Ada-WorkSpace\normal_client_disasm.txt(19MB) 供对照。

---

---

## [2026-10-05] AB03 实测结论（用户反馈）→ 已解决

**用户实测当前构建 D:\Client\Mir3_dx9.exe**：怪物有影子、玩家自己也有影子、NPC 也有影子（三者皆正常）。

**结论**：AB03 "怪物无阴影" 在当前构建**不可复现**，验收"怪物/动物在地面有阴影"**已满足**。与我静态取证一致（Actor.cpp 阴影逻辑与参考 Mir3China 代码逐行一致、共享阴影函数按 KB4.2 RLE 解码正确、backbuffer Lock 正确）。此前的"无阴影"报告应来自更早构建/已在此前提交中修复（阴影系统 T28 曾有反复，可能已随后续构建解决）。

**处置**：AB03 关闭。Actor.cpp 无需改动。无需移交共享代码。

---

---

## [2026-10-05] AB01 选角魔法特效位置修复（Agent B）

**任务**：选角界面魔法效果显示不正确（用户反馈：位置不对 + 周边黑色阴影/不够透明）。分配文件 CharSelectProcess\ChrProc.cpp。

### 用户关键帧表（法师男 800-1094，已与 WIL 离线提取双重确认）
- 本体 860-871（frm860 40x94 shpx=7 shpy=-44）
- 影子 880-891（frm880 56x36 深色红纹理, +20）
- **魔法效果 900-911（frm900 64x128→905 128x128→910 256x128 橙黄火焰, +40）**
- 微动作 920-930（frm920 72x86）
- 微动作影子 940-950（frm940 100x34, +20）
- 选中他人时法师微动作 980-990

### 帧偏移规律确认（与代码一致）
代码画 frm+20=影子、frm+40=DrawCharEffect 魔法火焰，**帧选择与原版一致**。用户确认"选的规律和原版一致，只是位置不一样"。

### 根因定位（静态+实证）
DrawCharEffect（ChrProc.cpp:278-287）画在裸基位 (nX,nY)，**未加特效帧 shPX/shPY**。参考原版 D3D 位置数学 
X + shWidth/2 + shPX → 帧左上=基位+shPX/shPY。法师 frm900 shPY=-53 → 本端火焰画低 53px，压到脚部 +20 影子（frm880 深色块）= 用户所见"魔法特效周边黑色阴影"。

### 修复（已提交 2a5972b，已部署 D:\Client\Mir3_dx9.exe）
DrawCharEffect 内 NewSetIndex 后补加：
`cpp
INT nEx = nX + m_xImageChr.m_lpstNewCurrWilImageInfo->shPX;
INT nEy = nY + m_xImageChr.m_lpstNewCurrWilImageInfo->shPY;
`
select/create 共用本函数一次修复两处。火焰上移脱离影子，位置错+黑框应同时解决。bOpa=50 未动（参考端亦 50；如需更透明再降）。

### 待用户实测
进选角界面观察法师男魔法火焰：位置是否回身周、周边黑框是否消失。

---

## [2026-10-05] AB01 选角魔法黑影修复（Agent B，二次修复）

**用户实测**：位置修复（shPX/shPY）后位置正确、效果好，但**仍有魔法黑影**。用户提供 SCREEN0080-0119.bmp 截图（15:55 会话）。

### 黑影根因（实证：WIL 火焰帧像素统计）
法师魔法火焰帧 frm900-911 的**非透明像素绝大多数是黑色**（帧内部背景）：
- frm900 64x128：fill=3574，暗色(lum<60)=3450（96%），纯黑(0,0,0)=2650
- frm905 128x128：fill=3698，暗色=3519（95%），纯黑=2714
- frm910 256x128：fill=12666，暗色=8574，纯黑=6363
- frm911 256x256：fill=10930，暗色=9363，纯黑=5722

### 原版处理（参考端 D3D 实证）
D3DWILTextr_CreateEmptyTexture(..., D3DTEXTR_TRANSPARENTBLACK) 把**黑色像素设为全透明**，再用 SetBlendRenderState(_BLEND_LIGHT)（加色混合）。黑像素加0=不变=不可见，只显橙火。

### 本端缺陷
DrawCharEffect 用 DrawWithABlendCompDataWithBackBuffer(...bOpa=50) **alpha 混合**，把火焰帧的黑色背景像素画成黑色（叠加暗背景→更黑）→ 用户所见"黑色放射状黑影"。

### 修复（已提交 e641ddb，已部署 D:\Client\Mir3_dx9.exe）
DrawCharEffect 改**自包含软件加色(additive)混合**（ChrProc.cpp 内，直接 g_xMainWnd.GetBackBuffer()->m_pBits 写 565 软缓冲）：
- 遍历 RLE：0xC0=透明跳过；0xC1/C2/C3=逐像素加色（565 各通道相加 clamp）。
- 黑像素(0x0000)加0=不变=透明；亮像素加亮。完全复刻原版 TRANSPARENTBLACK+_BLEND_LIGHT。
- 位置保留 shPX/shPY 修正（帧左上=基位+shPX/shPY）。
- select/create 共用本函数一次修复。无文件归属越界（未改 WHDXGraphic.cpp）。

### 待用户实测
进选角界面观察法师男魔法火焰：黑色放射状黑影是否消失、火焰是否亮橙正常。

---

## [2026-10-05] AB05 进游戏打开NPC对话框有黑边（Agent B）

**任务**：进游戏打开 NPC 对话框有黑边（用户实测反馈，截图 SCREEN0120/0121.bmp）。分配文件 GameProcess\NPCWnd.cpp。

### 根因（静态+截图实证）
ShowNpcWnd（NPCWnd.cpp:460-477）把对话框 surface（m_pddsSurface）**不透明 Blt**（CopyMemory）到 backbuffer，**忽略 m_bOpa(220) 与 m_bBright**：
- surface 创建时填充 RGB(0,0,0)（SetNPCWndTextr:396-397，与参考端一致，填充色非差异点）。
- surface 按 2 的幂 padding（nRW=386→m_nSurfaceW=512，nSX=63），Blt 画全 surface → 黑色 padding 也画出。
- **差异点**：参考端 DX7 `DrawBillBoard(..., &vecScale=(m_nSurfaceW,m_nSurfaceH), &mtrl, m_pddsSurface)` 用 `mtrl.diffuse.a=m_bOpa/255`（m_bOpa=220）+ 亮度 m_bBright → 半透明暗窗，黑 padding 半透明不突兀；本端 Blt 不透明 → 实心黑盒（含黑色 padding）= 黑边。

截图实证（dialog_bounds.py）：对话框实心黑盒 x=9..448 y=45..210（440px 宽 > 窗口 nRW=386，含 padding），金边框稀疏。

### 修复（NPCWnd.cpp ShowNpcWnd，软件 alpha 混合）
Blt 改自包含软件 alpha 混合（直接 g_xMainWnd.GetBackBuffer()->m_pBits 写 565 软缓冲，复刻参考端 DrawBillBoard alpha 行为）：
- 逐像素 565 三通道按 m_bOpa 混入：dst = (src*bright*a + dst*ia)/255，src 乘 m_bBright。
- 黑色 fill(src=0) 按 a 加权 → 半透明暗窗，黑 padding 随 alpha 变柔和，黑边消除。
- 复刻参考端 `mtrl.diffuse.a=m_bOpa/255` + `D3DUtil_InitMaterial(m_bBright)`。

### 状态
- 编译 compiled 82 fail 0 + LINK 0 + SUCCESS。已部署 D:\Client\Mir3_dx9.exe（2602892 字节）。
- Gitea issue #9 已建（含根因+修复方向）。待用户实测进游戏打开 NPC 对话框验证黑边消除。


### AB05 修复更正（官方原版色键，二次修复）

**用户实测第一次 alpha 混合修复后仍一样**（bOpa=220 为 86% 不透明，视觉几乎无差别）。用户指示"按照官方原版的来"。

**关键取证（参考端 SetNPCWndTextr / SetBlendRenderState）**：
- `SetNPCWndTextr:399`：`DDSetColorKey(m_pddsSurface, RGB(0,0,0))` —— **黑色设为透明色键**。
- `SetBlendRenderState(:339)`：`D3DRENDERSTATE_COLORKEYENABLE=TRUE` + `_BLEND_NORMAL`(case 0)。
- `_BLEND_NORMAL`(case 0, diffuse.a<1)：alpha=纹理alpha×材质alpha；SRCBLEND=D3DBLEND_ONE（源全加），DESTBLEND=D3DBLEND_INVSRCALPHA（dst 按 1−alpha 衰减）。
- DrawBillBoard 材质 diffuse = m_bBright/255（源色乘亮度）。

**官方渲染语义**：黑色像素(0x0000)=色键透明→场景透出（黑填充/padding 不画，**无黑盒/黑边**）；非黑像素 dst = src×bright + dst×(1−alpha)。

**本端缺陷**：ShowNpcWnd 不透明 Blt(CopyMemory) 忽略色键/alpha/亮度 → 黑填充+padding 不透明覆盖 → 实心黑盒。

**二次修复（NPCWnd.cpp ShowNpcWnd，软件复刻官方色键+混合）**：
- 逐像素 565：`if (s == 0x0000) continue;`（色键黑=透明，跳过）。
- 非黑：`dst = src*bright/255 + dst*(255-a)/255`，alpha=m_bOpa，src 乘 m_bBright。

**状态**：编译 82 fail 0 + LINK 0 + SUCCESS，已部署 D:\Client\Mir3_dx9.exe。待用户实测进游戏打开 NPC 对话框验证黑边消除、对话框正常（黑填充透明、场景透出、仅窗框/文字半透明显示）。

---

## [2026-10-05] AB06 快捷栏只显示竖版，横竖切换失效（Agent B）

**任务**：快捷栏(BeltWnd)只显示竖着的图像，应为横/竖两种模式切换（用户实测反馈，截图 16:52 会话蓝色圈标记竖版面板）。

### 根因（WIL 帧实证）
GameInter.wil（_IMAGE_INTER=index70）帧提取：
- **frm320 = 512x64（横版腰带框）**
- **frm321 = 52x274（竖版腰带框）**

端口 `Define.h:711` `_WNDIMGIDX_BELT=321`，参考端 Mir3China `_WNDIMGIDX_BELT=320`（off-by-one）。
- CreateBeltWnd(Interface.cpp:340) 用 `_WNDIMGIDX_BELT` 作窗口帧。
- ShowBeltWnd 横版(_HORIZONTAL_MODE=1)走 ShowGameWnd 画窗口帧；竖版硬编码 `NewSetIndex(321)`。
- 端口把 _WNDIMGIDX_BELT 错改成 321 → **横版也画竖版 frm321(52x274)** → 横竖都显示竖版；横版窗内画竖条 = 用户所见"反转/只显竖版"。

### 修复（Define.h:711）
`_WNDIMGIDX_BELT` 321→320（对齐参考端）。横版画 frm320(512x64)，竖版仍硬编码 frm321(52x274)，两模式正确。仅此一处引用，无副作用。

### 状态
编译 82 fail 0 + LINK 0 + SUCCESS，已部署 D:\Client\Mir3_dx9.exe。Gitea issue #12。待用户实测快捷栏横/竖切换。

---

## [2026-10-05] AB07 聊天窗口还有黑边（Agent B）

**任务**：进入游戏后聊天窗口仍有黑边（用户实测反馈，最新截图 17:13 会话，蓝圈标注聊天窗区块；并附注要求合并最新代码）。

### 根因
ChatWnd.cpp `ShowChatWnd()` 与 AB05 NPC 对话框**完全同源 bug**：
- 聊天表面 `mChat_pddsSurface` 用 `CDX9SoftSurface` 建（:350-355），填黑 `RGB(0,0,0)`（:362-363）；
- 窗框 DrawWithImageForComp 画到表面（:382-404）；
- 最后 `GetBackBuffer()->Blt(&rcDst, mChat_pddsSurface, ...)`（原 :101）**不透明 Blt** → 黑填充+padding 被不透明覆盖成实心黑盒（黑边）。
- 参考端 `D:\Mir3China\GameProcess\ChatWnd.cpp:100` 用 `DrawBillBoard(_BLEND_NORMAL: SRCBLEND=ONE, DESTBLEND=INVSRCALPHA, mtrl.Diffuse.a=100/255)` 色键+alpha 渲染。

### 修复（软件复刻，同 AB05 已验证色键模式）
ShowChatWnd 原不透明 Blt（:100-101）替换为软件逐像素混合：
- 黑色像素 `s==0x0000` → 跳过（色键透明，场景透出，无黑边）；
- 非黑 `dst = src*bright + dst*(1-alpha)`，alpha=100/255（参考端 mtrl.Diffuse.a），brightness=m_bBright；
- backbuffer 565 逐像素，offset baseX=mChat_nSX+m_rcWnd.left / baseY=m_rcWnd.top-nStartY。
- **字节级拼接替换**（避免 Edit 工具重编码损坏 GBK 韩文注释，吸取 Define.h AB06 教训），ChatWnd.cpp 仅 37+/2- 干净 diff。

### 状态
编译 82 fail 0 + LINK 0 + SUCCESS，已部署 D:\Client\Mir3_dx9.exe（2603916 字节）。commit 743b17c。待用户实测聊天窗口无黑边。

---

## [2026-10-05] T84 环境设置窗口不显示（Agent B / admin）

**任务来源**：Gitea issue #11 任务分配，T84 分配给我（admin/Agent B）。现象：点击游戏内"环境设置"后无窗口/无图像。

### 根因（WIL 帧实证）
环境设置窗口 = CSettingWnd，Interface.cpp:357 CreateSettingWnd 用 `_WNDIMGIDX_SETTING` 作窗框帧号。
- 端口 Define.h:710 `_WNDIMGIDX_SETTING = 311`
- 参考端 Mir3China Define.h:710 `_WNDIMGIDX_SETTING = 310`（off-by-one，同 AB06 belt 模式）

正确客户端数据 `D:\Client\Data\GameInter.wil`（60MB，非 D:\传奇3客户端 13MB 版）帧提取实证：
- **frm310 = 256x512 环境设置窗框**（竖版面板，9 个按钮槽 + 金色描边 + 右下红 X 关闭钮，与 ShowSettingWnd 布局吻合）
- **frm311 = 92x20 小按钮帧**（非窗框）

端口错用 311 画窗框 → 画成 92x20 小图 → 环境设置窗口"无窗口/无图像"。按钮帧（311）与参考端一致，不动。

### 修复（Define.h:710 字节级）
`_WNDIMGIDX_SETTING` 311→310（字节级修改末字节 0x31→0x30，保留 GBK 韩文注释）。仅 1+/1- 干净 diff。

### 附带发现：12 个 _WNDIMGIDX_* 宏 off-by-one（端口=参考+1）
完整对比 Define.h，除 SETTING 外还有 11 个同类 off-by-one（T72/T73 修正 22 宏后残留）：
- _WNDIMGIDX_CHAT 151/150、EXIT 411/410、FACEIMGUP 681/680、GUILD 281/280、NOTICEEDIT 285/284、OPTION 291/290、RANKEDIT 285/284、STATUS 181/180、STAUS_OTHERS 201/200、STORE 391/390、VIEWMINIMAP 671/670
- 尚未逐一 WIL 实证（本次仅实证 SETTING=310）。是否批量修复待用户/MainAgent 决定。

### 状态
编译 82 fail 0 + LINK 0 + SUCCESS，已部署 D:\Client\Mir3_dx9.exe（2603916 字节）。commit b5e22f7。待用户实测环境设置窗口正常显示。

---

## [2026-10-05] T84 续：批量修复 11 个 _WNDIMGIDX_* 宏 off-by-one（Agent B）

**任务**：用户确认需批量修复，**记录保留便于回退**。

### WIL 实证表（D:\Client\Data\GameInter.wil，60MB，客户端实际数据）
| 宏 | 端口值(原) | 参考值 | frm[端口]（错误） | frm[参考]（正确） |
|----|-----------|--------|------------------|-------------------|
| _WNDIMGIDX_CHAT | 151 | **150** | 12x18 小图标 | 512x32 聊天条 |
| _WNDIMGIDX_STATUS | 181 | **180** | 20x18 小图标 | 512x512 状态窗 |
| _WNDIMGIDX_STAUS_OTHERS | 201 | **200** | 28x26 小图标 | 512x512 状态(他人)窗 |
| _WNDIMGIDX_GUILD | 281 | **280** | 76x26 小按钮 | 1024x512 行会窗 |
| _WNDIMGIDX_NOTICEEDIT | 285 | **284** | 52x24 小按钮 | 1024x256 公告编辑窗 |
| _WNDIMGIDX_RANKEDIT | 285 | **284** | 52x24 小按钮 | 1024x256 排名编辑窗 |
| _WNDIMGIDX_OPTION | 291 | **290** | 512x512(5行ON面板) | 512x512 选项窗(NEXT+ON) |
| _WNDIMGIDX_STORE | 391 | **390** | 512x256 仓库窗(左右双面板) | 512x256 商店窗(4展示栏) |
| _WNDIMGIDX_EXIT | 411 | **410** | 72x20 小按钮 | 256x128 退出窗 |
| _WNDIMGIDX_VIEWMINIMAP | 671 | **670** | 52x22 小按钮 | 1024x1024 小地图 |
| _WNDIMGIDX_FACEIMGUP | 681 | **680** | 32x32 小图标 | 1024x512 头像/脸图 |

规律：端口值=参考+1（off-by-one），端口值为小按钮/错误帧，参考值为正确窗框。

### 修复
Define.h 字节级（Python 脚本 fix_t84.py：跳过 `_NEW_` 版本 + 仅 `#define _WNDIMGIDX_XXX` 行 + 改末位数字字节 0x31→0x30 / 0x35→0x34），保留 GBK 韩文注释。11+/11- 干净 diff，commit 154cfa4。

**回退**：`git revert 154cfa4` 或 `git checkout 154cfa4~1 -- Define.h` 即还原 11 宏原值。提取脚本 extract_t84.py 保留。

### 状态
编译 82 fail 0 + LINK 0 + SUCCESS，已部署 D:\Client\Mir3_dx9.exe（2604428 字节）。待用户实测各窗口（聊天/状态/行会/选项/商店/小地图等）帧正常。
============================================================
[REG] 商业服务器模式 - 登录界面创建账号 + 充值120小时 (2026-10-05)
后端: D:\Ada-WorkSpace\mir3-backend\mir3backend.cpp (C++, mingw)
  - TCP 8501 行式协议: REGISTER|账号|密码 / CHARGE|账号|小时 / QUERY|账号
  - ODBC 连 127.0.0.1,1433 sa/<已移除> DATABASE=MirAccount
  - 注册写 TBL_ACCOUNT: FLD_LOGINID/FLD_PASSWORD(明文)/FLD_SECEDE='N'/FLD_MAKETIME
  - 充值120h写基础时间字段: FLD_VALIDUNTIL=+5天, FLD_SECONDS=+432000, FLD_RemainDays=+5
  - backend.ini: ConnStr/ChargePrefix(空=基础字段,可设M2/M3)/Port=8501/DefaultHours=120
  - 实测: 注册OK/重复EXIST/充值OK:120/查询时间字段正确; SQL直连复核一致
客户端: LoginProcess\LoginReg.cpp (Win32模态对话框)
  - 登录界面点"创建账号"弹出弹窗(账号/密码/确认密码+确定/取消), 不再跳网页
  - 提交 REGISTER 到 mir3.ini [Initial] ServerAddr 的 8501 端口(RegPort可配)
  - 成功自动回填登录ID并切到密码框; 重复/网络错误/密码不一致均有提示
  - LoginProc.cpp REG按钮处理器: ShellExecute url -> DoCreateAccountDialog()


---

## REG-崩溃修复: 注册弹窗 PostQuitMessage 污染主消息循环

**现象**: 登录界面"创建账号"弹窗, 注册成功后游戏崩溃/退出。

**根因**: LoginReg.cpp 模态弹窗在 WM_DESTROY 里调用 PostQuitMessage(0),
把 WM_QUIT 注入到线程消息队列。模态循环(GetMessageW)退出后, 残留的
WM_QUIT 被游戏主消息循环 GetMessage 收到, 导致主循环终止 -> 游戏退出。

**修复**: 改为标志位结束模态循环, 不再 PostQuitMessage:
- 新增全局 volatile g_dlgDone
- WM_DESTROY: g_dlgDone=1 (不注入 WM_QUIT)
- 模态循环: while(!g_dlgDone){ PeekMessageW+Dispatch / WaitMessage }

**验证**: 编译 83 fail 0, LINK 0, 部署 D:\Client\Mir3_dx9.exe (2611921B)。


## [RECH] 客户端登录主界面卡密充值入口 (2026-10-05)
**需求**: 商业服卡密收费。服务器已有卡密系统(后端 GEN/CARD 协议, TBL_CARD 表, 月卡30天/点卡120h), 客户端登录主界面缺卡密提交入口 -> 客户端-服务器不匹配。
**实现**:
- LoginRecharge.cpp (UTF-8 编码, 对齐 LoginReg.cpp): 充值弹窗(账号+卡密两格+确定/取消+状态栏), 提交 CARD|卡密|账号 到 mir3.ini [Initial] ServerAddr:RegPort(默认8501)。
  响应: OK:month:30 -> "充值成功：月卡30天"; OK:hour:120 -> "点卡120小时"; USED/NOTFOUND/网络错误分别提示。
- LoginProc.h: 新增成员 m_rcRecharge + 声明 int DoRechargeDialog(void)。
- LoginProc.cpp: Init 置零 m_rcRecharge; RenderLogin 用 PutsHan 绘制"卡密充值"文字热区(500,380,585,408, 退出按钮右侧, 悬停反色); OnLButtonUp 检测点击弹窗。
- 模态循环沿用 g_dlgDone 标志(不 PostQuitMessage, 避免污染主消息循环崩溃)。
**注意**: 客户端源码大部分 GBK/EUC-KR, 但新源文件必须 UTF-8(gcc 默认 UTF-8 输入, 宽字符串 L".." 内 GBK 字节报 Illegal byte sequence)。
**验证**: 编译 84 fail 0, LINK 0, 部署 D:\Client\Mir3_dx9.exe。后端 8501 已启动待实测。


## [T89] 换装/穿衣显示异常修复 + 调试日志清理 (2026-10-06)
**现象**: ①穿衣布衣后衣服落段错误(落到 dress 7 / 21000 段, 布衣应为 dress 2 / 帧 6000-7915); ②武器显示错误; ③走路残影/拖影。
**根因(日志实证)**: 在 GameProc 主消息 switch 入口加全局 g_wCurrWIdent 追踪 + CHero::ChangeFeature 日志输出, 确认 wId=41 (SM_FEATURECHANGED) 在 SM_TAKEON_OK(615)/SM_TAKEOFF_OK(619) 之后紧接广播, 携带角色**旧状态**特征 {dress=7, hair=65, wpn=144}, 覆盖穿衣后的新状态。
  日志序列: DX9-TAKEON recog=... wpn=9 dress=2 hair=129(正确穿衣布衣) → DX9-FEATCHG dress=7 hair=65 wpn=144 → DX9-CHGFEAT wId=41 dress=7(被覆盖)。
**修复(采纳ADA分析, 弃用"只保 dress"方案)**:
- 修复A: GameProc.cpp case SM_FEATURECHANGED 主角**完全忽略** OnFeatureChanged(注释调用), 其他玩家照常处理; 主角外观由 TAKEON_OK/TAKEOFF_OK 权威。
- 残影: CHero::DrawActorAfterImage 开头 return FALSE 直接禁用绘制(此前 ZeroMemory 清零已证无效——跑步持续重存快照)。
- 保留既有: NewSetIndex 失败向前搜有效帧(段首帧占位) + weapon NULL 防崩溃 + bOldDress 换段跳转 + ChangeFeature 残影清零。
**验证**: 用户实测确认衣服+武器显示问题解决, T89 验收通过。
**后续清理(按用户要求"删除调试日志与影响性能代码")**: 删除全部 DX9- 诊断日志(SEG/CHGDRESS/HPMSG/HPBAR/CHARDESC/FEATCHG/CHGFEAT/WPNF/WPNFAIL/WEARFRAME/OUTRANGE/DRAWFRM/BEFORE/AFTER/TAKEON/TAKEOFF/T70/ABILITY/QUEST/STORE/GAMEOVER 等) + 临时全局 g_wCurrWIdent/g_dwWearLogEnd/g_dwWearDumpCnt + 性能杀手(每帧后缓冲 Lock 采样 + D:\Client\FR*.bmp 写盘)。仅保留修复逻辑。编译 84 fail 0, LINK 0。


## [T90] ��װƫ�Ƹ����޸���shPX ������ֵ (2026-10-06)
**����**: ��װ�޸�(3268df9/T89)���������ܶ�+ͷ��ƫ��; ���ύ 953132c �ܶ�/ͷ��/�·�ȫ������(��֤ 3268df9 ����)��
**��λ����(���ֻ���ʵ֤)**: �ѻ��� ChangeFeature fst+1��DrawActor ����֡�������ָ� SM_FEATURECHANGED ���Ǵ���������ƫ�� -> �ų���
**����(ADA+����ʵ֤)**: Actor.cpp:9132 3268df9 �� shPX ��ֵ���� `if (m_dwCurrFrame Խ��)` �����ڣ�������Χ�� shPX ������ֵ������ -> SetRect(����/ͷ��) �þ� shPX -> �ܶ�ÿ֡����ƫ�ơ�
**�޸�**: ɾ����Խ�� if��shPX �ָ���������ֵ���� 953132c һ�£�������ȫ�� T89 ��װ�޸���SM_FEATURECHANGED ����/��Ӱ����+����/����֡����/fst+1����
**��֤**: �û�ʵ�� �ܶ���ƫ�ơ�ͷ����������װ���������� 84 fail 0, LINK 0, ���� D:\Client\Mir3_dx9.exe��


## [天气T23] 特效层根因修复: 雨/雪/雾全不显示 (2026-10-14)
**现象**: 自动/雪/雨/雾轮巡切换后, 三种天气全部不显示(又绕回去)。
**根因定位(代码实证)**:
- Present()(WHDXGraphic.cpp L6818-6906)内部自己 BeginScene -> sprite.Draw(软缓冲纹理) -> EndScene -> Present(swap), 完成主渲染输出。
- 特效层 g_EffectLayer.Render() 原在 Mir2Ex.cpp L348 于 Present() **之后**执行, 画到已 swap 的 backbuffer, **没有再次 swap**; 且下一帧 Present 会用软缓冲全屏覆盖 -> 特效层内容**永远不显示**。
- 之前加的正交投影方向正确(粒子 RenderSystem 假设正交800x600), 但特效层整体根本没被显示, 投影再对也无济于事。
- 散射/尾迹等 RenderScene 直绘(Present 前)同样会被软缓冲覆盖不可见。
**修复**:
1. WHDXGraphic.cpp Present() 末尾: 软缓冲 EndScene() 后、Present(swap) 前插入 `g_EffectLayer.Render()`, 特效叠加在同一 backbuffer 之上一起 swap 显示。
2. WHDXGraphic.cpp 顶部 `#include "EffectLayer.h"`(build inc 含 GameProcess)。
3. Mir2Ex.cpp 删除主循环 L348 的 `g_EffectLayer.Render()`(避免重复)。
- 特效层 Render() 内各 RenderSystem 自己 Begin/End(此时无外层 BeginScene), 正常执行。
**验证**: 编译 85 fail 0, LINK 0; 部署 D:\Client\Mir3.exe 21:48:08。
**注意(待实测)**: 特效层现叠加在含 UI 的软缓冲之上, 天气粒子落在 UI 区域会盖住 UI; 用户要求天气在 UI 之下, 若实测遮挡明显需后续处理。

## T25 [2026-10-14] Weather soft-render (snow/rain/mist) - D3D effect-layer abandoned
- Root cause: D3D billboard effect-layer rendered in Present() AFTER soft backbuffer already uploaded -> always overwritten next frame; alpha blind-tuning was a dead end.
- Fix (per SKILL_Mir3_weather_softrender): soft-render particles directly onto 565 soft backbuffer in RenderScene (after scene, before UI).
- Particle.cpp: WeatherSoftBlit (nearest scale + colorkey 0x0000 transparent + screen clamp), CSnow::RenderSoft (tex 500+frame, <=16px), CRain::RenderSoft (air line 500+frame 2xH / splash 510+frame 32x32), CMist::RenderSoft (tex 550, 50%% semi-transparent full-screen).
- Particle.h: 3 RenderSoft() decls.
- GameProc.cpp: removed g_EffectLayer.Add(DrawWeatherLayer); inserted soft-render weather after DropItemShow, before UI.
- Compiled 85 fail 0, LINK 0. Deployed D:\Client\Mir3.exe size 2621302, backup Mir3.exe.bak_20261014225302.

## T26 [2026-10-14] Weather soft-render switched to DrawBillBoard (black-flake bug fixed)
- User test: snow/rain now SHOW but rendered BLACK (old code used m_xImageList[_IMAGE_PROGUSE].NewSetIndex + DrawWithImageForCompToMem decode -> wrong colours / black blocks).
- Root fix: weather particles now render via CImageHandler::DrawBillBoard, which is ALREADY software-blitted to the soft backbuffer (T59) and uses GetTextrImg(_TEXTR_FILE_PARTICLE,_IMAGE_PROGUSE,frame) textures (same as working D3D path).
- Particle.cpp: CSnow::RenderSoft / CRain::RenderSoft / CMist::RenderSoft rewritten to DrawBillBoard. Removed broken WeatherSoftBlit + DrawWithImageForCompToMem path.
  - snow: tex 500+frame, scale fSize*4, white
  - rain: air line 500+frame scale(2, h); splash 510+frame scale(32,32)
  - mist: full-screen light-cyan 35%% alpha colour fill (DrawBillBoard NULL-tex blend), background stays visible
- Compiled 85 fail 0. Deployed D:\Client\Mir3.exe size 2620175, backup Mir3.exe.bak_20261014230927.
