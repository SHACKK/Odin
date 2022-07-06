#include "pch.h"
#include "Ftp.h"

CFtp::CFtp()
{
	m_pFtp = NULL;
	m_pSession = NULL;
	m_pInternetFile = NULL;
	m_pFinder = NULL;
	m_bConnectFlag = FALSE;

	//임시
	m_bDiscovered = FALSE;

}

CFtp::CFtp(LPCTSTR pszIP, WORD wPort, LPCTSTR pszUserID, LPCTSTR pszUserPW, BOOL bMode)
{
	m_pFtp = NULL;
	m_pSession = NULL;
	m_pInternetFile = NULL;
	m_bConnectFlag = FALSE;

	BOOL bRes = this->Connect(pszIP, wPort, pszUserID, pszUserPW, bMode);
}

CFtp::~CFtp()
{
	if (m_bConnectFlag)
		this->DisConnect();
}

BOOL CFtp::Connect(LPCTSTR pszIP, WORD wPort, LPCTSTR pszUserID, LPCTSTR pszUserPW, BOOL bMode)
{
	if (m_bConnectFlag)
		return TRUE;

	try
	{
		m_pSession = new CInternetSession;
		m_pFtp = m_pSession->GetFtpConnection(pszIP, pszUserID, pszUserPW, (INTERNET_PORT)wPort, bMode);
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

std::vector<CString> CFtp::GetEveryThing(LPCTSTR pszDirectory, BOOL bIncludePath)
{
	if (!m_bConnectFlag) throw CString(TEXT("연결 상태를 확인하세요"));

	if (m_pFinder == NULL)
		m_pFinder = new CFtpFileFind(m_pFtp);
	
	try
	{
		std::vector<CString> vecRet;
		CString strFileName;
		CString strOriginDirectory;
		m_pFtp->GetCurrentDirectory(strOriginDirectory);
		m_pFtp->SetCurrentDirectory(pszDirectory);

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
		if(m_pFinder != NULL)
			delete m_pFinder;

		DWORD dwErrorCode = ::GetLastError();
		CString strErrMsg;
		pEx->GetErrorMessage(strErrMsg.GetBuffer(0), 255);
		m_strErrorMessage.Format(TEXT("%d : %s"), dwErrorCode, strErrMsg.GetBuffer(0));
		pEx->Delete();
		return std::vector<CString>();
	}
	catch (CString ErrorMessage)
	{
		if (m_pFinder != NULL)
			delete m_pFinder;

		m_strErrorMessage = ErrorMessage;
		return std::vector<CString>();
	}
}

void CFtp::GetFilePath(LPCTSTR pszFileName, CString* strDestBuffer, LPCTSTR pszStartDirectory)
{
	try
	{
		if (m_pFinder == NULL)
			m_pFinder = new CFtpFileFind(m_pFtp);

		CString strOriginDirectory;
		m_pFtp->GetCurrentDirectory(strOriginDirectory);

		if (!m_strRecentPath.IsEmpty())
		{
			CString strTemp;
			strTemp.Format(TEXT("%s/%s"), m_strRecentPath.GetBuffer(0), pszFileName);
			BOOL bTemp = m_pFinder->FindFile(strTemp);
			if (bTemp)
			{
				strDestBuffer->Format(TEXT("%s/%s"), m_strRecentPath.GetBuffer(0), pszFileName);
				return;
			}
		}

		std::vector<CString> vecChildDir;
		m_pFtp->SetCurrentDirectory(pszStartDirectory);
		BOOL bWorking = m_pFinder->FindFile(TEXT("*"));
		while (bWorking)
		{
			bWorking = m_pFinder->FindNextFile();
			if (pszFileName == m_pFinder->GetFileName())
			{
				m_strRecentPath = m_pFinder->GetFilePath();
				strDestBuffer->Format(TEXT("%s/%s"), m_strRecentPath.GetBuffer(0), m_pFinder->GetFileName().GetBuffer(0));
				m_bDiscovered = TRUE;

				return;
			}
			if (m_pFinder->IsDirectory())
			{
				CString strChildDir;
				strChildDir.Format(TEXT("%s/%s"), m_pFinder->GetFilePath().GetBuffer(0), m_pFinder->GetFileName().GetBuffer(0));
				vecChildDir.push_back(strChildDir);
			}
		}

		// 벡터 정렬
		std::sort(vecChildDir.begin(), vecChildDir.end());
		std::vector<CString>::reverse_iterator iter;
		for (iter = vecChildDir.rbegin(); iter != vecChildDir.rend(); iter++)
		{
			if (m_bDiscovered)
				return;

			this->GetFilePath(pszFileName, strDestBuffer, *iter);
		}

		m_pFtp->SetCurrentDirectory(strOriginDirectory);
		if(m_bDiscovered && (m_pFinder != NULL))
			delete m_pFinder;

		return;
	}
	catch (CInternetException* pEx)
	{
		if (m_bDiscovered && (m_pFinder != NULL))
			delete m_pFinder;

		DWORD nErrorCode = ::GetLastError();
		CString strErrMsg;
		pEx->GetErrorMessage(strErrMsg.GetBuffer(0), 255);
		m_strErrorMessage.Format(TEXT("%d : %s"), nErrorCode, strErrMsg.GetBuffer(0));
		pEx->Delete();

		return;
	}
}

BYTE* CFtp::DownloadFileInMemory(LPCTSTR pszFilePath)
{
	if (!m_bConnectFlag)
	{
		m_strErrorMessage = TEXT("연결상태를 확인하세요");
		return nullptr;
	}

	try
	{
		m_pInternetFile = m_pFtp->OpenFile(pszFilePath, GENERIC_READ);
		ULONGLONG nFileSize, nLeavingSize;
		nFileSize = m_pInternetFile->GetLength();
		nLeavingSize = nFileSize;
		BYTE* pFileData = new BYTE[nFileSize];
		UINT readSize;
		do
		{
			if (nLeavingSize < MAX_BUFFER_SIZE)
			{
				readSize = m_pInternetFile->Read(pFileData, (UINT)nLeavingSize);
				pFileData += readSize;
				nLeavingSize -= readSize;
				continue;
			}
			readSize = m_pInternetFile->Read(pFileData, MAX_BUFFER_SIZE);
			pFileData += readSize;
			nLeavingSize -= readSize;
		} while (nLeavingSize != 0);
		m_pInternetFile->Close();
		pFileData -= nFileSize;

		return pFileData;
	}
	catch (CInternetException* pEx)
	{
		if (m_pInternetFile != NULL)
			m_pInternetFile->Close();

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

ULONGLONG CFtp::GetFileSize(LPCTSTR pszFilePath)
{
	try
	{
		if (!m_bConnectFlag)	throw CString(TEXT("연결상태를 확인하세요"));

		m_pInternetFile = m_pFtp->OpenFile(pszFilePath, GENERIC_READ);
		ULONGLONG nFileSize = m_pInternetFile->GetLength();

		delete m_pInternetFile;
		return nFileSize;
	}
	catch (CInternetException* pEx)
	{
		if (m_pInternetFile != NULL)
			delete m_pInternetFile;

		DWORD nErrorCode = ::GetLastError();
		CString strErrMsg;
		pEx->GetErrorMessage(strErrMsg.GetBuffer(0), 255);
		m_strErrorMessage.Format(TEXT("%d : %s"), nErrorCode, strErrMsg.GetBuffer(0));
		pEx->Delete();
		return 0;
	}
	catch (CString ErrorMessage)
	{
		if(m_pInternetFile != NULL)
			delete m_pInternetFile;

		m_strErrorMessage = ErrorMessage;
		return 0;
	}
}

CString CFtp::GetFullFilePath(LPCTSTR pszFileName, LPCTSTR pszRemoteDir)
{
	try
	{
		if (!m_bConnectFlag)	throw CString(TEXT("연결상태를 확인하세요"));

		m_bDiscovered = FALSE;
		CString strOriginDir;
		std::vector<CString> vecChildDir;

		if (!(m_pFtp->GetCurrentDirectory(strOriginDir))) return m_strFullFilePath;
		if (!(m_pFtp->SetCurrentDirectory(pszRemoteDir))) return m_strFullFilePath;

		CFtpFileFind finder(m_pFtp);
		SearchFile(&finder, pszFileName, pszRemoteDir);
		m_pFtp->SetCurrentDirectory(strOriginDir);
		return m_strFullFilePath;
	}
	catch (CInternetException* pEx)
	{
		int nLength = m_strErrorMessage.GetLength();
		pEx->GetErrorMessage(m_strErrorMessage.GetBuffer(nLength), nLength);
		pEx->Delete();

		return CString(TEXT(""));
	}
	catch (CString pstrErrorMessage)
	{
		m_strErrorMessage = pstrErrorMessage.GetBuffer(0);
		return CString(TEXT(""));
	}
}

void CFtp::SearchFile(CFtpFileFind* finder, LPCTSTR pszFileName, LPCTSTR pszStartDir)
{
	try
	{
		if (!m_strRecentPath.IsEmpty())
		{
			CString strTemp;
			strTemp.Format(TEXT("%s/%s"), m_strRecentPath.GetBuffer(0), pszFileName);
			BOOL bTemp = finder->FindFile(strTemp);
			if (bTemp)
			{
				m_strFullFilePath.Format(TEXT("%s/%s"), m_strRecentPath.GetBuffer(0), pszFileName);
				return;
			}
		}

		std::vector<CString> vecChildDir;
		m_pFtp->SetCurrentDirectory(pszStartDir);
		BOOL bWorking = finder->FindFile(TEXT("*"));
		while (bWorking)
		{
			bWorking = finder->FindNextFile();
			if (pszFileName == finder->GetFileName())
			{
				m_strRecentPath = finder->GetFilePath();
				m_strFullFilePath.Format(TEXT("%s/%s"), m_strRecentPath.GetBuffer(0), finder->GetFileName().GetBuffer(0));
				m_bDiscovered = TRUE;

				return;
			}
			if (finder->IsDirectory())
			{
				CString strChildDir;
				strChildDir.Format(TEXT("%s/%s"), finder->GetFilePath().GetBuffer(0), finder->GetFileName().GetBuffer(0));
				vecChildDir.push_back(strChildDir);
			}
		}

		// 벡터 정렬
		std::sort(vecChildDir.begin(), vecChildDir.end());
		std::vector<CString>::reverse_iterator iter;
		for (iter = vecChildDir.rbegin(); iter != vecChildDir.rend(); iter++)
		{
			if (m_bDiscovered)
				return;

			SearchFile(finder, pszFileName, *iter);
		}
	}
	catch (CInternetException* pEx)
	{
		int nLength = m_strErrorMessage.GetLength();
		pEx->GetErrorMessage(m_strErrorMessage.GetBuffer(nLength), nLength);
		pEx->Delete();
	}
}