/******************************************************************************************************************
                                                                                                                   
	����:																											
																													
	�ۼ���:																											
	�ۼ���:																											
																													
	[����][������] : ���� ����																						
                                                                                                                   
*******************************************************************************************************************/



#include "StdAfx.h"



/******************************************************************************************************************

	CImageHandler Class

*******************************************************************************************************************/
/******************************************************************************************************************

	�Լ��� : CImageHandler::CImageHandler()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	���   : 

	[����][������] : ��������

*******************************************************************************************************************/
CImageHandler::CImageHandler()
: m_helper( m_xImageList )
{
	ZeroMemory(m_xImageList, sizeof(CWHWilImageData)*_MAX_IMAGE);
	ZeroMemory(m_pszFileList, _MAX_IMAGE*MAX_PATH);
}



/******************************************************************************************************************

	�Լ��� : CImageHandler::~CImageHandler()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	���   : 

	[����][������] : ��������

*******************************************************************************************************************/
CImageHandler::~CImageHandler()
{
	DeleteAllImage();
}



/******************************************************************************************************************

	�Լ��� : CImageHandler::LoadAllImage()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	�Է�   : BOOL bIsMemMapped
	         BOOL bComp
	���   : VOID 

	[����][������] : ��������

*******************************************************************************************************************/
VOID CImageHandler::LoadAllImage(BOOL bIsMemMapped, BOOL bComp)
{
	InitAllImage();

/*	for ( INT nCnt = 0; nCnt < 4; nCnt++ )
	{
		m_xImageList[nCnt].NewLoad(m_pszFileList[nCnt]);
	}
*/
	for ( INT nCnt = 0; nCnt < 14; nCnt++ )
	{
		if ( !m_xImageList[nCnt].NewLoad(m_pszFileList[nCnt], 1) )
		{
			m_xImageList[nCnt].Destroy();
			m_xImageList[nCnt].NewLoad(m_pszFileList[nCnt]);
		}
	}

	for ( INT nCnt = 70; nCnt < _MAX_IMAGE; nCnt++ )
	{
		m_xImageList[nCnt].NewLoad(m_pszFileList[nCnt]);
	}
}


