#pragma once
#include <zip.h>

#define MAX_BUFFER_SIZE 1024 * 128

class CFTPConnector
{
private:
	CString m_strFullFilePath;
	CString m_strRecentPath;
	BOOL m_bDiscovered;
	BOOL m_bConnect;
	CString m_strErrorMessage;
	CInternetSession* m_pSession;
	CInternetFile* m_pInternetFile;

	void SearchFile(CFtpFileFind* finder, LPCTSTR strFileName, LPCTSTR strStartDir);
	LPCTSTR GetFullFilePath(LPCTSTR strFileName, LPCTSTR strRemoteDir);
	uLong GetCRC(LPCTSTR strFilePath);


public:
	CFTPConnector();
	~CFTPConnector();

	// 오픈하기 싫은데 스레드 처리때문에 열어둬야하네
	CFtpConnection* m_pFtp;

	// FTP 연결
	BOOL Connect(LPCTSTR strIP, int wPort, LPCTSTR strUserID, LPCTSTR strUserPW, BOOL bPassive);
	BOOL DisConnect();

	// 단일 파일 다운로드
	BOOL DownloadSingleFile(LPCTSTR strHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);
	// 다중 파일 다운로드
	BOOL DownloadMultiFile(std::vector<CString> vecHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);
	// 파일 업로드
	BOOL UploadFile(LPCTSTR strLocalFilePath, LPCTSTR strRemoteDir);
	// 에러 메세지
	CString GetErrMsg();
};