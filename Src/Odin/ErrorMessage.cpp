#include "pch.h"
#include "ErrorMessage.h"

CErrorMessage* CErrorMessage::m_pInst = nullptr;

CErrorMessage* CErrorMessage::GetInst()
{
	if (!m_pInst)
		m_pInst = new CErrorMessage();

	return m_pInst;
}

void CErrorMessage::SetErrorMessage(CString strErrorMessage)
{
	m_strErrorMessage = strErrorMessage;
}

CString CErrorMessage::GetErrorMessage()
{
	return m_strErrorMessage;
}
