//-------------------------------------------------------------------------- 
// 
// Include file for HidAPI.cpp 
// 
//-------------------------------------------------------------------------- 
// $Archive: /WirelessUSB/LS2/Firmware/LP KBM RDK/Software/Source Code/CUSBHidAPI/HidAPI.h $ 
// $Modtime: 10/01/04 2:03p $ 
// $Revision: 2 $ 
//-------------------------------------------------------------------------- 
// 
// Copyright 2003-2004, Cypress Semiconductor Corporation. 
// 
// This software is owned by Cypress Semiconductor Corporation (Cypress) 
// and is protected by and subject to worldwide patent protection (United 
// States and foreign), United States copyright laws and international 
// treaty provisions. Cypress hereby grants to licensee a personal, 
// non-exclusive, non-transferable license to copy, use, modify, create 
// derivative works of, and compile the Cypress Source Code and derivative 
// works for the sole purpose of creating custom software in support of 
// licensee product to be used only in conjunction with a Cypress integrated 
// circuit as specified in the applicable agreement. Any reproduction, 
// modification, translation, compilation, or representation of this 
// software except as specified above is prohibited without the express 
// written permission of Cypress. 
// 
// Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
// WITH REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. 
// Cypress reserves the right to make changes without further notice to the 
// materials described herein. Cypress does not assume any liability arising 
// out of the application or use of any product or circuit described herein. 
// Cypress does not authorize its products for use as critical components in 
// life-support systems where a malfunction or failure may reasonably be 
// expected to result in significant injury to the user. The inclusion of 
// Cypress?product in a life-support systems application implies that the 
// manufacturer assumes all risk of such use and in doing so indemnifies 
// Cypress against all charges. 
// 
// Use may be limited by and subject to the applicable Cypress software 
// license agreement. 
// 
//-------------------------------------------------------------------------- 
#ifndef _HIDAPI_H_ 
#define _HIDAPI_H_ 
 
#include <setupapi.h> 
 
extern "C" { 
// declare the WDMDDL C libraries used 
#include "hidsdi.h" 
} 
 
#define HIDCB_NOTIFY_DEVICE_REGISTERED		0x0000 
#define HIDCB_NOTIFY_DEVICE_UNREGISTERED	0x0001 
#define HIDCB_NOTIFY_DEVICE_ADDED			0x0002 
#define HIDCB_NOTIFY_DEVICE_REMOVED			0x0003 
 
#define WM_UNREGISTER_HANDLE				WM_USER+1 
 
#define HIDMGR_SUPPORTED_HID_DEVICES		0xFF 
 
 
// 
// structure to hold the state of the data for the Hid device.  each 
// time a packet is received, the structure gets filled in.  each time 
// a packet is sent, this structure gets filled in. 
// 
typedef struct _HID_DATA { 
   bool        IsButtonData; 
   UCHAR       Reserved; 
   USAGE       UsagePage;   // usage page assiciated with this data 
   ULONG       Status;      // last status returned  
   ULONG       ReportID;    // ReportID this data structure 
   bool        IsDataSet;   // boolean to determine if this data structure 
                            // has already been added to a report structure 
 
   union { 
      struct { 
         ULONG       UsageMin;       // usage minimum  
         ULONG       UsageMax;       // usage maximum 
         ULONG       MaxUsageLength; // usages buffer length 
         PUSAGE      Usages;         // list of usages 
 
      } ButtonData; 
      struct { // usage describing this value 
         USAGE       Usage;			 
         USHORT      Reserved; 
         ULONG       Value; 
         LONG        ScaledValue; 
      } ValueData; 
   }; 
} HID_DATA, *PHID_DATA; 
 
 
class CHidDevice 
{ 
// construction/destruction 
public: 
	CHidDevice(void) {} 
	~CHidDevice(void) 
		{  
			DestroyHidDevice();  
		} 
 
