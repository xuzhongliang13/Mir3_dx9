
#define _DEFBLOCKSIZE		16
#define _FEATURESIZE		6
#define _CHARDESCSIZE		22
//#define _FEATURESIZEEX		8
#define _FEATURESIZEEX		14	// �߰��ǻ� ����Ʈ nEft �߰�
//#define _FEATURESIZEEX		19		// ������ nIcon
#define _MESSAGEBODYWLSIZE  22

// [DX9][FIX-BAG] 接收缓冲区大小
//   原 szEncodeData[_MAX_STRING_LEN]=1024, 而合法的 SM_BAGITEMS 负载可达
//   46 件 * sizeof(CLIENTITEMRCD)=81 = 3726 字节, 旧区写坏内存.
//   同时 OnMessageReceive 的长度校验会把超过 1040 字符的包
//   静默丢弃(背包>=10 件即被丢), 导致背包空白.
//   扩到 8192, 与接收缓冲 recvBuf[8192] 一致.
#define _MAX_PACKET_DATA_LEN	8192

typedef struct tag_TDEFAULTMESSAGE
{
	int		nRecog;
	WORD	wIdent;
	WORD	wParam;
	WORD	wTag;
	WORD	wSeries;
} _TDEFAULTMESSAGE, *_LPTDEFAULTMESSAGE;

typedef struct tag_TSHORTMSSEAGE
{
	WORD	wIdent;
	WORD	wMsg;
} _TSHORTMSSEAGE, *_LPTSHORTMSSEAGE;


typedef struct tagPACKETMSG
{
	_TDEFAULTMESSAGE	stDefMsg;
	CHAR				szEncodeData[_MAX_PACKET_DATA_LEN];
}PACKETMSG, *LPPACKETMSG;

typedef struct tag_TMSGHEADER
{
	int		nCode;
	int		nSocket;
	WORD	wUserGateIndex;
	WORD	wIdent;
	WORD	wUserListIndex;
	WORD	wTemp;
	int		nLength;
} _TMSGHEADER, *_LPTMSGHEADER;

/*typedef struct tagTGateToSvrHeader
{
	char	szPrefix;				// Always is '%'
	char	szID;					// Identifier.
	BYTE	btGateIndex;			// Gate server index.
	int		nSocket;				// Socket(owner of msg) in gate server.
	WORD	wDataLength;			// Length of msg that following header.
} TGateToSvrHeader, *LPTGateToSvrHeader; 

#define GTS_HEADER_SIZE		sizeof(TGateToSvrHeader) */

void WINAPI fnMakeDefMessage(_LPTDEFAULTMESSAGE lptdm, WORD wIdent, int nRecog, WORD wParam, WORD wTag, WORD wSeries);
int  WINAPI fnEncode6BitBuf(unsigned char *pszSrc, char *pszDest, int nSrcLen, int nDestLen);
int  WINAPI fnDecode6BitBuf(char *pszSrc, char *pszDest, int nDestLen);
int  WINAPI fnDecode6BitBufOriginal(char *pszSrc, char *pszDest, int nDestLen);
int  WINAPI fnEncodeMessage(_LPTDEFAULTMESSAGE lptdm, char *pszBuf, int nLen);
int  WINAPI fnDecodeMessage(_LPTDEFAULTMESSAGE lptdm, char *pszBuf);

int  WINAPI fnDecodeMessageGameProcess(_LPTDEFAULTMESSAGE lptdm, char *pszBuf);  //�̿ 20051031
int  WINAPI fnDecode6BitBufGameMessage(char *pszSrc, char *pszDest, int nDestLen);	//�̿ 20051031
int  WINAPI fnEncode6BitBufGameMessage(unsigned char *pszSrc, char *pszDest, int nSrcLen, int nDestLen); //�̿ 20051031
int  WINAPI fnEncode6BitBufGameString(unsigned char *pszSrc, char *pszDest, int nSrcLen, int nDestLen); //�̿ 20051031
