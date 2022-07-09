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


// [���� ���� �ٿ�ε�]
//	_in			stFtpInfo		: FTP ���� ��������
//	_in 		strHash			: ���� ������ �ؽð�
//	_in 		strLocalPath	: .zip������ ����� ���� ��� / .zip������ �̸����� ������־�� �Ѵ�.
//	_in_opt		strPassword		: ��й�ȣ ����
//	_in_opt		ProgressBar		: Progress Control�� ���� ������
extern BOOL DownloadSingleFile(ST_FTP_INFO stFtpInfo, LPCTSTR strHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);

// [�ټ� ���� �ٿ�ε�]
//	_in			stFtpInfo		: FTP ���� ��������
//	_in 		vecHash			: �ؽð��� ����
//	_in 		strLocalPath	: .zip������ ����� ���� ��� / .zip������ �̸����� ������־�� �Ѵ�.
//	_in_opt		strPassword		: ��й�ȣ ����
//	_in_opt		ProgressBar		: Progress Control�� ���� ������
extern BOOL DownloadMultiFile(ST_FTP_INFO stFtpInfo, std::vector<CString> vecHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);