VOID CImageHandler::InitAllImage()
{
	/* IMAGE DEFINE���� ��ġ�ؾ��Ѵ�.--------------------------------------------------------------*/
	//~~~ ����̹���. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[0], ".\\Data\\tilesc.wil");							//0
	strcpy(m_pszFileList[1], ".\\Data\\tiles30c.wil");							//1
	strcpy(m_pszFileList[2], ".\\Data\\Tiles5c.wil");							//2
	strcpy(m_pszFileList[3], ".\\Data\\smtilesc.wil");							//3
	strcpy(m_pszFileList[4], ".\\Data\\housesc.wil");							//4
	strcpy(m_pszFileList[5], ".\\Data\\cliffsc.wil");							//5
	strcpy(m_pszFileList[6], ".\\Data\\dungeonsc.wil");							//6
	strcpy(m_pszFileList[7], ".\\Data\\innersc.wil");							//7
	strcpy(m_pszFileList[8], ".\\Data\\furnituresc.wil");						//8
	strcpy(m_pszFileList[9], ".\\Data\\wallsc.wil");							//9
	strcpy(m_pszFileList[10], ".\\Data\\smobjectsc.wil");						//10
	strcpy(m_pszFileList[11], ".\\Data\\animationsc.wil");						//11
	strcpy(m_pszFileList[12], ".\\Data\\object1c.wil");							//12
	strcpy(m_pszFileList[13], ".\\Data\\object2c.wil");							//13
	// Ǫ������.
	strcpy(m_pszFileList[14], ".\\Data\\Wood\\tilesc.wil");						//14
	strcpy(m_pszFileList[15], ".\\Data\\Wood\\tiles30c.wil");					//15
	strcpy(m_pszFileList[16], ".\\Data\\Wood\\Tiles5c.wil");					//16
	strcpy(m_pszFileList[17], ".\\Data\\Wood\\smtilesc.wil");					//17
	strcpy(m_pszFileList[18], ".\\Data\\Wood\\housesc.wil");					//18
	strcpy(m_pszFileList[19], ".\\Data\\Wood\\cliffsc.wil");					//19
	strcpy(m_pszFileList[20], ".\\Data\\Wood\\dungeonsc.wil");					//20
	strcpy(m_pszFileList[21], ".\\Data\\Wood\\innersc.wil");					//21
	strcpy(m_pszFileList[22], ".\\Data\\Wood\\furnituresc.wil");				//22
	strcpy(m_pszFileList[23], ".\\Data\\Wood\\wallsc.wil");						//23
	strcpy(m_pszFileList[24], ".\\Data\\Wood\\smobjectsc.wil");					//24
	strcpy(m_pszFileList[25], ".\\Data\\Wood\\animationsc.wil");				//25
	strcpy(m_pszFileList[26], ".\\Data\\Wood\\object1c.wil");					//26
	strcpy(m_pszFileList[27], ".\\Data\\Wood\\object2c.wil");					//27
	// �縷����.
	strcpy(m_pszFileList[28], ".\\Data\\Sand\\tilesc.wil");						//28
	strcpy(m_pszFileList[29], ".\\Data\\Sand\\tiles30c.wil");					//29
	strcpy(m_pszFileList[30], ".\\Data\\Sand\\Tiles5c.wil");					//30
	strcpy(m_pszFileList[31], ".\\Data\\Sand\\smtilesc.wil");					//31
	strcpy(m_pszFileList[32], ".\\Data\\Sand\\housesc.wil");					//32
	strcpy(m_pszFileList[33], ".\\Data\\Sand\\cliffsc.wil");					//33
	strcpy(m_pszFileList[34], ".\\Data\\Sand\\dungeonsc.wil");					//34
	strcpy(m_pszFileList[35], ".\\Data\\Sand\\innersc.wil");					//35
	strcpy(m_pszFileList[36], ".\\Data\\Sand\\furnituresc.wil");				//36
	strcpy(m_pszFileList[37], ".\\Data\\Sand\\wallsc.wil");						//37
	strcpy(m_pszFileList[38], ".\\Data\\Sand\\smobjectsc.wil");					//38
	strcpy(m_pszFileList[39], ".\\Data\\Sand\\animationsc.wil");				//39
	strcpy(m_pszFileList[40], ".\\Data\\Sand\\object1c.wil");					//40
	strcpy(m_pszFileList[41], ".\\Data\\Sand\\object2c.wil");					//41

	// ��������.
	strcpy(m_pszFileList[42], ".\\Data\\Snow\\tilesc.wil");						//42
	strcpy(m_pszFileList[43], ".\\Data\\Snow\\tiles30c.wil");					//43
	strcpy(m_pszFileList[44], ".\\Data\\Snow\\Tiles5c.wil");					//44
	strcpy(m_pszFileList[45], ".\\Data\\Snow\\smtilesc.wil");					//45
	strcpy(m_pszFileList[46], ".\\Data\\Snow\\housesc.wil");					//46
	strcpy(m_pszFileList[47], ".\\Data\\Snow\\cliffsc.wil");					//47
	strcpy(m_pszFileList[48], ".\\Data\\Snow\\dungeonsc.wil");					//48
	strcpy(m_pszFileList[49], ".\\Data\\Snow\\innersc.wil");					//49
	strcpy(m_pszFileList[50], ".\\Data\\Snow\\furnituresc.wil");				//50
	strcpy(m_pszFileList[51], ".\\Data\\Snow\\wallsc.wil");						//51
	strcpy(m_pszFileList[52], ".\\Data\\Snow\\smobjectsc.wil");					//52
	strcpy(m_pszFileList[53], ".\\Data\\Snow\\animationsc.wil");				//53
	strcpy(m_pszFileList[54], ".\\Data\\Snow\\object1c.wil");					//54
	strcpy(m_pszFileList[55], ".\\Data\\Snow\\object2c.wil");					//55

	// ��������.
	strcpy(m_pszFileList[56], ".\\Data\\Forest\\tilesc.wil");					//56
	strcpy(m_pszFileList[57], ".\\Data\\Forest\\tiles30c.wil");					//57
	strcpy(m_pszFileList[58], ".\\Data\\Forest\\Tiles5c.wil");					//58
	strcpy(m_pszFileList[59], ".\\Data\\Forest\\smtilesc.wil");					//59
	strcpy(m_pszFileList[60], ".\\Data\\Forest\\housesc.wil");					//60
	strcpy(m_pszFileList[61], ".\\Data\\Forest\\cliffsc.wil");					//61
	strcpy(m_pszFileList[62], ".\\Data\\Forest\\dungeonsc.wil");				//62
	strcpy(m_pszFileList[63], ".\\Data\\Forest\\innersc.wil");					//63
	strcpy(m_pszFileList[64], ".\\Data\\Forest\\furnituresc.wil");				//64
	strcpy(m_pszFileList[65], ".\\Data\\Forest\\wallsc.wil");					//65
	strcpy(m_pszFileList[66], ".\\Data\\Forest\\smobjectsc.wil");				//66
	strcpy(m_pszFileList[67], ".\\Data\\Forest\\animationsc.wil");				//67
	strcpy(m_pszFileList[68], ".\\Data\\Forest\\object1c.wil");					//68
	strcpy(m_pszFileList[69], ".\\Data\\Forest\\object2c.wil");					//69

	//~~~ �������̽��̹���. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[70], ".\\Data\\GameInter.wil");						//70 

	//~~~ �����̹���. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[71], ".\\Data\\M-Hum.wil");							//71 	
	strcpy(m_pszFileList[72], ".\\Data\\M-Weapon1.wil");						//72
	strcpy(m_pszFileList[73], ".\\Data\\M-Weapon2.wil");						//73
	strcpy(m_pszFileList[74], ".\\Data\\M-Weapon3.wil");						//74
	strcpy(m_pszFileList[75], ".\\Data\\M-Weapon4.wil");						//75

	//~~~ �����̹���. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[76], ".\\Data\\WM-Hum.wil");							//76
	strcpy(m_pszFileList[77], ".\\Data\\WM-Weapon1.wil");						//77
	strcpy(m_pszFileList[78], ".\\Data\\WM-Weapon2.wil");						//78
	strcpy(m_pszFileList[79], ".\\Data\\WM-Weapon3.wil");						//79
	strcpy(m_pszFileList[80], ".\\Data\\WM-Weapon4.wil");						//80

	//~~~ ������ �� �������̽�, ����, ��Ÿ�̹���. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[81], ".\\Data\\Magic.wil");							//81
	strcpy(m_pszFileList[82], ".\\Data\\Inventory.wil");						//82
	strcpy(m_pszFileList[83], ".\\Data\\Equip.wil");							//83
	strcpy(m_pszFileList[84], ".\\Data\\Ground.wil");							//84
	strcpy(m_pszFileList[85], ".\\Data\\MIcon.wil");							//85
	strcpy(m_pszFileList[86], ".\\Data\\ProgUse.wil");							//86
	strcpy(m_pszFileList[87], ".\\Data\\Horse.wil");							//87

	//~~~ �����̹���. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[88], ".\\Data\\Mon-1.wil");							//88
	strcpy(m_pszFileList[89], ".\\Data\\Mon-2.wil");							//89
	strcpy(m_pszFileList[90], ".\\Data\\Mon-3.wil");							//90
	strcpy(m_pszFileList[91], ".\\Data\\Mon-4.wil");							//91
	strcpy(m_pszFileList[92], ".\\Data\\Mon-5.wil");							//92
	strcpy(m_pszFileList[93], ".\\Data\\Mon-6.wil");							//93
	strcpy(m_pszFileList[94], ".\\Data\\Mon-7.wil");							//94
	strcpy(m_pszFileList[95], ".\\Data\\Mon-8.wil");							//95
	strcpy(m_pszFileList[96], ".\\Data\\Mon-9.wil");							//96
	strcpy(m_pszFileList[97], ".\\Data\\Mon-10.wil");							//97
	strcpy(m_pszFileList[98], ".\\Data\\Mon-11.wil");							//98
	strcpy(m_pszFileList[99], ".\\Data\\Mon-12.wil");							//99
	strcpy(m_pszFileList[100], ".\\Data\\Mon-13.wil");							//100
	strcpy(m_pszFileList[101], ".\\Data\\Mon-14.wil");							//101
	strcpy(m_pszFileList[102], ".\\Data\\Mon-15.wil");							//102
	strcpy(m_pszFileList[103], ".\\Data\\Mon-16.wil");							//103				
	strcpy(m_pszFileList[104], ".\\Data\\Mon-17.wil");							//104
	strcpy(m_pszFileList[105], ".\\Data\\Mon-18.wil");							//105
	strcpy(m_pszFileList[106], ".\\Data\\Mon-19.wil");							//106
	strcpy(m_pszFileList[107], ".\\Data\\Mon-20.wil");							//107

	//~~~ ���ͱ׸����̹���. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[108], ".\\Data\\MonS-1.wil");							//108
	strcpy(m_pszFileList[109], ".\\Data\\MonS-2.wil");							//109
	strcpy(m_pszFileList[110], ".\\Data\\MonS-3.wil");							//110
	strcpy(m_pszFileList[111], ".\\Data\\MonS-4.wil");							//111
	strcpy(m_pszFileList[112], ".\\Data\\MonS-5.wil");							//112
	strcpy(m_pszFileList[113], ".\\Data\\MonS-6.wil");							//113
	strcpy(m_pszFileList[114], ".\\Data\\MonS-7.wil");							//114
	strcpy(m_pszFileList[115], ".\\Data\\MonS-8.wil");							//115
	strcpy(m_pszFileList[116], ".\\Data\\MonS-9.wil");							//116
	strcpy(m_pszFileList[117], ".\\Data\\MonS-10.wil");							//117
	strcpy(m_pszFileList[118], ".\\Data\\MonS-11.wil");							//118
	strcpy(m_pszFileList[119], ".\\Data\\MonS-12.wil");							//119
	strcpy(m_pszFileList[120], ".\\Data\\MonS-13.wil");							//120
	strcpy(m_pszFileList[121], ".\\Data\\MonS-14.wil");							//121
	strcpy(m_pszFileList[122], ".\\Data\\MonS-15.wil");							//122
	strcpy(m_pszFileList[123], ".\\Data\\MonS-16.wil");							//123
	strcpy(m_pszFileList[124], ".\\Data\\MonS-17.wil");							//124
	strcpy(m_pszFileList[125], ".\\Data\\MonS-18.wil");							//125
	strcpy(m_pszFileList[126], ".\\Data\\MonS-19.wil");							//126
	strcpy(m_pszFileList[127], ".\\Data\\MonS-20.wil");							//127

	//~~~ NPC �̹���. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[128], ".\\Data\\NPC.wil");								//128

	//~~~ ���͸��� �̹���. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[129], ".\\Data\\MonMagic.wil");						//129
	strcpy(m_pszFileList[130], ".\\Data\\MonImg.wil");							//130
	strcpy(m_pszFileList[131], ".\\Data\\M-Hair.wil");							//131
	strcpy(m_pszFileList[132], ".\\Data\\M-Helmet1.wil");						//132	
	strcpy(m_pszFileList[133], ".\\Data\\WM-Hair.wil");							//133
	strcpy(m_pszFileList[134], ".\\Data\\WM-Helmet1.wil");						//134

	//~~~ ��������. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[135], ".\\Data\\DMon-1.wil");							//135
	strcpy(m_pszFileList[136], ".\\Data\\DMonS-1.wil");							//136

	strcpy(m_pszFileList[137], ".\\Data\\MagicEx.wil");							//137
	strcpy(m_pszFileList[138], ".\\Data\\MonMagicEx.wil");						//138
	strcpy(m_pszFileList[139], ".\\Data\\StoreItem.wil");						//139
	strcpy(m_pszFileList[140], ".\\Data\\MonMagicEx2.wil");						//140

	//~~~ ��ϼ���. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[141], ".\\Data\\Mon-54.wil");							//141

	//~~~ ��� ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[142], ".\\Data\\Flag.wil");							//142

	//~~~ MonMagicEx3 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[143], ".\\Data\\MonMagicEx3.wil");						//143

	//~~~ ��� ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[144], ".\\Data\\M-Helmet2.wil");						//144
	strcpy(m_pszFileList[145], ".\\Data\\WM-Helmet2.wil");						//145

	//~~~ ���� ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[146], ".\\Data\\M-Weapon5.wil");						//146
	strcpy(m_pszFileList[147], ".\\Data\\WM-Weapon5.wil");						//147

	//~~~ MonMagicEx4 (����������) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[148], ".\\Data\\MonMagicEx4.wil");						//148

	//~~~ MonMagicEx4 (����������) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	strcpy(m_pszFileList[149], ".\\Data\\MagicEx2.wil");						//149

	strcpy(m_pszFileList[150], ".\\Data\\Mon-21.wil");							//150
	strcpy(m_pszFileList[151], ".\\Data\\MonS-21.wil");							//151

	strcpy(m_pszFileList[152], ".\\Data\\HorseS.wil");							//152
	strcpy(m_pszFileList[153], ".\\Data\\M-Weapon10.wil");						//153
	strcpy(m_pszFileList[154], ".\\Data\\WM-Weapon10.wil");						//154

	strcpy(m_pszFileList[155], ".\\Data\\Mon-22.wil");							//155
	strcpy(m_pszFileList[156], ".\\Data\\MonS-22.wil");							//156

	strcpy(m_pszFileList[157], ".\\Data\\MagicEx3.wil");						//157

	strcpy(m_pszFileList[158], ".\\Data\\MonMagicEx5.wil");						//157

	strcpy(m_pszFileList[159], ".\\Data\\GameInter1.wil");						//159

	strcpy(m_pszFileList[160], ".\\Data\\M-SHum.wil");							//160 - �ű��Ǻ� ����Ʈ 2006.07.03
	strcpy(m_pszFileList[161], ".\\Data\\WM-SHum.wil");							//161
	strcpy(m_pszFileList[162], ".\\Data\\SaleItem.wil");						//162 - SaleItem : �������� ������ �̹���, �������̽� �̹����� GameInter

	for ( INT nCnt = 0; nCnt < _MAX_IMAGE; nCnt++ )
	{
		m_xImageList[nCnt].Init();
	}

 	D3DVECTOR vNorm(0, 0, -1);
	m_avBillBoard[0]  = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
	m_avBillBoard[1]  = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
	m_avBillBoard[2]  = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
	m_avBillBoard[3]  = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);
}


