/**
 */
#pragma once

#include "hid.h"
#include "CSingleton.h"
#include <Windows.h>

typedef void (CALLBACK *DispatcherCallback)(int,int*,int,int);

class CDispatcher : public Singleton<CDispatcher>
{
public:
	CDispatcher();
	void SetCallbackData(DispatcherCallback Callback, DWORD Param);
	void ExecDispatch(int uiEvent, int* hRawDeviceHandle, int Param);
	static void Dispatch(int uiEvent, int* hRawDeviceHandle, int Param);
	~CDispatcher();
protected:
private:
	DispatcherCallback m_Callback;
	DWORD m_CallbackParam;
	CRITICAL_SECTION m_CriticalSection;
};
