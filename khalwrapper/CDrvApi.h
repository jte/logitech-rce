/** 
 */
#pragma once

#include <stdio.h>
#include <Windows.h>

typedef DWORD LD_DUID;
typedef BYTE LD_POSITION;
typedef DWORD LD_DEVICE_MODEL;
typedef DWORD LD_UMD_TYPE;
typedef DWORD LD_BUS_TYPE;
typedef DWORD LD_DEVICETYPE;
typedef DWORD DEVINST;
typedef DWORD LD_POWER_STATE;
typedef DWORD LD_HANDLE;

enum LD_STATUS
{
	LD_SUCCESS,
	LD_MORE_PROCESSING_REQUIRED,
	LD_HARDWARE_ERROR,
	LD_CLIENT_UNLOADED,
	LD_FORWARD_TO_ANOTHER_UMD,
	LD_INVALID_PARAMETER,
	LD_INVALID_SIZE,
	LD_INVALID_POINTER,
	LD_LOW_RESOURCES,
	LD_MAIN_PROCESS_ERROR,
	LD_INVALID_DUID,
	LD_INVALID_FUNCTION,
	LD_INVALID_EVENT,
	LD_DEVICE_REMOVAL,
	LD_LIST_OVERFLOW,
	LD_INTERNAL_ERROR,
	LD_INVALID_CALLBACK,
	LD_INVALID_HANDLE,
	LD_EXCLUSIVE_CONFLICT,
	LD_IOCONTROL_ERROR,
	LD_UNSUCCESSFULL,
	LD_TIMEOUT,
	LD_NOT_IMPLEMENTED,
	LD_SEQUENCE_ERROR,
	LD_DEVICE_NOT_CAPABLE,
	LD_DEVICE_INVALID_STATUS,
	LD_DEVICE_NOT_CONNECTED,
	LD_SESSION_LOCKED,
	LD_EVENT_DUPLICATION,
	LD_DLL_OR_FUNCTION_NOT_FOUND,
	LD_SYSTEM_ERROR,
	LD_REGISTRY_ERROR,
	LD_DEVICE_RETURNED_INVALID_DATA,
	LD_MUTEX_ERROR
};

typedef struct _LD_DEVICEINFO
{
	DWORD dwSize;
	DWORD dwDUID;
	DWORD ParentDevice;
	DWORD Model;
	DWORD BusType;
	DWORD DeviceType;
	char sPersistentName[0x40];
	char sModelName[0x10];
} LD_DEVICEINFO, *PLD_DEVICE_INFO;

typedef int (__cdecl *EventCallback)(struct _LD_DEVICE* /*pDevice*/, int/*sizeof = 0x30*/, int/*ulEventParam*/, int/*pExtra*/);

typedef struct _LD_EVENT_REGISTRATION
{
	DWORD pSize;
	DWORD DUID;
	DWORD EventType;
	DWORD Param;
	DWORD ExtraCallbackParam;
	DWORD Flags;
	HWND hNotifyWnd;
	UINT ulNotifyMsg;
	EventCallback Callback;
	DWORD pad4;
	DWORD Status;
	DWORD pad5;
	DWORD pad6;
} LD_EVENT_REGISTRATION, *PLD_EVENT_REGISTRATION;

typedef struct _LD_DEVICE
{
	LD_DUID DUID;
	LD_POSITION ThisPos[56];
	LONG bStarted;
	LONG bVisible;
	LD_DEVICE_MODEL Model;
	LD_UMD_TYPE ModelSetByUMD;
	LD_DUID ParentDevice;
	LD_DUID MasterDevice;
	LD_BUS_TYPE BusType;
	DEVINST AncestorInstance;
	LD_DEVICETYPE DeviceType;
	ULONG DeviceIndex;
	CHAR sPersistentName[0x40];
	CHAR sModelName[0x40];
	DWORD CapsList[0x44];
	BOOL bCapsLoaded;
	DWORD dwDataValidFlags;
	DWORD dwEventEnabledFlags;
	ULONG ulChannel;
	BOOL bChannelError;
	ULONG ulBatteryStatus;
	LD_POWER_STATE PowerState;
	ULONG ulBatteryLevel;
	ULONG ulBatteryRange;
	ULONG ulRawBatteryValue;
	ULONG ulNominalBatteryMileage;
	LD_HANDLE hEnhancedBatteryNotification;
	FILETIME LogFileLastModifiedTime;
	double CurrentDischargeRate;
	__int64 LastChangeTime;
	BOOL bCapsLoaded_;
} LD_DEVICE, *PLD_DEVICE;