/******************************************************************************************************************

	�Լ��� : CImageHandler::DeleteAllImage()

	�ۼ��� : 
	�ۼ��� : 

	����   : 
	���   : VOID 

	[����][������] : ��������

*******************************************************************************************************************/
VOID CImageHandler::DeleteAllImage()
{
	INT nCnt;

	for ( nCnt = 0; nCnt < _MAX_IMAGE; nCnt++ )
	{
		m_xImageList[nCnt].Destroy();
	}

	DelAllTextrFileList();
}

IDirect3DTexture9* CImageHandler::AddTextr(WORD wFileType, WORD wFileIdx, INT nImgIdx)
{
	if ( wFileType >= _MAX_TEXTR_FILE || wFileIdx >= _MAX_IMAGE )
	{
		return NULL;
	}

	return m_helper.AddTexture( wFileType, wFileIdx, (WORD)nImgIdx );
}

IDirect3DTexture9* CImageHandler::GetTextrImg(WORD wFileType, WORD wFileIdx, INT nImgIdx)
{
	if ( wFileType >= _MAX_TEXTR_FILE || wFileIdx >= _MAX_IMAGE )
	{
		return NULL;
	}

	return m_helper.GetTexture( wFileType, wFileIdx, (WORD)nImgIdx );
}

VOID CImageHandler::DelAllTextrFileList()
{
	m_helper.DelAllTexture();
}



