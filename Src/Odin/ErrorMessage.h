#pragma once
#include <vector>
#include <atlstr.h>

#define MAX_MESSAGE_COUNT 5

class CErrorMessage
{
private:
	CErrorMessage() {}
	~CErrorMessage() {}

private:
	static CErrorMessage* m_pInst;
	CString m_strErrorMessage;

public:
	CErrorMessage* GetInst();
	void SetErrorMessage(CString strErrorMessage);
	CString GetErrorMessage();
};