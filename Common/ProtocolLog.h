#ifndef _PROTOCOL_LOG_H
#define _PROTOCOL_LOG_H

#include <windows.h>
#include <stdio.h>
#include <time.h>

// 编译宏控制LOG开�?
#if !defined(MIR3_RELEASE_BUILD) && !defined(PROTOCOL_LOG_DISABLED)
#define PROTOCOL_LOG_ENABLED
#endif

#ifdef PROTOCOL_LOG_ENABLED

// LOG目录
#define PROTOCOL_LOG_DIR "logs"
#define PROTOCOL_LOG_MAX_SIZE (10 * 1024 * 1024)  // 10MB per log file
#define PROTOCOL_LOG_MAX_FILES 10

class CProtocolLog
{
public:
    static CProtocolLog& GetInstance();
    
    // 记录发送的封包
    void LogSendPacket(const char* pszFunction, WORD wIdent, int nRecog, WORD wParam, WORD wTag, WORD wSeries,
                       const char* pszData, int nDataLen, const char* pszRawPacket, int nRawPacketLen);
    
    // 记录接收的封�?
    void LogRecvPacket(const char* pszFunction, WORD wIdent, int nRecog, WORD wParam, WORD wTag, WORD wSeries,
                       const char* pszData, int nDataLen, const char* pszRawPacket, int nRawPacketLen);
    
    // 记录协议异常
    void LogProtocolError(const char* pszFunction, const char* pszError, WORD wIdent, 
                          const char* pszData, int nDataLen);
    
    // 记录一般信�?
    void LogInfo(const char* pszFormat, ...);
    
    // 初始化LOG目录
    void Initialize();
    
    // 刷新日志到磁�?
    void Flush();

private:
    CProtocolLog();
    ~CProtocolLog();
    CProtocolLog(const CProtocolLog&);
    CProtocolLog& operator=(const CProtocolLog&);
    
    // 获取当前时间戳（毫秒精度�?
    void GetTimestamp(char* pszBuffer, int nBufferSize);
    
    // 将数据转换为Hex字符�?
    void DataToHex(const char* pszData, int nDataLen, char* pszHexBuffer, int nHexBufferSize);
    
    // 写入日志
    void WriteLog(const char* pszLogLine);
    
    // 检查并轮转日志文件
    void RotateLogFile();
    
    // 获取当前日志文件路径
    void GetCurrentLogFilePath(char* pszPath, int nPathSize);
    
    FILE* m_pLogFile;
    char m_szCurrentLogFile[MAX_PATH];
    DWORD m_dwLastRotateCheck;
    CRITICAL_SECTION m_csLog;
    bool m_bInitialized;
};

// 便捷�?
#define PROTOCOL_LOG_SEND(func, wIdent, nRecog, wParam, wTag, wSeries, pData, nDataLen, pRaw, nRawLen) \
    CProtocolLog::GetInstance().LogSendPacket(func, wIdent, nRecog, wParam, wTag, wSeries, pData, nDataLen, pRaw, nRawLen)

#define PROTOCOL_LOG_RECV(func, wIdent, nRecog, wParam, wTag, wSeries, pData, nDataLen, pRaw, nRawLen) \
    CProtocolLog::GetInstance().LogRecvPacket(func, wIdent, nRecog, wParam, wTag, wSeries, pData, nDataLen, pRaw, nRawLen)

#define PROTOCOL_LOG_ERROR(func, error, wIdent, pData, nDataLen) \
    CProtocolLog::GetInstance().LogProtocolError(func, error, wIdent, pData, nDataLen)

#define PROTOCOL_LOG_INFO CProtocolLog::GetInstance().LogInfo

#else

// 如果未启用LOG，宏展开为空
#define PROTOCOL_LOG_SEND(func, wIdent, nRecog, wParam, wTag, wSeries, pData, nDataLen, pRaw, nRawLen)
#define PROTOCOL_LOG_RECV(func, wIdent, nRecog, wParam, wTag, wSeries, pData, nDataLen, pRaw, nRawLen)
#define PROTOCOL_LOG_ERROR(func, error, wIdent, pData, nDataLen)
#define PROTOCOL_LOG_INFO __noop

#endif // PROTOCOL_LOG_ENABLED

#endif // _PROTOCOL_LOG_H
