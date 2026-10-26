# MIR3-DX9 代码审计报告

**审计日期**: 2026-10-14  
**审计范围**: 渲染系统核心代码  
**审计标准**: DX9 技术规范 + 代码质量最佳实践  
**审计结论**: **CHANGES REQUIRED** (需要修改)

---

## 一、严重问题 (BLOCKING)

### 1.1 设备丢失恢复不完整

**位置**: `WindHorn/WHDXGraphic.cpp:6818-6880` (Present 函数)

**问题描述**:
设备丢失时只释放了部分资源，未调用 D3DX 对象的 OnLostDevice/OnResetDevice 方法，可能导致设备恢复失败或资源泄漏。

**当前代码**:
```cpp
if ( hrCoop == D3DERR_DEVICENOTRESET )
{
    // 设备需要重置
    // 释放 D3DPOOL_DEFAULT 资源
    if ( m_pd3dPresentTex ) { m_pd3dPresentTex->Release(); m_pd3dPresentTex = NULL; }
    if ( m_pSprite ) { m_pSprite->Release(); m_pSprite = NULL; }
    // 重置设备
    ...
    if ( SUCCEEDED(hrCoop) )
    {
        // 重建资源
        ...
        if ( SUCCEEDED(hrCoop) )
            D3DXCreateSprite(m_pd3dDevice, &m_pSprite);
    }
}
```

**缺失项**:
- ❌ 未调用 `m_pSprite->OnLostDevice()` 和 `m_pFont->OnLostDevice()`
- ❌ 未调用 `m_pSprite->OnResetDevice()` 和 `m_pFont->OnResetDevice()`
- ❌ 未释放 `m_pFont` 资源

**DX9 规范要求**:
根据 DirectX 9 SDK 文档，设备丢失时必须：
1. 调用所有 D3DX 对象的 `OnLostDevice()` 方法
2. 释放所有 `D3DPOOL_DEFAULT` 资源
3. 调用 `Reset()` 重置设备
4. 重建所有资源
5. 调用所有 D3DX 对象的 `OnResetDevice()` 方法

**整改方案**:
```cpp
if ( hrCoop == D3DERR_DEVICENOTRESET )
{
    // 1. 通知 D3DX 对象释放资源
    if ( m_pSprite ) { m_pSprite->OnLostDevice(); }
    if ( m_pFont ) { m_pFont->OnLostDevice(); }
    
    // 2. 释放所有 D3DPOOL_DEFAULT 资源
    if ( m_pd3dPresentTex ) { m_pd3dPresentTex->Release(); m_pd3dPresentTex = NULL; }
    if ( m_pSprite ) { m_pSprite->Release(); m_pSprite = NULL; }
    if ( m_pFont ) { m_pFont->Release(); m_pFont = NULL; }
    
    // 3. 重置设备
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed          = TRUE;
    d3dpp.SwapEffect        = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow     = m_hWnd;
    d3dpp.BackBufferFormat  = D3DFMT_UNKNOWN;
    d3dpp.BackBufferWidth   = m_stDisplayInfo.wWidth;
    d3dpp.BackBufferHeight  = m_stDisplayInfo.wHeight;
    hrCoop = m_pd3dDevice->Reset(&d3dpp);
    
    if ( FAILED(hrCoop) )
    {
        return hrCoop;
    }
    
    // 4. 重建所有资源
    DWORD w = m_stDisplayInfo.wWidth, h = m_stDisplayInfo.wHeight;
    hrCoop = m_pd3dDevice->CreateTexture(w, h, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pd3dPresentTex, NULL);
    if ( FAILED(hrCoop) )
        hrCoop = m_pd3dDevice->CreateTexture(w, h, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pd3dPresentTex, NULL);
    if ( FAILED(hrCoop) || !m_pd3dPresentTex )
        return hrCoop;
    
    // 5. 重建 D3DX 对象并通知恢复
    D3DXCreateSprite(m_pd3dDevice, &m_pSprite);
    D3DXCreateFontA(m_pd3dDevice, 14, 0, FW_NORMAL, 1, FALSE,
                    DEFAULT_CHARSET, 0, 0, 0, "System", &m_pFont);
    if ( m_pSprite ) { m_pSprite->OnResetDevice(); }
    if ( m_pFont ) { m_pFont->OnResetDevice(); }
    
    return D3DERR_DEVICENOTRESET;  // 本帧跳过，下一帧正常渲染
}
```

