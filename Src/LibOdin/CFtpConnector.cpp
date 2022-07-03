#include "pch.h"
#include "CFtpConnector.h"

CFTPConnector::CFTPConnector()
{

}

CFTPConnector::~CFTPConnector()
{

}

BOOL CFTPConnector::Connect(LPCTSTR szIP, int wPort, LPCTSTR strUserID, LPCTSTR strUserPW, BOOL bPassive)
{
	if (m_bConnect)
		return TRUE;

	try
	{
		m_pSession = new CInternetSession;
		m_pFtp = m_pSession->GetFtpConnection(szIP, strUserID, strUserPW, wPort, 0);
		m_bConnect = TRUE;
		return TRUE;
	}
	catch (CInternetException* pEx)
	{
		pEx->GetErrorMessage(m_strErrorMessage.GetBuffer(0), 255);
		pEx->Delete();

		return FALSE;
	}
}

BOOL CFTPConnector::DisConnect()
{
	try
	{
		if (NULL != m_pFtp)
		{
			m_pFtp->Close();
			delete m_pFtp;
			m_pFtp = NULL;
		}

		if (NULL != m_pSession)
		{
			m_pSession->Close();
			delete m_pSession;
			m_pSession = NULL;
		}

		m_bConnect = FALSE;

		return TRUE;
	}
	catch (CInternetException* pEx)
	{
		int nLength = m_strErrorMessage.GetLength();
		pEx->GetErrorMessage(m_strErrorMessage.GetBuffer(nLength), nLength);
		pEx->Delete();

		return FALSE;
	}
}

LPCTSTR CFTPConnector::GetFullFilePath(LPCTSTR strFileName, LPCTSTR strRemoteDir)
{
	if (!m_bConnect)
		return TEXT("");

	try
	{
		m_bDiscovered = FALSE;
		CString strOriginDir;
		std::vector<CString> vecChildDir;

		if (!(m_pFtp->GetCurrentDirectory(strOriginDir))) return m_strFullFilePath;
		if (!(m_pFtp->SetCurrentDirectory(strRemoteDir))) return m_strFullFilePath;

		CFtpFileFind finder(m_pFtp);
		SearchFile(&finder, strFileName, strRemoteDir);
		m_pFtp->SetCurrentDirectory(strOriginDir);
		return m_strFullFilePath;
	}
	catch (CInternetException* pEx)
	{
		int nLength = m_strErrorMessage.GetLength();
		pEx->GetErrorMessage(m_strErrorMessage.GetBuffer(nLength), nLength);
		pEx->Delete();

		return TEXT("");
	}
}

