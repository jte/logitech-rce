#include "main.h"
#include <windows.h>
#include "CDrvApi.h"

void PrintDeviceInfo(DWORD dwDeviceDUID, PLD_DEVICE_INFO pDeviceInfo)
{
	char szDeviceName[128];
	memset(&szDeviceName, 0, sizeof(szDeviceName));
	CDrvApi::m_LD_GetDeviceName(dwDeviceDUID, szDeviceName, sizeof(szDeviceName));
	printf("Device %s:\n", szDeviceName);
	printf("\tDUID: %x\n", pDeviceInfo->dwDUID);
	printf("\tBus type: %x\n", pDeviceInfo->BusType);
	printf("\tDevice type: %x\n", pDeviceInfo->DeviceType);
	printf("\tModel: %x\n", pDeviceInfo->Model);
	printf("\tModel name: %s\n", pDeviceInfo->sModelName);
	printf("\tParent: %x\n", pDeviceInfo->ParentDevice);
	printf("\tPersistentName: %s\n", pDeviceInfo->sPersistentName);
}

void PrintFeatureList(DWORD dwDeviceDUID)
{
	/* Get Feature List Size */
	printf("PrintFeatureList %x\n", dwDeviceDUID);
	DWORD dwSize = 0;
	printf("m_LD_GetFeatureList:%x\n", CDrvApi::m_LD_GetFeatureList(-2, NULL, &dwSize));
	PDWORD pFeatureList = new DWORD[dwSize];
	memset(pFeatureList, 0, sizeof(DWORD)*dwSize);
	printf("m_LD_GetFeatureList$2:%x\n", CDrvApi::m_LD_GetFeatureList(-2, (LD_FEATURE_ID*)pFeatureList, &dwSize));
	for(size_t i = 0; i < dwSize; i++)
	{
		printf("Feature %d: %x\n", i, pFeatureList[i]);
	}
	delete[] pFeatureList;
}

int OnDeviceArrival(PLD_DEVICE pDevice, int* a, int b, int c)
{
	if(!a || !a[1])
	{
		static std::map<int, bool> devs;
		if(devs.find(a[3]) == devs.end())
		{
			if(pDevice)
			{
				printf("Battery Level: %d\n", pDevice->ulBatteryLevel);
				system("pause");
			}
			printf("arrived %p %p %p %p %x %x %x\n", pDevice, a, b, c, a[1], a[0], a[2]);
			LD_DEVICEINFO DeviceInfo;
			memset(&DeviceInfo, 0, sizeof(DeviceInfo));
			DeviceInfo.dwSize = sizeof(DeviceInfo);
			DeviceInfo.dwDUID = a[3];
			CDrvApi::m_LD_GetDeviceInfo(-1, &DeviceInfo);
			{ /* Set to D1 state (Full on) */
				LD_GET_POWER_STATUS_INFO pwrstats;
				pwrstats.dwSize = sizeof(pwrstats);
				pwrstats.PowerState = 1;
				CDrvApi::m_LD_DeviceControl(a[3], LD_SET_POWER_STATUS, (DWORD*)&pwrstats);
			}
			PrintDeviceInfo(a[3], &DeviceInfo);
			PrintFeatureList(a[3]);
			{ /* Check power stats */
				LD_GET_POWER_STATUS_INFO pwrstats;
				memset(&pwrstats, 0, sizeof(pwrstats));
				pwrstats.dwSize = sizeof(pwrstats);
				CDrvApi::m_LD_DeviceControl(a[3], LD_GET_POWER_STATUS, (DWORD*)&pwrstats);
				printf("pad1: %x\n"
					   "pad2: %x\n"
					   "PowerState: %x\n"
					   "ulBatteryLevel: %x\n"
					   "ulBatteryRange: %x\n"
					   "ulRawBatteryValue: %x\n", 
					pwrstats.pad1, 
					pwrstats.pad2,  
					pwrstats.PowerState,
					pwrstats.ulBatteryLevel,
					pwrstats.ulBatteryRange,
					pwrstats.ulRawBatteryValue
					);

			}
			devs[a[3]] = true;
		}	
	}
	return 1;
}

int GetCapability(DWORD dwDeviceDUID, const char* szCaps)
{
	LD_GET_CAPABILITY_INFO
		caps;
	caps.dwSize = sizeof(LD_GET_CAPABILITY_INFO);
	caps.szCaps = szCaps;
	 CDrvApi::m_LD_DeviceControl(dwDeviceDUID, 8, (DWORD*)&caps);
	return caps.dwOutput;
}
std::map<DWORD,bool> g_devicesdiscovered;

