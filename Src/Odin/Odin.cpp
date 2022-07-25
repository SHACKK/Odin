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
}

BOOL DownloadFile(HANDLE hFtp, LPCSTR pszHash, std::vector<unsigned char>& outBin)
{
	return 0;
}

CString IpFromDomain(CString strDomain, WORD wPort)
{
	WSADATA wsaData;
	int nRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nRes != -0)
	{
		//ERROR
	}

	struct addrinfo* result = NULL;
	struct addrinfo* ptr = NULL;
	struct addrinfo hints;

	struct sockaddr_in* sockaddr_ipv4;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char szPort[5];
	sprintf_s(szPort, "%d", wPort);
	DWORD dwRes = getaddrinfo(CT2A(strDomain), szPort, &hints, &result);
	if (dwRes != 0)
	{
		//ERROR
	}

	char ipv4buffer[46] = { 0, };

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		if (ptr->ai_family == AF_INET)
		{
			sockaddr_ipv4 = (struct sockaddr_in*)ptr->ai_addr;
			inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ipv4buffer, 46);
		}
	}

	CString strRet = ipv4buffer;
	WSACleanup();
	return ipv4buffer;
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
		int nFileCount = (int)vecHash.size();
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

