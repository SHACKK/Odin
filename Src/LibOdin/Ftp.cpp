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
		m_bDiscovered = FALSE;
		if (!m_bConnectFlag)	throw CString(TEXT("연결상태를 확인하세요"));

		if (m_pFinder == NULL)
			m_pFinder = new CFtpFileFind(m_pFtp);

		CString strOriginDirectory;
		m_pFtp->GetCurrentDirectory(strOriginDirectory);

		if (!m_strDirectoryCache.IsEmpty())
		{
			CString strTemp;
			strTemp.Format(TEXT("%s/%s"), m_strDirectoryCache.GetBuffer(0), pszFileName);
			BOOL bTemp = m_pFinder->FindFile(strTemp);
			if (bTemp)
			{
				strDestBuffer->Format(TEXT("%s/%s"), m_strDirectoryCache.GetBuffer(0), pszFileName);
				delete m_pFinder;
				m_pFinder = NULL;

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
				m_strDirectoryCache = m_pFinder->GetFilePath();
				strDestBuffer->Format(TEXT("%s/%s"), m_strDirectoryCache.GetBuffer(0), m_pFinder->GetFileName().GetBuffer(0));
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
			{
				m_pFtp->SetCurrentDirectory(strOriginDirectory);
				if (m_bDiscovered && (m_pFinder != NULL))
				{
					delete m_pFinder;
					m_pFinder = NULL;
				}

				return;
			}

			this->GetFilePath(pszFileName, strDestBuffer, *iter);
		}

		m_pFtp->SetCurrentDirectory(strOriginDirectory);
		if (m_bDiscovered && (m_pFinder != NULL))
		{
			delete m_pFinder;
			m_pFinder = NULL;
		}

		return;
	}
	catch (CInternetException* pEx)
	{
		if (m_bDiscovered && (m_pFinder != NULL))
		{
			delete m_pFinder;
			m_pFinder = NULL;
		}

		DWORD nErrorCode = ::GetLastError();
		CString strErrMsg;
		pEx->GetErrorMessage(strErrMsg.GetBuffer(0), 255);
		m_strErrorMessage.Format(TEXT("%d : %s"), nErrorCode, strErrMsg.GetBuffer(0));
		pEx->Delete();

		return;
	}
	catch (CString ErrorMessage)
	{
		if (m_bDiscovered && (m_pFinder != NULL))
		{
			delete m_pFinder;
			m_pFinder = NULL;
		}

		m_strErrorMessage = ErrorMessage;

		return;
	}
}

BOOL CFtp::DownloadFileInMemory(LPCTSTR pszFilePath, BYTE* pszBuffer, ULONGLONG ulBufferSize)
{
	if (!m_bConnectFlag)
	{
		m_strErrorMessage = TEXT("연결상태를 확인하세요");
		return FALSE;
	}

	try
	{
		m_pInternetFile = m_pFtp->OpenFile(pszFilePath, GENERIC_READ);
		ULONGLONG nLeavingSize;
		nLeavingSize = ulBufferSize;
		UINT readSize;
		do
		{
			if (nLeavingSize < MAX_BUFFER_SIZE)
			{
				readSize = m_pInternetFile->Read(pszBuffer, (UINT)nLeavingSize);
				pszBuffer += readSize;
				nLeavingSize -= readSize;
				continue;
			}
			readSize = m_pInternetFile->Read(pszBuffer, MAX_BUFFER_SIZE);
			pszBuffer += readSize;
			nLeavingSize -= readSize;
		} while (nLeavingSize != 0);
		m_pInternetFile->Close();
		m_pInternetFile = NULL;
		pszBuffer -= ulBufferSize;

		return TRUE;
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
		return FALSE;
	}
}

CString CFtp::GetErrorMessage()
{
	return m_strErrorMessage;
}

CString CFtp::GetDirectoryCache()
{
	return m_strDirectoryCache;
}

ULONGLONG CFtp::GetFileSize(LPCTSTR pszFilePath)
{
	try
	{
		if (!m_bConnectFlag)	throw CString(TEXT("연결상태를 확인하세요"));

		CFtpFileFind finder(m_pFtp);
		BOOL bWorking = finder.FindFile(pszFilePath);
		if (!bWorking)	throw CString(TEXT("파일을 찾을 수 없습니다"));
		bWorking = finder.FindNextFile();

		ULONGLONG ulSize = finder.GetLength();

		finder.Close();
		return ulSize;
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