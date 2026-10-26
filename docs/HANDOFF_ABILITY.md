# MIR3-DX9 客户端修复 — 交接文档

> 生成时间：2026-10-22
> 仓库：`C:\Users\X\Desktop\MIR3-DX9`
> 测试客户端：`D:\Mir3G_2007\`（exe 名为 `Mir3_dx9.exe`）
> 测试服务器：**传奇3**（`192.0.2.10:7000`）

---

## 一、环境与构建

| 项 | 值 |
|---|---|
| 源码根 | `C:\Users\X\Desktop\MIR3-DX9` |
| 编译器 | w64devkit i686：`D:\w64devkit\w64devkit\bin\i686-w64-mingw32-g++.exe` |
| 构建命令 | `cd C:\Users\X\Desktop\MIR3-DX9; python build_current.py` |
| 成功标志 | `=== compiled 84 fail 0` / `LINK 0` / `SUCCESS: ...\Mir3_dx9.exe` |
| 部署 | 把 `Mir3_dx9.exe` 复制到 `D:\Mir3G_2007\Mir3_dx9.exe` |

**注意**：源码文件是 **韩文 CP949 / 中文 CP936 混合编码 + CRLF**，**不是 UTF-8**。
用编辑器直接改写容易破坏编码。建议改文件时用 Python 以 `rb`/`wb` 做**字节级替换**，
并断言匹配次数唯一。本项目此前多次因编码问题踩坑。

---

## 二、已完成并验证的修复

### 1. 卸下武器后背包出现重复副本 — 已修复（提交 `88ddc98`）

**根因**：审计提交 `7c5caf3` 的 `#33` 补丁误判"卸装后物品没回背包"，在
`SM_TAKEOFF_OK` 里补了一次 `AddInvenItem`。但物品本就由后续 `SM_ITEMSHOW`
物品包入包，于是产生第二份副本，反复装备/卸下会持续累加。

**修复**：删除该段（`GameProcess/GameProc.cpp`，`case SM_TAKEOFF_OK` 内 9 行），
恢复原行为。

---

### 2. 背包物品不显示 — 已修复（提交 `b692a0b`）

**根因**：同一提交 `7c5caf3` 的 `[#19-FIX]` 长度校验会**静默丢弃大包**：

```cpp
// GameProcess/GameProc.cpp  CGameProcess::OnMessageReceive
int fragLen = (int)(pEnd - pStart - 1);
if ( fragLen > 0 && fragLen <= _DEFBLOCKSIZE + _MAX_STRING_LEN )   // ≤ 1040
    OnSocketMessageReceive( pStart + 1 );      // 超出整包丢弃
```

尺寸实测：

- `sizeof(CLIENTITEMRCD)` = **81 字节/件**（`GameProcess/Item.h`）
- 线上字符数 ≈ 解码字节 × 8/6（用真实 `SM_LOGON` 验证：52 wire / 39 decoded = 1.333）
- **背包 ≥ 10 件 → 约 1096 字符 > 1040 → 整包被丢**
- 而客户端背包容量是 `_MAX_INVEN_ITEM = 46`，46 件需 3726 字节

**更深层问题**：`PACKETMSG.szEncodeData` 只有 **1024 字节**，本协议自己的合法包
就装不下。原校验只是把"缓冲区溢出"变成了"静默丢包"。

**修复**（两处，必须同时改）：

1. `Common/EnDecode.h`
   - 新增 `#define _MAX_PACKET_DATA_LEN 8192`（与接收缓冲 `recvBuf[8192]` 一致）
   - `PACKETMSG.szEncodeData[_MAX_STRING_LEN]` → `[_MAX_PACKET_DATA_LEN]`
2. `GameProcess/GameProc.cpp` 的 `OnMessageReceive`
   - 校验上限 `_DEFBLOCKSIZE + _MAX_STRING_LEN` → `_DEFBLOCKSIZE + _MAX_PACKET_DATA_LEN`

**安全性已核对**：`CWHQueue` 只存 `BYTE*` 指针；所有 `PACKETMSG` 均 `new PACKETMSG`
或 `sizeof(PACKETMSG)` 分配，自动适配。

**结果**：用户已确认背包可见、人物可见（"人物不显示"实际是同一根因：
装备/物品包被丢弃，导致属性页裸体、背包空白）。

---

## 三、未完成：人物属性数值错乱

### 现象

1 级法师（男法师）属性面板显示：

