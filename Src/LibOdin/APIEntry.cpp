#include "pch.h"
#include "APIEntry.h"

BOOL DownloadSingleFile(ST_FTP_INFO stFtpInfo, LPCTSTR strHash, LPCTSTR strLocalPath, LPCTSTR strPassword, CProgressCtrl* ProgressBar)
{
	CFtp Ftp;
	CZipper Zipper;
	ProgressBar->SetRange(0, 100);

	Ftp.Connect(stFtpInfo.strIP, stFtpInfo.wPort, stFtpInfo.strUserID, stFtpInfo.strUserPW, stFtpInfo.bPassive);
	Zipper.CreateZipFile(strLocalPath);
	ProgressBar->SetPos(10);
	CString strFilePath = Ftp.GetFilePath(strHash);
	ProgressBar->SetPos(30);
	BYTE* pFileBuffer = Ftp.DownloadFileInMemory(strFilePath);
	ProgressBar->SetPos(50);
	Zipper.Zip(strHash, pFileBuffer, strPassword);
	delete[] pFileBuffer;
	ProgressBar->SetPos(99);

	Zipper.CloseZipFile();
	Ftp.DisConnect();
	ProgressBar->SetPos(100);

	return TRUE;
}

BOOL DownloadMultiFile(ST_FTP_INFO stFtpInfo, std::vector<CString> vecHash, LPCTSTR strLocalPath, LPCTSTR strPassword, CProgressCtrl* ProgressBar)
{
	CFtp Ftp;
	Ftp.Connect(stFtpInfo.strIP, stFtpInfo.wPort, stFtpInfo.strUserID, stFtpInfo.strUserPW, stFtpInfo.bPassive);
	ProgressBar->SetRange(0, (short)vecHash.size());

	CZipper Zipper;
	Zipper.CreateZipFile(strLocalPath);
	int nCount = 0;
	for (auto iter : vecHash)
	{
		CString strFilePath = Ftp.GetFilePath(iter);
		BYTE* pFileBuffer = Ftp.DownloadFileInMemory(strFilePath);
		Zipper.Zip(iter, pFileBuffer, strPassword);
		delete[] pFileBuffer;
		ProgressBar->SetStep(++nCount);
	}

	Zipper.CloseZipFile();
	Ftp.DisConnect();

	return TRUE;
}