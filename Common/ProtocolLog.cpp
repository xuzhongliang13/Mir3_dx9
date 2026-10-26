#include "stdafx.h"
#include "ProtocolLog.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

#ifdef PROTOCOL_LOG_ENABLED

CProtocolLog& CProtocolLog::GetInstance()
{
    static CProtocolLog instance;
    return instance;
}

CProtocolLog::CProtocolLog()
{
    m_pLogFile = NULL;
    m_szCurrentLogFile[0] = '\0';
    m_dwLastRotateCheck = 0;
    m_bInitialized = false;
    InitializeCriticalSection(&m_csLog);
}

CProtocolLog::~CProtocolLog()
{
    if (m_pLogFile)
    {
        fclose(m_pLogFile);
        m_pLogFile = NULL;
    }
    DeleteCriticalSection(&m_csLog);
}

void CProtocolLog::Initialize()
{
    EnterCriticalSection(&m_csLog);
    
    if (m_bInitialized)
    {
        LeaveCriticalSection(&m_csLog);
        return;
    }
    
    // 创建LOG目录
    _mkdir(PROTOCOL_LOG_DIR);
    
    // 打开当前日志文件
    GetCurrentLogFilePath(m_szCurrentLogFile, sizeof(m_szCurrentLogFile));
    m_pLogFile = fopen(m_szCurrentLogFile, "ab");
    
    m_bInitialized = true;
    m_dwLastRotateCheck = GetTickCount();
    
    if (m_pLogFile)
    {
        char szTimestamp[64];
        GetTimestamp(szTimestamp, sizeof(szTimestamp));
        fprintf(m_pLogFile, "\n=== Protocol Log Started at %s ===\n", szTimestamp);
        fflush(m_pLogFile);
    }
    
    LeaveCriticalSection(&m_csLog);
}

void CProtocolLog::GetCurrentLogFilePath(char* pszPath, int nPathSize)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    _snprintf(pszPath, nPathSize, "%s/protocol_%04d%02d%02d_%02d%02d%02d.log",
              PROTOCOL_LOG_DIR, st.wYear, st.wMonth, st.wDay,
              st.wHour, st.wMinute, st.wSecond);
}

