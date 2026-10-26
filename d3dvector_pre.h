// d3dvector_pre.h — DX7 兼容顶点类型预定义（在 dsound.h / d3d9types.h 之前 -include）
// 提供 D3DVECTOR(三参构造) + D3DVERTEX + D3DLVERTEX + D3DFVF_* 组合常量

#ifndef D3DVECTOR_PRE_H
#define D3DVECTOR_PRE_H

#include <math.h>
#include <windows.h>

// ---- 阻止 dsound.h 重新定义 D3DVECTOR ----
#define D3DVECTOR_DEFINED
#define LPD3DVECTOR_DEFINED

// ---- D3DVECTOR：带三参构造 ----
typedef struct _D3DVECTOR {
    float x;
    float y;
    float z;
#ifdef __cplusplus
    _D3DVECTOR() : x(0), y(0), z(0) {}
    _D3DVECTOR(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    _D3DVECTOR(const _D3DVECTOR& v) : x(v.x), y(v.y), z(v.z) {}
    // 运算符重载（DX7 风格）
    _D3DVECTOR& operator += (const _D3DVECTOR& v) { x += v.x; y += v.y; z += v.z; return *this; }
    _D3DVECTOR& operator -= (const _D3DVECTOR& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    _D3DVECTOR& operator *= (float d) { x *= d; y *= d; z *= d; return *this; }
    _D3DVECTOR& operator /= (float d) { float inv = 1.0f / d; x *= inv; y *= inv; z *= inv; return *this; }
    friend _D3DVECTOR operator - (const _D3DVECTOR& a, const _D3DVECTOR& b) { return _D3DVECTOR(a.x-b.x, a.y-b.y, a.z-b.z); }
    friend _D3DVECTOR operator + (const _D3DVECTOR& a, const _D3DVECTOR& b) { return _D3DVECTOR(a.x+b.x, a.y+b.y, a.z+b.z); }
    friend _D3DVECTOR operator * (const _D3DVECTOR& v, float d) { return _D3DVECTOR(v.x*d, v.y*d, v.z*d); }
    friend _D3DVECTOR operator * (float d, const _D3DVECTOR& v) { return _D3DVECTOR(v.x*d, v.y*d, v.z*d); }
    friend int operator == (const _D3DVECTOR& a, const _D3DVECTOR& b) { return a.x==b.x && a.y==b.y && a.z==b.z; }
    friend int operator != (const _D3DVECTOR& a, const _D3DVECTOR& b) { return !(a==b); }
#endif
} D3DVECTOR;
typedef D3DVECTOR *LPD3DVECTOR;

// ---- D3DVERTEX：DX7 非光照非变换顶点 (position + normal + texcoord) ----
#ifndef D3DVERTEX_DEFINED
typedef struct _D3DVERTEX {
    float x, y, z;       // position
    float nx, ny, nz;    // normal
    float tu, tv;        // texture coordinates
#ifdef __cplusplus
    _D3DVERTEX() : x(0),y(0),z(0), nx(0),ny(0),nz(0), tu(0),tv(0) {}
    _D3DVERTEX(float _x, float _y, float _z,
               float _nx, float _ny, float _nz,
               float _tu, float _tv)
        : x(_x), y(_y), z(_z), nx(_nx), ny(_ny), nz(_nz), tu(_tu), tv(_tv) {}
    // DX7 风格构造：(D3DVECTOR position, D3DVECTOR normal, float tu, float tv)
    _D3DVERTEX(const D3DVECTOR& p, const D3DVECTOR& n, float _tu, float _tv)
        : x(p.x), y(p.y), z(p.z), nx(n.x), ny(n.y), nz(n.z), tu(_tu), tv(_tv) {}
#endif
} D3DVERTEX;
#define D3DVERTEX_DEFINED
#endif
typedef D3DVERTEX *LPD3DVERTEX;

// ---- D3DLVERTEX：DX7 光照顶点 (position + color + specular + texcoord) ----
#ifndef D3DLVERTEX_DEFINED
typedef struct _D3DLVERTEX {
    float x, y, z;       // position
    DWORD dwReserved;     // reserved (aligned to 16 bytes)
    DWORD dwColor;        // diffuse
    DWORD dwSpecular;     // specular
    float tu, tv;         // texture coordinates
#ifdef __cplusplus
    _D3DLVERTEX() : x(0),y(0),z(0), dwReserved(0), dwColor(0), dwSpecular(0), tu(0),tv(0) {}
#endif
} D3DLVERTEX;
#define D3DLVERTEX_DEFINED
#endif
typedef D3DLVERTEX *LPD3DLVERTEX;

// ---- D3DTLVERTEX：DX7 光照变换顶点 ----
#ifndef D3DTLVERTEX_DEFINED
typedef struct _D3DTLVERTEX {
    float sx, sy, sz;    // screen position
    float rhw;           // reciprocal homogeneous w
    DWORD dwColor;        // diffuse
    DWORD dwSpecular;     // specular
    float tu, tv;         // texture coordinates
#ifdef __cplusplus
    _D3DTLVERTEX() : sx(0),sy(0),sz(0), rhw(1.0f), dwColor(0), dwSpecular(0), tu(0),tv(0) {}
#endif
} D3DTLVERTEX;
#define D3DTLVERTEX_DEFINED
#endif
typedef D3DTLVERTEX *LPD3DTLVERTEX;

// ---- FVF 组合常量（DX7 风格，DX9 头文件未定义这些） ----
#ifndef D3DFVF_VERTEX
#define D3DFVF_VERTEX   (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)       // 0x112
#endif
#ifndef D3DFVF_LVERTEX
#define D3DFVF_LVERTEX  (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)  // 0x1C2
#endif
#ifndef D3DFVF_TLVERTEX
#define D3DFVF_TLVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1) // 0x1C4
#endif

// ---- 颜色通道提取宏（RGB = r | (g<<8) | (b<<16)）----
#ifndef RGB_GETRED
#define RGB_GETRED(c)   ((INT)((c) & 0xff))
#endif
#ifndef RGB_GETGREEN
#define RGB_GETGREEN(c) ((INT)(((c) >> 8) & 0xff))
#endif
#ifndef RGB_GETBLUE
#define RGB_GETBLUE(c)  ((INT)(((c) >> 16) & 0xff))
#endif

#endif // D3DVECTOR_PRE_H
