#ifndef _UTILWND_H
#define _UTILWND_H


typedef struct tagCLIENTUTILITEMINFO
{
	BOOL			bCheck;
	CItem			xItem;
	INT				nValue;
	INT				nPriority;
}CLIENTUTILITEMINFO, *LPCLIENTUTILITEMINFO;

typedef struct tagCLIENTUTILMAGICINFO
{
	BOOL			bCheck;
	CHAR			szName[64];
	INT				nValue;
	INT				nPriority;
}CLIENTUTILMAGICINFO, *LPCLIENTUTILMAGICINFO;


class CUtilWnd : public CGameWnd
{ 
// Constructor & Destructor
public:
	CUtilWnd();
	~CUtilWnd();
// Attributes
public:
	CGameBtn				mUtil_BtnClose;					// �ݱ�

	CGameBtn				mUtil_BtnDownofHP;			// ü��� �޴� ��ħ ��ư
	CGameBtn				mUtil_BtnDownofMP;			// ����� �޴� ��ħ ��ư
	CGameBtn				mUtil_BtnDownofScroll;		// ������ �޴� ��ħ ��ư

	INT						mUtil_nHPValue;				// ü��� ������
	INT						mUtil_nScrollValue;			// ������ ������
	INT						mUtil_nMPValue;				// ����� ������

	RECT					mUtil_rcHPValue;			// ü��� ������
	RECT					mUtil_rcScrollValue;		// ������ ������
	RECT					mUtil_rcMPValue;			// ����� ������

	CHAR					mUtil_szHPpotion[16];		// ���õ� ü��ȸ���� ����
	CHAR					mUtil_szScroll[16];			// ���õ� ���� ����
	CHAR					mUtil_szMPpotion[16];		// ���õ� ����ȸ���� ����

	CGameBtn				mUtil_BtnItemMode;				// �޴� ��ħ ��ư(�����۸�� �ǹ�ư)
	CGameBtn				mUtil_BtnMagicMode;				// �޴� ��ħ ��ư(������� �ǹ�ư)
	CGameBtn				mUtil_BtnEtcMode;				// �޴� ��ħ ��ư(��Ÿ������� �ǹ�ư)

	CGameBtn				mUtil_BtnDownofMagicKind[20];		// �޴� ��ħ ��ư(��������)
	CGameBtn				mUtil_BtnDownofCharmSize[20];		// �޴� ��ħ ��ư(����ũ��)	
	CGameBtn				mUtil_BtnDownofCharmKind[20];		// �޴� ��ħ ��ư(��������)

	INT						mUtil_nSeletedMenu;
	INT						mUtil_nMenuCount;
	

	CGameBtn				mUtil_BtnLeftfCharm;
	CGameBtn				mUtil_BtnRightCharm;

	CGameBtn				mUtil_BtnDownofPoison1;			// �޴� ��ħ ��ư(�����۸�� �ǹ�ư)
	CGameBtn				mUtil_BtnDownofPoison2;			// �޴� ��ħ ��ư(������� �ǹ�ư)

	BOOL					mUtil_bMode;

	INT						mUtil_nSettedItemCount;
	INT						mUtil_nKindofSettedItem;

	INT						mUtil_nFocusedValue;

	CLIENTUTILITEMINFO		mUtil_xUtilItemInfo[10];

	CLIENTUTILITEMINFO		mUtil_xUtilItemInfo2[4];
	CLIENTUTILMAGICINFO		mUtil_xUtilMagicInfo[10];

	RECT					mUtil_rcCheck[10];
	RECT					mUtil_rcItem[10];
	RECT					mUtil_rcValue[10];
	RECT					mUtil_rcPriority[10];
	RECT					mUtil_rcName[10];

	RECT					mUtil_rcMenu[24];

	RECT					mUtil_rcCheck2[4];				// ����� ������ �ڵ� ��ü 
	RECT					mUtil_rcPriority2[4];			// ����� ������ �ڵ� ��ü 

	DWORD					mUtil_dwItemUseTimer;			// ���� �Ա�	
	DWORD					mUtil_dwItemTakeonTimer;		// ����� ������ ��ü
	DWORD					mUtil_dwMagicUseTimer;			// ����� �ڵ� ��
	DWORD					mUtil_dwScrollUseTimer;			// �������	

	INT						mUtil_nPageInCharm;

	CHAR					mUtil_szCharmName[40][16];
	CHAR					mUtil_szCharmKind[40][4];
	CHAR					mUtil_szMagicName[40][16];


	BOOL					mUtil_bShowSysTime;
	RECT					mUtil_rchowSysTime;

	CGameBtn				mUtil_BtnChangeWeather;

	BOOL					mUtil_bShowMagicTime;
	RECT					mUtil_rcMagicTime;

	BOOL					mUtil_bShowHumanHP;
	RECT					mUtil_rcShowHumanHP;

	CGameBtn				mUtil_BtnWeather;			// 天气 NEXT 按钮(复用图18/19样式)
	INT						mUtil_nWeatherState;		// 天气状态 0=无 1=雪 2=雨 3=雾 (NEXT轮巡)
	RECT					mUtil_rcWeather;			// 天气按钮点击区域

	BOOL					mUtil_bGrayPotion;

	INT						TEST;
	

// Operations
public:
	virtual VOID			Init();
	virtual VOID			Destroy();

	VOID					CreateUtilWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove, BYTE bBlend);
	VOID					ShowUtilWnd();

	VOID					ShowItemSetMode();
	VOID					ShowMagicSetMode();
	VOID					ShowEtcSetMode();

	BOOL					OnLButtonDown(POINT ptMouse);
	BOOL					OnLButtonUp(POINT ptMouse);
	BOOL					OnMouseMove(POINT ptMouse);

	BOOL					OnKeyDown(WPARAM wParam, LPARAM lParam);
	BOOL					OnSysKeyDown(WPARAM wParam, LPARAM lParam);

	BOOL					LBtnDownOnItemSetMode(POINT ptMouse);
	BOOL					LBtnDownOnMagicSetMode(POINT ptMouse);
	BOOL					LBtnDownOnEtcSetMode(POINT ptMouse);
	BOOL					LBtnUpOnItemSetMode(POINT ptMouse);
	BOOL					LBtnUpOnMagicSetMode(POINT ptMouse);
	BOOL					LBtnUpOnEtcSetMode(POINT ptMouse);
	BOOL					MouseMoveOnItemSetMode(POINT ptMouse);
	BOOL					MouseMoveOnMagicSetMode(POINT ptMouse);
	BOOL					MouseMoveOnEtcSetMode(POINT ptMouse);

	
	VOID					SetMenuItemToHPItem();
	VOID					SetMenuItemToMPItem();
	VOID					SetMenuItemToHPMPItem();
	VOID					SetMenuItemToScrollItem();
	VOID					SetMenuItemToCharmItem();
	VOID					SetMenuItemToPoisonItem();

	VOID					ShowMenuItemState();

	VOID					SetMenuRect();

	VOID					ProcessUtilFunc();

	BOOL					SaveUtilSet();
	BOOL					LoadUtilSet();

	VOID					SetKey(BYTE bKey);
	VOID					CheckKey(BYTE bKey);
	BOOL					UseKey(BYTE bKey);
	VOID					UseCharm(INT nMagicID);
	VOID					UseHPPotion();
	VOID					UseMPPotion();
};



#endif // _UTILWND_H