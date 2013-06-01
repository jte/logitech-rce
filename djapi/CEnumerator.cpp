#include "CEnumerator.h"
#include "hid.h"
#include <dbt.h>
#include "CHidDevice.h"

#pragma push_macro("RegisterDeviceNotification")
#undef RegisterDeviceNotification


CEnumerator::CEnumerator()
{
	m_Epsilon.Init();
	InitializeCriticalSection(&m_DeviceOpCS);
	m_hEnumHidDevicesEvent = CreateEventW(0, 1, 0, 0);
	pThis->field_1A4 = (int)"++";
	pThis->field_E0 = 0;
	m_EnumerateHidDevicesThreadID = 0;
	pThis->field_A0 = 0;
	pThis->field_E8 = 0;
	pThis->field_EC = 0;
	m_hRawDeviceHandle = 0;
	wcscpy_s(m_szLanguage, 80u, L"enu");
	m_DisableUpdater = 0;
	pThis->field_19C = 0;
	m_ModelID = 0;
	m_EnumerationStarted = 0;
}

bool CEnumerator::StartEnumeration(HINSTANCE hInstance)
{
	if(ResolveHidDllFunctions(hInstance))
	{
		RunUpdater(0);
		m_hDevNotify = RegisterDeviceNotification(m_HidGuid);
		SetEvent(m_hEnumHidDevicesEvent);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)CEnumerator::EnumeratorThreadStatic, (LPVOID)this, 0, &m_EnumerateHidDevicesThreadID);
		m_EnumerationStarted = 1;
		return true;
	}
	return false;
}

HDEVNOTIFY CEnumerator::RegisterDeviceNotification(GUID HidGuid)
{
	DEV_BROADCAST_DEVICEINTERFACE broadcastInterface = {0};   
   
    broadcastInterface.dbcc_size = sizeof(broadcastInterface);   
    broadcastInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
#pragma pop_macro("RegisterDeviceNotification")
	return RegisterDeviceNotification(m_hEnumNotifWnd, &broadcastInterface, 0);
#pragma push_macro("RegisterDeviceNotification")
}

DWORD CALLBACK CEnumerator::EnumeratorThreadStatic(LPVOID pEnumerator)
{
	CEnumerator* pThis = (CEnumerator*)pEnumerator;
	return pThis->EnumeratorThread();
}

int CEnumerator::EnumeratorThread()
{
	StartEnum(0);
	WaitForSingleObject(m_hEnumHidDevicesEvent, 30000);
	InitiateEventCallback(NotificationEvents::OnEnumerationComplete, 0, 0);
	pad4 = 1;
	m_EnumerateHidDevicesThreadID = 0;
	DFUController::GetSingleton()->UnknownX();
	return 0;
}

int CEnumerator::StartEnum(int a1)
{
	return EnumHidDevices(m_HidGuid);
}