void __cdecl DevDumpCB(int/*Extra1*/, int/*Extra2*/, int/*Extra3*/, int Prefix , DWORD Name , DWORD Type, signed int, signed int, int Value )
{
	printf("%s %s%s=%x\n", (char*)Type, (char*)Prefix, (char*)Name, (void*)Value);
}

int OnEnumerationComplete(PLD_DEVICE pDevice, int* a, int b, int c)
{
	if(pDevice)
		pDevice->CapsList[9] = 1;
	
		//DWORD dwDeviceDUID = 0;
		// CDrvApi::m_LD_GetFirstDevicePosition(&dwDeviceDUID);
		//while(dwDeviceDUID && dwDeviceDUID != -1)
		//{
		//	if(g_devicesdiscovered.find(dwDeviceDUID) == g_devicesdiscovered.end())
		//	{
		//		g_devicesdiscovered[dwDeviceDUID] = true;

		//		//LD_DEVICE_DUMP dmpinf;
		//		//memset(&dmpinf, 0, sizeof(dmpinf));
		//		//dmpinf.pCallback = DevDumpCB;
		//	//	CDrvApi::m_LD_DeviceControl(dwDeviceDUID, LD_DUMP_DEVICE, (DWORD*)&dmpinf);
		//	
		//		//if(DeviceInfo.ParentDevice == 0xFFFFFFFF)
		//		/*{
		//			// transceiver /
		//			typedef struct
		//			{
		//				DWORD dwSize;
		//				BYTE dwBuffer[7];
		//			} wdi;
		//			wdi wdibuf = { {7+4}, {0x10,0xff,0x80,0x00,0xff,0xff,0xff}};
		//			CDrvApi::m_LD_DeviceControl(dwDeviceDUID, LD_DEVICE_WRITE, (DWORD*)&wdibuf);
		//		}
		//		*/
		//		LD_DEVICEINFO DeviceInfo;
		//		DeviceInfo.dwSize = sizeof(LD_DEVICEINFO);
		//		 CDrvApi::m_LD_GetDeviceInfo(dwDeviceDUID, &DeviceInfo);
		//		PrintDeviceInfo(dwDeviceDUID, &DeviceInfo);
		//		{ /* Set to D1 state (Full on) */
		//			LD_GET_POWER_STATUS_INFO pwrstats;
		//			pwrstats.PowerState = 1;
		//			CDrvApi::m_LD_DeviceControl(dwDeviceDUID, LD_SET_POWER_STATUS, (DWORD*)&pwrstats);
		//		}
		//	
		//		//if(0 && DeviceInfo.ParentDevice == 0xFFFFFFFF)
		//		/*
		//		{
		//			// transceiver
		//			LD_GETSET_REGISTER_INFO regset;
		//			regset.Address = 0xB5;
		//			regset.lpOut = (DWORD*)new BYTE[128];
		//			regset.lpBuf = (DWORD*)new BYTE[128];
		//			memset(regset.lpOut, 0, 128*sizeof(BYTE));
		//			memset(regset.lpBuf, 0, 128*sizeof(BYTE));
		//			*(BYTE*)regset.lpBuf = 0x20;
		//			*(BYTE*)regset.lpOut = 0x20;
		//			regset.Length = 128;
		//			 CDrvApi::m_LD_DeviceControl(dwDeviceDUID, LD_GET_REGISTER, (DWORD*)&regset);
		//			printf("exec$1\n");
		//			for(size_t i = 0; i < 64; i++)
		//			{
		//				if(*(BYTE*)(regset.lpOut + i) != 0)
		//				{
		//					printf("O:%d:%c %d %x\n",i,*(BYTE*)(regset.lpOut + i),*(BYTE*)(regset.lpOut + i),*(BYTE*)(regset.lpOut + i));
		//				}
		//				if(*(BYTE*)(regset.lpBuf + i) != 0)
		//				{
		//					printf("B:%d:%c %d %x\n",i,*(BYTE*)(regset.lpBuf + i),*(BYTE*)(regset.lpBuf + i),*(BYTE*)(regset.lpBuf + i));
		//					system("pause");
		//				}
		//			
		//			}
		//			printf("exec$2\n");
		//		}*/
		//	
		//		printf("BATTERY: %x\n", GetCapability(dwDeviceDUID, "BATTERY"));
		//		printf("BATTERY_MILEAGE: %x\n", GetCapability(dwDeviceDUID, "BATTERY_MILEAGE"));
		//		printf("QUAD: %x\n", GetCapability(dwDeviceDUID, "QUAD"));
		//		printf("HPP2PROBED: %x\n", GetCapability(dwDeviceDUID, "HPP2PROBED"));
		//		printf("PGM_TEMPERATURE: %x\n", GetCapability(dwDeviceDUID, "PGM_TEMPERATURE"));
		//		printf("PGM_POWER_LEVEL: %x\n", GetCapability(dwDeviceDUID, "PGM_POWER_LEVEL"));
		//		printf("REPORT_RATE: %x\n", GetCapability(dwDeviceDUID, "REPORT_RATE"));
		//		printf("RECHARGABLE: %x\n", GetCapability(dwDeviceDUID, "RECHARGABLE"));
		//		{
		//			PrintFeatureList(dwDeviceDUID);
		//		}
		//		{
		//			LD_GET_POWER_STATUS_INFO powerstats;
		//			powerstats.dwSize = 0x18;
		//			 CDrvApi::m_LD_DeviceControl(dwDeviceDUID, LD_GET_POWER_STATUS, (DWORD*)&powerstats);
		//			printf("PowerState:%d\n", powerstats.PowerState);
		//			printf("ulBatteryLevel:%d\n", powerstats.ulBatteryLevel);
		//			printf("ulBatteryRange:%d\n",powerstats.ulBatteryRange);
		//			printf("ulRawBatteryValue:%d\n",powerstats.ulRawBatteryValue);
		//			LD_GET_BATTERY_STATUS_INFO battstat;
		//			battstat.dwSize = sizeof(battstat);
		//			 CDrvApi::m_LD_DeviceControl(dwDeviceDUID, LD_GET_BATTERY_STATUS, (DWORD*)&battstat);
		//			printf("Info:%d\n",battstat.dwInfo);
		//			printf("Battery status: %d\n",battstat.dwBatteryStatus);
		//		}
		//	
		//	}
		//	 CDrvApi::m_LD_GetNextDevicePosition(&dwDeviceDUID);
		//}
		////static bool done = false;
		////if(!done)
		 CDrvApi::m_LD_DeviceControl(-2,11,0)/*, done=true*/;
//	}
	return 1;
}

