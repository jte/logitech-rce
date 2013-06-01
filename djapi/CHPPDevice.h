/**
 */
#pragma once

#include <Windows.h>
#include "hid.h"
#include "CHPPTransporter.h"

enum
{
	HPP_ERR_LINK_LOSS,
	HPP_ERR_DEVICE_STOPPING,
	HPP_ERR_COLLECTION,
	HPP_ERR_LOW_RESSOURCES,
	HPP_ERR_CONTENTION,
	HPP_ERR_IO_FAILED,
	HPP_ERR_DEVICE_REMOVAL,
	HPP_ERR_NOT_RESPONDING,
	HPP_ERR_UNKNOWN,
	HPP_ERR_INVALID_PIN_CODE,
	HPP_ERR_INVALID_PARAMETER,
	HPP_ERR_CONTEXT_ERROR,
	HPP_ERR_RESOURCE_ERROR,
	HPP_ERR_UNKNOWN_DEVICE,
	HPP_ERR_BUZY,
	HPP_ERR_ALREADY_EXISTS,
	HPP_ERR_TOO_MANY_DEVICES,
	HPP_ERR_CONNECT_FAIL,
	HPP_ERR_INVALID_VALUE,
	HPP_ERR_INVALID_ADDRESS,
	HPP_ERR_INVALID_SUBID
};

class CHPPDevice
{
public:
	int AddHPPTransporter(DEVINST dnDevInst, HANDLE HidDeviceObject, const wchar_t *DevicePath, PHIDP_CAPS Capabilities, BYTE ReportID, HDEVNOTIFY hDevNotify);
	int SendCommand(CHPPTransporter* pTransporter, BYTE* lpCommandBuffer, BYTE* lpResponseBuffer, DWORD TimeOut, int a6);
	static int CALLBACK HPPThreadStatic(LPVOID Context);
	int HPPThread();
	virtual int OnReadData(BYTE *InputBuffer, DWORD lpNumberOfBytesRead);
	virtual int OnProcessReadData(BYTE *InputBuffer, DWORD lpNumberOfBytesRead);
	virtual int OnProcessSpontaneous(BYTE *InputBuffer, DWORD lpNumberOfBytesRead);
	int CreateThreadsForReceiver(HWND hEnumNotifyWnd, int wParamReceiverIdentifier);
	static int CALLBACK ForwardInputToReceiversThreadStatic(LPVOID Context);
	int ForwardInputToReceivers();
	virtual int ProcessSpontaneous(BYTE* InputBuffer, DWORD lpNumberOfBytesRead) = 0;
protected:
private:
	DWORD pad1;
	DEVINST m_DevInst;
	CHPPTransporter* m_pTransporters[3];
	BYTE* m_ReadBuffer[100];
	DWORD m_ReadBufferBytesRead[100];
	DWORD m_NumberInputBuffers;
	DWORD m_CurrentInputBuffer;
	DWORD pad2;
	HANDLE m_Event4;
	HANDLE m_ReadThread;
	HANDLE m_InputForwarderThread;
	HANDLE m_Event3;
	HWND m_hEnumNotifyWnd;
	DWORD m_wParamReceiverIdentifier;
	HANDLE m_ReadWriteOverlappedEvent;
	HANDLE m_DeviceMutex;
	DWORD m_MutexTimeout;
	DWORD Threshold;
	DWORD pad3;
	LARGE_INTEGER pad4;
	LARGE_INTEGER m_PerfCounter;
	LARGE_INTEGER m_PerformanceCounterFreq;
	DWORD m_ulOutputLength;
	BYTE* m_lpCommandBuffer;
	DWORD m_ArgumentsSize;
	HANDLE m_Event2;
	BYTE* m_lpResponseBuffer;
	DWORD pad5;
	BYTE pad6[1016];
	BYTE m_ResponseCode;
};