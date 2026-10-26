# 发布版 / 验证版 构建说明

同一份源码产出两个 exe，互不影响。

## 两种构建

| | 验证版（默认） | 发布版（分发） |
|---|---|---|
| 命令 | `python build_current.py` | `python build_current.py --release` |
| 诊断日志 | **保留**（原行为） | **全部编译掉** |
| 崩溃捕获 | 关闭 | 关闭 |
| obj 目录 | `build_dx9/` | `build_dx9_release/` |
| 用途 | 你自己排查问题 | 分发给测试人员 |
| 产物 | `Mir3_dx9_debug.exe` | `Mir3_dx9_release.exe` |

> 两个 obj 目录刻意分开：编译宏不同，混用会导致链接到错误的 .o。

## 发布版会去掉什么

以下诊断写盘在发布版中**完全不产生代码**（连文件名字符串都不在 exe 里）：

| 输出文件 | 位置 |
|---|---|
| `D:\ability_dump.txt` | `GameProc.cpp` `OnSvrMsgAbility` |
| `D:\bag_dump.txt` | `GameProc.cpp` `SM_BAGITEMS` |
| `D:\weight_log.txt` | `GameProc.cpp` `SM_WEIGHTCHANGED` |
| `D:\logon_dump.txt` | `GameProc.cpp` `SM_LOGON` |
| `D:\hero_create.txt` | `GameProc.cpp` 主角创建 |
| `D:\querychr_dump.txt` | `ChrProc.cpp` `SM_QUERYCHR` |
| `D:\text_diag.txt` | `WHDXGraphic.cpp` `Dx9TextDiag` |
| `LogMap.txt` | `WHDXGraphic.h` `GetDC` 内联 |
| `FLOG0(...)` 系列 | `log1.txt` / `LogGold.txt` / `Log000.txt` / `LogActor.txt` / `Log.txt` / `Chat.txt` |
| `logs\` 协议日志 | `PROTOCOL_LOG_*` 宏 |

**不受影响的功能文件**：`credit.txt`、`Notice.txt` 等游戏功能性读写**始终保留**。

## 崩溃捕获（CrashLog.txt）

发布版**默认关闭**。需要时单独打开：

```
python build_current.py --release
# 然后在 build_current.py 的 RELEASE 分支里加：
#   flags.append("-DMIR3_ENABLE_CRASHLOG")
```

或手工编译时加 `-DMIR3_ENABLE_CRASHLOG`。

> 建议：分发给多个测试人员做稳定性测试时**打开它**。别人报告崩溃时，
> 没有 `CrashLog.txt` 你拿不到任何信息。稳定后再关。

## 实现原理

`Common/DiagLog.h` 是唯一的开关：

```cpp
#ifdef MIR3_RELEASE_BUILD
    #define MIR3_DIAG_LOG 0     // 发布版
#else
    #define MIR3_DIAG_LOG 1     // 验证版（默认）
#endif
```

每个诊断块用 `#if MIR3_DIAG_LOG ... #endif` 包裹。
`GFun.h` 的 `FLOG0` 用两套 `#define`（发布版展开为 `((void)0)`）。

**注意**：`#if` 不能写在宏体内部。`FLOG0` 是在 `#define` 层级做条件，
不是把 `#if` 塞进宏里——后者是无效的 C 语法。

## 一个曾经踩过的坑

`WindHorn\WHDXGraphic.cpp` include 的是 **`WindHorn\StdAfx.h`**（一个精简版），
它**不包含** `DiagLog.h`。而根目录另有 `StdAfx.h`（包含 `DiagLog.h`）。

结果：`MIR3_DIAG_LOG` 在 `WHDXGraphic.cpp` 里未定义 → `#if` 视为 0 →
**验证版也不写 `text_diag.txt`**，且没有任何报错。

**修法**：让 `WHDXGraphic.h` 和 `GFun.h` **自己 include `DiagLog.h`**，
不依赖任何 `StdAfx.h`。这样无论从哪个 TU 进来，宏都是可见的。

判断宏是否生效的可靠办法（不要靠肉眼）：

```powershell
i686-w64-mingw32-g++.exe -E <flags> ... WindHorn\WHDXGraphic.cpp | Select-String Dx9TextDiag
```

验证版应有命中，发布版应为 0。编译单元里宏对不对，**只有预处理输出说了算**。

## DPI / 窗口尺寸：为什么刻意不加 manifest

实测 6 组对照（125% 缩放，1920x1080）：

| exe | 直接启动 | 经启动器 (UseShellExecute) |
|---|---|---|
| **无 manifest**（本工程） | **640x480** | **640x480** |
| 加 `dpiAware=true` | 512x384 | 512x384 |

**结论：本游戏必须保持 DPI 不感知（不加 manifest）。**
加了 `dpiAware` 反而让窗口变成 `640/1.25 = 512`，是**错误**的方向。

因此 `build_current.py` **刻意不嵌入任何 manifest**，保持既有 640x480 行为。

> 如果将来要改窗口尺寸，正确做法是改 `GameProc.cpp` 里的
> `ResetDXG(800, 600, ...)` 等调用，**而不是动 DPI 声明**。