// [FIX-F2] 565 像素合成, 按 DX7 的 SetBlendRenderState 语义实现 6 种混合。
// 原实现完全忽略 bBlendType, 所有 billboard 都按普通 alpha 处理,
// 导致 _BLEND_LIGHT(加色, 魔法/爆炸光效) 退化成普通半透明 -> 光效不亮。
static inline WORD BlendPixel565(WORD dst, WORD src565, BYTE a, BYTE bOpa, BYTE blend,
                                     BYTE matR = 255, BYTE matG = 255, BYTE matB = 255)
{
	// 有效不透明度: 纹理 alpha 与窗口/材质不透明度取小
	DWORD ea = a;
	if ( bOpa < ea ) ea = bOpa;
	if ( ea > 255 ) ea = 255;

	DWORD sr = (src565 >> 11) & 0x1F, sg = (src565 >> 5) & 0x3F, sb = src565 & 0x1F;
	DWORD dr = (dst    >> 11) & 0x1F, dg = (dst    >> 5) & 0x3F, db = dst    & 0x1F;
	DWORD nr, ng, nb;

	switch ( blend )
	{
	case 1:	// _BLEND_LIGHT: SRCBLEND=ONE, DESTBLEND=ONE  -> 加色
		nr = dr + sr * ea / 255; if ( nr > 31 ) nr = 31;
		ng = dg + sg * ea / 255; if ( ng > 63 ) ng = 63;
		nb = db + sb * ea / 255; if ( nb > 31 ) nb = 31;
		break;

	case 2:	// _BLEND_LIGHTINV: SRCBLEND=ONE, DESTBLEND=INVSRCCOLOR -> dst*(1-src) + src
	{		// [FIX-F1] D3D fixed function modulates the source with the material
		// Diffuse (MODULATE stage), and LIGHTINV uses src COLOR only - alpha is
		// ignored. So the material RGB is the only brightness knob; scaling it
		// here is what makes a 190/255 material dim the effect exactly like D3D.
		// All pre-existing callers pass white (255) and are therefore unaffected.
		DWORD mr = (DWORD)sr * matR / 255;
		DWORD mg = (DWORD)sg * matG / 255;
		DWORD mb = (DWORD)sb * matB / 255;
		nr = dr + (mr * (31 - dr)) / 31; if ( nr > 31 ) nr = 31;
		ng = dg + (mg * (63 - dg)) / 63; if ( ng > 63 ) ng = 63;
		nb = db + (mb * (31 - db)) / 31; if ( nb > 31 ) nb = 31;
		break;
	}

	default:	// 0 _BLEND_NORMAL(及其余): SRCALPHA / INVSRCALPHA
		if ( ea >= 255 )
		{
			nr = sr; ng = sg; nb = sb;
		}
		else
		{
			nr = ( sr * ea + dr * (255 - ea) ) / 255;
			ng = ( sg * ea + dg * (255 - ea) ) / 255;
			nb = ( sb * ea + db * (255 - ea) ) / 255;
		}
		break;
	}

	return (WORD)( (nr << 11) | (ng << 5) | nb );
}