int OnUnknownDevice(PLD_DEVICE pDevice, int*, int, int)
{
	printf("pDevice: %p", pDevice);
	system("pause");
	return 1;
}

int OnCSChanged(PLD_DEVICE pDevice,int*,int,int)
{
	printf("pDevice: %p", pDevice);
	return 1;
}

int OnUserSession(PLD_DEVICE pDev,int*,int,int)
{
	printf("WHAT?");
	return 1;
}

void Hidpp::Initialize()
{
	if(CDrvApi::Initialize())
	{
		printf("Registering device events\n");
		{	/* OnUnknownDevice */
			LD_EVENT_REGISTRATION Event;
			memset(&Event, 0, sizeof(Event));
			Event.EventType = LD_EVENT_UNKNOWN_DEVICE;
			Event.DUID = 0;
			Event.Param = 0;
			Event.ExtraCallbackParam = 0;
			Event.Callback = 0;//OnUnknownDevice;
			Event.pad4 = 0;
			Event.hNotifyWnd = m_hWnd;
			Event.pSize = sizeof(Event);
			Event.Flags = 0x10000;
			Event.ulNotifyMsg = WM_USER;
			 CDrvApi::m_LD_RegisterEvent(&Event);
		}
		{	/* OnDeviceArrival */
			LD_EVENT_REGISTRATION Event;
			memset(&Event, 0, sizeof(Event));
			Event.pSize = sizeof(Event);
			Event.DUID = 0;
			Event.EventType = LD_EVENT_DEVICE_ARRIVAL;
			Event.Param = 0;
			Event.ExtraCallbackParam = 0;
			Event.Flags = 0x20000;
			Event.hNotifyWnd = 0;
			Event.ulNotifyMsg = 0;
			Event.Callback = OnDeviceArrival;
			Event.pad4 = 0;
			 CDrvApi::m_LD_RegisterEvent(&Event);
		}
		{	/* OnEnumerationComplete */
			LD_EVENT_REGISTRATION Event;
			memset(&Event, 0, sizeof(Event));
			Event.EventType = LD_EVENT_ENUMERATION_COMPLETE;
			Event.pSize = sizeof(Event);
			Event.DUID = 0;
			Event.Param = 0;
			Event.ExtraCallbackParam = 0;
			Event.Flags = 0x20000;
			Event.hNotifyWnd = 0;
			Event.ulNotifyMsg = 0;
			Event.Callback = OnEnumerationComplete;
			Event.pad4 = 0;
			 CDrvApi::m_LD_RegisterEvent(&Event);
		}
		CDrvApi::m_LD_DeviceControl(-2,11,0);
		printf("Initialized\n");
	}
	else
	{
		printf("\nFailed to init; Is SetPoint running?\n");
	}
}

