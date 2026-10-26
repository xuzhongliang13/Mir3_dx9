# 传奇3网络协议文档

## 1. 通信基础

### 1.1 连接信息
- **协议类型**: TCP
- **字节序**: 小端序 (Little Endian)
- **字符编码**: GBK/GB2312

### 1.2 端口映射
| 端口 | 服务 | 说明 |
|------|------|------|
| 7000 | LoginGate | 登录网关 |
| 7100 | SelChrGate | 角色选择网关 |
| 7200 | RunGate | 游戏运行网关 |

### 1.3 数据包格式
```
# + 消息类型(1字节) + 编码数据 + !
```

- **起始符**: `#` (0x23)
- **消息类型**: 第2字节，ASCII字符
- **编码数据**: 6位编码后的数据
- **结束符**: `!` (0x21)

---

## 2. 加解密算法

### 2.1 基础6位编码 (fnEncode6BitBuf)

**用途**: 登录消息编码 (wIdent 2000-2004, 100-104)

**算法**:
```c
// 将8位数据转换为6位编码
chMade = ((chRest | (pszSrc[i] >> (2 + nRestCount))) & 0x3f);
chRest = (((pszSrc[i] << (8 - (2 + nRestCount))) >> 2) & 0x3f);
pszDest[nDestPos++] = chMade + 0x3c;
```

**特点**:
- 每个字节加 0x3C
- 输出范围: 0x3C-0x7B ('<' 到 '{')

### 2.2 游戏消息编码 (fnEncode6BitBufGameMessage)

**用途**: 游戏内消息编码

**算法**:
```c
// 先 XOR 24，然后6位编码
chMade = ((chRest | ((pszSrc[i]^24) >> (2 + nRestCount))) & 0x3f);
pszDest[nDestPos++] = chMade + 0x4a;
```

**特点**:
- 先 XOR 24 (0x18)
- 每个字节加 0x4A
- 输出范围: 0x4A-0x89

### 2.3 解码算法

**fnDecode6BitBufOriginal** (基础解码):
```c
ch = pszSrc[i] - 0x3c;
ch = ch & 0x3f;
// 6位解码回8位
```

**fnDecode6BitBufGameMessage** (游戏消息解码):
```c
ch = pszSrc[i] - 0x4d;
ch = ch & 0x3f;
// 6位解码
chCode = chCode ^ 134;  // XOR 134 (0x86)
```

---

## 3. 消息结构体

### 3.1 通用消息头 (_TDEFAULTMESSAGE)

```c
typedef struct tag_TDEFAULTMESSAGE {
    int   nRecog;    // 识别码 (4字节)
    WORD  wIdent;    // 消息ID (2字节)
    WORD  wParam;    // 参数 (2字节)
    WORD  wTag;      // 标签 (2字节)
    WORD  wSeries;   // 序列号 (2字节)
} _TDEFAULTMESSAGE;  // 总计 12字节
```

### 3.2 消息头制作函数

```c
void fnMakeDefMessage(_LPTDEFAULTMESSAGE lptdm, 
                      WORD wIdent,    // 消息ID
                      int nRecog,     // 识别码
                      WORD wParam,    // 参数
                      WORD wTag,      // 标签
                      WORD wSeries);  // 序列号
```

---

## 4. 消息ID列表

### 4.1 登录消息 (CM_)

| ID | 名称 | 值 | 方向 | 说明 |
|----|------|-----|------|------|
| CM_PROTOCOL | 协议版本 | 2000 | C→S | 协议握手 |
| CM_IDPASSWORD | 账号密码 | 2001 | C→S | 登录请求 |
| CM_ADDNEWUSER | 注册账号 | 2002 | C→S | 新用户注册 |
| CM_CHANGEPASSWORD | 修改密码 | 2003 | C→S | 修改密码 |
| CM_UPDATEUSER | 更新用户 | 2004 | C→S | 更新用户信息 |

### 4.2 角色选择消息

| ID | 名称 | 值 | 方向 | 说明 |
|----|------|-----|------|------|
| CM_QUERYUSERNAME | 查询角色 | 80 | C→S | 查询角色列表 |
| CM_CHANGENAME | 修改角色名 | 104 | C→S | 修改角色名称 |

### 4.3 游戏动作消息 (SM_)

