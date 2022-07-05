#include "pch.h"
#include "Ftp.h"

CFtp::CFtp()
{
	m_pFtp = NULL;
	m_pSession = NULL;
	m_pInternetFile = NULL;
	m_pFinder = NULL;
	m_bConnectFlag = FALSE;
}

CFtp::CFtp(LPCTSTR strIP, WORD wPort, LPCTSTR strUserID, LPCTSTR strUserPW, BOOL bMode)
{
	m_pFtp = NULL;
	m_pSession = NULL;
	m_pInternetFile = NULL;
	m_bConnectFlag = FALSE;

	BOOL bRes = this->Connect(strIP, wPort, strUserID, strUserPW, bMode);
}

CFtp::~CFtp()
{
	if (m_bConnectFlag)
		this->DisConnect();
}

BOOL CFtp::Connect(LPCTSTR strIP, WORD wPort, LPCTSTR strUserID, LPCTSTR strUserPW, BOOL bMode)
{
	if (m_bConnectFlag)
		return TRUE;

	try
	{
		m_pSession = new CInternetSession;
		m_pFtp = m_pSession->GetFtpConnection(strIP, strUserID, strUserPW, (INTERNET_PORT)wPort, bMode);
		m_bConnectFlag = TRUE;

		return TRUE;
	}
	catch (CInternetException* pEx)
	{
		DWORD nErrorCode = ::GetLastError();
		CString strErrMsg;
		pEx->GetErrorMessage(strErrMsg.GetBuffer(0), 255);
		m_strErrorMessage.Format(TEXT("%d : %s"), nErrorCode, strErrMsg.GetBuffer(0));
		pEx->Delete();
		return FALSE;
	}
}

BOOL CFtp::DisConnect()
{
	if (!m_bConnectFlag)
		return TRUE;

	try
	{
		if (m_pFtp != NULL)
		{
			m_pFtp->Close();
			delete m_pFtp;
			m_pFtp = NULL;
		}
		if (m_pSession != NULL)
		{
			m_pSession->Close();
			delete m_pSession;
			m_pSession = NULL;
		}
		m_bConnectFlag = FALSE;

		return TRUE;
	}
	catch (CInternetException* pEx)
	{
		DWORD nErrorCode = ::GetLastError();
		CString strErrMsg;
		pEx->GetErrorMessage(strErrMsg.GetBuffer(0), 255);
		m_strErrorMessage.Format(TEXT("%d : %s"), nErrorCode, strErrMsg.GetBuffer(0));
		pEx->Delete();
		return FALSE;
	}
}

std::vector<CString> CFtp::GetEveryThing(LPCTSTR strDirectory, BOOL bIncludePath)
{
	if (!m_bConnectFlag)
	{
		m_strErrorMessage = TEXT("연결상태를 확인하세요");
		return std::vector<CString>();
	}

	if (m_pFinder == NULL)
		m_pFinder = new CFtpFileFind(m_pFtp);
	
	try
	{
		std::vector<CString> vecRet;
		CString strFileName;
		CString strOriginDirectory;
		m_pFtp->GetCurrentDirectory(strOriginDirectory);
		m_pFtp->SetCurrentDirectory(strDirectory);

		BOOL bWorking = m_pFinder->FindFile(TEXT("*"));
		while (bWorking)
		{
			bWorking = m_pFinder->FindNextFile();
			if (bIncludePath)
			{
				strFileName.Format(TEXT("%s/%s"), m_pFinder->GetFilePath().GetBuffer(0), m_pFinder->GetFileName().GetBuffer(0));
				vecRet.push_back(strFileName);
			}
			else
			{
				strFileName = m_pFinder->GetFileName();
				if (m_pFinder->IsDirectory())
					strFileName.Format(TEXT("/%s"), strFileName.GetBuffer(0));
				vecRet.push_back(strFileName);
			}
		}
		std::sort(vecRet.begin(), vecRet.end());

		m_pFtp->SetCurrentDirectory(strOriginDirectory);
		delete m_pFinder;
		return vecRet;
	}
	catch (CInternetException* pEx)
	{
		delete m_pFinder;
		DWORD dwErrorCode = ::GetLastError();
		CString strErrMsg;
		pEx->GetErrorMessage(strErrMsg.GetBuffer(0), 255);
		m_strErrorMessage.Format(TEXT("%d : %s"), dwErrorCode, strErrMsg.GetBuffer(0));
		pEx->Delete();
		return std::vector<CString>();
	}
}

CString CFtp::GetFilePath(LPCTSTR strFileName, LPCTSTR strStartDirectory)
{
	if (!m_bConnectFlag)
	{
		m_strErrorMessage = TEXT("연결상태를 확인하세요");
		return TEXT("");
	}

	try
	{
		CString strOriginDirectory;
		if (!m_pFtp->GetCurrentDirectory(strOriginDirectory)) throw;
		if (!m_pFtp->SetCurrentDirectory(strStartDirectory)) throw;

		m_pFinder = new CFtpFileFind(m_pFtp);
		BOOL bFound = FALSE;
		CString strFilePath;
		
		while (!bFound)
		{
			std::vector<CString> vecElement = this->GetEveryThing(strStartDirectory, TRUE);
			for (auto iter : vecElement)
			{
				CString strName = iter.GetBuffer(iter.ReverseFind('/'));
				if (strName == strFileName)
				{
					bFound = TRUE;
					return iter;
				}
			}
		}

		if (!m_pFtp->SetCurrentDirectory(strOriginDirectory)) throw;
		delete m_pFinder;

		return strFilePath;
	}
	catch (CInternetException* pEx)
	{
		delete m_pFinder;
		DWORD nErrorCode = ::GetLastError();
		CString strErrMsg;
		pEx->GetErrorMessage(strErrMsg.GetBuffer(0), 255);
		m_strErrorMessage.Format(TEXT("%d : %s"), nErrorCode, strErrMsg.GetBuffer(0));
		pEx->Delete();
		return TEXT("");
	}
}

BYTE* CFtp::DownloadFileInMemory(LPCTSTR strFilePath)
{
	if (!m_bConnectFlag)
	{
		m_strErrorMessage = TEXT("연결상태를 확인하세요");
		return nullptr;
	}

	try
	{
		m_pInternetFile = m_pFtp->OpenFile(strFilePath, GENERIC_READ);
		ULONGLONG nFileSize, nLeavingSize;
		nFileSize = m_pInternetFile->GetLength();
		nLeavingSize = nFileSize;
		BYTE* pFileData = new BYTE[nFileSize];
		UINT readSize;
		do
		{
			if (nLeavingSize < MAX_BUFFER_SIZE)
			{
				readSize = m_pInternetFile->Read(pFileData, nLeavingSize);
				pFileData += readSize;
				nLeavingSize -= readSize;
				continue;
			}
			readSize = m_pInternetFile->Read(pFileData, MAX_BUFFER_SIZE);
			pFileData += readSize;
			nLeavingSize -= readSize;
		} while (readSize != 0);
		m_pInternetFile->Close();
		pFileData -= nFileSize;

		return pFileData;
	}
	catch (CInternetException* pEx)
	{
		DWORD nErrorCode = ::GetLastError();
		CString strErrMsg;
		pEx->GetErrorMessage(strErrMsg.GetBuffer(0), 255);
		m_strErrorMessage.Format(TEXT("%d : %s"), nErrorCode, strErrMsg.GetBuffer(0));
		pEx->Delete();
		return nullptr;
	}
}

CString CFtp::GetErrorMessage()
{
	return m_strErrorMessage;
}

