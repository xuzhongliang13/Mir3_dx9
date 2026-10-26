#pragma once
#include <vector>

// =====================================================================
// [SCHEME_B] 特效层（方案B：两层渲染）
// ---------------------------------------------------------------------
// 背景：主渲染为软渲染到 565 软缓冲，Present() 用软缓冲全屏覆盖
// backbuffer。原版遗留的 D3D 特效直绘在 Present 前画到 backbuffer，
// 会被 Present 覆盖而不可见。
//
// 方案B：所有 D3D 特效在 Present() 之后绘制到 backbuffer，叠加在
// 软渲染之上（对齐 Mir3_patched_v3_backup.exe 的软渲染主流程 +
// D3D 特效独立绘制互不覆盖）。
//
// 用法：
//   1. Present 前：各特效绘制代码只"登记"绘制回调，
//      g_EffectLayer.Add(&fn, data) —— 不真正画 D3D。
//   2. Present 后：主循环调用 g_EffectLayer.Render()，统一执行所有
//      登记的 D3D 绘制回调。
// =====================================================================
typedef void (*EF_DRAWFN)(void*);

struct EF_Cmd
{
	EF_DRAWFN fn;
	void* data;
};

class CEffectLayer
{
public:
	CEffectLayer(){}

	// Present 前登记一个特效绘制回调（Present 后执行）
	void Add(EF_DRAWFN fn, void* data){ if(fn) m_cmds.push_back(EF_Cmd{fn,data}); }

	// Present 后统一执行所有登记的绘制
	void Render()
	{
		for ( size_t i = 0; i < m_cmds.size(); i++ )
		{
			if ( m_cmds[i].fn ) m_cmds[i].fn(m_cmds[i].data);
		}
		m_cmds.clear();
	}

	void Clear(){ m_cmds.clear(); }
	BOOL IsEmpty() const { return m_cmds.empty(); }

private:
	std::vector<EF_Cmd> m_cmds;
};

extern CEffectLayer g_EffectLayer;