**优先级**: 🔴 **必须立即修复**

---

### 1.2 DrawBillBoard 使用软件渲染，性能严重下降

**位置**: `Common/ImageHandler.cpp:384-518`

**问题描述**:
将原本应该由 GPU 处理的 billboard 渲染改为 CPU 逐像素软件混合，导致每帧大量 CPU 计算。

**当前代码**:
```cpp
HRESULT CImageHandler::DrawBillBoard(LPDIRECT3DDEVICE9 lpDevice, D3DVECTOR* pvTrans, D3DVECTOR* pvScale, 
                                     D3DMATERIAL9* pmtrl, IDirect3DTexture9* lpddsTextr, BYTE bBlendType)
{
    // [DX9][T59] billboard 软件化: D3D 管线绘制到 swapchain backbuffer, 随后 Present()
    // 用不透明纹理覆盖全屏 → 所有 billboard 内容不可见(窗口背景/角色/特效等 39 处调用)。
    // 改为软件 blit 到主软缓冲 m_pddsBackBuffer, 与软渲染合流后一起 Present。
    
    // ... 130+ 行软件渲染代码 ...
    
    for ( LONG y = 0; y < nH; y++ )
    {
        for ( LONG x = 0; x < nW; x++ )
        {
            // 逐像素 Alpha 混合计算
            DWORD c = pRow[x];
            BYTE  a = (BYTE)(c >> 24);
            if ( a == 0 ) continue;
            // ... 大量 CPU 计算 ...
        }
    }
}
```

**问题分析**:
1. **性能问题**: 每帧处理 800x600 = 480,000 像素，39 处调用可能导致数百万次 CPU 计算
2. **架构问题**: 注释说明这是为了绕过 DX9 管线问题的临时方案
3. **职责过重**: 单个函数处理纹理渲染、纯色填充、Alpha 混合等多种职责

**根因分析**:
根据代码注释，问题出在：
- D3D 管线绘制到 swapchain backbuffer
- 随后 Present() 用不透明纹理覆盖全屏
- 导致所有 billboard 内容不可见

**整改方案**:

**方案 A: 修复 D3D 管线问题（推荐）**
```cpp
HRESULT CImageHandler::DrawBillBoard(LPDIRECT3DDEVICE9 lpDevice, D3DVECTOR* pvTrans, D3DVECTOR* pvScale, 
                                     D3DMATERIAL9* pmtrl, IDirect3DTexture9* lpddsTextr, BYTE bBlendType)
{
    if ( !lpDevice || !pvTrans || !pvScale ) return E_FAIL;
    
    // 使用 DrawPrimitiveUP 直接绘制，不依赖 Sprite->Begin/End
    // 这样可以避免与 Present() 的冲突
    
    // 1. 保存渲染状态
    DWORD oldAlphaBlend, oldSrcBlend, oldDestBlend, oldLighting;
    DWORD oldColorOp0, oldColorArg1_0, oldColorArg2_0;
    lpDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlphaBlend);
    lpDevice->GetRenderState(D3DRS_SRCBLEND, &oldSrcBlend);
    lpDevice->GetRenderState(D3DRS_DESTBLEND, &oldDestBlend);
    lpDevice->GetRenderState(D3DRS_LIGHTING, &oldLighting);
    
    // 2. 设置渲染状态
    lpDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    
    // 根据混合类型设置
    switch ( bBlendType )
    {
    case 0: // 正常 Alpha 混合
        lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        break;
    case 6: // 加色混合
        lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        break;
    }
    
    // 3. 设置纹理阶段
    lpDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    lpDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    lpDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    lpDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    
    // 4. 构建顶点
    FLOAT x = pvTrans->x, y = pvTrans->y;
    FLOAT w = pvScale->x, h = pvScale->y;
    
    DWORD vertexColor = 0xFFFFFFFF;
    if ( pmtrl )
    {
        BYTE r = (BYTE)(pmtrl->Diffuse.r * 255.0f);
        BYTE g = (BYTE)(pmtrl->Diffuse.g * 255.0f);
        BYTE b = (BYTE)(pmtrl->Diffuse.b * 255.0f);
        BYTE a = (BYTE)(pmtrl->Diffuse.a * 255.0f);
        vertexColor = D3DCOLOR_ARGB(a, r, g, b);
    }
    
    // FVF 字段顺序必须严格匹配: XYZRHW → DIFFUSE → TEX1
    struct D3DVERTEX {
        FLOAT x, y, z, rhw;
        DWORD color;
        FLOAT u, v;
    };
    
    D3DVERTEX vertices[4] = {
        { x,     y,     0.0f, 1.0f, vertexColor, 0.0f, 0.0f },
        { x + w, y,     0.0f, 1.0f, vertexColor, 1.0f, 0.0f },
        { x,     y + h, 0.0f, 1.0f, vertexColor, 0.0f, 1.0f },
        { x + w, y + h, 0.0f, 1.0f, vertexColor, 1.0f, 1.0f }
    };
    
    // 5. 绘制
    lpDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE);
    lpDevice->SetTexture(0, lpddsTextr);
    lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(D3DVERTEX));
    
    // 6. 恢复渲染状态
    lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlphaBlend);
    lpDevice->SetRenderState(D3DRS_SRCBLEND, oldSrcBlend);
    lpDevice->SetRenderState(D3DRS_DESTBLEND, oldDestBlend);
    lpDevice->SetRenderState(D3DRS_LIGHTING, oldLighting);
    
    return S_OK;
}
```