    bool InitializeHidDevice(PCHAR pDevicePath, USHORT uID = 0xFFFF); 
    void DestroyHidDevice(void); 
 
// operations 
public: 
	bool OpenHidDevice(bool bReadAccess = true, bool bWriteAccess = true, bool bOverlapped = false, bool bExclusive = false); 
	bool CloseHidDevice(void); 
 
	bool RegisterHidDevice(HWND hWnd); 
 
	bool IsOpened(void) {return m_bOpened; } 
	bool IsOpenForRead(void) { return m_bOpenedForRead; } 
	bool IsOpenForWrite(void) { return m_bOpenedForWrite; } 
	bool IsOpenOverlapped(void) { return m_bOpenedOverlapped; } 
	bool IsOpenExclusive(void) { return m_bOpenedExclusive; } 
 
	HANDLE GetDeviceHandle(void) { return m_hDevice; } 
 
 
	bool Read(void); 
	bool Write(void); 
 
	bool GetFeature(void); 
	bool SetFeature(void); 
 
	bool GetReport( 
				   HIDP_REPORT_TYPE ReportType, 
				   PCHAR ReportBuffer, 
				   USHORT ReportBufferLength, 
				   PHID_DATA Data, 
				   ULONG DataLength, 
				   PHIDP_PREPARSED_DATA Ppd 
				  ); 
	bool SetReport( 
				   HIDP_REPORT_TYPE ReportType, 
				   PCHAR ReportBuffer, 
				   USHORT ReportBufferLength, 
				   PHID_DATA Data, 
				   ULONG DataLength, 
				   PHIDP_PREPARSED_DATA Ppd 
				  ); 
 
	bool UnpackReport( 
					  HIDP_REPORT_TYPE ReportType, 
					  PCHAR ReportBuffer, 
					  USHORT ReportBufferLength, 
					  PHID_DATA Data, 
					  ULONG DataLength, 
					  PHIDP_PREPARSED_DATA Ppd 
					 ); 
	bool PackReport( 
					HIDP_REPORT_TYPE ReportType, 
					PCHAR ReportBuffer, 
					USHORT ReportBufferLength, 
					PHID_DATA Data, 
					ULONG DataLength, 
					PHIDP_PREPARSED_DATA Ppd 
				   ); 
 
	bool GetManufacturerString(PCHAR szManufacturerString, ULONG ulManufacturerStringLength); 
	bool GetProductString(PCHAR szProductString, ULONG ulProductStringLength); 
	bool GetSerialNumberString(PCHAR szSerialNumberString, ULONG ulSerialNumberStringLength); 
 
	bool RegQueryValue(LPCTSTR lpValueName, 
						LPDWORD lpType, 
						LPBYTE lpData, 
						LPDWORD lpcbData); 
	bool RegSetValue(LPCTSTR lpValueName, 
						DWORD Type, 
						LPBYTE lpData, 
						DWORD cbData); 
 
// operations data 
public: 
	CHidDevice				*nextHidDevice; 
 
	PCHAR					m_pHidDevicePath; 
 
	PHIDP_PREPARSED_DATA	m_pHidPpd; // The opaque parser info describing this device 
	HIDP_CAPS				m_HidCaps; // The Capabilities of this hid device. 
	HIDD_ATTRIBUTES			m_HidAttributes; 
 
    PCHAR					m_pInputReportBuffer; 
    PHID_DATA				m_pInputData; // array of hid data structures 
    ULONG					m_ulInputDataLength; // Num elements in this array. 
    PHIDP_BUTTON_CAPS		m_pInputButtonCaps; 
    PHIDP_VALUE_CAPS		m_pInputValueCaps; 
 
    PCHAR					m_pOutputReportBuffer; 
    PHID_DATA				m_pOutputData; 
    ULONG					m_ulOutputDataLength; 
    PHIDP_BUTTON_CAPS		m_pOutputButtonCaps; 
    PHIDP_VALUE_CAPS		m_pOutputValueCaps; 
 
    PCHAR					m_pFeatureReportBuffer; 
    PHID_DATA				m_pFeatureData; 
    ULONG					m_ulFeatureDataLength; 
    PHIDP_BUTTON_CAPS		m_pFeatureButtonCaps; 
    PHIDP_VALUE_CAPS		m_pFeatureValueCaps; 
 