```
破坏 5122-3072      ← 应为个位数到几十
自然系魔法 2816-21
灵魂系魔法 19-22
负重 3077/0
```

用户判断"数值太高、不对"。

### 已知事实（均已实证）

**A. 客户端结构体 `ACTORABILITY`（`GameProcess/Actor.h`，`#pragma pack(1)`）**

| 字段 | 类型 | 偏移 | 大小 |
|---|---|---|---|
| bLevel | BYTE | 0 | 1 |
| wAC | WORD | 1 | 2 |
| wMAC | WORD | 3 | 2 |
| **dwDC** | **DWORD** | **5** | **4** |
| **dwMC** | **DWORD** | **9** | **4** |
| **dwSC** | **DWORD** | **13** | **4** |
| wHP | WORD | 17 | 2 |
| wMP | WORD | 19 | 2 |
| wMaxHP | WORD | 21 | 2 |
| wMaxMP | WORD | 23 | 2 |
| dlngExp | ULONGLONG | 25 | 8 |
| dlngMaxExp | ULONGLONG | 33 | 8 |
| wWeight | WORD | 41 | 2 |
| wMaxWeight | WORD | 43 | 2 |
| bWearWeight / bMaxWearWeight / bHandWeight / bMaxHandWeight | BYTE | 45/46/47/48 | 1 各 |
| ATOM_DC[7] | WORD | 49 | 14 |
| ATOM_AC_GOOD[7] | CHAR | 63 | 7 |
| ATOM_AC_BAD[7] | CHAR | 70 | 7 |

**总大小 = 77 字节**

**B. 属性面板显示方式**（`GameProcess/StatusWnd.cpp:1489 / 1507 / 1523`）

```cpp
sprintf(szBuff, "%d-%d", LOWORD(m_stAbility.dwDC), HIWORD(m_stAbility.dwDC));
```

即把 DWORD 的**低 16 位当"下限"、高 16 位当"上限"**。

**C. 服务器下发**

`CGameProcess::OnSvrMsgAbility`（`GameProcess/GameProc.cpp`）**直接把原始字节灌进结构体**：

```cpp
INT nDec = fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&m_xMyHero.m_stAbility, sizeof(ACTORABILITY));
```

实测 `nDec = 63`，即**服务器只发 63 字节**，而客户端结构体是 **77 字节**。
两者布局不可能一致。

**D. 错位机制（已解释"数值巨大"）**

若服务器把相邻两个属性各按 WORD 发，客户端按 DWORD 读，就会把两个无关属性
拼成一个 32 位数，再按 `LOWORD-HIWORD` 显示成"巨大-巨大"。

以男法师带装备的一组数据为例，原始字节：

```
idx:  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 ...
byte: 01 02 02 02 03 02 14 00 0C 00 0B 15 00 13 00 16 ...
```

`dwDC = DWORD@5` = bytes[5..8] = `02 14 00 0C` = `0x0C001402`
→ 显示 `LOWORD=0x1402=5122`、`HIWORD=0x0C00=3072` → **"5122-3072"**（就是用户看到的）

**E. 重要教训：上一轮"修复"是 no-op（提交 `4f524a1`）**

该提交把 `dwDC` 改成 `_RDW(5) | _RDW(7)<<16`，但算式结果与直接读 DWORD **完全相同**
（`0x1402 | 0x0C00<<16` == 原 DWORD），所以显示毫无变化。
**只改变"拼接方式"而不改变"起始偏移"是无效的。**

### 服务器字段语义（来自开源重写版，**仅供参考，未验证**）

`D:\Mir3-master\Mir3EIServerDev\Mir3server\GameSvr\ObjectEngine.h:116`（用户已说明：
这是开源重写版，**不是**其服务器的源码，仅作参考）：

```cpp
class CObjectAbility {
    BYTE Level;
    WORD HP; WORD MP; WORD MaxHP; WORD MaxMP;
    WORD Weight; WORD MaxWeight;
    DWORD Exp; DWORD MaxExp;
    BYTE WearWeight; BYTE MaxWearWeight;
    BYTE HandWeight; BYTE MaxHandWeight;
    WORD DC; WORD MC; WORD SC; WORD AC; WORD MAC;   // ← DC/MC/SC 是 WORD
    WORD m_wWater; ... m_wEarth;
};
```

该版打包后 45 字节（默认对齐 48），**与实测 63 字节不符**，说明用户服务器的结构
在此基础上扩展过（DB 里有 14 个原子属性列，`45-10+28=63` 恰好吻合）。
**但这些都只是推测，字段顺序未经证实。**

