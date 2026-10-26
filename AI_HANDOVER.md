# 传奇3G DX9 客户端开发 - AI交接文档

**文档生成时间**: 2026-10-02 21:50  
**当前负责人**: AI-B (协助者B)  
**文档目的**: 完整交接工作环境、进度和问题，供后续AI接手

---

## 一、仓库信息

### 1.1 Gitea 仓库
- **地址**: `http://192.0.2.10:8418/mir3-dev/mir3-dx9.git`
- **账号**: `jack`
- **密码**: `xzlwy5200`
- **当前分支**: `master`
- **最新提交**: `6fe2e26` - [DX9][FIX] 修复窗口尺寸

### 1.2 本地工作目录
- **代码路径**: `D:da\mir3-dx9`
- **编译输出**: `D:da\mir3-dx9uild_dx9`
- **部署目标**: `D:\Client\Mir3_dx9.exe`

### 1.3 Git 配置
```bash
# 克隆仓库
git clone http://<user>:<token>@192.0.2.10:8418/mir3-dev/mir3-dx9.git

# 提交规范（AI-B）
git commit --author="协助者B <jack>" -m "[模块] 提交信息"

# 提交规范（AI-A）
git commit --author="Ada Auditor <ada@audit.local>" -m "[DX9][模块] 提交信息"
```

---

## 二、编译环境

### 2.1 编译器
- **编译器**: MinGW-w64 (i686-w64-mingw32-g++)
- **路径**: `D:\w64devkit\w64devkitin\i686-w64-mingw32-g++.exe`
- **版本**: GCC 16.2.0
- **目标架构**: 32位 Windows (i386)

### 2.2 编译脚本位置
- **编译脚本**: `D:da\mir3-dx9uild_dx9\compile_all.sh`
- **链接脚本**: `D:da\mir3-dx9uild_dx9\link_all.sh`

### 2.3 编译流程
```bash
# 1. 拉取最新代码
cd /d/ada/mir3-dx9
git pull origin master

# 2. 清理旧文件
cd build_dx9
rm -f *.o

# 3. 编译
bash compile_all.sh

# 4. 链接
bash link_all.sh

# 5. 部署
taskkill //F //IM Mir3_dx9.exe 2>&1
sleep 2
cp Mir3_dx9.exe /d/Client/Mir3_dx9.exe
```

### 2.4 关键依赖文件
- **d3dvector_pre.h**: `D:\d3dvector_pre.h` (D3D向量预处理头文件)
- **dsound.h**: `D:\w64devkit\w64devkit\include\dsound.h`
- **Resource.res**: `D:da\mir3-dx9uild_dx9\Resource.res` (从旧版本复制)

---

## 三、代码结构

### 3.1 目录结构
```
mir3-dx9/
├── Common/              # 通用模块
│   ├── ClientSocket.cpp/h    # 网络通信
│   ├── EnDecode.cpp/h        # 编解码（关键！）
│   ├── ProtocolLog.cpp/h     # 协议LOG（AI-B添加）
│   └── ...
├── GameProcess/         # 游戏主流程
│   ├── GameProc.cpp/h        # 游戏处理（消息接收）
│   └── ...
├── LoginProcess/        # 登录流程
├── SelLoginProcess/     # 选择登录流程
├── CharSelectProcess/   # 角色选择流程
├── WindHorn/            # 渲染引擎
│   ├── WHDXGraphic.cpp/h     # DirectX图形
│   └── ...
├── OrzLib/              # 工具库
├── Sound/               # 音频模块
├── AVI/                 # 视频模块
├── Reference/           # 反编译参考资料
│   ├── Mir3_ref_反编译报告.md
│   ├── Mir3_ref_完整反汇编.asm
│   └── Mir3_patched_disasm.txt  # 原始程序反汇编
└── build_dx9/           # 编译输出
    ├── compile_all.sh
    ├── link_all.sh
    └── *.o, *.exe
```

### 3.2 关键文件说明

#### EnDecode.cpp/h（编解码模块）
**位置**: `Common/EnDecode.cpp`

**核心函数**:
- `fnDecode6BitBufOriginal()`: 6-bit解码，偏移0x3c
- `fnDecodeMessage()`: 解码消息头（12字节）
- `fnEncode6BitBuf()`: 6-bit编码
- `fnEncodeMessage()`: 编码消息头

