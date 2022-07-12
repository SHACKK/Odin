#pragma once

#define MODE_PASSIVE 0
#define MODE_ACTIVE 1
#define MAX_BUFFER_SIZE 1024*128

typedef struct ST_FILE_INFO
{
	CString strFileName;
	CString strFilePath;
	CString strFileTitle;
	CString strFileURL;
	ULONGLONG nFileSize;
}ST_FILE_INFO;

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
	BOOL m_bDiscovered;
	std::vector<CString> m_vecFailed;

public:
	CFtp();
	CFtp(LPCTSTR pszIP, WORD wPort, LPCTSTR pszUserID, LPCTSTR pszUserPW, BOOL bMode = MODE_PASSIVE);
	~CFtp();
	
	// Connect Method
	BOOL Connect(LPCTSTR pszIP, WORD wPort, LPCTSTR pszUserID, LPCTSTR pszUserPW, BOOL bMode = MODE_PASSIVE);
	BOOL DisConnect();

	// strFile : c:\test.txt
	// strExt : txt
	// strDir = c:\
	// strFIleName : test

	//BOOL GrepFiles(LPCTSTR pszDir, std::vector<std::string>& outFIlesInDir, bool bIncludeDir = false);

	std::vector<CString> GetEveryThing(LPCTSTR pszDirectory, BOOL bIncludePath = FALSE);
	void GetFilePath(LPCTSTR pszFileName, CString* strDestBuffer, LPCTSTR pszStartDirectory = TEXT("/"));
	ULONGLONG GetFileSize(LPCTSTR pszFilePath);

	// Download Method
	BOOL DownloadFileInMemory(LPCTSTR pszRemotePath, BYTE* pszBuffer, ULONGLONG ulBufferSize);
	CString GetErrorMessage();
};