typedef struct _LD_DEVMGR_DEVICE
{
	LD_HANDLE DeviceEventArrival;
	LD_HANDLE DeviceEventRemoval;
	LD_HANDLE UnknownDeviceEvent;
	LD_HANDLE DeviceEventDump;
	LD_HANDLE UserSessionEvent;
	LD_HANDLE EnumerationCompleteEvent;
	LD_HANDLE CursorSettingsChangeEvent;
	LD_HANDLE ExclusiveClient;
	LD_HANDLE ExclusiveFilter;
} LD_DEVMGR_DEVICE, *PLD_DEVMGR_DEVICE;

enum LD_EVENT
{
	LD_EVENT_DEVICE_ARRIVAL = 0,
	LD_EVENT_DEVICE_REMOVAL = 1,
	LD_EVENT_READ_COMPLETE = 2,
	LD_EVENT_UNKNOWN_DEVICE = 3,
	LD_EVENT_HOT_STATUS = 4,
	LD_EVENT_USER_SESSION = 5,
	LD_EVENT_ENUMERATION_COMPLETE = 6,
	LD_EVENT_DEVICE_DUMP = 7,
	LD_EVENT_CURSOR_SETTINGS_CHANGED = 8,
	LD_EVENT_MOUSE_BUTTON = 0x1000000,
	LD_EVENT_MOUSE_MOVE,
	LD_EVENT_MOUSE_WHEEL,
	LD_EVENT_MOUSE_STATUS,
	LD_EVENT_TOUCHPAD_RAW_XY = 0x1000008,
	LD_EVENT_TOUCHMOUSE_RAW_POINTS = 0x1000009,
	LD_EVENT_TOUCHPAD_MODELESS_XY = 0x100000A,
	LD_EVENT_MOUSE_EXTRA_BUTTONS,
	LD_EVENT_MOUSE_GESTURE,
	LD_EVENT_MOUSE_H_SCROLL,
	LD_EVENT_KEYBOARD_KEY = 0x2000000,
	LD_EVENT_KEYBOARD_STATUS,
	LD_EVENT_RECEIVER_STATUS = 0x3000000,
	LD_EVENT_DISPLAY_CLEAR = 0x4000000,
	LD_EVENT_CALCULATOR_RESULT,
	LD_EVENT_DISPLAY_STATUS,
	LD_EVENT_MENU_ITEM,
	LD_EVENT_LAST_EVENT = 0x6000000,
	LD_EVENT_NO_EVENT = 0x6000001
};

typedef struct _LD_GET_POWER_STATUS_INFO
{
	DWORD dwSize;
	DWORD PowerState;
	DWORD ulBatteryRange;
	DWORD ulBatteryLevel;
	DWORD ulRawBatteryValue;
	DWORD pad1;
	DWORD pad2;
	DWORD pad3;
} LD_GET_POWER_STATUS_INFO;

