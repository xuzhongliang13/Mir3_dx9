/******************************************************************************************************************
                                                                                                                   
	����:																											
																													
	�ۼ���:																											
	�ۼ���:																											
																													
	[����][������] : ���� ����																						
                                                                                                                   
*******************************************************************************************************************/



#ifndef _CHRPROC_H
#define	_CHRPROC_H



typedef struct tagCHARSPRINFO
{
	WORD			wFstFrm;						// ������ ���� ������.
	WORD			wEndFrm;						// ������ ������ ������.
	WORD			wDelay;							// ������ �����ð�.
}CHARSPRINFO, *LPCHARSPRINFO;

typedef struct tagCURRFRMINFO
{
	INT				nPosX;							// ��ǥ.
	INT				nPosY;
	WORD			wCurrMtn;						// ���� ����.
	WORD			wCurrFrm;						// ���� ������.
	WORD			wCurrDelay;						// ������ �����ð�.
	RECT			rcChrRgn;
	BYTE			bBlend;
	LPCHARSPRINFO	pstCurrSprInfo;					// ���� Animation�� ���� ����.
}CURRFRMINFO, *LPCURRFRMINFO;

typedef struct tagCHARINFO
{
	BOOL			bSetted;
	BYTE			bClass;
	BYTE			bGender;
	BYTE			bLevel;
	CHAR			pszChrName[25];
	CURRFRMINFO		stCurrFrmInfo;
	BYTE			bUpgradeLevel; //lee wun gyo;
}CHARINFO, *LPCHARINFO;

typedef struct tagSELCHRSHOWINFO
{
	CHAR			pszName[MAX_PATH];
	CHAR			pszJob[MAX_PATH];
	CHAR			pszLevel[MAX_PATH];
	CHAR			pszHighLevel[MAX_PATH];
}SELCHRSHOWINFO, *LPSELCHRSHOWINFO;


class CChrProcess : public CWHDefProcess
{
protected:

public:
	 CChrProcess();
	~CChrProcess();

private:
	CWHWilImageData		m_xImageInter;		// GameInter.wil
	CWHWilImageData		m_xImageChr;		// interface1c.wil

	DWORD				m_dwWaitTick;

	CGameMsgBox			m_xMsgBox;

	D3DVERTEX			m_avBillBoard[4];
	POINT				m_ptMousePos;
	HBRUSH				m_hBrush;
	CAvi				m_xAvi;

	BYTE				m_bChrProcState;
	CHARSPRINFO			m_stChrSprInfo[_MAX_SPR_KIND];

	CGameBtn			m_xSelectBtn[_MAX_CHR_SELECT_BTN];
	CHARINFO			m_stSelectChrInfo[_MAX_CHAR];

	CGameBtn			m_xCreateBtn[_CHR_CHR_CREATE_BTN];
	CHARINFO			m_stCreateChrInfo[_MAX_CHAR];

	HSNDOBJ				m_stEffctWav[3];
	HSNDOBJ				m_stCreateWav[2][3];
	HSNDOBJ				m_stSelectWav[2][3];

	BOOL				m_bBGMPlay;
	DWORD				m_bBGMPlayTime;			
	
	INT					m_nSelectedChr;
	RECT				m_rcSelectChrInfo;
	SELCHRSHOWINFO		m_stSelShowInfo;

	INT					m_nCreatedChr;
	INT					m_nDividedExplain;
	RECT				m_rcChrExplain;
	CHAR				m_pszChrExplain[15][MAX_PATH];

	BOOL				m_bFrontSetofSelChar;
	BOOL				m_bMoreTwoChar;

private:
	VOID	OnConnectToServer();
	CHAR*	OnMessageReceive(CHAR* pszBuf, INT nLen);
	VOID	OnSocketMessageReceive(CHAR* pszMsg);
//	VOID	GetErrorMsg(INT nMsg, CHAR* pszMsg);

	VOID	DrawCharEffect(INT nIdx, INT nX, INT nY);
	HRESULT DrawBillBoard(LPDIRECT3DDEVICE9 lpDevice, D3DVECTOR* pvTrans, D3DVECTOR* pvScale, /*D3DVECTOR vRot, */D3DMATERIAL9* pmtrl, /*FLOAT fRotRad, */IDirect3DTexture9* lpddsTextr, BYTE bBlendType = _BLEND_NORMAL);

	VOID	RenderSelect(INT nLoopTime);
	VOID	RenderCreate(INT nLoopTime);

	VOID	ShowSelectedChrInfo();
	VOID	SetSelectChrShowInfo();

	VOID	SetChrInfo(LPCHARINFO pstChrInfo, BYTE bGender, BYTE bJob, BYTE bLevel, CHAR* pszName = NULL);
	VOID	SetChrSelSprite();
	VOID	SetCharSprInfo(LPCHARSPRINFO pstChrSpr, WORD wFstFrm, WORD wEndFrm, WORD wDelay);
	VOID	SetMotion(INT nNum, BYTE bMtn);
	VOID	SetCharRect(INT nNum, BYTE bMtn);
	LPCHARSPRINFO GetCharSprInfo(BYTE bGender, BYTE bJob, BYTE bMtn);

	BOOL	SetCharExplain(BYTE bGender, BYTE bJob);
	VOID	ShowCharExplain();
	POINT	GetChrCreatePos(BYTE bGender, BYTE bJob);

	BOOL	CreateNameValidate(CHAR* pszName);
	BOOL	IsAlphabet(CHAR cChar);
	BOOL	IsNumeric(CHAR cChar);
	BOOL	IsHangul(CHAR* pszChar);

	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
	LRESULT	OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT	OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT	OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT	OnMsgBoxReturn(WPARAM wParam, LPARAM lParam);
	LRESULT	OnMove(WPARAM wParam, LPARAM lParam);

	virtual LRESULT DefMainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	VOID	Init();
	VOID	Load();
	VOID	DeleteProc();
	VOID	SetNextProc();
	VOID	RenderScene(INT nLoopTime);
};


#endif // _CHRPROC_H