| ID | 名称 | 值 | 方向 | 说明 |
|----|------|-----|------|------|
| SM_TURN | 转向 | 10 | S→C | 角色转向 |
| SM_WALK | 行走 | 11 | S→C | 角色行走 |
| SM_RUN | 跑步 | 13 | S→C | 角色跑步 |
| SM_HIT | 攻击 | 14 | S→C | 普通攻击 |
| SM_HEAR | 听到 | 40 | S→C | 听到消息 |
| SM_LOGON | 登录 | 50 | S→C | 登录成功 |
| SM_NEWMAP | 新地图 | 51 | S→C | 切换地图 |
| SM_SYSMESSAGE | 系统消息 | 100 | S→C | 系统消息 |

### 4.4 客户端命令 (CM_)

| ID | 名称 | 值 | 方向 | 说明 |
|----|------|-----|------|------|
| CM_PICKUP | 拾取 | 1000 | C→S | 拾取物品 |
| CM_DROPITEM | 丢弃 | 1001 | C→S | 丢弃物品 |
| CM_TAKEONITEM | 装备 | 1003 | C→S | 装备物品 |
| CM_TAKEOFFITEM | 卸下 | 1004 | C→S | 卸下装备 |
| CM_SAY | 说话 | 3030 | C→S | 聊天消息 |
| CM_TRUN | 转向 | 3010 | C→S | 客户端转向 |
| CM_WALK | 行走 | 3011 | C→S | 客户端行走 |
| CM_RUN | 跑步 | 3013 | C→S | 客户端跑步 |

---

## 5. 登录流程

### 5.1 登录网关握手

```
客户端 → LoginGate (7000):
  #2<<<<<I@C<<<<<<<<ZCehYs`qH_<kZCehYs`qH_<!
  
服务端 → 客户端:
  #<<<<<=@><<<<<<<<lxwKvxcmjw<dHN`w!
```

### 5.2 登录请求

```
客户端 → LoginGate:
  消息类型: CM_IDPASSWORD (2001)
  数据: 账号/密码（6位编码）
```

### 5.3 角色选择

```
客户端 → SelChrGate (7100):
  消息类型: CM_QUERYUSERNAME (80)
  数据: 查询角色列表
```

### 5.4 进入游戏

```
客户端 → RunGate (7200):
  消息类型: SM_LOGON (50)
  数据: 角色信息
```

---

## 6. 数据包示例

### 6.1 登录请求包

**HEX**:
```
23 32 3C 3C 3C 3C 3C 49 40 43 3C 3C 3C 3C 3C 3C
3C 3C 5A 43 65 68 59 73 60 71 48 5F 3C 6B 5A 43
65 68 59 73 60 71 48 5F 3C 21
```

**解析**:
- `23` = `#` (起始符)
- `32` = `2` (消息类型)
- `3C 3C 3C 3C 3C` = `<<<<<` (固定填充)
- `49 40 43` = `I@C` (标识)
- `5A 43 65 68 59 73 60 71 48 5F` = 编码数据
- `21` = `!` (结束符)

### 6.2 心跳包

**HEX**:
```
23 33 3C 3C 3C 3C 3C 42 5C 3C 3C 3C 3C 3C 3C 3C
3C 3C 6C 78 77 4B 76 78 63 6D 6A 77 3C 21
```

**解析**:
- `33` = `3` (消息类型，序列号递增)
- 数据部分为固定填充

---

## 7. 已知限制

### 7.1 编码限制
- 6位编码后数据长度增加约33%
- 输出字符范围受限（可打印ASCII）

### 7.2 消息长度
- 单个数据包最大长度: 约 8KB
- 超过长度需要分包

### 7.3 未确定的字段
- nRecog 的具体用途
- wTag 和 wSeries 的详细含义

---

## 8. 枚举定义

### 8.1 消息ID枚举

```c
// 登录消息
enum CM_LOGIN {
    CM_PROTOCOL = 2000,
    CM_IDPASSWORD = 2001,
    CM_ADDNEWUSER = 2002,
    CM_CHANGEPASSWORD = 2003,
    CM_UPDATEUSER = 2004
};

// 游戏动作
enum SM_ACTION {
    SM_TURN = 10,
    SM_WALK = 11,
    SM_RUN = 13,
    SM_HIT = 14,
    SM_HEAR = 40,
    SM_LOGON = 50,
    SM_NEWMAP = 51
};
```

---

## 9. 参考资料

- 源码路径: `D:\Ada-WorkSpace\mir3-dx9`
- 加密算法: `Common/EnDecode.cpp`
- 消息定义: `Define.h`
- 数据包捕获: `D:\Ada-WorkSpace\packets_extracted.txt`

---

**文档版本**: v1.0  
**最后更新**: 2026-10-03  
**作者**: AI Assistant