void CFTPConnector::SearchFile(CFtpFileFind* finder, LPCTSTR strFileName, LPCTSTR strStartDir)
{
	try
	{
		if (!m_strRecentPath.IsEmpty())
		{
			CString strTemp;
			strTemp.Format(TEXT("%s/%s"), m_strRecentPath, strFileName);
			BOOL bTemp = finder->FindFile(strTemp);
			if (bTemp)
			{
				m_strFullFilePath.Format(TEXT("%s/%s"), m_strRecentPath, strFileName);
				return;
			}
		}

		std::vector<CString> vecChildDir;
		m_pFtp->SetCurrentDirectory(strStartDir);
		BOOL bWorking = finder->FindFile(TEXT("*"));
		while (bWorking)
		{
			bWorking = finder->FindNextFile();
			if (strFileName == finder->GetFileName())
			{
				m_strRecentPath = finder->GetFilePath();
				m_strFullFilePath.Format(TEXT("%s/%s"), m_strRecentPath, finder->GetFileName());
				m_bDiscovered = TRUE;

				return;
			}
			if (finder->IsDirectory())
			{
				CString strChildDir;
				strChildDir.Format(TEXT("%s/%s"), finder->GetFilePath(), finder->GetFileName());
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

			SearchFile(finder, strFileName, *iter);
		}
	}
	catch (CInternetException* pEx)
	{
		int nLength = m_strErrorMessage.GetLength();
		pEx->GetErrorMessage(m_strErrorMessage.GetBuffer(nLength), nLength);
		pEx->Delete();
	}
}

uLong CFTPConnector::GetCRC(LPCTSTR strFilePath)
{
	try
	{
		CInternetFile* pInternet = m_pFtp->OpenFile(strFilePath, GENERIC_READ);
		uLong crc = crc32(0L, Z_NULL, 0);
		BYTE byFileBuf[MAX_BUFFER_SIZE];
		UINT readSize;
		do
		{
			readSize = pInternet->Read(byFileBuf, MAX_BUFFER_SIZE);
			crc = crc32(crc, (const Bytef*)byFileBuf, readSize);
		} while (readSize != 0);
		pInternet->Close();

		return crc;
	}
	catch (CInternetException* pEx)
	{
		pEx->GetErrorMessage(m_strErrorMessage.GetBuffer(0), 255);
		pEx->Delete();
		return 0;
	}
}

BOOL CFTPConnector::DownloadSingleFile(LPCTSTR strHash, LPCTSTR strLocalPath, LPCTSTR strPassword, CProgressCtrl* ProgressBar)
{
	if (!m_bConnect) //throw(TEXT("연결상태를 확인하세요"));
	{
		m_strErrorMessage = TEXT("연결상태를 확인하세요");
		return FALSE;
	}

	CString strLocal = strLocalPath;
	int nLength = strLocal.GetLength();
	CString strExt = strLocal.Mid(nLength - 4, nLength);
	if (!_tcscmp(strExt, TEXT(".zip")) || nLength == 0)
	{
		m_strErrorMessage = TEXT("Local 확장자를 확인하세요");
		return FALSE;
	}

	try
	{
		// 파일 경로 얻어오기
		CString strFullFilePath = GetFullFilePath(strHash, TEXT("/"));
		if (strFullFilePath.IsEmpty()) //throw(TEXT("파일을 찾을 수 없습니다"));
		{
			m_strErrorMessage = TEXT("파일을 찾을 수 없습니다");
			return FALSE;
		}

		// zip 파일 생성 및 설정
		zipFile zf = zipOpen(CT2A(strLocalPath), APPEND_STATUS_CREATE);
		zip_fileinfo zf_info;
		memset(&zf_info, 0, sizeof(zip_fileinfo));

		time_t nt;
		time(&nt);
		tm* tmdata = NULL;
		localtime_s(tmdata, &nt);
		zf_info.tmz_date.tm_hour = tmdata->tm_hour;
		zf_info.tmz_date.tm_mday = tmdata->tm_mday;
		zf_info.tmz_date.tm_min = tmdata->tm_min;
		zf_info.tmz_date.tm_mon = tmdata->tm_mon;
		zf_info.tmz_date.tm_sec = tmdata->tm_sec;
		zf_info.tmz_date.tm_year = tmdata->tm_year;

		uLong crc = GetCRC(strFullFilePath);
		if (crc == 0)
		{
			m_strErrorMessage = TEXT("CRC 값 계산 실패");
			return FALSE;
		}

		m_pInternetFile = m_pFtp->OpenFile(strFullFilePath, GENERIC_READ);

		if (strPassword != NULL)
		{
			int nRet = zipOpenNewFileInZip3(zf,
				/*                    */	CT2A(m_pInternetFile->GetFileName()),
				/*                    */	&zf_info,
				/*                    */	NULL,
				/*                    */	0,
				/*                    */	NULL,
				/*                    */	0,
				/*                    */	"",
				/*                    */	Z_DEFLATED,
				/*                    */	Z_DEFAULT_COMPRESSION,
				/*                    */	0,
				/*                    */	MAX_WBITS,
				/*                    */	DEF_MEM_LEVEL,
				/*                    */	Z_DEFAULT_STRATEGY,
				/*                    */	CT2A(strPassword),
				/*                    */	crc);
		}
		else
		{
			int nRet = zipOpenNewFileInZip(zf,
				/*                    */   CT2A(m_pInternetFile->GetFileName()),
				/*                    */   &zf_info,
				/*                    */   NULL,
				/*                    */   0,
				/*                    */   NULL,
				/*                    */   0,
				/*                    */   "",
				/*                    */   Z_DEFLATED,
				/*                    */   Z_DEFAULT_COMPRESSION);
		}

		if (ProgressBar != NULL)	ProgressBar->SetRange(0, 100);
		ULONGLONG nTotalSize = m_pInternetFile->GetLength();
		ULONGLONG nReadSize = 0;

		m_pInternetFile = m_pFtp->OpenFile(strFullFilePath, GENERIC_READ);
		do
		{
			// ftp 파일 읽기
			BYTE byBuff[MAX_BUFFER_SIZE] = { 0, };
			UINT unRead = m_pInternetFile->Read(byBuff, MAX_BUFFER_SIZE);
			nReadSize += unRead;
			if (0 == unRead)	break;

			zipWriteInFileInZip(zf, (const void*)byBuff, unRead);
			int nPos = (nReadSize / nTotalSize) * 100;
			if (ProgressBar != NULL)	ProgressBar->SetPos(nPos);
		} while (true);

		zipCloseFileInZip(zf);
		m_pInternetFile->Close();
		zipClose(zf, "");

		return TRUE;
	}
	catch (CInternetException* pEx)
	{
		pEx->GetErrorMessage(m_strErrorMessage.GetBuffer(0), 255);
		pEx->Delete();
		m_pInternetFile->Flush();
		m_pInternetFile->Close();
		return FALSE;
	}
}

//폴더 압축
BOOL CFTPConnector::DownloadMultiFile(std::vector<CString> vecHash, LPCTSTR strLocalPath, LPCTSTR strPassword, CProgressCtrl* ProgressBar)
{
	if (!m_bConnect)	//throw(TEXT("연결상태를 확인하세요));
	{
		m_strErrorMessage = TEXT("연결상태를 확인하세요");
		return FALSE;
	}

	CString strLocal = strLocalPath;
	int nLength = strLocal.GetLength();
	CString strExt = strLocal.Mid(nLength - 4, nLength);
	if (_tcscmp(strExt, TEXT(".zip")) || nLength == 0)
	{
		m_strErrorMessage = TEXT("Local 확장자를 확인하세요");
		return FALSE;
	}

	try
	{
		zipFile zf = zipOpen(CT2A(strLocalPath), APPEND_STATUS_CREATE);

		zip_fileinfo zf_info;
		memset(&zf_info, 0, sizeof(zip_fileinfo));

		time_t nt;
		tm* tmdata = NULL;
		time(&nt);
		localtime_s(tmdata, &nt);

		zf_info.tmz_date.tm_hour = tmdata->tm_hour;
		zf_info.tmz_date.tm_mday = tmdata->tm_mday;
		zf_info.tmz_date.tm_min = tmdata->tm_min;
		zf_info.tmz_date.tm_mon = tmdata->tm_mon;
		zf_info.tmz_date.tm_sec = tmdata->tm_sec;
		zf_info.tmz_date.tm_year = tmdata->tm_year;

		if (ProgressBar != NULL)	ProgressBar->SetRange(0, (short)vecHash.size());
		int nComplete = 0;

		for (auto iter : vecHash)
		{
			CString strFileName = GetFullFilePath(iter, TEXT("/"));
			if (strFileName.IsEmpty())	continue;

			// Get CRC
			uLong crc = 0;
			m_pInternetFile = m_pFtp->OpenFile(strFileName, GENERIC_READ);
			crc = crc32(0L, Z_NULL, 0);
			BYTE byFileBuf[MAX_BUFFER_SIZE];
			UINT readSize;
			do
			{
				readSize = m_pInternetFile->Read(byFileBuf, MAX_BUFFER_SIZE);
				crc = crc32(crc, (const Bytef*)byFileBuf, readSize);
			} while (readSize != 0);
			m_pInternetFile->Close();

			if (crc == 0)
			{
				m_strErrorMessage = TEXT("CRC 계산 실패");
				zipClose(zf, "");
				return FALSE;
			}

			int nRet;
			if (strPassword != NULL)
			{
				nRet = zipOpenNewFileInZip3(zf,
					CT2A(m_pInternetFile->GetFileName()),
					&zf_info,
					NULL,
					0,
					NULL,
					0,
					"",
					Z_DEFLATED,
					Z_DEFAULT_COMPRESSION,
					0,
					MAX_WBITS,
					DEF_MEM_LEVEL,
					Z_DEFAULT_STRATEGY,
					CT2A(strPassword),
					crc);
			}
			else
			{
				nRet = zipOpenNewFileInZip(zf,
					CT2A(m_pInternetFile->GetFileName()),
					&zf_info,
					NULL,
					0,
					NULL,
					0,
					"",
					Z_DEFLATED,
					Z_DEFAULT_COMPRESSION);
			}

			m_pInternetFile = m_pFtp->OpenFile(strFileName, GENERIC_READ);
			do
			{
				// ftp 파일 읽기
				BYTE byBuff[MAX_BUFFER_SIZE] = { 0, };
				UINT unRead = m_pInternetFile->Read(byBuff, MAX_BUFFER_SIZE);

				if (0 == unRead)	break;

				zipWriteInFileInZip(zf, (const void*)byBuff, unRead);
			} while (true);

			zipCloseFileInZip(zf);
			m_pInternetFile->Close();
			if (ProgressBar != NULL)	ProgressBar->SetPos(++nComplete);
		}

		zipClose(zf, "");

		return TRUE;
	}
	catch (CInternetException* pEx)
	{
		pEx->GetErrorMessage(m_strErrorMessage.GetBuffer(0), 255);
		pEx->Delete();

		return FALSE;
	}
}

BOOL CFTPConnector::UploadFile(LPCTSTR strLocalFilePath, LPCTSTR strRemotePath)
{
	try
	{
		if (!m_bConnect)	//throw(TEXT("연결상태를 확인하세요));
		{
			m_strErrorMessage = TEXT("연결상태를 확인하세요");
			return FALSE;
		}

		time_t nt;
		tm* tmdata = NULL;
		time(&nt);
		localtime_s(tmdata, &nt);

		m_pInternetFile = m_pFtp->OpenFile(strRemotePath, GENERIC_WRITE);
		FILE* pFile;
		_tfopen_s(&pFile, strLocalFilePath, TEXT("rb"));

		if (pFile == NULL)
		{
			m_strErrorMessage = TEXT("파일을 찾을 수 없습니다");
			return FALSE;
		}

		BYTE byFileBuf[MAX_BUFFER_SIZE];
		UINT readSize;
		do
		{
			readSize = ::fread(&byFileBuf, sizeof(BYTE), MAX_BUFFER_SIZE, pFile);
			m_pInternetFile->Write(byFileBuf, MAX_BUFFER_SIZE);
		} while (readSize != 0);

		fclose(pFile);

		return TRUE;
	}
	catch (CInternetException* pEx)
	{
		pEx->GetErrorMessage(m_strErrorMessage.GetBuffer(0), 255);
		pEx->Delete();

		return FALSE;
	}
}

BOOL CFTPConnector::RenameFile(LPCTSTR strPreName, LPCTSTR strNewName)
{
	try
	{
		CString strOldDirectory;
		m_pFtp->GetCurrentDirectory(strOldDirectory);
		m_pFtp->SetCurrentDirectory(TEXT("/"));

		CFtpFileFind finder(m_pFtp);
		BOOL bWorking = finder.FindFile(TEXT("*"));
		if (!bWorking)
		{
			m_strErrorMessage = TEXT("파일을 찾을 수 없습니다");
			return FALSE;
		}
		if (finder.IsDirectory())
		{
			m_strErrorMessage = TEXT("파일이 아닙니다");
			return FALSE;
		}

		BOOL bRes = m_pFtp->Rename(strPreName, strNewName);
		if (!bRes)
		{
			m_strErrorMessage = TEXT("이름 변경에 실패하였습니다");
			return FALSE;
		}

		m_pFtp->SetCurrentDirectory(strOldDirectory);
		return bRes;
	}
	catch (CInternetException* pEx)
	{
		pEx->GetErrorMessage(m_strErrorMessage.GetBuffer(0), 255);
		pEx->Delete();

		return FALSE;
	}
}

BOOL CFTPConnector::RemoveFile(LPCTSTR strRemotePath)
{
	try
	{
		CString strOldDirectory;
		m_pFtp->GetCurrentDirectory(strOldDirectory);
		m_pFtp->SetCurrentDirectory(TEXT("/"));

		CFtpFileFind finder(m_pFtp);
		BOOL bWorking = finder.FindFile(strRemotePath);
		if (!bWorking)
		{
			m_strErrorMessage = TEXT("파일을 찾을 수 없습니다");
			return FALSE;
		}
		if (finder.IsDirectory())
		{
			m_strErrorMessage = TEXT("파일이 아닙니다");
			return FALSE;
		}

		BOOL bRes = m_pFtp->Remove(strRemotePath);
		if (!bRes)
		{
			m_strErrorMessage = TEXT("파일 삭제에 실패하였습니다");
			return FALSE;
		}

		m_pFtp->SetCurrentDirectory(strOldDirectory);
		return bRes;
	}
	catch (CInternetException* pEx)
	{
		pEx->GetErrorMessage(m_strErrorMessage.GetBuffer(0), 255);
		pEx->Delete();

		return FALSE;
	}
}

BOOL CFTPConnector::RemoveDir(LPCTSTR strRemotePath)
{
	try
	{
		CString strOldDirectory;
		m_pFtp->GetCurrentDirectory(strOldDirectory);
		m_pFtp->SetCurrentDirectory(TEXT("/"));

		CFtpFileFind finder(m_pFtp);
		BOOL bWorking = finder.FindFile(strRemotePath);
		if (!bWorking)
		{
			m_strErrorMessage = TEXT("파일을 찾을 수 없습니다");
			return FALSE;
		}

		if (!finder.IsDirectory())
		{
			m_strErrorMessage = TEXT("디렉토리가 아닙니다");
			return FALSE;
		}

		BOOL bRes = m_pFtp->RemoveDirectory(strRemotePath);
		if (!bRes)
		{
			m_strErrorMessage = TEXT("디렉토리 삭제에 실패하였습니다");
			return FALSE;
		}

		m_pFtp->SetCurrentDirectory(strOldDirectory);
		return bRes;
	}
	catch (CInternetException* pEx)
	{
		pEx->GetErrorMessage(m_strErrorMessage.GetBuffer(0), 255);
		pEx->Delete();

		return FALSE;
	}
}

CString CFTPConnector::GetErrMsg()
{
	return m_strErrorMessage;
}