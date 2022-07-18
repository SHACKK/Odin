#include "pch.h"
#include "Zipper.h"

CZipper::CZipper()
{
	zf = NULL;
	memset(&zf_info, 0, sizeof(zip_fileinfo));
}

CZipper::~CZipper()
{
	this->CloseZipFile();
}

BOOL CZipper::CreateZipFile(LPCTSTR strLocalPath)
{
	zf = zipOpen(CT2A(strLocalPath), APPEND_STATUS_CREATE);
	if (zf != NULL)
		return TRUE;
	else
		return FALSE;
}

BOOL CZipper::CloseZipFile()
{
	zipClose(zf, "");
	return TRUE;
}

BOOL CZipper::Zip(LPCTSTR strFileName,  BYTE* pFileBuffer, unsigned int nFileLength, LPCTSTR strPassword, FP_DOWNLOAD_PROGRESS fpCallback)
{
	if (zf == NULL)
		return FALSE;

	time_t nt = time(NULL);
	tm tmdata;
	errno_t timeError = localtime_s(&tmdata, &nt);
	if (timeError != 0)
		return FALSE;

	zf_info.tmz_date.tm_hour = tmdata.tm_hour;
	zf_info.tmz_date.tm_mday = tmdata.tm_mday;
	zf_info.tmz_date.tm_min = tmdata.tm_min;
	zf_info.tmz_date.tm_mon = tmdata.tm_mon;
	zf_info.tmz_date.tm_sec = tmdata.tm_sec;
	zf_info.tmz_date.tm_year = tmdata.tm_year;

	uLong crc = 0;
	if (strPassword != NULL)
	{
		crc = crc32(0L, Z_NULL, 0);
		crc = crc32(crc, (const Bytef*)pFileBuffer, nFileLength);
		if (crc == 0)
		{
			return FALSE;
		}
	}

	int nRet = 0;
	if (strPassword != NULL)
	{
		nRet = zipOpenNewFileInZip3(zf,
			CT2A(strFileName),
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
			CT2A(strFileName),
			&zf_info,
			NULL,
			0,
			NULL,
			0,
			"",
			Z_DEFLATED,
			Z_DEFAULT_COMPRESSION);
	}
	
	int nRead = 0;

	zipWriteInFileInZip(zf, (const void*)pFileBuffer, nFileLength);
	zipCloseFileInZip(zf);

	return TRUE;
}
