#include "CDispatcher.h"

CDispatcher::CDispatcher()
{
	m_Callback = NULL;
	m_CallbackParam = 0;
	InitializeCriticalSection(&m_CriticalSection);
}

void CDispatcher::SetCallbackData(DispatcherCallback Callback, DWORD Param)
{
	m_Callback = Callback;
	m_CallbackParam = Param;
}

void CDispatcher::ExecDispatch(int uiEvent, int* hRawDeviceHandle, int Param)
{
	if(m_Callback)
	{
		EnterCriticalSection(&m_CriticalSection);
		m_Callback(m_CallbackParam, hRawDeviceHandle, uiEvent, Param);
		LeaveCriticalSection(&m_CriticalSection);
	}
}

void CDispatcher::Dispatch(int uiEvent, int* hRawDeviceHandle, int Param)
{
	CDispatcher::GetSingleton()->ExecDispatch(uiEvent, hRawDeviceHandle, Param);
}

CDispatcher::~CDispatcher()
{
	DeleteCriticalSection(&m_CriticalSection);
	m_Callback = 0;
	m_CallbackParam = 0;
}