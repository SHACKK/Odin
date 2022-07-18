#pragma once
#include <zip.h>

#define MAX_BUFFER_SIZE 1024*128

// ���° �����ΰ�, �� ��� �����ΰ�, �󸶳� �Ǿ��°�, �� �۾���)
typedef void (*FP_DOWNLOAD_PROGRESS)(int nFileIndex, int nFileCount);

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
	BOOL Zip(LPCTSTR strFileName, BYTE* pFileBuffer, unsigned int nFileLength, LPCTSTR strPassword = NULL, FP_DOWNLOAD_PROGRESS fpCallback = nullptr);
};