void CProtocolLog::GetTimestamp(char* pszBuffer, int nBufferSize)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    _snprintf(pszBuffer, nBufferSize, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
              st.wYear, st.wMonth, st.wDay,
              st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

void CProtocolLog::DataToHex(const char* pszData, int nDataLen, char* pszHexBuffer, int nHexBufferSize)
{
    if (!pszData || nDataLen <= 0)
    {
        pszHexBuffer[0] = '\0';
        return;
    }
    
    int nMaxLen = (nHexBufferSize - 1) / 3;  // Each byte needs "XX " (3 chars)
    if (nDataLen > nMaxLen)
        nDataLen = nMaxLen;
    
    char* p = pszHexBuffer;
    for (int i = 0; i < nDataLen; i++)
    {
        _snprintf(p, 4, "%02X ", (unsigned char)pszData[i]);
        p += 3;
    }
    
    // 移除最后一个空格
    if (p > pszHexBuffer && *(p-1) == ' ')
        *(p-1) = '\0';
    else
        *p = '\0';
}

void CProtocolLog::RotateLogFile()
{
    DWORD dwCurrentTime = GetTickCount();
    
    // 每5分钟检查一次是否需要轮转
    if (dwCurrentTime - m_dwLastRotateCheck < 5 * 60 * 1000)
        return;
    
    m_dwLastRotateCheck = dwCurrentTime;
    
    // 检查当前文件大小
    struct _stat st;
    if (_stat(m_szCurrentLogFile, &st) == 0)
    {
        if (st.st_size >= PROTOCOL_LOG_MAX_SIZE)
        {
            // 关闭当前文件
            if (m_pLogFile)
            {
                fclose(m_pLogFile);
                m_pLogFile = NULL;
            }
            
            // 生成新的日志文件名
            GetCurrentLogFilePath(m_szCurrentLogFile, sizeof(m_szCurrentLogFile));
            m_pLogFile = fopen(m_szCurrentLogFile, "ab");
            
            if (m_pLogFile)
            {
                char szTimestamp[64];
                GetTimestamp(szTimestamp, sizeof(szTimestamp));
                fprintf(m_pLogFile, "\n=== Protocol Log Rotated at %s ===\n", szTimestamp);
                fflush(m_pLogFile);
            }
        }
    }
}

void CProtocolLog::WriteLog(const char* pszLogLine)
{
    if (!m_pLogFile)
        return;
    
    fprintf(m_pLogFile, "%s\n", pszLogLine);
    fflush(m_pLogFile);
}

void CProtocolLog::Flush()
{
    EnterCriticalSection(&m_csLog);
    if (m_pLogFile)
        fflush(m_pLogFile);
    LeaveCriticalSection(&m_csLog);
}

void CProtocolLog::LogSendPacket(const char* pszFunction, WORD wIdent, int nRecog, WORD wParam, WORD wTag, WORD wSeries,
                                  const char* pszData, int nDataLen, const char* pszRawPacket, int nRawPacketLen)
{
    if (!m_bInitialized)
        Initialize();
    
    EnterCriticalSection(&m_csLog);
    
    RotateLogFile();
    
    char szTimestamp[64];
    GetTimestamp(szTimestamp, sizeof(szTimestamp));
    
    char szHexData[8192];
    DataToHex(pszRawPacket, nRawPacketLen, szHexData, sizeof(szHexData));
    
    char szLogLine[16384];
    _snprintf(szLogLine, sizeof(szLogLine),
              "[%s] SEND  Func=%s Opcode=0x%04X(%d) Recog=%d Param=%d Tag=%d Series=%d DataLen=%d RawLen=%d\n"
              "  HEX: %s\n"
              "  DATA: %.200s",
              szTimestamp, pszFunction ? pszFunction : "Unknown",
              wIdent, wIdent, nRecog, wParam, wTag, wSeries,
              nDataLen, nRawPacketLen,
              szHexData,
              pszData ? pszData : "(null)");
    
    WriteLog(szLogLine);
    
    LeaveCriticalSection(&m_csLog);
}

void CProtocolLog::LogRecvPacket(const char* pszFunction, WORD wIdent, int nRecog, WORD wParam, WORD wTag, WORD wSeries,
                                  const char* pszData, int nDataLen, const char* pszRawPacket, int nRawPacketLen)
{
    if (!m_bInitialized)
        Initialize();
    
    EnterCriticalSection(&m_csLog);
    
    RotateLogFile();
    
    char szTimestamp[64];
    GetTimestamp(szTimestamp, sizeof(szTimestamp));
    
    char szHexData[8192];
    DataToHex(pszRawPacket, nRawPacketLen, szHexData, sizeof(szHexData));
    
    char szLogLine[16384];
    _snprintf(szLogLine, sizeof(szLogLine),
              "[%s] RECV  Func=%s Opcode=0x%04X(%d) Recog=%d Param=%d Tag=%d Series=%d DataLen=%d RawLen=%d\n"
              "  HEX: %s\n"
              "  DATA: %.200s",
              szTimestamp, pszFunction ? pszFunction : "Unknown",
              wIdent, wIdent, nRecog, wParam, wTag, wSeries,
              nDataLen, nRawPacketLen,
              szHexData,
              pszData ? pszData : "(null)");
    
    WriteLog(szLogLine);
    
    LeaveCriticalSection(&m_csLog);
}

void CProtocolLog::LogProtocolError(const char* pszFunction, const char* pszError, WORD wIdent,
                                     const char* pszData, int nDataLen)
{
    if (!m_bInitialized)
        Initialize();
    
    EnterCriticalSection(&m_csLog);
    
    RotateLogFile();
    
    char szTimestamp[64];
    GetTimestamp(szTimestamp, sizeof(szTimestamp));
    
    char szLogLine[4096];
    _snprintf(szLogLine, sizeof(szLogLine),
              "[%s] ERROR Func=%s Opcode=0x%04X(%d) Error=%s DataLen=%d\n"
              "  DATA: %.500s",
              szTimestamp, pszFunction ? pszFunction : "Unknown",
              wIdent, wIdent, pszError ? pszError : "Unknown",
              nDataLen,
              pszData ? pszData : "(null)");
    
    WriteLog(szLogLine);
    
    LeaveCriticalSection(&m_csLog);
}

void CProtocolLog::LogInfo(const char* pszFormat, ...)
{
    if (!m_bInitialized)
        Initialize();
    
    EnterCriticalSection(&m_csLog);
    
    RotateLogFile();
    
    char szTimestamp[64];
    GetTimestamp(szTimestamp, sizeof(szTimestamp));
    
    char szMessage[4096];
    va_list args;
    va_start(args, pszFormat);
    _vsnprintf(szMessage, sizeof(szMessage), pszFormat, args);
    va_end(args);
    
    char szLogLine[4096];
    _snprintf(szLogLine, sizeof(szLogLine), "[%s] INFO  %s", szTimestamp, szMessage);
    
    WriteLog(szLogLine);
    
    LeaveCriticalSection(&m_csLog);
}

#endif // PROTOCOL_LOG_ENABLED