HRESULT CImageHandler::DrawBillBoard(LPDIRECT3DDEVICE9 lpDevice, D3DVECTOR* pvTrans, D3DVECTOR* pvScale, /*D3DVECTOR vRot, */D3DMATERIAL9* pmtrl, /*FLOAT fRotRad, */IDirect3DTexture9* lpddsTextr, BYTE bBlendType)
{
	// [DX9][T59] billboard 软件化: D3D 管线绘制到 swapchain backbuffer, 随后 Present()
	// 用不透明纹理覆盖全屏 → 所有 billboard 内容不可见(窗口背景/角色/特效等 39 处调用)。
	// 改为软件 blit 到主软缓冲 m_pddsBackBuffer, 与软渲染合流后一起 Present。
	// [DX9][T62] 修复两个回归:
	//  1) NULL 纹理调用(纯色高亮/蒙层, 如物品拖拽放置格、按钮光效): 原 D3D 路径用材质
	//     Diffuse 颜色画实心四边形, 此处补回等效的 565 纯色半透明填充。
	//  2) 纹理读越界: sx>0 时 nW 截断到 texW 会读到 tex[sx..sx+texW-1] 超出纹理,
	//     修正为 sx+nW<=texW。
	if ( !pvTrans || !pvScale ) return E_FAIL;

	// 调用方传的是窗口/图像的屏幕坐标(左上角 800x600 系)
	LONG nX = (LONG)pvTrans->x, nY = (LONG)pvTrans->y;
	LONG nW = (LONG)pvScale->x, nH = (LONG)pvScale->y;

	CDX9SoftSurface* dst = g_xMainWnd.GetBackBuffer();
	if ( !dst || !dst->m_pBits ) return E_FAIL;

	// 全局窗口透明度(窗口 m_bOpa: 255 不透明 / NPC 220 等)
	BYTE bOpa = 255;
	if ( pmtrl ) { FLOAT fa = pmtrl->Diffuse.a; bOpa = (BYTE)(fa * 255.0f); if ( bOpa > 255 ) bOpa = 255; }

	// 材质基色(纯色 billboard 用; 纹理 billboard 混合时忽略)
	BYTE matR = 255, matG = 255, matB = 255;
	if ( pmtrl )
	{
		matR = (BYTE)(pmtrl->Diffuse.r * 255.0f + 0.5f);
		matG = (BYTE)(pmtrl->Diffuse.g * 255.0f + 0.5f);
		matB = (BYTE)(pmtrl->Diffuse.b * 255.0f + 0.5f);
	}
	WORD wMatColor = (WORD)((((DWORD)matR & 0xF8) << 8) | (((DWORD)matG & 0xFC) << 3) | ((DWORD)matB >> 3));

	// [FIX-F1] bOpa==0 表示完全透明(窗口淡出)。原判据 a>=bOpa 在 bOpa==0 时恒真,
	// 会导致淡出过程始终以全不透明绘制。
	if ( bOpa == 0 ) return S_OK;

	// 裁剪到目标表面
	LONG sx = 0, sy = 0;
	if ( nX < 0 ) { sx = -nX; nW += nX; nX = 0; }
	if ( nY < 0 ) { sy = -nY; nH += nY; nY = 0; }
	if ( nX + nW > (LONG)dst->m_dwWidth  ) nW = dst->m_dwWidth  - nX;
	if ( nY + nH > (LONG)dst->m_dwHeight ) nH = dst->m_dwHeight - nY;
	if ( nW <= 0 || nH <= 0 ) return S_OK;

	WORD* pDstBase = dst->m_pBits + (size_t)nY * (dst->m_lPitch >> 1) + nX;
	LONG  dstPitch = (LONG)(dst->m_lPitch >> 1);

	// ---- 纯色 billboard(NULL 纹理): 用材质颜色半透明填充 ----
	if ( !lpddsTextr )
	{
		// [DX9][T92] blend type 6 = D3DBLEND_ZERO/D3DBLEND_SRCCOLOR: 输出 = dst * srcColor
		// 用于 RenderFadeState 全屏黑幕淡入淡出(材质灰度 fRate/255)。之前缺失该分支,
		// 半透明白色叠加导致进游戏白屏闪烁。纹理分支同样需按 blend 6 处理(见下)。
		if ( bBlendType == 6 )
		{
			DWORD mr = matR, mg = matG, mb = matB;
			for ( LONG y = 0; y < nH; y++ )
			{
				WORD* pOut = pDstBase + (size_t)y * dstPitch;
				for ( LONG x = 0; x < nW; x++ )
				{
					WORD  d = pOut[x];
					BYTE  dr = (BYTE)((d >> 11) & 0x1F), dg = (BYTE)((d >> 5) & 0x3F), db = (BYTE)(d & 0x1F);
					BYTE  nr = (BYTE)(((DWORD)dr * mr) / 255);
					BYTE  ng = (BYTE)(((DWORD)dg * mg) / 255);
					BYTE  nb = (BYTE)(((DWORD)db * mb) / 255);
					pOut[x] = (WORD)((nr << 11) | (ng << 5) | nb);
				}
			}
			return S_OK;
		}

		for ( LONG y = 0; y < nH; y++ )
		{
			WORD* pOut = pDstBase + (size_t)y * dstPitch;
			for ( LONG x = 0; x < nW; x++ )
			{
				WORD  d = pOut[x];
				if ( bOpa >= 255 )
				{
					pOut[x] = wMatColor;
				}
				else
				{
					BYTE  dr = (BYTE)((d >> 11) & 0x1F), dg = (BYTE)((d >> 5) & 0x3F), db = (BYTE)(d & 0x1F);
					BYTE  sr = (BYTE)((wMatColor >> 11) & 0x1F), sg = (BYTE)((wMatColor >> 5) & 0x3F), sb = (BYTE)(wMatColor & 0x1F);
					DWORD ar = ((DWORD)bOpa * 100) / 255;
					pOut[x] = (WORD)(((((DWORD)dr * (100 - ar) + (DWORD)sr * ar) / 100) << 11) |
					                 ((((DWORD)dg * (100 - ar) + (DWORD)sg * ar) / 100) << 5) |
					                 ((((DWORD)db * (100 - ar) + (DWORD)sb * ar) / 100)));
				}
			}
		}
		return S_OK;
	}

	D3DSURFACE_DESC sd;
	if ( FAILED(lpddsTextr->GetLevelDesc(0, &sd)) ) return E_FAIL;

	D3DLOCKED_RECT lr;
	if ( FAILED(lpddsTextr->LockRect(0, &lr, NULL, D3DLOCK_READONLY)) ) return E_FAIL;

	DWORD* pTex = (DWORD*)lr.pBits;
	LONG   texPitch = (LONG)(lr.Pitch / sizeof(DWORD));
	LONG   texW = (LONG)sd.Width, texH = (LONG)sd.Height;

	// 纹理裁剪(注意 sx 起点偏移, 防止读到 tex[sx..sx+nW-1] 越界)
	if ( nW > texW - sx ) nW = texW - sx;
	if ( nH > texH - sy ) nH = texH - sy;
	if ( nW <= 0 || nH <= 0 ) { lpddsTextr->UnlockRect(0); return S_OK; }

	for ( LONG y = 0; y < nH; y++ )
	{
		DWORD* pRow = pTex + (size_t)(sy + y) * texPitch + sx;
		WORD*  pOut = pDstBase + (size_t)y * dstPitch;

		for ( LONG x = 0; x < nW; x++ )
		{
			DWORD c = pRow[x];
			BYTE  a = (BYTE)(c >> 24);
			if ( a == 0 ) continue;                     // 透明(含 wil 黑色作透明)

			WORD rgb565 = (WORD)((((c >> 16) & 0xF8) << 8) | (((c >> 8) & 0xFC) << 3) | ((c >> 3) & 0x1F));

			pOut[x] = BlendPixel565(pOut[x], rgb565, a, bOpa, bBlendType, matR, matG, matB);
		}
	}

	lpddsTextr->UnlockRect(0);
	return S_OK;
}