**方案 B: 如果必须使用软件渲染（不推荐）**
- 使用 SIMD 指令优化像素处理
- 考虑使用多线程并行处理
- 减少调用次数（合并绘制）

**优先级**: 🔴 **必须立即修复**

---

## 二、中等问题 (WARNING)

### 2.1 DrawWeatherLayer 全局函数访问全局变量

**位置**: `GameProcess/GameProc.cpp:2340-2385`

**问题描述**:
DrawWeatherLayer 全局函数直接访问全局变量 g_xGameProc，违反封装原则。

**当前代码**:
```cpp
VOID DrawWeatherLayer(void* pData)
{
    (void)pData;
    
    LPDIRECT3DDEVICE9 pDev = g_xMainWnd.Get3DDevice();
    if ( !pDev ) return;
    
    // ... 直接访问全局变量 ...
    if ( g_xGameProc.m_bSnowOn && g_xGameProc.m_bShowMist )
    {
        g_xGameProc.m_xMist.ProgressMist();
    }
    
    if ( g_xGameProc.m_bSnowOn && g_xGameProc.m_bShowSnow )
    {
        g_xGameProc.m_xSnow.UpdateSystem(g_xGameProc.m_nWeatherLoopTime);
        g_xGameProc.m_xSnow.RenderSystem(g_xMainWnd.Get3DDevice());
    }
    
    if ( g_xGameProc.m_bShowRain )
    {
        g_xGameProc.m_xRain.UpdateSystem(g_xGameProc.m_nWeatherLoopTime);
        g_xGameProc.m_xRain.RenderSystem(g_xMainWnd.Get3DDevice());
    }
}
```

**问题**:
- 违反依赖注入原则
- 难以测试和维护
- 函数签名 `void* pData` 未使用

**整改方案**:
```cpp
// 方案 1: 将函数改为 CGameProcess 的成员函数
VOID CGameProcess::DrawWeatherLayer()
{
    LPDIRECT3DDEVICE9 pDev = g_xMainWnd.Get3DDevice();
    if ( !pDev ) return;
    
    // ... 保存/设置变换矩阵 ...
    
    if ( m_bSnowOn && m_bShowMist )
    {
        m_xMist.ProgressMist();
    }
    
    if ( m_bSnowOn && m_bShowSnow )
    {
        m_xSnow.UpdateSystem(m_nWeatherLoopTime);
        m_xSnow.RenderSystem(pDev);
    }
    
    if ( m_bShowRain )
    {
        m_xRain.UpdateSystem(m_nWeatherLoopTime);
        m_xRain.RenderSystem(pDev);
    }
    
    // ... 恢复变换矩阵 ...
}

// 调用方式
g_xGameProc.DrawWeatherLayer();
```

**优先级**: 🟡 **建议修复**

---

### 2.2 DrawBillBoard 函数过长，职责过重