**结构体定义**:
```c
typedef struct tag_TDEFAULTMESSAGE
{
    int     nRecog;     // 偏移 0，大小 4
    WORD    wIdent;     // 偏移 4，大小 2  ← 注意位置！
    WORD    wParam;     // 偏移 6，大小 2
    WORD    wTag;       // 偏移 8，大小 2
    WORD    wSeries;    // 偏移 10，大小 2
} _TDEFAULTMESSAGE;     // 总大小 12 字节
```

#### GameProc.cpp（消息处理）
**位置**: `GameProcess/GameProc.cpp`

**关键函数**: `CGameProcess::OnSocketMessageReceive(char *pszMsg)`
- 行号: 6055-6085
- 功能: 接收并解码服务器消息
- 当前状态: 已移除错误的XOR和字节重排，但解码结果仍异常

---

## 四、反汇编分析结果

### 4.1 参考程序
- **文件**: `D:\Client\Mir3_patched_v3_backup.exe`
- **大小**: 1.4MB
- **类型**: PE32 executable (GUI), Intel i386
- **反汇编**: `Reference/Mir3_patched_disasm.txt` (356725行)

### 4.2 关键函数地址

#### fnDecode6BitBufOriginal (0x40e010)
**算法**:
- 偏移值: **0x3c** (不是0x4d)
- 6-bit解码算法
- **无XOR操作**
- **无字节重排**

#### 消息处理函数 (0x406b90)
**关键发现**:
- wIdent范围: **520-536** (0x208-0x218)
- 共17个case分支
- **解码后直接使用，无XOR/字节重排**

### 4.3 消息格式

根据反汇编分析，消息格式为：
```
#消息ID(编码后)/消息头(编码后)/消息体(编码后)!
```

- `#`: 消息起始符 (0x23)
- 消息ID: 16字节编码数据
- 消息头: 12字节（nRecog/wIdent/wParam/wTag/wSeries）
- 消息体: 可变长度
- `!`: 消息结束符 (0x21)

---

## 五、当前问题

### 5.1 协议解码异常

**现象**:
- 解码后 wIdent = 58882 (不在合理范围 520-536)
- nRecog = 1479903122 (异常大)
- 其他字段也异常

**日志示例**:
```
[2026-10-02 20:56:40.022] RECV  Func=OnSocketMessageReceive 
  Opcode=0xE602(58882) Recog=1479903122 Param=44192 Tag=54969 Series=48029
```

**已尝试的修复**:
1. ✓ 移除错误的XOR操作（m_wNoticeSeed3等）
2. ✓ 移除错误的字节重排（MAKELONG/MAKEWORD组合）
3. ✓ 直接使用fnDecodeMessage解码后的值
4. ✗ 问题仍然存在

**可能的原因**:
1. 消息格式解析不正确（消息ID/消息头/消息体的边界）
2. 解码函数实现与反编译不一致
3. 消息偏移计算错误

### 5.2 待验证的假设

**假设1**: 消息头不是从pszMsg[0]开始
- 反汇编显示 `pszMsg+0x10` 跳过16字节
- 这16字节可能是消息ID部分
- 消息头可能从 pszMsg+16 开始

**假设2**: 消息ID部分需要特殊处理
- 反汇编中有复杂的消息ID解析逻辑（0x406d29-0x406e9a）
- 使用 '/' 分隔多个字段
- 第一个字段是 nRecog

**假设3**: 解码函数可能有多个版本
- `fnDecode6BitBufOriginal` (偏移0x3c)
- `fnDecode6BitBufGameMessage` (偏移0x4d, XOR 24)
- 需要确认使用哪个版本

---

## 六、协议LOG模块

### 6.1 功能
**位置**: `Common/ProtocolLog.cpp/h`

记录所有网络通信：
- 时间戳（毫秒精度）
- 方向（SEND/RECV）
- Opcode（wIdent）
- 所有字段（nRecog/wParam/wTag/wSeries）
- 完整Hex数据
- 解析后的消息体

