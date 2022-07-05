#pragma once

#define MODE_PASSIVE 0
#define MODE_ACTIVE 1
#define MAX_BUFFER_SIZE 1024*128

class CFtp
{
private:
	CFtpConnection* m_pFtp;
	CInternetSession* m_pSession;
	CInternetFile* m_pInternetFile;
	CFtpFileFind* m_pFinder;
	CString m_strErrorMessage;
	CString m_strDirectoryCache;
	BOOL m_bConnectFlag;

public:
	CFtp();
	CFtp(LPCTSTR strIP, WORD wPort, LPCTSTR strUserID, LPCTSTR strUserPW, BOOL bMode = MODE_PASSIVE);
	~CFtp();

	BOOL Connect(LPCTSTR strIP, WORD wPort, LPCTSTR strUserID, LPCTSTR strUserPW, BOOL bMode = MODE_PASSIVE);
	BOOL DisConnect();
	std::vector<CString> GetEveryThing(LPCTSTR strDirectory, BOOL bIncludePath = FALSE);
	CString GetFilePath(LPCTSTR strFileName, LPCTSTR strStartDirectory = TEXT("/"));
	BYTE* DownloadFileInMemory(LPCTSTR strRemotePath);
	CString GetErrorMessage();
};

