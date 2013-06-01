/**
*/
#pragma once

#include "hidsdi.h"
#include "setupapi.h"

typedef USHORT USAGE;

typedef BOOLEAN (WINAPI HidD_GetPreparsedData)(HANDLE HidDeviceObject, PHIDP_PREPARSED_DATA *PreparsedData);
typedef BOOLEAN (WINAPI HidD_FreePreparsedData)(PHIDP_PREPARSED_DATA PreparsedData);
typedef BOOLEAN (WINAPI HidD_GetAttributes)(HANDLE HidDeviceObject, PHIDD_ATTRIBUTES Attributes);
typedef BOOLEAN (WINAPI HidD_GetSerialNumberString)(HANDLE HidDeviceObject, void *Buffer, ULONG BufferLength);
typedef void (WINAPI HidD_GetHidGuid)(LPGUID HidGuid);
typedef NTSTATUS (WINAPI HidP_GetCaps)(void *PreparsedData, PHIDP_CAPS caps);
typedef NTSTATUS (WINAPI HidP_GetButtonCaps)(HIDP_REPORT_TYPE ReportType, PHIDP_BUTTON_CAPS *ButtonCaps, USHORT *ButtonCapsLength, void *PreparsedData);
typedef NTSTATUS (WINAPI HidP_GetValueCaps)(HIDP_REPORT_TYPE ReportType, PHIDP_VALUE_CAPS *ValueCaps, USHORT *ValueCapsLength, void *PreparsedData);
typedef NTSTATUS (WINAPI HidP_GetUsages)(HIDP_REPORT_TYPE ReportType, USAGE UsagePage, USHORT LinkCollection, USAGE *UsageList, ULONG *UsageLength, void *PreparsedData, void *Report, ULONG ReportLength);
typedef NTSTATUS (WINAPI HidP_SetUsages)(HIDP_REPORT_TYPE ReportType, USAGE UsagePage, USHORT LinkCollection, USAGE *UsageList, ULONG *UsageLength, void *PreparsedData, void *Report, ULONG ReportLength);
typedef NTSTATUS (WINAPI HidP_UnsetUsages)(HIDP_REPORT_TYPE ReportType, USAGE UsagePage, USHORT LinkCollection, USAGE *UsageList, ULONG *UsageLength, void *PreparsedData, void *Report, ULONG ReportLength);
typedef NTSTATUS (WINAPI HidP_GetUsagesEx)(HIDP_REPORT_TYPE ReportType, USHORT LinkCollection, USAGE_AND_PAGE *ButtonList, ULONG *UsageLength, void *PreparsedData, PCHAR Report, ULONG ReportLength);
typedef NTSTATUS (WINAPI HidP_GetUsageValue)(HIDP_REPORT_TYPE ReportType, USAGE UsagePage, USHORT LinkCollection, USAGE Usage, ULONG *UsageValue, void *PreparsedData, void *Report, ULONG ReportLength);
typedef NTSTATUS (WINAPI HidP_GetLinkCollectionNodes)(PHIDP_LINK_COLLECTION_NODE LinkCollectionNodes, PULONG LinkCollectionNodesLength, PHIDP_PREPARSED_DATA PreparsedData);
typedef NTSTATUS (WINAPI HidP_GetSpecificButtonCaps)(HIDP_REPORT_TYPE ReportType, USAGE UsagePage, USHORT LinkCollection, USAGE Usage, PHIDP_BUTTON_CAPS *ButtonCaps, USHORT *ButtonCapsLength, void *PreparsedData);
typedef NTSTATUS (WINAPI HidP_TranslateUsagesToI8042ScanCodes)(PUSAGE ChangedUsageList, ULONG UsageListLength, HIDP_KEYBOARD_DIRECTION KeyAction, PHIDP_KEYBOARD_MODIFIER_STATE ModifierState, PHIDP_INSERT_SCANCODES InsertCodesProcedure, PVOID InsertCodesContext);

typedef DWORD DEVINST;
typedef DWORD *PDEVINST;