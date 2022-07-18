#pragma once
#include <zip.h>

#define MAX_BUFFER_SIZE 1024*128

// 몇번째 파일인가, 총 몇개의 파일인가, 얼마나 되었는가, 총 작업량)
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