服务端用法（若该语义成立）：`DC` 的低字节=下限、高字节=上限，例如
`GetAttackPower(LOBYTE(m_WAbility.DC), HIBYTE(m_WAbility.DC) - LOBYTE(m_WAbility.DC))`。
`AC/MAC/MC/SC` 同理。

### DB 表结构（已由用户截图核实列顺序）

SQL Schema 参考：`D:\MUD3\OTHER\DataBase\MSSQL2008R2\sql\IGNIS.sql` 中的 `TBL_ABILITY`
（**注意**：`D:\MUD3` 是另一套引擎，用户说明它与开源版最早版本匹配；但其列顺序与用户 DB
截图表头完全一致，故列顺序可用）：

```
FLD_CHARACTER, FLD_LEVEL, FLD_RESERVED1,
FLD_AC, FLD_MAC, FLD_DC, FLD_MC, FLD_SC,
FLD_HP, FLD_MP, FLD_MAXHP, FLD_MAXMP,
FLD_EXP(bigint), FLD_MAXEXP(bigint),
FLD_WEIGHT, FLD_MAXWEIGHT, FLD_WEARWEIGHT, FLD_MAXWEARWEIGHT,
FLD_HANDWEIGHT, FLD_MAXHANDWEIGHT,
FLD_ATOMFIRE_MC ... FLD_ATOMPHANTOM_MAC (14 个 smallint)
```

用户 DB 中 `男法师` 一行为（截图读数，可能有误）：

```
LEVEL=1  RESERVED1=0  AC=0  MAC=256  DC=0  MC=22  SC=21
HP=22  MP=21  MAXHP=0  MAXMP=100  EXP=0  MAXEXP=50
WEIGHT=0  MAXWEIGHT=15  WEARWEIGHT=0  MAXWEARWEIGHT=12
```

**注意**：DB 存的是**基础属性**，服务器下发的是**基础+装备+等级加成后的总值**，
所以两者本来就不该完全相等，不能直接一一对应。

---

## 四、当前诊断代码（已部署，位于 `22ba555`）

诊断会把原始字节写到 `D:\` 下（**工作目录是 `D:\Mir3G_2007` 时也会落在 `D:\`**，
因为代码里写死了绝对路径 `D:\\xxx.txt`）：

| 文件 | 内容 | 代码位置 |
|---|---|---|
| `D:\ability_dump.txt` | `SM_ABILITY` 原始解码字节 + **角色名/job** + 全部非零 WORD 扫描 | `OnSvrMsgAbility` |
| `D:\bag_dump.txt` | `SM_BAGITEMS` 负载与每件物品字段 | `case SM_BAGITEMS` |
| `D:\querychr_dump.txt` | `SM_QUERYCHR` 原始字符串 + 解析出的 class/gender/level | `ChrProc.cpp` |
| `D:\logon_dump.txt` | `SM_LOGON` 原始字节 + BYTE/WORD/LONG 三种视角 | `OnSvrMsgLogon` |
| `D:\hero_create.txt` | FEATURE / FEATUREEX / Create() 返回值 | `OnSvrMsgLogon` |

### 现有关键实测字节（男法师，63 字节，可直接用于分析）

**无装备时**：
```
01 00 00 00 00 00 01 00 01 00 00 0F 00 0F 00 16 00 15 00 00
00 00 00 64 00 00 00 2B 00 32 00 00 0F 00 0C 00
```

**最终（带装备）**：
```
01 02 02 02 03 02 14 00 0C 00 0B 15 00 13 00 16 00 15 00 00
00 00 00 64 00 00 00 1C 00 32 00 0A 0F 05 0C 00
```

**用户当时在面板与顶栏看到的值（均为"错误显示"，不能当 ground truth）**：
```
破坏 5122-3072 | 自然系魔法 2816-21 | 灵魂系魔法 19-22
负重 3077/0 | HP 22/22 | MP 21/100 | Lv 1 | 防御 2-2 | 魔法防御 2-3
```

> 提示：`防御 2-2` 和 `魔法防御 2-3` 与 `wAC@1`/`wMAC@3` 的低/高字节吻合
> （`0x0202`→2-2，`0x0302`→2-3），说明**AC/MAC 的显示是对的**，
> 这可以作为验证"WORD 低字节=下限、高字节=上限"语义的旁证。

---

## 五、下一步（建议按顺序）

### 优先方案 A：用 DB 标记值一次性定案（最快、零歧义）

在**测试库**上（先备份！）把一个角色的属性改成互不相同的唯一值：

```sql
SELECT * INTO TBL_ABILITY_BAK FROM TBL_ABILITY WHERE FLD_CHARACTER='男法师';

