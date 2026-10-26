#ifndef _DIAG_LOG_H
#define _DIAG_LOG_H

// ============================================================================
// [DX9][LOGGATE] 诊断日志总开关
//
// 目的：同一份源码产出两种 exe，互不影响
//   - 验证版（默认，带日志）：沿用原行为，所有诊断 dump 照常写出
//   - 发布版（无日志）：编译时加 -DMIR3_RELEASE_BUILD，所有诊断写盘全部消失
//
// 用法：
//   验证版： python build_current.py                     （不加任何宏）
//   发布版： python build_current.py --release           （自动加宏）
//
// 说明：这里只控制【诊断性质】的写盘。游戏功能性文件（credit.txt、
//       Notice.txt 等）不受影响，始终照常读写。
//
// 被本开关覆盖的诊断输出：
//   D:\querychr_dump.txt   D:\bag_dump.txt     D:\weight_log.txt
//   D:\logon_dump.txt      D:\hero_create.txt  D:\ability_dump.txt
//   D:\text_diag.txt       LogMap.txt
//   FLOG0(...) 产生的 log1.txt / LogGold.txt / Log000.txt /
//              LogActor.txt / Log.txt / Chat.txt
//   logs\ 下的协议日志（PROTOCOL_LOG_*）
//
// 崩溃捕获（CrashLog.txt）单独由 MIR3_ENABLE_CRASHLOG 控制，见 Mir2Ex.cpp。
// ============================================================================

#ifdef MIR3_RELEASE_BUILD
    // 发布版：诊断宏全部消失
    #undef  MIR3_DIAG_LOG
    #define MIR3_DIAG_LOG 0
#else
    // 验证版：默认开启（若外部未显式定义）
    #ifndef MIR3_DIAG_LOG
        #define MIR3_DIAG_LOG 1
    #endif
#endif

// 诊断写盘块包裹宏：MIR3_DIAG_BLOCK { ...诊断代码... }
// 发布版会整块消失，不产生任何代码，也不留字符串常量。
#if MIR3_DIAG_LOG
    #define MIR3_DIAG_BLOCK
#else
    #define MIR3_DIAG_BLOCK if (0)
#endif

// FLOG0：原有宏在 GFun.h 中定义（直接 fopen 写盘）。
// 发布版把它替换成空操作，避免留下日志文件名常量。
#if MIR3_DIAG_LOG
    #define MIR3_FLOG(file, txt) FLOG0(file, txt)
#else
    #define MIR3_FLOG(file, txt) ((void)0)
#endif

// ---------------------------------------------------------------------------
// [FIX-R4] MIR3_SHIELD_TRACE: 魔法盾专项追踪（默认关闭）
// 盾的问题已定位并修复。shield_allmsg 那种"施法后 30 秒记录全部网络消息、
// 上限 5000 条、每条 fopen+fprintf+fflush+fclose"的暴力日志不再默认开启。
// 需要复查时编译加 -DMIR3_SHIELD_TRACE=1，发布版恒为 0。
// ---------------------------------------------------------------------------
#ifdef MIR3_RELEASE_BUILD
    #undef  MIR3_SHIELD_TRACE
    #define MIR3_SHIELD_TRACE 0
#else
    #ifndef MIR3_SHIELD_TRACE
        #define MIR3_SHIELD_TRACE 0
    #endif
#endif

// ---------------------------------------------------------------------------
// [FIX-R4] 崩溃捕获默认打开
// 原来 MIR3_ENABLE_CRASHLOG 未定义 → #if 判为 0 → 整个 CrashLog.txt 逻辑被编译掉，
// 线上崩溃没有任何栈信息。这个开关只在异常发生时才写盘，没有逐帧成本，因此默认开。
// 需要彻底关掉时编译加 -DMIR3_ENABLE_CRASHLOG=0。
// ---------------------------------------------------------------------------
#ifndef MIR3_ENABLE_CRASHLOG
    #define MIR3_ENABLE_CRASHLOG 1
#endif

#endif // _DIAG_LOG_H
