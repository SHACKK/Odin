#include <vector>
#include <string>
#include "pch.h"
#include "Odin.h"

HANDLE CreateFtpHandle(ST_FTP_INFO connectInfo)
{
	return HANDLE();
}

void DestroyFtpHandle(HANDLE hFtp)
{
	std::vector<std::string> vecStr;
	vecStr.
}

BOOL DownloadFile(HANDLE hFtp, LPCSTR pszHash, std::vector<unsigned char>& outBin)
{
	return 0;
}

BOOL DownloadSingleFile(ST_FTP_INFO stFtpInfo, LPCTSTR pszHash, LPCTSTR strLocalPath, LPCTSTR strPassword, FP_DOWNLOAD_PROGRESS fpCallback)
{
	try
	{
		CFtp Ftp;
		CZipper Zipper;
		
		if(!Ftp.Connect(stFtpInfo.strIP, stFtpInfo.wPort, stFtpInfo.strUserID, stFtpInfo.strUserPW, stFtpInfo.bPassive)) throw CString(TEXT("���� ���� ����"));
		if(!Zipper.CreateZipFile(strLocalPath)) throw CString(TEXT("ZIP ���� ���� ����"));

		CString strFilePath;
		Ftp.GetFilePath(pszHash, &strFilePath);

		if (strFilePath.IsEmpty())	throw CString(TEXT("������ ã�� �� �����ϴ�"));

		ULONGLONG ulBufferSize = Ftp.GetFileSize(strFilePath);
		if (ulBufferSize == 0)	throw CString(TEXT("����ũ�⸦ ������ �� �����ϴ�"));

		BYTE* pFileBuffer = new BYTE[ulBufferSize];
		if(!Ftp.DownloadFileInMemory(strFilePath.GetBuffer(0), pFileBuffer, ulBufferSize))
		{
			delete[] pFileBuffer;
			throw CString(TEXT("���� �ٿ�ε� ����"));
		}

		if (!Zipper.Zip(pszHash, pFileBuffer, (unsigned int)ulBufferSize, strPassword), fpCallback)
		{
			delete[] pFileBuffer;
			throw CString(TEXT("���� ���� ����"));
		}

		delete[] pFileBuffer;

		if (!Zipper.CloseZipFile()) throw CString(TEXT("ZIP���� �ݱ� ����"));
		if (!Ftp.DisConnect()) throw CString(TEXT("���� �������� ����"));

		return TRUE;
	}
	catch (CString ErrorMessage)
	{
		_tprintf(TEXT("%s"), ErrorMessage.GetBuffer(0));
		return FALSE;
	}
}

BOOL DownloadMultiFile(ST_FTP_INFO stFtpInfo, std::vector<CString> vecHash, LPCTSTR strLocalPath, LPCTSTR strPassword, FP_DOWNLOAD_PROGRESS fpCallback)
{
	try
	{
		CFtp Ftp;
		CZipper Zipper;
		if (!Ftp.Connect(stFtpInfo.strIP, stFtpInfo.wPort, stFtpInfo.strUserID, stFtpInfo.strUserPW, stFtpInfo.bPassive)) throw CString(TEXT("������¸� Ȯ���ϼ���"));

		if (!Zipper.CreateZipFile(strLocalPath)) throw CString(TEXT("ZIP���� ���� ����"));

		int nFileIndex = 0;
		int nFileCount = vecHash.size();
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

			if (!Zipper.Zip(iter, pFileBuffer, (unsigned int)ulBufferSize, strPassword), fpCallback)
			{
				delete[] pFileBuffer;
				throw CString(TEXT("���� ���� ����"));
			}

			if (fpCallback != nullptr)
			{
				fpCallback(nFileIndex, nFileCount);
				nFileIndex++;
			}

			delete[] pFileBuffer;
		}

		if (!Zipper.CloseZipFile()) throw CString(TEXT("ZIP���� �ݱ� ����"));
		if (!Ftp.DisConnect()) throw CString(TEXT("���� �������� ����"));

		return TRUE;
	}
	catch (CString ErrorMessage)
	{
		_tprintf(TEXT("%s"), ErrorMessage.GetBuffer(0));
		return FALSE;
	}
}