typedef struct _LD_GET_CAPABILITY_INFO
{
	DWORD dwSize;
	const char *szCaps;
	DWORD dwOutput;
} LD_GET_CAPABILITY_INFO;
/* CAPABILITIES (szCaps) */
/*
REPORT_RATE
RESOLUTION
BATTERY
RECHARGEABLE
HPP2PROBED
HASACHILD
SECURE
QUAD
DONTMESSWITHME
PGM_POWER_LEVEL
HIGH_RES_WHEEL
BATTERY_MILEAGE
ANLOG_HSCROLL (is this correct????)
ICONS
PGM_TEMPERATURE
CALCULATOR
SOUND_DISABLE
TOUCHPAD_DISABLE
FNKEYINVERSE
BACKLIGHT
BLUETOOTH
CLOCK
CLEAR_KEY
TEXT_LINES
TEXT_COLUMNS
ICON_STATES
LEDS
LED_STATES
LED_INTENSITY
LED_COLOR
SLEEP_SETTING
SWHEEL_ACC
3G_WHEEL
DISPLAY_MENU
PRESENCE_DETECTION
DFU
*/

typedef struct _LD_FEATURE_INFO
{

} LD_FEATURE_INFO, *PLD_FEATURE_INFO;

enum LD_FEATURE_ID
{
	LD_FID_ROOT = 0x0000,
	LD_FID_FEATURE_SET = 0x0001,
	LD_FID_FEATURE_INFO = 0x0002,
	LD_FID_DEVICE_FW_VERSION = 0x0003,
	LD_FID_DEVICE_NAME = 0x0005,
	LD_FID_BATTERY_STATUS = 0x1000,
	LD_FID_REPROG_CONTROLS = 0x1B00,
	LD_FID_REPROG_CONTROLS_V2 = 0x1B01,
	LD_FID_REPROG_CONTROLS_V3 = 0x1B03,
	LD_FID_WIRELESS_DEVICE_STATUS = 0x1D4B,
	LD_FID_VERTICAL_SCROLLING = 0x2100,
	LD_FID_HI_RES_SCROLLING = 0x2120,
	LD_FID_MOUSE_POINTER = 0x2200,
	LD_FID_FN_INVERSION = 0x40A0,
	LD_FID_ENCRYPTION = 0x4100,
	LD_FID_SOLAR_DASHBOARD = 0x4301,
	LD_FID_KEYBOARD_LAYOUT = 0x4520,
	LD_FID_DFUCONTROL = 0x00C0,
	LD_FID_TOUCHPAD_FW_ITEMS = 0x6010,
	LD_FID_TOUCHPAD_SW_ITEMS = 0x6011,
	LD_FID_LEFT_RIGHT_SWAP = 0x2001,
	LD_FID_BACKLIGHT = 0x1981,
	LD_FID_TOUCHPAD_WIN8_FW_ITEMS = 0x6012,
	LD_FID_NEW_FN_INVERSION = 0x40A2,
	LD_FID_TOUCHPAD_RAW_XY = 0x6100,
	LD_FID_TOUCHMOUSE_RAW_POINTS = 0x6110,
	LD_FID_DEVICE_GROUPS = 0x0006
};

enum LD_DEVICE_FUNCTION
{
	LD_REGISTER_EVENT,
	LD_UNREGSITER_EVENT,
	LD_DEVICE_READ,
	LD_DEVICE_WRITE,
	LD_GET_BATTERY_STATUS,
	LD_GET_CHANNEL_STATUS,
	LD_GET_DRIVER_VERSION,
	LD_SEND_INPUT,
	LD_GET_CAPABILITY,
	LD_GET_POWER_STATUS,
	LD_GET_LAST_USED_DEVICE,
	LD_FAKE_DEVICE_ARRIVAL,
	LD_GET_DEVICE_DETAILS,
	LD_GET_MOUSE_DRIVER_TYPE,
	LD_SET_MOUSE_DRIVER_TYPE,
	LD_HOT_WIRE,
	LD_SET_POWER_STATUS,
	LD_GET_REGISTER,
	LD_SET_REGISTER,
	LD_GET_DEVICE_FEATURE,
	LD_SET_DEVICE_FEATURE,
	LD_GET_3GWHEEL_STATE,
	LD_SET_3GWHEEL_STATE,
	LD_GET_TIME_STAMP,
	LD_GET_RADIO_SIGNAL,
	LD_GET_MOUSE_SPEED,
	LD_SET_MOUSE_SPEED,
	LD_GET_MOUSE_ACCELERATION,
	LD_SET_MOUSE_ACCELERATION,
	LD_GET_MOUSE_BUTTON_MAPPING,
	LD_SET_MOUSE_BUTTON_MAPPING,
	LD_GET_MOUSE_CHORDING_DELAY,
	LD_SET_MOUSE_CHORDING_DELAY,
	LD_GET_MOUSE_ORIENTATION,
	LD_SET_MOUSE_ORIENTATION,
	/*add others*/
};

