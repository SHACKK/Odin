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

	// �����ϱ� ������ ������ ó�������� ����־��ϳ�
	CFtpConnection* m_pFtp;

	// FTP ����
	BOOL Connect(LPCTSTR strIP, int wPort, LPCTSTR strUserID, LPCTSTR strUserPW, BOOL bPassive);
	BOOL DisConnect();

	// ���� ���� �ٿ�ε�
	BOOL DownloadSingleFile(LPCTSTR strHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);
	// ���� ���� �ٿ�ε�
	BOOL DownloadMultiFile(std::vector<CString> vecHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);
	// ���� ���ε�
	BOOL UploadFile(LPCTSTR strLocalFilePath, LPCTSTR strRemoteDir);
	// ���� �޼���
	CString GetErrMsg();
};