	USHORT					m_uDeviceID; 
 
// implementation 
protected: 
	bool SetupHidDevice(void); 
	virtual bool VerifyHidDevice(void) { return true; } // override to add validation 
 
// implementation data 
protected: 
	bool					m_bOpened; 
	HANDLE					m_hDevice; 
	HANDLE					m_hDeviceRegistered; 
	HWND					m_hWnd; 
 
	bool					m_bOpenedForRead; 
	bool					m_bOpenedForWrite; 
	bool					m_bOpenedOverlapped; 
	bool					m_bOpenedExclusive; 
}; 
 
 
typedef void (*PHidCallback)(void *context, CHidDevice *pHidDevice, UINT uHidCode); 
 
class CHidManager 
{ 
// construction/destruction 
public: 
	CHidManager(); 
	~CHidManager(void); 
 
// operations 
public: 
	bool Create(HINSTANCE hInst, HWND hParent, PHidCallback pHidCallback = NULL, void *pHidCallbackContext = NULL); 
	bool IsHidDevicePresent(CHidDevice *pHidDevice); 
	bool RefreshHidDevices(void); 
	UINT GetDeviceCount(void)  
		{ return m_nHidDevices; }; 
 
	CHidDevice *GetFirstHidDevice(void); 
	CHidDevice *GetNextHidDevice(void); 
	CHidDevice *GetCurrentHidDevice(void); 
 
	CHidDevice *GetHidDeviceWithPath(PCHAR pDevicePath); 
	CHidDevice *GetHidDeviceWithHandle(HANDLE hHidDevice); 
 
	bool HidDeviceAlreadyExist(CHidDevice *pHidDevice); 
	bool AddHidDevice(CHidDevice *pHidDevice); 
	bool RemoveHidDevice(CHidDevice *pHidDevice); 
	void RemoveAllHidDevices(void); 
 
	USHORT CreateUniqueDeviceID(void); 
	void FreeUniqueDeviceID(USHORT uID); 
 
	HWND GetSafeHwnd() const   
		{ return (this)? m_hWnd : NULL; } 
 
// implementation 
protected: 
    void InitializeHidManager(void); 
 
	virtual CHidDevice *NewHidDevice(PCHAR pDevicePath) // override to provide new CHidDevice 
		{  
			CHidDevice *pNewHidDevice = new CHidDevice();  
 
			if (pNewHidDevice) 
				pNewHidDevice->InitializeHidDevice(pDevicePath); 
 
			return pNewHidDevice; 
		} 
 
	virtual void DeleteHidDevice(CHidDevice *pHidDevice) // override to remove CHidDevice 
		{  
			delete pHidDevice;  
		} 
 
	ATOM RegisterClass(HINSTANCE hInstance); 
 
	bool RegisterHidNotification(HWND hWnd); 
 
	void HidDeviceArrival(PCHAR pDevicePath); 
	 
	void HidDeviceQueryRemoval(PCHAR pDevicePath); 
	void HidDeviceQueryRemoval(HANDLE hHidDevice); 
 
	void HidDeviceRemoval(PCHAR pDevicePath); 
	void HidDeviceRemoval(HANDLE hHidDevice); 
 
// implementation static functions 
protected: 
	static LRESULT PASCAL WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); 
 
 
// implementation data 
protected: 
	BOOL					m_bEnabled; 
	HWND					m_hWnd; 
	HANDLE					m_hDeviceNotification; 
 
	PHidCallback			m_pHidCallback; 
	void*					m_pHidCallbackParam; 
 
	CHidDevice*				m_pHidDeviceListHead; 
	CHidDevice*				m_pCurrentHidDevice; 
	UINT					m_nHidDevices; 
 
	bool					m_bUniqueIDs[HIDMGR_SUPPORTED_HID_DEVICES]; 
 
    static CHidManager* m_pThis; 
 
}; 
 
#endif // #ifndef _INCLUDED_HIDAPI_H_