/**
 */
#pragma once

#include <Windows.h>
#include <SetupAPI.h>
#include <Cfgmgr32.h>
#include <Dbt.h>

class CIntfEnum
{
public:
	CIntfEnum();
	static int CALLBACK InterfaceEnumeratorThreadStatic(LPVOID Param);
	void InterfaceEnumeratorThread();
	BOOL CloseHandle(HANDLE hObject);
	int CreateEnumNotifWnd(HINSTANCE hInstance);
	static LRESULT CALLBACK EnumNotifWndProcStatic(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK EnumNotifWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	virtual ~CIntfEnum();
protected:
private:
	typedef HDEVINFO (Def_SetupDiGetClassDevsW)(const GUID *ClassGuid, PCTSTR Enumerator, HWND hwndParent, DWORD Flags);
	Def_SetupDiGetClassDevsW* m_pfnSetupDiGetClassDevsW;
	typedef BOOL (Def_SetupDiEnumDeviceInterfaces)(HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, const GUID *InterfaceClassGuid, DWORD MemberIndex, PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData);
	Def_SetupDiEnumDeviceInterfaces* m_pfnSetupDiEnumDeviceInterfaces;
	typedef BOOL (Def_SetupDiGetDeviceInterfaceDetailW)(HDEVINFO DeviceInfoSet,PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData,DWORD DeviceInterfaceDetailDataSize,PDWORD RequiredSize,PSP_DEVINFO_DATA DeviceInfoData);
	Def_SetupDiGetDeviceInterfaceDetailW* m_pfnSetupDiGetDeviceInterfaceDetailW;
	typedef BOOL (Def_SetupDiDestroyDeviceInfoList)(HDEVINFO DeviceInfoSet);
	Def_SetupDiDestroyDeviceInfoList* m_pfnSetupDiDestroyDeviceInfoList;
	typedef BOOL (Def_SetupDiEnumDeviceInfo)(HDEVINFO DeviceInfoSet, DWORD MemberIndex, PSP_DEVINFO_DATA DeviceInfoData);
	Def_SetupDiEnumDeviceInfo* m_pfnSetupDiEnumDeviceInfo;
	typedef BOOL (Def_SetupDiGetDeviceRegistryPropertyW)(HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, DWORD Property, PDWORD PropertyRegDataType, PBYTE PropertyBuffer, DWORD PropertyBufferSize, PDWORD RequiredSize);
	Def_SetupDiGetDeviceRegistryPropertyW* m_pfnSetupDiGetDeviceRegistryPropertyW;
	typedef HKEY (Def_SetupDiOpenDeviceInterfaceRegKey)(HDEVINFO DeviceInfoSet, PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData, DWORD Reserved, REGSAM samDesired);
	Def_SetupDiOpenDeviceInterfaceRegKey* m_pfnSetupDiOpenDeviceInterfaceRegKey;
	typedef CMAPI CONFIGRET (WINAPI Def_CM_Get_Parent)(PDEVINST pdnDevInst, DEVINST dnDevInst, ULONG ulFlags);
	Def_CM_Get_Parent* m_pfnCM_Get_Parent;
	typedef CMAPI CONFIGRET (WINAPI Def_CM_Get_Depth)(PULONG pulDepth, DEVINST dnDevInst, ULONG ulFlags);
	Def_CM_Get_Depth* m_pfnCM_Get_Depth;
	typedef CMAPI CONFIGRET (WINAPI Def_CM_Get_Device_IDA)(DEVINST dnDevInst, PWSTR Buffer, ULONG BufferLen, ULONG ulFlags);
	Def_CM_Get_Device_IDA* m_pfnCM_Get_Device_IDA;
	typedef CMAPI CONFIGRET (WINAPI Def_CM_Locate_DevNodeA)(PDEVINST pdnDevInst, DEVINSTID_W pDeviceID, ULONG ulFlags);
	Def_CM_Locate_DevNodeA* m_pfnCM_Locate_DevNodeA;
	typedef CMAPI CONFIGRET (WINAPI Def_CMP_WaitNoPendingInstallEvents)(DWORD dwTimeout);
	Def_CMP_WaitNoPendingInstallEvents* m_pfnCMP_WaitNoPendingInstallEvents;
	HWND m_hEnumNotifWnd;
	HMODULE m_hSetupApiLibrary;
	HANDLE m_EnumeratorThread;
	HANDLE m_Events[2];
};
