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
};

// [���� ���� �ٿ�ε�]
//	_in			stFtpInfo		: FTP ���� ��������
//	_in 		strHash			: ���� ������ �ؽð�
//	_in 		strLocalPath	: .zip������ ����� ���� ��� / .zip������ �̸����� �������־�� �Ѵ�.
//	_in_opt		strPassword		: ��й�ȣ ����
//	_in_opt		ProgressBar		: Progress Control�� ���� ������
BOOL DownloadSingleFile(ST_FTP_INFO stFtpInfo, LPCTSTR strHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);

// [�ټ� ���� �ٿ�ε�]
//	_in			stFtpInfo		: FTP ���� ��������
//	_in 		vecHash			: �ؽð��� ����
//	_in 		strLocalPath	: .zip������ ����� ���� ��� / .zip������ �̸����� �������־�� �Ѵ�.
//	_in_opt		strPassword		: ��й�ȣ ����
//	_in_opt		ProgressBar		: Progress Control�� ���� ������
BOOL DownloadMultiFile(ST_FTP_INFO stFtpInfo, std::vector<CString> vecHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);