enum 
{
	UMD_DEVICE_MGR,
	UMD_MOUSEWARE,
	UMD_ITOUCH,
	UMD_DEBUG,
	UMD_HIDPP,
	UMD_USBFILT,
	UMD_HIDFILT_MOUSE,
	UMD_HIDFILT_CC,
	UMD_HIDFILT_NAV,
	UMD_MOUFILT,
	UMD_HIDFILT_POWER,
	UMD_HIDFILT_NONCC,
	UMD_MAX_INDEX
};

typedef enum  LD_STATUS (__cdecl *LD_GetDeviceInfo)(unsigned long, struct _LD_DEVICEINFO *);
typedef enum  LD_STATUS (__cdecl *LD_GetDeviceDetails)(unsigned long, void *);
typedef unsigned long (__cdecl *LD_RegisterEvent)(struct _LD_EVENT_REGISTRATION *);
typedef enum  LD_STATUS (__cdecl *LD_UnregisterEvent)(unsigned long);
typedef int (__cdecl *LD_DeviceControl)(DWORD, DWORD, DWORD*);
typedef enum  LD_STATUS (__cdecl *LD_GetDriverVersionString)(char *, unsigned long);
typedef int (*LD_Open)(void);
typedef enum  LD_STATUS (__cdecl *LD_Close)(void);
typedef enum  LD_STATUS (__cdecl *LD_GetDeviceName)(unsigned long dwDUID, char *szBuffer, unsigned long dwBufferSize);
typedef enum  LD_STATUS (__cdecl *LD_GetFeatureList)(unsigned long, enum  LD_FEATURE_ID *, unsigned long *);
typedef enum  LD_STATUS (__cdecl *LD_GetFeatureInfo)(unsigned long, enum  LD_FEATURE_ID, struct _LD_FEATURE_INFO *, unsigned long *);
typedef void (__cdecl *LD_Main)(struct HINSTANCE__ *);
typedef int (__cdecl *LD_GetDeviceCount)(LPVOID lpfn);
typedef int (__cdecl *LD_GetFirstDevicePosition)(LPVOID lpfn);
typedef int (__cdecl *LD_GetNextDevicePosition)(LPVOID lpfn);

class CDrvApi
{
public:
	CDrvApi() :
		m_hInstance(NULL),
		m_InitTried(false)
	{}
	bool CheckInit();
	bool Deinitialize();
	bool Initialize();
protected:
private:
public:
	HINSTANCE m_hInstance;
	bool m_InitTried;
	static LD_GetDeviceInfo m_LD_GetDeviceInfo;
	static LD_GetDeviceDetails m_LD_GetDeviceDetails;
	static LD_RegisterEvent m_LD_RegisterEvent;
	static LD_UnregisterEvent m_LD_UnregisterEvent;
	static LD_DeviceControl m_LD_DeviceControl;
	static LD_GetDriverVersionString m_LD_GetDriverVersionString;
	static LD_Open m_LD_Open;
	static LD_Close m_LD_Close;
	static LD_GetDeviceName m_LD_GetDeviceName;
	static LD_GetFeatureList m_LD_GetFeatureList;
	static LD_GetFeatureInfo m_LD_GetFeatureInfo;
	static LD_Main m_LD_Main;
	static LD_GetDeviceCount m_LD_GetDeviceCount;
	static LD_GetFirstDevicePosition m_LD_GetFirstDevicePosition;
	static LD_GetNextDevicePosition m_LD_GetNextDevicePosition;
};
