// LoginReg.cpp — 登录界面"创建账号"弹窗（标准Win32模态对话框）
// 提交 REGISTER 到 mir3.ini [Initial] ServerAddr 的 8501 端口后端
// 成功返回1并回填 newAcc；取消/失败返回0
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#pragma comment(lib,"ws2_32.lib")

static HWND g_editId, g_editPw, g_editCnf, g_status;
static HBITMAP g_hRegBg = 0;   // 背景: GameInter.wil 帧660 (RegBg.bmp)
static char g_newAcc[32];
static int  g_doneOk = 0;
static volatile int g_dlgDone = 0;
#define IDC_OK 101
#define IDC_CANCEL 102

// 读后端IP: mir3.ini [Initial] ServerAddr
static void get_backend_addr(char* ip, int ipsz, int* port){
    GetPrivateProfileStringA("Initial","ServerAddr","127.0.0.1",ip,ipsz,".\\mir3.ini");
    *port = GetPrivateProfileIntA("Initial","RegPort",8501,".\\mir3.ini");
    // 若IP含:端口(如192.168.1.1:8501)则拆分
    char* p=strchr(ip,':');
    if(p){ *p=0; *port=atoi(p+1); }
}

// 提交注册: 返回0成功, 1=已存在, 2=网络错误, 3=失败
static int reg_account(const char* id, const char* pw, char* resp, int rsz){
    char ip[64]; int port;
    get_backend_addr(ip,sizeof(ip),&port);
    WSADATA wsa; WSAStartup(MAKEWORD(2,2),&wsa);
    SOCKET s=socket(AF_INET,SOCK_STREAM,0);
    if(s==INVALID_SOCKET){ WSACleanup(); return 2; }
    struct sockaddr_in sa; sa.sin_family=AF_INET; sa.sin_port=htons((u_short)port);
    sa.sin_addr.s_addr=inet_addr(ip);
    // 短超时(4s)
    unsigned long tv=4000; setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(tv));
    if(connect(s,(struct sockaddr*)&sa,sizeof(sa))==SOCKET_ERROR){ closesocket(s); WSACleanup(); return 2; }
    char req[256]; wsprintfA(req,"REGISTER|%s|%s\r\n",id,pw);
    send(s,req,(int)strlen(req),0);
    int n=recv(s,resp,rsz-1,0);
    if(n<=0){ closesocket(s); WSACleanup(); return 2; }
    resp[n]=0; closesocket(s); WSACleanup();
    // 去尾部空白
    while(n>0 && (resp[n-1]=='\r'||resp[n-1]=='\n'||resp[n-1]==' ')) resp[--n]=0;
    if(strcmp(resp,"OK")==0) return 0;
    if(strcmp(resp,"EXIST")==0) return 1;
    return 3;
}

static void set_status(HWND h, const wchar_t* s){ SetWindowTextW(g_status, s); }