int CEnumerator::EnumHidDevices(GUID HidGuid)
{
	HDEVINFO
		DeviceInfoSet = m_pfnSetupDiGetClassDevsW(&HidGuid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if(DeviceInfoSet == (HDEVINFO)-1)
	{
		return 0;
	}
	SP_DEVICE_INTERFACE_DATA
		DeviceInterfaceData;
	memset(&DeviceInterfaceData, 0, sizeof(DeviceInterfaceData));
	DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
	for(size_t MemberIndex = 0; m_pfnSetupDiEnumDeviceInterfaces(DeviceInfoSet, NULL, &HidGuid, MemberIndex, &DeviceInterfaceData); MemberIndex++)
	{
		SIZE_T DeviceInterfaceDetailDataSize;
		pfnSetupDiGetDeviceInterfaceDetailW(DeviceInfoSet, &DeviceInterfaceData, 0, 0, &DeviceInterfaceDetailDataSize, 0);
		HANDLE hProcessHeap = GetProcessHeap();
		PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)HeapAlloc(hProcessHeap, HEAP_ZERO_MEMORY, DeviceInterfaceDetailDataSize);
		if(DeviceInterfaceDetailData)
		{
			memset(DeviceInterfaceDetailData, 0, DeviceInterfaceDetailDataSize);
			DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			SP_DEVINFO_DATA DeviceInfoData;
			DeviceInfoData.cbSize = sizeof(DeviceInfoData);
			SIZE_T RequiredSize = 0;
			if(m_pfnSetupDiGetDeviceInterfaceDetailW(DeviceInfoSet, &DeviceInterfaceData, DeviceInterfaceDetailData, DeviceInterfaceDetailDataSize,
                 &RequiredSize, &DeviceInfoData))
			{
				bool bKeepProcessing = OnEnumFindDevice(DeviceInfoSet, &DeviceInfoData, DeviceInterfaceDetailData, v21);
				HeapFree(hProcessHeap, 0, DeviceInterfaceDetailData);
				if(!bKeepProcessing)
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	m_pfnSetupDiDestroyDeviceInfoList(DeviceInfoSet);
	return 1;
}

int CEnumerator::OnEnumFindDevice(HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData, int a5)
{
	if(PowerUp(0, DeviceInfoData->DevInst))
	{
		return 1;
	}
	return AddDevice(DeviceInfoSet, DeviceInfoData, DeviceInterfaceDetailData, a5);
}

int CEnumerator::PowerUp(int a2, PDEVINST pdnDevInst)
{
	EnterCriticalSection(&m_DeviceOpCS);
	Epsilon_t::Unknown7(&pThis->Epsilon, (int)&v10);
	while ( 1 )
	{
	v3 = Epsilon_t::Unknown2(&pThis->Epsilon, (CDFULite1 *)&v8);
	if ( !CDFULite1::Unknown18(&v10, v3) )
		break;
	if ( *(_DWORD *)(sub_10002140(&v10) + 4) )
	{
		v4 = sub_10002140(&v10);
		if ( (*(int (__thiscall **)(_DWORD, int, PDEVINST))(**(_DWORD **)(v4 + 4) + 20))(
				*(_DWORD *)(v4 + 4),
				a2,
				pdnDevInst) )
		{
		v5 = sub_10002140(&v10);
		v11 = CDevice::GetRawDeviceHandle(*(CReceiver **)(v5 + 4));
		break;
		}
	}
	CDFULite1::Unknown0(&v10, (int)&v9, 0);
	}
	LeaveCriticalSection(&pThis->DeviceOpCS);
	return v11;
}

int CEnumerator::AddDevice(HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData, int a5)
{
	wchar_t FullDevPath[MAX_PATH];
	wcscpy_s(FullDevPath, MAX_PATH, (const wchar_t *)DeviceInterfaceDetailData->DevicePath);
	wcscat_s(FullDevPath, MAX_PATH, L"\\DJAPI");
	HANDLE hDevObject = CreateFileW(FullDevPath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	int res = hDevObject != (HANDLE)-1;
	if(res)
	{
		HIDD_ATTRIBUTES Attributes;
		res = m_pfnHidD_GetAttributes(hDevObject, &Attributes);
		if(res == 1)
		{
			res = OnAddDevice(DeviceInfoData->DevInst, hDevObject, DeviceInterfaceDetailData->DevicePath, &Attributes, a5);
		}
	}
	if(!res)
	{
		CloseHandle(hDevObject);
	}
	return 1;
}

int CEnumerator::OnAddDevice(DWORD DevInst, HANDLE HidDeviceObject, CHAR* DevicePath, HIDD_ATTRIBUTES *Attributes, int a6)
{
	if(Attributes->VendorID != VENDOR_ID_LOGITECH || MatchProductIDs(this, Attributes->ProductID, (int)&v8) == 7)
	{
		return 0;
	}
	return ProcessAddedDevice(DevInst, HidDeviceObject, DevicePath, Attributes, a6);
}

int CEnumerator::ProcessAddedDevice(DWORD DevInst, HANDLE HidDeviceObject, CHAR* DevicePath, HIDD_ATTRIBUTES *Attributes, int a6)
{
	HIDP_PREPARSED_DATA PreparsedData;
	int res = 0;
	if(m_pfnHidD_GetPreparsedData(HidDeviceObject, &PreparsedData))
	{
		HIDP_CAPS Capabilities;
		if(m_pfnHidP_GetCaps(PreparsedData, &Capabilities) >= 0)
		{
			if(Capabilities.UsagePage != HID_USAGE_PAGE_LOGITECH_1 || Capabilities.Usage != 1 && Capabilities.Usage != 2)
			{
				if(Capabilities.UsagePage == HID_USAGE_PAGE_LOGITECH_2 && Capabilities.Usage == 1)
				{
					res = OnAddUnifyingReceiver(DevInst, HidDeviceObject, DevicePath, Attributes, PreparsedData, &Capabilities, a6);
				}
			}
			else
			{
				res = OnAddReceiver(DevInst, HidDeviceObject, DevicePath, Attributes, PreparsedData, &Capabilities, a6);
			}
		}
		m_pfnHidD_FreePreparsedData(PreparsedData);
	}
	return res;
}

int CEnumerator::ResolveHidDllFunctions(HINSTANCE hInstance)
{
	m_hHidLibrary = LoadLibraryW(L"Hid.dll");
#define RESOLVE_HID_FUNC(def) do{if(!(m_pfn##def = (def*)GetProcAddress(m_hHidLibrary, #def)))return 0;}while(0)
	if(m_hHidLibrary)
	{
		RESOLVE_HID_FUNC(HidP_GetSpecificButtonCaps);
		RESOLVE_HID_FUNC(HidP_GetLinkCollectionNodes);
		RESOLVE_HID_FUNC(HidP_GetUsages);
		RESOLVE_HID_FUNC(HidP_SetUsages);
		RESOLVE_HID_FUNC(HidP_UnsetUsages);
		RESOLVE_HID_FUNC(HidP_GetUsagesEx);
		RESOLVE_HID_FUNC(HidP_TranslateUsagesToI8042ScanCodes);
		RESOLVE_HID_FUNC(HidP_GetUsageValue);
		RESOLVE_HID_FUNC(HidP_GetCaps);
		RESOLVE_HID_FUNC(HidP_GetButtonCaps);
		RESOLVE_HID_FUNC(HidP_GetValueCaps);
		RESOLVE_HID_FUNC(HidD_GetHidGuid);
		RESOLVE_HID_FUNC(HidD_GetPreparsedData);
		RESOLVE_HID_FUNC(HidD_FreePreparsedData);
		RESOLVE_HID_FuNC(HidD_GetAttributes);
		RESOLVE_HID_FUNC(HidD_GetSerialNumberString);
		RESOLVE_HID_FUNC(HidD_GetIndexedString);
		m_pfnHidD_GetHidGuid(&m_HidGuid);
		return CreateEnumNotifWnd(hInstance);
	}
	return 0;
#undef RESOLVE_HID_FUNC
}



int CEnumerator::OnAddReceiver(DEVINST dnDevInst, HANDLE HidDeviceObject, const wchar_t *DevicePath, HIDD_ATTRIBUTES *Attributes, int PreparsedData, HIDP_CAPS *Capabilities, int a8)
{
	pReceiver = 0;
	v31 = 0;
	v30 = 0;
	v26 = 0;
	DEVINST* pdnDevInst;
	DEVINST dnDevInst;
	m_pfnCM_Get_Parent(&pdnDevInst, dnDevInst, 0);
	DWORD a2 = PowerUp(0, (PDEVINST)pdnDevInst);
	if ( a2 )
	{
		pReceiver = (CReceiver *)GetDeviceHandle(a2);
		v30 = 1;
	}
	else
	{
		v29 = MatchProductIDs(pThis, Attributes->ProductID, (int)&v26);
		switch ( v29 )
		{
			case 4:                                   // CDJReceiver
				pReceiver = (CReceiver *)new CDJReceiver(pdnDevInst, 4);
			break;
			case 3:                                   // CEQuadReceiver
				pReceiver = (CReceiver *)new CEQuadReceiver(pdnDevInst, 3);
			break;
			case 2:                                   // CEQuadReceiver
				pReceiver = (CReceiver *)new CEQuadReceiver(pdnDevInst, 2);
			break;
			case 9:                                   // CDJGamingReceiver
				pReceiver = (CReceiver *)new CDJGamingReceiver(pdnDevInst, 9);
			break;
			case 0:                                   // CPalomarReceiver
				pReceiver = (CReceiver *)new CPalomarReceiver(pdnDevInst);
			break;
			case 10:                                  // CEQuadReceiver
				pReceiver = (CReceiver *)new CEQuadReceiver(pdnDevInst, 10);
			break;
			default:
			break;
		}
		if(pReceiver && !LinkRawDeviceHandleWithDevice(pReceiver, 0xFFu))
		{
			if(pReceiver)
			{
				delete pReceiver;
			}
			pReceiver = 0;
		}
	}
	if (pReceiver)
	{
		char
			ReportID = GetReportID(PreparsedData, Capabilities);
		if(!CHPPDevice::IsDeviceWithMatchingReportID((CHPPDevice *)&pReceiver->HPPDevice, ReportID))
		{
			HDEVNOTIFY
				hDevNotify = CEnumerator::GetRegisteredDevNotify(pThis, (int)HidDeviceObject);
			v31 = CHPPDevice::AppendHidDevice((CHPPDevice *)&pReceiver->HPPDevice,dnDevInst,(int)HidDeviceObject,DevicePath,Capabilities,ReportID,hDevNotify);
			v23 = CHPPDevice::AreDevicesActive((CHPPDevice *)&pReceiver->HPPDevice, v26);
			if ( v31 )
			{
				if ( v23 )
				{
					(*(void (__thiscall **)(CReceiver *, HIDD_ATTRIBUTES *))(pReceiver->__parent.vtbl + 0x2C))(pReceiver,Attributes);
					(*(void (__thiscall **)(CReceiver *))(pReceiver->__parent.vtbl + 4))(pReceiver);
					CReceiver::ReceiverScan(pReceiver);
				}
			}
		}
		if ( v30 )
		{
			pReceiver->DecrementRefCount();
		}
	}
	return v31;
}

int CEnumerator::OnAddUnifyingReceiver(DEVINST dnDevInst, HANDLE HidDeviceObject, const wchar_t *DevicePath, PHIDD_ATTRIBUTES Attributes, int PreparsedData, HIDP_CAPS *Capabilities, int a8)
{
	if(PowerUp(0, dnDevInst))
	{
		return 0;
	}
	CHidDevice* pHidDevice = new CHidDevice(dnDevInst);
	if(!pHidDevice)
	{
		return 0;
	}
	if(CEnumerator::LinkRawDeviceHandleWithDevice(pThis, (CReceiver *)pHidDevice, 0xFFu))
	{
		char 
			ReportID = GetReportID(PreparsedData, Capabilities);
		HDEVNOTIFY
			hDevNotify = GetRegisteredDevNotify(HidDeviceObject);
		if (pHidDevice->Setup(HidDeviceObject, ReportID, (int)Capabilities, Attributes, DevicePath, hDevNotify) )
		{
			pHidDevice->GenerateDeviceArrivalEvent();
			if(m_ModelID)
			{
				pThis->field_EC = CDevice::GetRawDeviceHandle((CReceiver *)pHidDevice);
				v11 = (DFU *)DFUController::GetSingleton();
				DFUController::UnknownY(v11, 6);
			}
		}
		return 1;
	}
	if(pHidDevice)
	{
		delete pHidDevice;
	}
	return 0;
}

char CEnumerator::GetReportID(PHIDP_PREPARSED_DATA PreparsedData, HIDP_CAPS *a3)
{
	char ReportID = -1;
	USHORT
		ButtonCapsLength = a3->NumberInputValueCaps;
	if(ButtonCapsLength)
	{
		PHIDP_BUTTON_CAPS 
			ButtonCaps = malloc(sizeof(HIDP_BUTTON_CAPS) * ButtonCapsLength);
		if ( ButtonCaps )
		{
			if(m_pfnHidP_GetButtonCaps(0, ButtonCaps, &ButtonCapsLength, PreparsedData) >= 0)
			{
				ReportID = ButtonCaps->ReportID;
			}
			free(ButtonCaps);
		}
	}
	return ReportID;
}

HDEVNOTIFY CEnumerator::GetRegisteredDevNotify(HANDLE HidDeviceObject)
{
	DEV_BROADCAST_HANDLE NotificationFilter;

	memset(&NotificationFilter, 0, sizeof(DEV_BROADCAST_HANDLE));
	NotificationFilter.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
	NotificationFilter.dbch_devicetype = 6;
	NotificationFilter.dbch_handle = HidDeviceObject;

	return RegisterDeviceNotificationW(m_hEnumNotifWnd, &NotificationFilter, 0);
}


#pragma pop_macro("RegisterDeviceNotification")
