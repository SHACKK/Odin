#pragma once
#include <zip.h>

#define MAX_BUFFER_SIZE 1024*128

class CZipper
{
private:
	zipFile zf;
	zip_fileinfo zf_info;
public:
	CZipper();
	~CZipper();
	BOOL CreateZipFile(LPCTSTR strLocalPath);
	BOOL CloseZipFile();
	BOOL Zip(LPCTSTR strFileName, BYTE* pFileBuffer, unsigned int nFileLength, LPCTSTR strPassword = NULL);
};