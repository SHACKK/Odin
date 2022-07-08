# LibOdin
- The Odin library can download, compress, and encrypt files using the FTP protocol.

## [ 단일 파일 다운로드 ]
BOOL DownloadSingleFile(ST_FILE_INFO stFileInfo, LPCTSTR strHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);

- _in 		strHash 	    : 단일 파일의 해시값
- _in 		strLocalPath    : .zip파일이 저장될 로컬 경로, .zip파일의 이름까지 명시해주어야 한다.
- _in_opt	strPassword     : 비밀번호 여부
- _in_opt	ProgressBar     : Progress Control에 대한 포인터


## [ 다수 파일 다운로드 ]
```c++
BOOL DownloadMultiFile(ST_FILE_INFO stFileInfo, std::vector<CString> vecHash, LPCTSTR strLocalPath, LPCTSTR strPassword = NULL, CProgressCtrl* ProgressBar = NULL);
    - _in 		vecHash 	: 해시값의 벡터
    - _in 		strLocalPath : .zip파일이 저장될 로컬 경로, .zip파일의 이름까지 명시해주어야 한다.
    - _in_opt	strPassword : 비밀번호 여부
    - _in_opt	ProgressBar : Progress Control에 대한 포인터
```

## [ 에러 메시지 반환 ]
- CString GetErrMsg();
- _example
```c++
CString strErrorMessage = this.GetErrMsg();
_tprintf(TEXT("%s\n"), strErrorMessage);
```