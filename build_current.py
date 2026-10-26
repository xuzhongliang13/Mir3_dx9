# -*- coding: utf-8 -*-
# 编译 mir3-dx9 -> Mir3_dx9.exe
#
# 两种构建模式：
#   默认        验证版：保留全部诊断日志，便于排查
#   --release   发布版：诊断日志全部编译掉 + 嵌入 DPI 感知 manifest
#
# 参数化：root 基于本脚本所在目录（clone 到任意路径均可编译）；
# w64devkit 从环境变量 W64DEVKIT 或常见路径自动探测，找不到时回退默认路径。
import os, subprocess, sys, shutil
os.environ["PATH"] = os.environ.get("SystemRoot","C:\Windows") + "\\System32;" + os.environ.get("PATH","")
root = os.path.dirname(os.path.abspath(__file__))

RELEASE = ("--release" in sys.argv) or ("-r" in sys.argv)

def _find_w64bin():
    cands = []
    env = os.environ.get("W64DEVKIT")
    if env:
        cands.append(env)
    cands += [r"D:\w64devkit\w64devkit\bin", r"C:\w64devkit\w64devkit\bin",
              r"D:\w64devkit\bin", r"C:\w64devkit\bin"]
    for c in cands:
        if os.path.isfile(os.path.join(c, "i686-w64-mingw32-g++.exe")):
            return c
    return r"D:\w64devkit\w64devkit\bin"
W64BIN = _find_w64bin()
cxx = os.path.join(W64BIN, "i686-w64-mingw32-g++.exe")
windres_exe = os.path.join(W64BIN, "windres.exe")

# 发布版用独立的 obj 目录，避免与验证版的 .o 混用（编译宏不同！）
objroot = os.path.join(root, "build_dx9_release" if RELEASE else "build_dx9")

flags=["-m32","-DWIN32","-D_WINDOWS","-D_MBCS","-DNDEBUG","-std=gnu++98","-fpermissive","-w"]
flags.extend(["-include", os.path.join(root, "d3dvector_pre.h")])
if RELEASE:
    # 诊断日志全部编译掉；崩溃捕获也默认关闭，需要时再单独加 -DMIR3_ENABLE_CRASHLOG
    flags.append("-DMIR3_RELEASE_BUILD")
inc=["-I"+root]
for sub in ["OrzLib","Common","WindHorn","WindHorn\\D3DCommon","LoginProcess","SelLoginProcess","CharSelectProcess","GameProcess","AVI","Sound"]:
    inc.append("-I"+os.path.join(root,sub))
libs=["-mwindows","-ld3dim","-lddraw","-ldxguid","-ldsound","-lwinmm","-lwininet","-ld3d9","-ld3dx9",
      "-lgdi32","-lmsimg32","-lwinspool","-lcomdlg32","-ladvapi32","-lshell32","-lole32","-loleaut32","-luuid",
      "-lodbc32","-lodbccp32","-limm32","-luser32","-lkernel32","-lcomctl32","-lrpcrt4","-lws2_32",
      "-lstrmiids","-lquartz","-ldmoguids","-lvfw32","-ldbghelp",
      ]
def run(args):
    r=subprocess.run(args,capture_output=True)
    return r.returncode,(r.stdout+r.stderr).decode('utf-8',errors='replace')

print("=== BUILD MODE: %s ===" % ("RELEASE (no diagnostics + DPI manifest)" if RELEASE else "DEBUG (diagnostics on)"))

skip={"StdAfx.cpp"}
objs=[]; errs=[]
for dp,_,fs in os.walk(root):
    if 'build_dx9' in dp or 'tools' in dp: continue
    for f in fs:
        if not f.endswith('.cpp') or f in skip: continue
        cp=os.path.join(dp,f)
        rel=os.path.relpath(cp,root)
        ofile=os.path.join(objroot,rel[:-4]+".o")
        os.makedirs(os.path.dirname(ofile),exist_ok=True)
        rc,out=run([cxx,"-c",cp,"-o",ofile]+flags+inc)
        if rc!=0: errs.append((rel,out)); print("FAIL",rel)
        else: objs.append(ofile); print("OK",rel)
print("=== compiled",len(objs),"fail",len(errs))
if errs:
    for _r,_o in errs:
        print('=== ERR '+_r); print(_o[:500])
    raise SystemExit(1)

exe=os.path.join(root,"Mir3_dx9.exe")
rc_file=os.path.join(root,"Resource.rc"); res=os.path.join(root,"app_res.o")
rc_,rout = run([windres_exe, "--target=pe-i386", rc_file, "-o", res])
if rc_ != 0:
    print('WINDRES_FAIL'); print(rout[:500]); raise SystemExit(1)

# ---- 不嵌入 DPI manifest ----
# 实测结论（6 组对照，见 docs）：本游戏必须保持 DPI 不感知（无 manifest）。
#   无 manifest  : 直接启动 = 640x480，经启动器 = 640x480   <- 正确
#   有 dpiAware  : 直接启动 = 512x384，经启动器 = 512x384   <- 错误
# 因此刻意不加 manifest，保持既有行为。
extra_res = []

rc_,out=run([cxx]+objs+[res]+extra_res+libs+["-o",exe])
open(os.path.join(root,"link_status.txt"),"w",encoding='utf-8').write("LINK,%s\n%s"%(rc_,out[-300:]))
print("LINK",rc_)
if rc_==0:
    print("SUCCESS: "+exe)