// =================================================================================================================
//  [FIX-R2] Software rasterisers for rotated / non axis aligned billboards
//
//  Every frame ends with Present() blitting the 565 soft backbuffer over the whole
//  swapchain as an opaque texture, so anything the fixed function pipeline drew is
//  discarded. CScatter / CElec build rotated geometry that DrawBillBoard() cannot
//  express, hence a small affine textured triangle rasteriser + a rotated quad
//  wrapper on top of it.
// =================================================================================================================
HRESULT CImageHandler::DrawTexTriangleSoft(IDirect3DTexture9* lpddsTextr,
                                           FLOAT x0, FLOAT y0, FLOAT u0, FLOAT v0,
                                           FLOAT x1, FLOAT y1, FLOAT u1, FLOAT v1,
                                           FLOAT x2, FLOAT y2, FLOAT u2, FLOAT v2,
                                           BYTE bBlendType, BYTE matR, BYTE matG, BYTE matB)
{
	if ( !lpddsTextr ) return E_FAIL;

	CDX9SoftSurface* dst = g_xMainWnd.GetBackBuffer();
	if ( !dst || !dst->m_pBits ) return E_FAIL;

	// signed area, used both for the inside test and for the barycentric weights
	FLOAT fArea = ( y1 - y2 ) * ( x0 - x2 ) + ( x2 - x1 ) * ( y0 - y2 );
	if ( fArea > -0.0001f && fArea < 0.0001f ) return S_OK;   // degenerate
	FLOAT fInvArea = 1.0f / fArea;

	FLOAT fMinX = x0, fMaxX = x0, fMinY = y0, fMaxY = y0;
	if ( x1 < fMinX ) fMinX = x1;
	if ( x1 > fMaxX ) fMaxX = x1;
	if ( x2 < fMinX ) fMinX = x2;
	if ( x2 > fMaxX ) fMaxX = x2;
	if ( y1 < fMinY ) fMinY = y1;
	if ( y1 > fMaxY ) fMaxY = y1;
	if ( y2 < fMinY ) fMinY = y2;
	if ( y2 > fMaxY ) fMaxY = y2;

	LONG nMinX = (LONG)fMinX, nMinY = (LONG)fMinY;
	LONG nMaxX = (LONG)fMaxX, nMaxY = (LONG)fMaxY;
	if ( (FLOAT)nMinX > fMinX ) nMinX--;
	if ( (FLOAT)nMinY > fMinY ) nMinY--;
	nMaxX++;   // inclusive bound, +1 keeps the AA-free edge pixels
	nMaxY++;

	if ( nMinX < 0 ) nMinX = 0;
	if ( nMinY < 0 ) nMinY = 0;
	if ( nMaxX > (LONG)dst->m_dwWidth  - 1 ) nMaxX = (LONG)dst->m_dwWidth  - 1;
	if ( nMaxY > (LONG)dst->m_dwHeight - 1 ) nMaxY = (LONG)dst->m_dwHeight - 1;
	if ( nMinX > nMaxX || nMinY > nMaxY ) return S_OK;

	D3DSURFACE_DESC sd;
	if ( FAILED(lpddsTextr->GetLevelDesc(0, &sd)) ) return E_FAIL;

	D3DLOCKED_RECT lr;
	if ( FAILED(lpddsTextr->LockRect(0, &lr, NULL, D3DLOCK_READONLY)) ) return E_FAIL;

	DWORD* pTex     = (DWORD*)lr.pBits;
	LONG   texPitch = (LONG)(lr.Pitch / sizeof(DWORD));
	LONG   texW     = (LONG)sd.Width;
	LONG   texH     = (LONG)sd.Height;
	FLOAT  fTexW    = (FLOAT)texW;
	FLOAT  fTexH    = (FLOAT)texH;

	WORD*  pDst     = dst->m_pBits + (size_t)nMinY * (dst->m_lPitch >> 1) + nMinX;
	LONG   dstPitch = (LONG)(dst->m_lPitch >> 1);

	for ( LONG y = nMinY; y <= nMaxY; y++ )
	{
		FLOAT fy = (FLOAT)y + 0.5f;
		WORD* pOut = pDst + (size_t)( y - nMinY ) * dstPitch;

		for ( LONG x = nMinX; x <= nMaxX; x++ )
		{
			FLOAT fx = (FLOAT)x + 0.5f;

			FLOAT w0 = ( ( y1 - y2 ) * ( fx - x2 ) + ( x2 - x1 ) * ( fy - y2 ) ) * fInvArea;
			if ( w0 < 0.0f || w0 > 1.0f ) continue;
			FLOAT w1 = ( ( y2 - y0 ) * ( fx - x2 ) + ( x0 - x2 ) * ( fy - y2 ) ) * fInvArea;
			if ( w1 < 0.0f ) continue;
			FLOAT w2 = 1.0f - w0 - w1;
			if ( w2 < 0.0f ) continue;

			FLOAT fu = w0 * u0 + w1 * u1 + w2 * u2;
			FLOAT fv = w0 * v0 + w1 * v1 + w2 * v2;

			LONG tx = (LONG)( fu * fTexW );
			LONG ty = (LONG)( fv * fTexH );
			if ( tx < 0 ) tx = 0;  else if ( tx >= texW ) tx = texW - 1;
			if ( ty < 0 ) ty = 0;  else if ( ty >= texH ) ty = texH - 1;

			DWORD c = pTex[ (size_t)ty * texPitch + tx ];
			BYTE  a = (BYTE)( c >> 24 );
			if ( a == 0 ) continue;                 // transparent (wil black == transparent)

			WORD rgb565 = (WORD)( ( ( ( c >> 16 ) & 0xF8 ) << 8 ) | ( ( ( c >> 8 ) & 0xFC ) << 3 ) | ( ( c >> 3 ) & 0x1F ) );

			pOut[ x - nMinX ] = BlendPixel565( pOut[ x - nMinX ], rgb565, a, 255, bBlendType, matR, matG, matB );
		}
	}

	lpddsTextr->UnlockRect(0);
	return S_OK;
}