UPDATE TBL_ABILITY SET
  FLD_AC=101, FLD_MAC=102, FLD_DC=103, FLD_MC=104, FLD_SC=105,
  FLD_HP=106, FLD_MP=107, FLD_MAXHP=108, FLD_MAXMP=109,
  FLD_EXP=110, FLD_MAXEXP=111, FLD_WEIGHT=112, FLD_MAXWEIGHT=113,
  FLD_WEARWEIGHT=114, FLD_MAXWEARWEIGHT=115,
  FLD_HANDWEIGHT=116, FLD_MAXHANDWEIGHT=117
WHERE FLD_CHARACTER='男法师';
```

然后登录该角色一次，把 `D:\ability_dump.txt` 取出。每个数字（101…117）在字节流里
只出现一次 → **17 个字段偏移一次全部锁定**。测完用 `TBL_ABILITY_BAK` 还原。

**注意**：服务器会做 `基础+装备+等级` 加成（如 AC 会加 `2 + Level/7`），
但 101..117 这类偏离极大的值仍可无歧义识别。

### 方案 B：用数值特征明显的两个角色（不需要改 DB）

让用户分别用 **`堕落天使`** 和 **`大法师`** 登录，**进游戏后什么都不要动**，直接退出；
然后提供 `D:\ability_dump.txt` + 这两个角色 DB 行的**文字**（不要截图，避免读错列）。

这两个角色的数值辨识度很高，可唯一定位偏移：

- `堕落天使`：HP=961、MP=270、MaxHP=1027、MaxMP=**20154**、DC=1284、SC=270、Exp=**11000000**、Weight=370、WW=31、HW=30
- `大法师`：HP=55、MP=116、MaxHP=55、MaxMP=116、Exp=**4260**、MaxExp=**6000**、MaxWeight=70、MWW=16、MHW=13

`男法师`/`女法师` 数值完全相同且太小（22/21/15/12），**无法互相区分，不要再用它们做定位**。

### 定位后如何改代码

`OnSvrMsgAbility` 目前已是"按字段显式赋值"的结构（提交 `4f524a1`），
只需把每个 `_RDW(n)` 的**偏移数字**改成实测值即可，例如：

```cpp
BYTE* pAb = (BYTE*)&m_xMyHero.m_stAbility;
#define _RDW(o)  ( (WORD)( pAb[(o)] | (pAb[(o)+1] << 8) ) )

