#pragma once
#include <zip.h>
#include <map>

#define MAX_BUFFER_SIZE 1024 * 128

class CFTPConnector
{
private:
	CString m_strFullFilePath;
	CString m_strRecentPath;
	BOOL m_bDiscovered;
	BOOL m_bConnect;
	CString m_strErrorMessage;
	CFtpConnection* m_pFtp;
	CInternetSession* m_pSession;
	CInternetFile* m_pInternetFile;

	void SearchFile(CFtpFileFind* finder, LPCTSTR strFileName, LPCTSTR strStartDir);
	LPCTSTR GetFullFilePath(LPCTSTR strFileName, LPCTSTR strRemotePath);
	uLong GetCRC(LPCTSTR strFilePath);


public:
	CFTPConnector();
	~CFTPConnector();

	// [FTP 연결]
	//	- _in 		strIP : FTP 서버의 IP
	//	- _in 		strUserID : 유저의 ID
	//	- _in 		strUserPW : 유저의 PW
	//	- _in 		bPassive : Passvie(0) 모드와 Active(1) 모드 플래그
	BOOL Connect(LPCTSTR strIP, int wPort, LPCTSTR strUserID, LPCTSTR strUserPW, BOOL bPassive = 0);

	// [FTP 연결 종료]
	BOOL DisConnect();

	// [단일 파일 다운로드]
	//	_in 		strHash : 단일 파일의 해시값
	//	_in 		strLocalPath : .zip파일이 저장될 로컬 경로 / .zip파일의 이름까지 명시해주어야 한다.
	//	_in_opt		strPassword : 비밀번호 여부
	//	_in_opt		ProgressBar : Progress Control에 대한 포인터
	BOOL DownloadSingleFile(LPCTSTR strHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);

	// [다수 파일 다운로드]
	//	_in 		vecHash : 해시값의 벡터
	//	_in 		strLocalPath : .zip파일이 저장될 로컬 경로 / .zip파일의 이름까지 명시해주어야 한다.
	//	_in_opt		strPassword : 비밀번호 여부
	//	_in_opt		ProgressBar : Progress Control에 대한 포인터
	BOOL DownloadMultiFile(std::vector<CString> vecHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);

	// [파일 업로드]
	// _in			strLocalFilePath : 업로드할 파일의 로컬 경로 
	// _in			strRemoteDir : 서버에 저장할 위치(경로, 파일명 포함)
	BOOL UploadFile(LPCTSTR strLocalFilePath, LPCTSTR strRemoteDir);

	// [파일 이름 수정]
	// _in			strPreName : 변경할 파일의 경로
	// _in			strNewName : 변경할 파일의 이름
	BOOL RenameFile(LPCTSTR strPreName, LPCTSTR strNewName);

	// [파일 삭제]
	// _in			strRemotePath : 삭제할 파일의 전체 경로
	BOOL RemoveFile(LPCTSTR strRemotePath);

	// [폴더 삭제]
	// _in			strRemotePath : 삭제할 디렉토리의 전체 경로
	BOOL RemoveDir(LPCTSTR strRemotePath);

	// [에러 메시지 반환]
	//	- _example
	//	CString strErrorMessage = this.GetErrMsg();
	//	_tprintf(TEXT("%s\n"), strErrorMessage);
	CString GetErrMsg();
};