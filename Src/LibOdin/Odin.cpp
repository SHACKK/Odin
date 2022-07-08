#include "pch.h"
#include "Odin.h"

BOOL DownloadSingleFile(ST_FTP_INFO stFtpInfo, LPCTSTR pszHash, LPCTSTR strLocalPath, LPCTSTR strPassword, CProgressCtrl* ProgressBar)
{
	try
	{
		CFtp Ftp;
		CZipper Zipper;
		if(ProgressBar != NULL)		
			ProgressBar->SetRange(0, 100);
		
		if(!Ftp.Connect(stFtpInfo.strIP, stFtpInfo.wPort, stFtpInfo.strUserID, stFtpInfo.strUserPW, stFtpInfo.bPassive)) throw CString(TEXT("���� ���� ����"));
		if(!Zipper.CreateZipFile(strLocalPath)) throw CString(TEXT("ZIP ���� ���� ����"));
		if (ProgressBar != NULL)	
			ProgressBar->SetPos(10);

		CString strFilePath;
		Ftp.GetFilePath(pszHash, &strFilePath);

		if (strFilePath.IsEmpty())	throw CString(TEXT("������ ã�� �� �����ϴ�"));
		if (ProgressBar != NULL)	
			ProgressBar->SetPos(30);

		ULONGLONG ulBufferSize = Ftp.GetFileSize(strFilePath);
		if (ulBufferSize == 0)	throw CString(TEXT("����ũ�⸦ ������ �� �����ϴ�"));

		BYTE* pFileBuffer = new BYTE[ulBufferSize];
		if(!Ftp.DownloadFileInMemory(strFilePath.GetBuffer(0), pFileBuffer, ulBufferSize))
		{
			delete[] pFileBuffer;
			throw CString(TEXT("���� �ٿ�ε� ����"));
		}
		if (ProgressBar != NULL)	
			ProgressBar->SetPos(50);


		if (!Zipper.Zip(pszHash, pFileBuffer, (unsigned int)ulBufferSize, strPassword))
		{
			delete[] pFileBuffer;
			throw CString(TEXT("���� ���� ����"));
		}

		delete[] pFileBuffer;
		if (ProgressBar != NULL)	
			ProgressBar->SetPos(99);

		if (!Zipper.CloseZipFile()) throw CString(TEXT("ZIP���� �ݱ� ����"));
		if (!Ftp.DisConnect()) throw CString(TEXT("���� �������� ����"));
		if (ProgressBar != NULL)	
			ProgressBar->SetPos(100);

		return TRUE;
	}
	catch (CString ErrorMessage)
	{
		if (ProgressBar != NULL)
		{
			ProgressBar->SetBarColor(RGB(255, 0, 0));
			ProgressBar->SetPos(100);
		}

		_tprintf(TEXT("%s"), ErrorMessage.GetBuffer(0));
		return FALSE;
	}
}

BOOL DownloadMultiFile(ST_FTP_INFO stFtpInfo, std::vector<CString> vecHash, LPCTSTR strLocalPath, LPCTSTR strPassword, CProgressCtrl* ProgressBar)
{
	try
	{
		CFtp Ftp;
		CZipper Zipper;
		if (!Ftp.Connect(stFtpInfo.strIP, stFtpInfo.wPort, stFtpInfo.strUserID, stFtpInfo.strUserPW, stFtpInfo.bPassive)) throw CString(TEXT("������¸� Ȯ���ϼ���"));
		if(ProgressBar != NULL)	
			ProgressBar->SetRange(0, (short)vecHash.size());

		if (!Zipper.CreateZipFile(strLocalPath)) throw CString(TEXT("ZIP���� ���� ����"));
		int nCount = 0;
		for (auto iter : vecHash)
		{

			CString strFilePath;
			Ftp.GetFilePath(iter, &strFilePath);
			if (strFilePath.IsEmpty())	throw CString(TEXT("������ ã�� �� �����ϴ�"));

			ULONGLONG ulBufferSize = Ftp.GetFileSize(strFilePath.GetBuffer(0));
			if (ulBufferSize == 0)	throw CString(TEXT("����ũ�⸦ ������ �� �����ϴ�"));

			BYTE* pFileBuffer = new BYTE[ulBufferSize];
			if (!Ftp.DownloadFileInMemory(strFilePath, pFileBuffer, ulBufferSize))
			{
				delete[] pFileBuffer;
				throw CString(TEXT("���� �ٿ�ε� ����"));
			}


			if (!Zipper.Zip(iter, pFileBuffer, (unsigned int)ulBufferSize, strPassword))
			{
				delete[] pFileBuffer;
				throw CString(TEXT("���� ���� ����"));
			}
			delete[] pFileBuffer;
			if (ProgressBar != NULL)
				ProgressBar->SetStep(++nCount);
		}

		if (!Zipper.CloseZipFile()) throw CString(TEXT("ZIP���� �ݱ� ����"));
		if (!Ftp.DisConnect()) throw CString(TEXT("���� �������� ����"));

		return TRUE;
	}
	catch (CString ErrorMessage)
	{
		if (ProgressBar != NULL)
		{
			ProgressBar->SetBarColor(RGB(255, 0, 0));
			ProgressBar->SetPos(100);
		}

		_tprintf(TEXT("%s"), ErrorMessage.GetBuffer(0));
		return FALSE;
	}
}