m_xMyHero.m_stAbility.bLevel = pAb[0];
m_xMyHero.m_stAbility.wAC    = _RDW(1);
m_xMyHero.m_stAbility.wMAC   = _RDW(3);
// ↓ 以下偏移均为“待证实”，需按实测替换
m_xMyHero.m_stAbility.dwDC   = (DWORD)( _RDW(5)  | (_RDW(7)  << 16) );
m_xMyHero.m_stAbility.dwMC   = (DWORD)( _RDW(9)  | (_RDW(11) << 16) );
m_xMyHero.m_stAbility.dwSC   = (DWORD)( _RDW(13) | (_RDW(15) << 16) );
m_xMyHero.m_stAbility.wHP    = _RDW(17);
m_xMyHero.m_stAbility.wMP    = _RDW(19);
m_xMyHero.m_stAbility.wMaxHP = _RDW(21);
m_xMyHero.m_stAbility.wMaxMP = _RDW(23);
m_xMyHero.m_stAbility.wWeight    = _RDW(33);
m_xMyHero.m_stAbility.wMaxWeight = _RDW(35);
#undef _RDW
```

**若实测证明服务器 `DC/MC/SC` 是 WORD（低字节=下限、高字节=上限）**，则应改成
从**单个 WORD** 拆字节，而不是拼两个 WORD：

```cpp
WORD wDC = _RDW(?, );
m_stAbility.dwDC = MAKEWORD( LOBYTE(wDC), HIBYTE(wDC) ); // 使 LOWORD=min, HIWORD=max
```

这样才能让 `StatusWnd.cpp` 的 `LOWORD-HIWORD` 显示正确（例如 `2-20`）。

---

## 六、另一个未解决问题：选人界面性别

### 现象

新建的「男法师」在**选人界面显示女法师立绘**，但**进游戏后显示正确（男）**。

### 已实证结论

`SM_QUERYCHR` 报文（传奇3，实测）：

```
堕落天使/1/1/40/0/男法师/1/1/1/0/
```

字段切分 `名字 / A / B / 等级 / C`：

| 角色 | A | B | 等级 | C | 真实性别（用户确认） |
|---|---|---|---|---|---|
| 堕落天使 | 1 | 1 | 40 | 0 | 女 |
| 男法师 | 1 | 1 | 1 | 0 | 男 |

**等级字段（40 / 1）完全正确 → 切分位置无误**；但两个性别不同的角色
**A、B 都是 1**，因此 **A、B 都不是性别字段** —— 该服务器的 `SM_QUERYCHR`
**不携带性别信息**。

而代码把 `B` 当性别用：

```cpp
// CharSelectProcess/ChrProc.cpp  case SM_QUERYCHR
m_stSelectChrInfo[nCnt].bGender = atoi(pszFace);   // pszFace 即字段 B
```

帧号表（`ChrProc.cpp` 的 `SetCharSprInfo` / `GetCharSprInfo`）：
`index = bJob*10 + bGender*5 + bMtn`，由 job2(道士) 区段注释确定
**`gender 0 = 男`、`gender 1 = 女`**（`[20]=道士男NS`…、`[25]=道士女NS`…）。

因 `B` 恒为 1 → 选人界面**永远显示女立绘**。

**游戏内性别来自 `SM_LOGON` 的 FEATURE，是正确的**（实测
`FEATURE{bGender,bWeapon,bDress,bHair} = {0,...}` → 男）。

### 历史改动（两次都基于错误假设）

| 提交 | 内容 | 结论 |
|---|---|---|
| `6959863` | 加入 `bGender = 1 - bGender` | 依据"服务器1=男0=女"，错误 |
| `e6fa4ad` | 修正字段顺序（等级/性别互换） | **正确** |
| `c14c422` | 删除取反 | **依据不足**（基于常量推断），导致现状"永远显示女" |

### 建议方案

**不要再在"取反/不取反"之间反复。** 既然该字段不携带性别，建议：
选人界面**不要用它决定性别**，改为中性处理（如统一用默认立绘，或按职业取默认），
并在代码里注释说明"该服务器 SM_QUERYCHR 不含性别，性别以 SM_LOGON 为准"。

---

## 七、其他待验证的下发数据（本次未处理）

| 包 | 可疑点 |
|---|---|
| `SM_SENDUSEITEMS` | 属性页裸体可能与此有关（装备栏未正确解析） |
| `SM_WEIGHTCHANGED` | 负重有独立包，可能与 `SM_ABILITY` 里的负重冲突 |
| `SM_SUBABILITY` | 命中/敏捷/幸运等，同样存在 WORD/BYTE 宽度问题 |
| `SM_HEALTHSPELLCHANGED` | HP/MP 走这条，目前显示正常，可顺带核对 |

**建议一个一个来**，避免同时改多处难以定位。

---

## 八、给接手者的关键提醒

1. **不要再把 `D:\Mir3-master`（开源重写版）当作权威参照**。用户明确说明那不是
   其服务器源码，字段顺序和大小都不匹配（45/48 vs 实测 63）。只能当"可能的语义"参考。
2. **不要用 `男法师`/`女法师` 做属性定位**，它们数值完全相同且太小，会产生歧义。
3. **改 `.cpp/.h` 前务必确认编码（CP949/CP936）与 CRLF**，用字节级替换 + 唯一性断言。
4. **"改拼接方式"不等于"改偏移"**：上一轮 `4f524a1` 因此成为 no-op，务必用真实数据验证。
5. 诊断日志写死在 `D:\`（绝对路径），不是 exe 所在目录。
6. 证据已归档：`C:\Users\X\Desktop\MIR3-DX9\docs\evidence_20261022\`
   （含 `querychr_dump.txt`、`logon_dump.txt`、`hero_create.txt`、`ability_dump.txt`、
   `chr_oldserver.txt`、`SERVER_NOTES.md`）
7. 用户服务器：**传奇3 `192.0.2.10:7000`**；DB 为 SQL Server（LiteSQL）。
   另有 `华网娱乐2005沙之怒`（`127.0.0.1:7000`）为另一台，协议可能有差异。
