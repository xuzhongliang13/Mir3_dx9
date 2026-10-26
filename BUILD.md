# Mir3_DX9 客户端编译说明

本文档说明如何从源码编译 mir3-dx9 客户端（传奇3国服 DX9 版，产物 `Mir3_dx9.exe`），
保证任何环境按此步骤可复现编译，产物与作者一致。

## 环境要求
- **Windows** 操作系统
- **Python 3**（运行编译脚本）
- **w64devkit**（MinGW-w64 i686 工具链）：提供 `i686-w64-mingw32-g++.exe` 与 `windres.exe`
- **DirectX SDK**：提供 `d3dim/ddraw/dxguid/d3d9/d3dx9` 等链接库

## 编译步骤
1. **安装 w64devkit**：解压到 `D:\w64devkit\w64devkit\`。
   若解压到其他路径，请修改 `build_current.py` 中：
   - `cxx`（g++ 路径）
   - windres 路径
2. **准备 DirectX SDK 库**：确保链接器能找到 `d3dim/ddraw/d3d9/d3dx9` 等库
   （`build_current.py` 的 `libs` 列表已声明所需库）。
3. **设置仓库根路径**：`build_current.py` 顶部 `root = r"D:\Ada-WorkSpace\mir3-dx9"`。
   若 clone 到其他目录，改为对应路径。
4. 在仓库根目录运行：
   ```
   python build_current.py
   ```
5. 编译成功标志：
   - `=== compiled N fail 0`
   - `LINK 0`
   - `SUCCESS: <仓库路径>\Mir3_dx9.exe`

## 产物
- `Mir3_dx9.exe`：编译生成的客户端可执行文件。
- 中间对象在 `build_dx9/`（已被 `.gitignore` 忽略）。

## 复现一致性
- 当前 master HEAD 编译结果与作者一致：**84 fail 0 / LINK 0**。
- 关键前提：**同一工具链版本（w64devkit i686）+ 同一源码（master HEAD）** → 编译产物一致。

## 常见问题
- **找不到 g++/windres**：检查 w64devkit 解压路径是否匹配 `build_current.py` 的 `cxx`。
- **链接缺库**：确认 DirectX SDK 已安装且库路径可被链接器找到。
- **目标平台报错**：脚本使用 `-m32`（i386 目标），需 i686 工具链。
