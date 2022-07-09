#pragma once
#include "Ftp.h"
#include "Zipper.h"

typedef struct ST_FTP_INFO
{
	LPCTSTR strIP;
	WORD wPort;
	LPCTSTR strUserID;
	LPCTSTR strUserPW;
	BOOL bPassive;

	ST_FTP_INFO()
		: strIP(NULL)
		, wPort(0)
		, strUserID(NULL)
		, strUserPW(NULL)
		, bPassive(MODE_PASSIVE)
	{}
}ST_FTP_INFO;

HANDLE CreateFtpHandle(ST_FTP_INFO connectInfo);
void DestroyFtpHandle(HANDLE hFtp);
BOOL DownloadFile(HANDLE hFtp, LPCSTR pszHash, std::vector<unsigned char>& outBin);


// [단일 파일 다운로드]
//	_in			stFtpInfo		: FTP 서버 연결정보
//	_in 		strHash			: 단일 파일의 해시값
//	_in 		strLocalPath	: .zip파일이 저장될 로컬 경로 / .zip파일의 이름까지 명시해주어야 한다.
//	_in_opt		strPassword		: 비밀번호 여부
//	_in_opt		ProgressBar		: Progress Control에 대한 포인터
extern BOOL DownloadSingleFile(ST_FTP_INFO stFtpInfo, LPCTSTR strHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);

// [다수 파일 다운로드]
//	_in			stFtpInfo		: FTP 서버 연결정보
//	_in 		vecHash			: 해시값의 벡터
//	_in 		strLocalPath	: .zip파일이 저장될 로컬 경로 / .zip파일의 이름까지 명시해주어야 한다.
//	_in_opt		strPassword		: 비밀번호 여부
//	_in_opt		ProgressBar		: Progress Control에 대한 포인터
extern BOOL DownloadMultiFile(ST_FTP_INFO stFtpInfo, std::vector<CString> vecHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);