static LRESULT CALLBACK DlgProc(HWND h, UINT msg, WPARAM w, LPARAM l){
    switch(msg){
    case WM_ERASEBKGND:{
        if(g_hRegBg){
            HDC hdc=(HDC)w, m=CreateCompatibleDC(hdc);
            HGDIOBJ o=SelectObject(m,g_hRegBg);
            BITMAP bm; GetObjectW(g_hRegBg,sizeof(bm),&bm);
            StretchBlt(hdc,0,0,320,235,m,0,0,bm.bmWidth,bm.bmHeight,SRCCOPY);
            SelectObject(m,o); DeleteDC(m);
            return TRUE;
        }
        break;
    }
    case WM_CTLCOLORSTATIC:{
        HDC hdc=(HDC)w;
        SetBkMode(hdc,TRANSPARENT);
        SetTextColor(hdc,RGB(236,196,120));   // 金白字, 透明背景露出龙纹底
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }
    case WM_CTLCOLOREDIT:{
        HDC hdc=(HDC)w;
        SetBkColor(hdc,RGB(30,26,20));
        SetTextColor(hdc,RGB(255,248,220));
        return (LRESULT)GetStockObject(BLACK_BRUSH);
    }
    case WM_CREATE:{
        // 标签
        CreateWindowExW(0,L"STATIC",L"账  号",WS_CHILD|WS_VISIBLE,22,22,70,18,h,0,0,0);
        CreateWindowExW(0,L"STATIC",L"密  码",WS_CHILD|WS_VISIBLE,22,62,70,18,h,0,0,0);
        CreateWindowExW(0,L"STATIC",L"确认密码",WS_CHILD|WS_VISIBLE,22,102,70,18,h,0,0,0);
        // 输入框
        g_editId =CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,100,20,180,22,h,0,0,0);
        g_editPw =CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|ES_PASSWORD,100,60,180,22,h,0,0,0);
        g_editCnf=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|ES_PASSWORD,100,100,180,22,h,0,0,0);
        // 状态
        g_status=CreateWindowExW(0,L"STATIC",L"",WS_CHILD|WS_VISIBLE,22,128,258,18,h,0,0,0);
        // 按钮
        CreateWindowExW(0,L"BUTTON",L"确定",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,70,155,72,26,h,(HMENU)IDC_OK,0,0);
        CreateWindowExW(0,L"BUTTON",L"取消",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,170,155,72,26,h,(HMENU)IDC_CANCEL,0,0);
        SetFocus(g_editId);
        break;
    }
    case WM_COMMAND:{
        if(LOWORD(w)==IDC_OK){
            char id[32],pw[32],cnf[32];
            GetWindowTextA(g_editId,id,sizeof(id));
            GetWindowTextA(g_editPw,pw,sizeof(pw));
            GetWindowTextA(g_editCnf,cnf,sizeof(cnf));
            if(id[0]==0||pw[0]==0){ set_status(h,L"请填写账号和密码"); break; }
            if(strcmp(pw,cnf)!=0){ set_status(h,L"两次密码不一致"); break; }
            char resp[64];
            int r=reg_account(id,pw,resp,sizeof(resp));
            if(r==0){
                strncpy(g_newAcc,id,31); g_newAcc[31]=0; g_doneOk=1;
                set_status(h,L"注册成功，请登录");
                SetTimer(h,1,600,NULL);  // 0.6s后自动关闭
            } else if(r==1){ set_status(h,L"该账号已存在"); }
            else if(r==2){ set_status(h,L"无法连接注册服务器"); }
            else { set_status(h,L"注册失败"); }
        } else if(LOWORD(w)==IDC_CANCEL){ DestroyWindow(h); }
        break;
    }
    case WM_TIMER: DestroyWindow(h); break;
    case WM_CLOSE: DestroyWindow(h); break;
    case WM_DESTROY: g_dlgDone = 1; break;
    }
    return DefWindowProcW(h,msg,w,l);
}

// 弹出创建账号对话框；成功创建返回1，newAcc回填账号；取消/失败返回0
int DoCreateAccountDialog(char* newAcc, int accMax){
    static int wcInited=0;
    WNDCLASSW wc; ZeroMemory(&wc,sizeof(wc));
    wc.style=CS_HREDRAW|CS_VREDRAW; wc.lpfnWndProc=DlgProc; wc.hInstance=GetModuleHandleW(0);
    wc.hCursor=(HCURSOR)LoadCursorW(0,(LPCWSTR)IDC_ARROW); wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszClassName=L"Mir3CreateAccDlg";
    if(!wcInited){
        RegisterClassW(&wc); wcInited=1;
        g_hRegBg=(HBITMAP)LoadImageA(NULL,".\\Data\\RegBg.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    }
    g_doneOk=0; g_newAcc[0]=0;
    HWND h=CreateWindowExW(0,wc.lpszClassName,L"创建账号",
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
        CW_USEDEFAULT,CW_USEDEFAULT,320,235,NULL,0,wc.hInstance,0);
    if(!h) return 0;
    ShowWindow(h,SW_SHOW);
    // 模态消息循环(仅该窗口)
    MSG msg;
    while(!g_dlgDone){
        if(PeekMessageW(&msg,0,0,0,PM_REMOVE)){
            if(msg.message==WM_QUIT) break;
            TranslateMessage(&msg); DispatchMessageW(&msg);
        } else WaitMessage();
    }
    DestroyWindow(h);
    if(g_doneOk && newAcc && g_newAcc[0]){
        strncpy(newAcc,g_newAcc,accMax-1); if(accMax>0) newAcc[accMax-1]=0;
        return 1;
    }
    return 0;
}
