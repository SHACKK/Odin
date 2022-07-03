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

	// [FTP ����]
	//	- _in 		strIP : FTP ������ IP
	//	- _in 		strUserID : ������ ID
	//	- _in 		strUserPW : ������ PW
	//	- _in 		bPassive : Passvie(0) ���� Active(1) ��� �÷���
	BOOL Connect(LPCTSTR strIP, int wPort, LPCTSTR strUserID, LPCTSTR strUserPW, BOOL bPassive = 0);

	// [FTP ���� ����]
	BOOL DisConnect();

	// [���� ���� �ٿ�ε�]
	//	_in 		strHash : ���� ������ �ؽð�
	//	_in 		strLocalPath : .zip������ ����� ���� ��� / .zip������ �̸����� ������־�� �Ѵ�.
	//	_in_opt		strPassword : ��й�ȣ ����
	//	_in_opt		ProgressBar : Progress Control�� ���� ������
	BOOL DownloadSingleFile(LPCTSTR strHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);

	// [�ټ� ���� �ٿ�ε�]
	//	_in 		vecHash : �ؽð��� ����
	//	_in 		strLocalPath : .zip������ ����� ���� ��� / .zip������ �̸����� ������־�� �Ѵ�.
	//	_in_opt		strPassword : ��й�ȣ ����
	//	_in_opt		ProgressBar : Progress Control�� ���� ������
	BOOL DownloadMultiFile(std::vector<CString> vecHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);

	// [���� ���ε�]
	// _in			strLocalFilePath : ���ε��� ������ ���� ��� 
	// _in			strRemoteDir : ������ ������ ��ġ(���, ���ϸ� ����)
	BOOL UploadFile(LPCTSTR strLocalFilePath, LPCTSTR strRemoteDir);

	// [���� �̸� ����]
	// _in			strPreName : ������ ������ ���
	// _in			strNewName : ������ ������ �̸�
	BOOL RenameFile(LPCTSTR strPreName, LPCTSTR strNewName);

	// [���� ����]
	// _in			strRemotePath : ������ ������ ��ü ���
	BOOL RemoveFile(LPCTSTR strRemotePath);

	// [���� ����]
	// _in			strRemotePath : ������ ���丮�� ��ü ���
	BOOL RemoveDir(LPCTSTR strRemotePath);

	// [���� �޽��� ��ȯ]
	//	- _example
	//	CString strErrorMessage = this.GetErrMsg();
	//	_tprintf(TEXT("%s\n"), strErrorMessage);
	CString GetErrMsg();
};