**位置**: `Common/ImageHandler.cpp:384-518`

**问题描述**:
单个函数超过 130 行，同时处理纹理渲染、纯色填充、Alpha 混合等多种职责。

**整改方案**:
拆分为多个函数：
```cpp
// 1. 纯色 billboard 绘制
HRESULT DrawSolidColorBillboard(LPDIRECT3DDEVICE9 lpDevice, LONG nX, LONG nY, LONG nW, LONG nH, 
                                WORD wMatColor, BYTE bOpa, BYTE bBlendType);

// 2. 纹理 billboard 绘制
HRESULT DrawTexturedBillboard(LPDIRECT3DDEVICE9 lpDevice, LONG nX, LONG nY, LONG nW, LONG nH,
                              IDirect3DTexture9* lpddsTextr, BYTE bOpa, BYTE bBlendType);

// 3. 主入口函数
HRESULT DrawBillBoard(LPDIRECT3DDEVICE9 lpDevice, D3DVECTOR* pvTrans, D3DVECTOR* pvScale,
                      D3DMATERIAL9* pmtrl, IDirect3DTexture9* lpddsTextr, BYTE bBlendType)
{
    if ( !lpddsTextr )
    {
        return DrawSolidColorBillboard(...);
    }
    else
    {
        return DrawTexturedBillboard(...);
    }
}
```

**优先级**: 🟡 **建议修复**

---

## 三、轻微问题 (INFO)

### 3.1 缺少文档注释

**问题**: 公共方法缺少文档注释

**整改方案**:
```cpp
/**
 * @brief 绘制 Billboard（2D 精灵）
 * 
 * @param lpDevice D3D9 设备指针
 * @param pvTrans 平移向量（屏幕坐标）
 * @param pvScale 缩放向量（宽高）
 * @param pmtrl 材质（颜色和透明度）
 * @param lpddsTextr 纹理指针（NULL 表示纯色填充）
 * @param bBlendType 混合模式（0=正常，6=加色，7=纹理Alpha）
 * 
 * @return HRESULT 成功返回 S_OK，失败返回 E_FAIL
 * 
 * @note 该函数使用 DrawPrimitiveUP 直接绘制，不依赖 Sprite->Begin/End
 */
HRESULT CImageHandler::DrawBillBoard(...);
```

**优先级**: 🟢 **可选修复**

---

### 3.2 魔法数字

**问题**: 代码中使用魔法数字（800, 600, 255 等）

**整改方案**:
```cpp
// 定义常量
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const BYTE MAX_ALPHA = 255;

// 使用常量
D3DXMatrixOrthoLH(&matProj, (FLOAT)SCREEN_WIDTH, (FLOAT)SCREEN_HEIGHT, -1.0f, 1.0f);
```

**优先级**: 🟢 **可选修复**

---

## 四、审计总结

### 问题统计

| 严重级别 | 数量 | 说明 |
|---------|------|------|
| 🔴 BLOCKING | 2 | 必须立即修复 |
| 🟡 WARNING | 2 | 建议修复 |
| 🟢 INFO | 2 | 可选修复 |

### 核心问题

1. **设备丢失恢复不完整** - 可能导致程序崩溃或资源泄漏
2. **DrawBillBoard 使用软件渲染** - 性能严重下降，架构不合理

### 整改优先级

1. **立即修复** (BLOCKING):
   - 完善设备丢失恢复流程
   - 重构 DrawBillBoard，使用硬件加速

2. **尽快修复** (WARNING):
   - 消除全局状态依赖
   - 拆分过长函数

3. **后续优化** (INFO):
   - 添加文档注释
   - 消除魔法数字

### 风险评估

- **不修复 BLOCKING 问题的风险**: 
  - 程序可能在设备丢失时崩溃
  - 帧率可能下降到不可接受的程度
  
- **修复成本**:
  - 设备丢失恢复: 约 30 分钟
  - DrawBillBoard 重构: 约 2-4 小时

### 建议

1. **立即修复 BLOCKING 问题**，确保程序稳定性
2. **在下一个迭代中修复 WARNING 问题**，提升代码质量
3. **INFO 问题可在日常维护中逐步改进**

---

**审计结论**: **CHANGES REQUIRED**

**审计人**: AI Code Reviewer  
**审计日期**: 2026-10-14