### 6.2 日志位置
- **路径**: `D:\Client\logs\`
- **文件名**: `protocol_YYYYMMDD_HHMMSS.log`
- **格式**: 文本文件，可直接查看

---

## 七、工作流程

### 7.1 开发流程
1. **拉取代码**: `git pull origin master`
2. **修改代码**: 编辑源文件
3. **编译**: `bash compile_all.sh`
4. **链接**: `bash link_all.sh`
5. **部署**: 复制到 `D:\Client\Mir3_dx9.exe`
6. **测试**: 运行客户端，查看日志
7. **提交**: `git commit --author="..." -m "..."`
8. **推送**: `git push origin master`

### 7.2 调试流程
1. 运行客户端
2. 执行操作（登录、选角、进游戏等）
3. 查看日志: `D:\Client\logs\protocol_*.log`
4. 分析解码结果
5. 对比反汇编代码
6. 修改代码
7. 重新编译部署

---

## 八、关键发现总结

### 8.1 已确认的事实
1. ✓ 解码函数使用偏移 **0x3c**（不是0x4d）
2. ✓ 消息头结构体中 wIdent 在偏移 4 的位置
3. ✓ 反编译代码中**无XOR操作**
4. ✓ 反编译代码中**无字节重排**
5. ✓ wIdent 合理范围: **520-536**

### 8.2 待解决的问题
1. ✗ 解码后 wIdent 异常（58882）
2. ✗ 消息格式边界不确定
3. ✗ 消息ID解析逻辑未完全实现

### 8.3 下一步建议
1. **验证消息格式**: 确认消息头从哪个偏移开始
2. **实现消息ID解析**: 参考反汇编 0x406d29-0x406e9a
3. **对比多个解码函数**: 测试 fnDecode6BitBufGameMessage
4. **添加更多日志**: 在解码前后添加详细日志

---

## 九、重要文件清单

### 9.1 源代码文件
- `Common/EnDecode.cpp/h` - 编解码（关键！）
- `Common/ClientSocket.cpp/h` - 网络通信
- `Common/ProtocolLog.cpp/h` - 协议LOG
- `GameProcess/GameProc.cpp/h` - 消息处理
- `Define.h` - 宏定义

### 9.2 编译脚本
- `build_dx9/compile_all.sh` - 编译脚本
- `build_dx9/link_all.sh` - 链接脚本

### 9.3 参考文档
- `Reference/Mir3_ref_反编译报告.md` - 反编译分析
- `Reference/Mir3_ref_完整反汇编.asm` - 完整反汇编
- `Reference/Mir3_patched_disasm.txt` - 原始程序反汇编

### 9.4 日志文件
- `D:\Client\logs\protocol_*.log` - 协议日志
- `D:\Client\LogGame.txt` - 游戏日志
- `D:\Client\LogMap.txt` - 地图日志

---

## 十、联系信息

### 10.1 仓库访问
- **Gitea地址**: http://192.0.2.10:8418
- **仓库**: mir3-dev/mir3-dx9
- **账号**: jack / xzlwy5200

### 10.2 本地路径
- **代码**: D:da\mir3-dx9
- **客户端**: D:\Client
- **编译器**: D:\w64devkit\w64devkitin

### 10.3 关键提交
- `6fe2e26` - 最新提交（窗口尺寸修复）
- `d9ac830` - 协议解码修复（移除XOR/字节重排）
- `90f8fff` - 部署报告

---

## 十一、快速开始指南

### 11.1 环境检查
```bash
# 检查编译器
D:\w64devkit\w64devkitin\i686-w64-mingw32-g++.exe --version

# 检查仓库
cd D:da\mir3-dx9
git status
git log --oneline -5

# 检查客户端
ls -lh D:\Client\Mir3_dx9.exe
```

### 11.2 编译部署
```bash
cd D:da\mir3-dx9
git pull origin master
cd build_dx9
rm -f *.o
bash compile_all.sh
bash link_all.sh
taskkill //F //IM Mir3_dx9.exe 2>&1
sleep 2
cp Mir3_dx9.exe /d/Client/Mir3_dx9.exe
```

### 11.3 测试验证
```bash
# 运行客户端
D:\Client\Mir3_dx9.exe

# 查看日志
ls -lht D:\Client\logscat D:\Client\logs\protocol_*.log | tail -50
```

---

## 十二、注意事项

1. **编译环境**: 必须使用 MinGW-w64 (i686)，不能用 MSVC
2. **提交署名**: AI-B 使用 `--author="协助者B <jack>"`
3. **代码风格**: 保持与现有代码一致
4. **测试**: 每次修改后必须编译部署测试
5. **日志**: 协议LOG已启用，会自动记录所有通信
6. **反汇编**: 所有协议相关修改必须参考反编译代码

---

**文档结束**

如有问题，请查看：
- 反汇编文件: `Reference/Mir3_patched_disasm.txt`
- 协议日志: `D:\Client\logs\`
- Git历史: `git log --all --oneline`