// Rotated textured quad, fed exactly like the old world-space matrix path:
//   world = local * Scale(w,h) * RotZ(fRotZ) * Translate, screen = (wx+400, 300-wy)
// which collapses to
//   sx = cx + lx*cos - ly*sin
//   sy = cy - (lx*sin + ly*cos)
// UV follows CParticleSystem::m_avShape, i.e. tu runs along +Y and tv along -X.
HRESULT CImageHandler::DrawTexQuadSoft(IDirect3DTexture9* lpddsTextr, FLOAT fCenterX, FLOAT fCenterY,
                                       FLOAT fWidth, FLOAT fHeight, FLOAT fRotZ,
                                       BYTE bBlendType, BYTE matR, BYTE matG, BYTE matB)
{
	if ( !lpddsTextr || fWidth <= 0.0f || fHeight <= 0.0f ) return E_FAIL;

	FLOAT fCos = (FLOAT)cos( fRotZ );
	FLOAT fSin = (FLOAT)sin( fRotZ );
	FLOAT fHx  = fWidth  * 0.5f;
	FLOAT fHy  = fHeight * 0.5f;

	FLOAT lx[4] = { -fHx,  fHx,  fHx, -fHx };
	FLOAT ly[4] = { -fHy, -fHy,  fHy,  fHy };
	FLOAT sx[4], sy[4], tu[4], tv[4];

	for ( INT i = 0; i < 4; i++ )
	{
		sx[i] = fCenterX + lx[i] * fCos - ly[i] * fSin;
		sy[i] = fCenterY - ( lx[i] * fSin + ly[i] * fCos );
		tu[i] = ( ly[i] + fHy ) / fHeight;      // 0 at local bottom, 1 at local top
		tv[i] = ( fHx - lx[i] ) / fWidth;       // 1 at local left,  0 at local right
	}

	DrawTexTriangleSoft( lpddsTextr, sx[0], sy[0], tu[0], tv[0],
	                                 sx[1], sy[1], tu[1], tv[1],
	                                 sx[2], sy[2], tu[2], tv[2],
	                                 bBlendType, matR, matG, matB );
	DrawTexTriangleSoft( lpddsTextr, sx[0], sy[0], tu[0], tv[0],
	                                 sx[2], sy[2], tu[2], tv[2],
	                                 sx[3], sy[3], tu[3], tv[3],
	                                 bBlendType, matR, matG, matB );
	return S_OK;
}
