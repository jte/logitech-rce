`//--------------------------------------------------------------------------   
//   
// USB HID Device and USB HID Device Manager Helper Classes   
//   
// The building blocks for this code was derived from the HCLIENT sample code   
// provided by Microsoft in the Windows XP DDK.     
//   
//--------------------------------------------------------------------------   
// $Archive: /WirelessUSB/LS2/Firmware/LP KBM RDK/Software/Source Code/CUSBHidAPI/HidAPI.cpp $   
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
#include "StdAfx.h"   
#include <dbt.h>   
   
#include "hidapi.h"   
   
   
#define HIDMANAGER_CLASS _T("CHidManager")   
CHidManager* CHidManager::m_pThis = NULL;   
OSVERSIONINFO m_OS;   
   
bool CHidDevice::InitializeHidDevice(PCHAR pDevicePath, USHORT uID)   
{   
    nextHidDevice = NULL;   
   
    m_bOpened = false;   
   
    m_hDevice = NULL;   
    m_hDeviceRegistered = NULL;   
    m_hWnd = NULL;   
   
    m_bOpenedForRead = false;   
    m_bOpenedForWrite = false;   
    m_bOpenedOverlapped = false;   
    m_bOpenedExclusive = false;   
   
    m_uDeviceID = uID;   
   
    m_pHidDevicePath = NULL;   
    m_pHidPpd = NULL;   
    ::ZeroMemory(&m_HidCaps, sizeof(m_HidCaps));   
    ::ZeroMemory(&m_HidAttributes, sizeof(m_HidAttributes));   
   
    m_pInputReportBuffer = NULL;   
    m_pInputData = NULL;   
    m_ulInputDataLength = 0;   
    m_pInputButtonCaps = NULL;   
    m_pInputValueCaps = NULL;   
   
    m_pOutputReportBuffer = NULL;   
    m_pOutputData = NULL;   
    m_ulOutputDataLength = 0;   
    m_pOutputButtonCaps = NULL;   
    m_pOutputValueCaps = NULL;   
   
    m_pFeatureReportBuffer = NULL;   
    m_pFeatureData = NULL;   
    m_ulFeatureDataLength = 0;   
    m_pFeatureButtonCaps = NULL;   
    m_pFeatureValueCaps = NULL;   
   
    m_pHidDevicePath = new CHAR[lstrlen(pDevicePath)+1];   
    if (!m_pHidDevicePath)   
        return false;   
   
    ::strcpy(m_pHidDevicePath, pDevicePath);   
   
    return true;   
}   
   
void CHidDevice::DestroyHidDevice()   
{   
    if (m_bOpened)   
    {   
        CloseHidDevice();   
    }   
   
    if (m_pHidDevicePath)   
        delete [] m_pHidDevicePath;   
       
    m_uDeviceID = 0xFFFF;   
}   
   
// sets appropriate access rights, attempts to open a    
// handle to the HID device, obtains the top collection data, and    
// makes a call to setup input, output, and feature data buffers.    
bool CHidDevice::OpenHidDevice( bool bReadAccess,    
                                bool bWriteAccess,    
                                bool bUseOverlapped,    
                                bool bExclusive)   
{   
    DWORD   accessFlags = 0;   
    DWORD   sharingFlags = 0;   
   
    // Check for valid device path   
    if (!m_pHidDevicePath)    
    {   
        return false;   
    }   
   
    // setup access flag values   
    if (bReadAccess)   
    {   
        accessFlags |= GENERIC_READ;   
    }   
   
    if (bWriteAccess)   
    {   
        accessFlags |= GENERIC_WRITE;   
    }   
   
    if (!bExclusive)   
    {   
        sharingFlags = FILE_SHARE_READ | FILE_SHARE_WRITE;   
    }   
       
    //   
    //  The hid.dll api's do not pass the overlapped structure into deviceiocontrol   
    //  so to use them we must have a non overlapped device.  If the request is for   
    //  an overlapped device we will close the device below and get a handle to an   
    //  overlapped device   
    //   
       
    m_hDevice = ::CreateFile (m_pHidDevicePath,   
                              accessFlags,   
                              sharingFlags,   
                              NULL,        // no SECURITY_ATTRIBUTES structure   
                              OPEN_EXISTING, // No special create flags   
                              0,   // Open device as non-overlapped so we can get data   
                              NULL);       // No template file   
   
    if (m_hDevice == INVALID_HANDLE_VALUE)    
    {   
        return false;   
    }   
   
    // open was successful   
    m_bOpened = true;   
   
    // set access booleans appropriately   
    m_bOpenedForRead = bReadAccess;   
    m_bOpenedForWrite = bWriteAccess;   
    m_bOpenedOverlapped = bUseOverlapped;   
    m_bOpenedExclusive = bExclusive;   
       
    // obtain the top level collection's preparsed data from the HID device.     
    // the top level collection is a HID collection that is not nested with    
    // other collections.  a HID collection is a meaningful group of HID controls   
    // and their respective HID usages.   
    if (!::HidD_GetPreparsedData (m_hDevice, &m_pHidPpd))    
    {   
        // if failure, close HID handle and return failure   
        CloseHidDevice();   
        return false;   
    }   
   
    // get the attributes of the top level collection   
    if (!::HidD_GetAttributes (m_hDevice, &m_HidAttributes))    
    {   
        // if failure, close HID handle, free preparsed data   
        // and return failure   
        ::HidD_FreePreparsedData (m_pHidPpd);   
        m_pHidPpd = NULL;   
   
        CloseHidDevice();   
        return false;   
    }   
   
    // get the top level collections capability   
    if (!::HidP_GetCaps (m_pHidPpd, &m_HidCaps))   
    {   
        // if failure, close HID handle, free preparsed data   
        // and return failure   
        ::HidD_FreePreparsedData (m_pHidPpd);   
        m_pHidPpd = NULL;   
   
        CloseHidDevice();   
        return false;   
    }   
   
    // attempt to setup the HID device   
    if (!SetupHidDevice())   
    {   
        // if failure, close HID handle, free preparsed data   
        // and return failure   
        ::HidD_FreePreparsedData (m_pHidPpd);   
        m_pHidPpd = NULL;   
   
        CloseHidDevice();   
        return false;   
    }   
   
    //   
    // At this point the client has a choice.  It may chose to look at the   
    // Usage and Page of the top level collection found in the HIDP_CAPS   
    // structure.  In this way it could just use the usages it knows about.   
    // If either HidP_GetUsages or HidP_GetUsageValue return an error then   
    // that particular usage does not exist in the report.   
    //   
   
    // verify that this is a HID device we care about   
    if (!VerifyHidDevice())   
    {   
        // if failure, close HID handle, free preparsed data   
        // and return failure   
        ::HidD_FreePreparsedData (m_pHidPpd);   
        m_pHidPpd = NULL;   
   
        CloseHidDevice();   
        return false;   
    }   
   
    // if overlapped I/O requested, close handle and reopen using overlapped I/O   
    if (bUseOverlapped)   
    {   
        ::CloseHandle(m_hDevice);   
   
        m_hDevice = ::CreateFile (m_pHidDevicePath,   
                                  accessFlags,   
                                  sharingFlags,   
                                  NULL,        // no SECURITY_ATTRIBUTES structure   
                                  OPEN_EXISTING, // No special create flags   
                                  FILE_FLAG_OVERLAPPED, // Now we open the device as overlapped   
                                  NULL);       // No template file   
       
        if (m_hDevice == INVALID_HANDLE_VALUE)    
        {   
            // if failure, close HID handle, free preparsed data   
            // and return failure   
            ::HidD_FreePreparsedData (m_pHidPpd);   
            m_pHidPpd = NULL;   
   
            CloseHidDevice();   
            return false;   
        }   
    }   
   
    return true;   
}   
   
// registers the HID device handle for event notification, mainly to get   
// notified when the device is removed   
bool CHidDevice::RegisterHidDevice(HWND hWnd)   
{   
    DEV_BROADCAST_HANDLE broadcastHandle;   
   
    broadcastHandle.dbch_size = sizeof(DEV_BROADCAST_HANDLE);   
    broadcastHandle.dbch_devicetype = DBT_DEVTYP_HANDLE;   
    broadcastHandle.dbch_handle = m_hDevice;   
      
    m_hDeviceRegistered = ::RegisterDeviceNotification(hWnd, (PVOID) &broadcastHandle, DEVICE_NOTIFY_WINDOW_HANDLE);   
    if (!m_hDeviceRegistered)    
        return false;   
   
    m_hWnd = hWnd;   
   
    return true;   
}   
   
// sets up HID input, output and feature data buffers    
// used to simplify communication with HID devices   
bool CHidDevice::SetupHidDevice()   
{   
    USHORT              numCaps;   
    ULONG               i;   
    USAGE               usage;   
   
    PHIDP_VALUE_CAPS valueCaps;   
    PHIDP_BUTTON_CAPS buttonCaps;   
    PHID_DATA data;   
    USHORT numValues;   
   
    //   
    // setup Input Data buffers.   
    //   
   
    //   
    // Allocate memory to hold on input report   
    //   
   
    if (m_HidCaps.InputReportByteLength)   
    {   
        m_pInputReportBuffer = new CHAR[m_HidCaps.InputReportByteLength];   
        if (m_pInputReportBuffer)   
            ZeroMemory (m_pInputReportBuffer, m_HidCaps.InputReportByteLength);   
    }   
   
    //   
    // Allocate memory to hold the button and value capabilities.   
    // NumberXXCaps is in terms of array elements.   
    //   
       
    if (m_HidCaps.NumberInputButtonCaps)   
    {   
        m_pInputButtonCaps = new HIDP_BUTTON_CAPS[m_HidCaps.NumberInputButtonCaps];   
        if (!m_pInputButtonCaps)   
            return false;   
   
        ZeroMemory (m_pInputButtonCaps, m_HidCaps.NumberInputButtonCaps * sizeof(HIDP_BUTTON_CAPS));   
   
        numCaps = m_HidCaps.NumberInputButtonCaps;   
        ::HidP_GetButtonCaps (HidP_Input,   
                            m_pInputButtonCaps,   
                            &numCaps,   
                            m_pHidPpd);   
    }   
   
    if (m_HidCaps.NumberInputValueCaps)   
    {   
        m_pInputValueCaps = new HIDP_VALUE_CAPS[m_HidCaps.NumberInputValueCaps];   
        if (!m_pInputValueCaps)   
            return false;   
   
        ZeroMemory (m_pInputValueCaps, m_HidCaps.NumberInputValueCaps * sizeof(HIDP_VALUE_CAPS));   
   
        numCaps = m_HidCaps.NumberInputValueCaps;   
        ::HidP_GetValueCaps (HidP_Input,   
                            m_pInputValueCaps,   
                            &numCaps,   
                            m_pHidPpd);   
    }   
   
    //   
    // Depending on the device, some value caps structures may represent more   
    // than one value.  (A range).  In the interest of being verbose, over   
    // efficient, we will expand these so that we have one and only one   
    // struct _HID_DATA for each value.   
    //   
    // To do this we need to count up the total number of values are listed   
    // in the value caps structure.  For each element in the array we test   
    // for range if it is a range then UsageMax and UsageMin describe the   
    // usages for this range INCLUSIVE.   
    //   
   
    valueCaps = m_pInputValueCaps;   
    numValues = 0;   
   
    if (valueCaps)   
    {   
        for (i = 0; i < m_HidCaps.NumberInputValueCaps; i++, valueCaps++)    
        {   
            if (valueCaps->IsRange)    
            {   
                numValues += valueCaps->Range.UsageMax - valueCaps->Range.UsageMin + 1;   
            }   
            else   
            {   
                numValues++;   
            }   
        }   
    }   
   
    //   
    // Allocate a buffer to hold the struct _HID_DATA structures.   
    // One element for each set of buttons, and one element for each value   
    // found.   
    //   
   
    m_ulInputDataLength = m_HidCaps.NumberInputButtonCaps + numValues;   
   
    if (m_ulInputDataLength)   
    {   
        m_pInputData = new HID_DATA[m_ulInputDataLength];   
        if (!m_pInputData)   
            return false;   
   
        ZeroMemory (m_pInputData, m_ulInputDataLength * sizeof(HID_DATA));   
    }   
   
    //   
    // Fill in the button data   
    //   
   
    buttonCaps = m_pInputButtonCaps;   
    data = m_pInputData;   
   
    if (buttonCaps && data)   
    {   
        for (i = 0;   
            i < m_HidCaps.NumberInputButtonCaps;   
            i++, data++, buttonCaps++)    
        {   
            data->IsButtonData = true;   
            data->Status = HIDP_STATUS_SUCCESS;   
            data->UsagePage = buttonCaps->UsagePage;   
            if (buttonCaps->IsRange)    
            {   
                data->ButtonData.UsageMin = buttonCaps->Range.UsageMin;   
                data->ButtonData.UsageMax = buttonCaps->Range.UsageMax;   
            }   
            else   
            {   
                data->ButtonData.UsageMin = data->ButtonData.UsageMax = buttonCaps->NotRange.Usage;   
            }   
               
            data->ButtonData.MaxUsageLength = ::HidP_MaxUsageListLength (   
                                                    HidP_Input,   
                                                    buttonCaps->UsagePage,   
                                                    m_pHidPpd);   
           
            data->ButtonData.Usages = new USAGE[data->ButtonData.MaxUsageLength];   
            ZeroMemory (data->ButtonData.Usages, data->ButtonData.MaxUsageLength * sizeof(USAGE));   
   
            data->ReportID = buttonCaps->ReportID;   
        }   
    }   
   
    //   
    // Fill in the value data   
    //   
   
    valueCaps = m_pInputValueCaps;   
   
    if (valueCaps)   
    {   
        for (i = 0; i < numValues; i++, valueCaps++)   
        {   
            if (valueCaps->IsRange)    
            {   
                for (usage = valueCaps->Range.UsageMin;   
                    usage <= valueCaps->Range.UsageMax;   
                    usage++)    
                {   
                    data->IsButtonData = false;   
                    data->Status = HIDP_STATUS_SUCCESS;   
                    data->UsagePage = valueCaps->UsagePage;   
                    data->ValueData.Usage = usage;   
                    data->ReportID = valueCaps->ReportID;   
                    data++;   
                }   
            }    
            else   
            {   
                data->IsButtonData = false;   
                data->Status = HIDP_STATUS_SUCCESS;   
                data->UsagePage = valueCaps->UsagePage;   
                data->ValueData.Usage = valueCaps->NotRange.Usage;   
                data->ReportID = valueCaps->ReportID;   
                data++;   
            }   
        }   
    }   
   
    //   
    // setup Output Data buffers.   
    //   
   
    if (m_HidCaps.OutputReportByteLength)   
    {   
        m_pOutputReportBuffer = new CHAR[m_HidCaps.OutputReportByteLength];   
        if (m_pOutputReportBuffer)   
            ZeroMemory (m_pOutputReportBuffer, m_HidCaps.OutputReportByteLength);   
    }   
   
    if (m_HidCaps.NumberOutputButtonCaps)   
    {   
        m_pOutputButtonCaps = new HIDP_BUTTON_CAPS[m_HidCaps.NumberOutputButtonCaps];   
        if (!m_pOutputButtonCaps)   
            return false;   
   
        ZeroMemory (m_pOutputButtonCaps, m_HidCaps.NumberOutputButtonCaps * sizeof(HIDP_BUTTON_CAPS));   
   
        numCaps = m_HidCaps.NumberOutputButtonCaps;   
        ::HidP_GetButtonCaps (HidP_Output,   
                            m_pOutputButtonCaps,   
                            &numCaps,   
                            m_pHidPpd);   
    }   
   
    if (m_HidCaps.NumberOutputValueCaps)   
    {   
        m_pOutputValueCaps = new HIDP_VALUE_CAPS[m_HidCaps.NumberOutputValueCaps];   
        if (!m_pOutputValueCaps)   
            return false;   
   
        ZeroMemory (m_pOutputValueCaps, m_HidCaps.NumberOutputValueCaps * sizeof(HIDP_VALUE_CAPS));   
   
        numCaps = m_HidCaps.NumberOutputValueCaps;   
        ::HidP_GetValueCaps (HidP_Output,   
                            m_pOutputValueCaps,   
                            &numCaps,   
                            m_pHidPpd);   
    }   
   
    valueCaps = m_pOutputValueCaps;   
    numValues = 0;   
   
    if (valueCaps)   
    {   
        for (i = 0; i < m_HidCaps.NumberOutputValueCaps; i++, valueCaps++)    
        {   
            if (valueCaps->IsRange)    
            {   
                numValues += valueCaps->Range.UsageMax   
                        - valueCaps->Range.UsageMin + 1;   
            }    
            else   
            {   
                numValues++;   
            }   
        }   
    }   
   
    m_ulOutputDataLength = m_HidCaps.NumberOutputButtonCaps + numValues;   
   
    if (m_ulOutputDataLength)   
    {   
        m_pOutputData = new HID_DATA[m_ulOutputDataLength];   
        if (!m_pOutputData)   
            return false;   
   
        ZeroMemory (m_pOutputData, m_ulOutputDataLength * sizeof(HID_DATA));   
    }   
   
    buttonCaps = m_pOutputButtonCaps;   
    data = m_pOutputData;   
   
    if (buttonCaps && data)   
    {   
        for (i = 0;   
            i < m_HidCaps.NumberOutputButtonCaps;   
            i++, data++, buttonCaps++)    
        {   
            data->IsButtonData = true;   
            data->Status = HIDP_STATUS_SUCCESS;   
            data->UsagePage = buttonCaps->UsagePage;   
   
            if (buttonCaps->IsRange)   
            {   
                data->ButtonData.UsageMin = buttonCaps->Range.UsageMin;   
                data->ButtonData.UsageMax = buttonCaps->Range.UsageMax;   
            }   
            else   
            {   
                data->ButtonData.UsageMin = data->ButtonData.UsageMax = buttonCaps->NotRange.Usage;   
            }   
   
            data->ButtonData.MaxUsageLength = ::HidP_MaxUsageListLength (   
                                                        HidP_Output,   
                                                        buttonCaps->UsagePage,   
                                                        m_pHidPpd);   
   
            data->ButtonData.Usages = new USAGE[data->ButtonData.MaxUsageLength];   
            ZeroMemory (data->ButtonData.Usages, data->ButtonData.MaxUsageLength * sizeof(USAGE));   
   
            data->ReportID = buttonCaps->ReportID;   
        }   
    }   
   
    valueCaps = m_pOutputValueCaps;   
   
    if (valueCaps)   
    {   
        for (i = 0; i < numValues; i++, valueCaps++)   
        {   
            if (valueCaps->IsRange)   
            {   
                for (usage = valueCaps->Range.UsageMin;   
                    usage <= valueCaps->Range.UsageMax;   
                    usage++)    
                {   
                    data->IsButtonData = false;   
                    data->Status = HIDP_STATUS_SUCCESS;   
                    data->UsagePage = valueCaps->UsagePage;   
                    data->ValueData.Usage = usage;   
                    data->ReportID = valueCaps->ReportID;   
                    data++;   
                }   
            }   
            else   
            {   
                data->IsButtonData = false;   
                data->Status = HIDP_STATUS_SUCCESS;   
                data->UsagePage = valueCaps->UsagePage;   
                data->ValueData.Usage = valueCaps->NotRange.Usage;   
                data->ReportID = valueCaps->ReportID;   
                data++;   
            }   
        }   
    }   
   
    //   
    // setup Feature Data buffers.   
    //   
   
    if (m_HidCaps.FeatureReportByteLength)   
    {   
        m_pFeatureReportBuffer = new CHAR[m_HidCaps.FeatureReportByteLength];   
        if (m_pFeatureReportBuffer)   
            ZeroMemory (m_pFeatureReportBuffer, m_HidCaps.FeatureReportByteLength);   
    }   
   
    if (m_HidCaps.NumberFeatureButtonCaps)   
    {   
        m_pFeatureButtonCaps = new HIDP_BUTTON_CAPS[m_HidCaps.NumberFeatureButtonCaps];   
        if (!m_pFeatureButtonCaps)   
            return false;   
   
        ZeroMemory (m_pFeatureButtonCaps, m_HidCaps.NumberFeatureButtonCaps * sizeof(HIDP_BUTTON_CAPS));   
   
        numCaps = m_HidCaps.NumberFeatureButtonCaps;   
        ::HidP_GetButtonCaps (HidP_Feature,   
                            m_pFeatureButtonCaps,   
                            &numCaps,   
                            m_pHidPpd);   
    }   
   
    if (m_HidCaps.NumberFeatureValueCaps)   
    {   
        m_pFeatureValueCaps = new HIDP_VALUE_CAPS[m_HidCaps.NumberFeatureValueCaps];   
        if (!m_pFeatureValueCaps)   
            return false;   
   
        ZeroMemory (m_pFeatureValueCaps, m_HidCaps.NumberFeatureValueCaps * sizeof(HIDP_VALUE_CAPS));   
   
        numCaps = m_HidCaps.NumberFeatureValueCaps;   
        ::HidP_GetValueCaps (HidP_Feature,   
                            m_pFeatureValueCaps,   
                            &numCaps,   
                            m_pHidPpd);   
    }   
   
    valueCaps = m_pFeatureValueCaps;   
    numValues = 0;   
   
    if (valueCaps)   
    {   
        for (i = 0; i < m_HidCaps.NumberFeatureValueCaps; i++, valueCaps++)    
        {   
            if (valueCaps->IsRange)    
            {   
                numValues += valueCaps->Range.UsageMax   
                        - valueCaps->Range.UsageMin + 1;   
            }   
            else   
            {   
                numValues++;   
            }   
        }   
    }   
   
    m_ulFeatureDataLength = m_HidCaps.NumberFeatureButtonCaps + numValues;   
   
    if (m_ulFeatureDataLength)   
    {   
        m_pFeatureData = new HID_DATA[m_ulFeatureDataLength];   
        if (!m_pFeatureData)   
            return false;   
   
        ZeroMemory (m_pFeatureData, m_ulFeatureDataLength * sizeof(HID_DATA));   
    }   
   
    buttonCaps = m_pFeatureButtonCaps;   
    data = m_pFeatureData;   
   
    if (buttonCaps && data)   
    {   
        for (i = 0;   
            i < m_HidCaps.NumberFeatureButtonCaps;   
            i++, data++, buttonCaps++)    
        {   
            data->IsButtonData = true;   
            data->Status = HIDP_STATUS_SUCCESS;   
            data->UsagePage = buttonCaps->UsagePage;   
   
            if (buttonCaps->IsRange)   
            {   
                data->ButtonData.UsageMin = buttonCaps->Range.UsageMin;   
                data->ButtonData.UsageMax = buttonCaps->Range.UsageMax;   
            }   
            else   
            {   
                data->ButtonData.UsageMin = data->ButtonData.UsageMax = buttonCaps->NotRange.Usage;   
            }   
               
            data->ButtonData.MaxUsageLength = ::HidP_MaxUsageListLength (   
                                                    HidP_Feature,   
                                                    buttonCaps->UsagePage,   
                                                    m_pHidPpd);   
   
            data->ButtonData.Usages = new USAGE[data->ButtonData.MaxUsageLength];   
            ZeroMemory (data->ButtonData.Usages, data->ButtonData.MaxUsageLength * sizeof(USAGE));   
   
            data->ReportID = buttonCaps->ReportID;   
        }   
    }   
   
    valueCaps = m_pFeatureValueCaps;   
   
    if (valueCaps)   
    {   
        for (i = 0; i < numValues; i++, valueCaps++)    
        {   
            if (valueCaps->IsRange)   
            {   
                for (usage = valueCaps->Range.UsageMin;   
                    usage <= valueCaps->Range.UsageMax;   
                    usage++)   
                {   
                    data->IsButtonData = false;   
                    data->Status = HIDP_STATUS_SUCCESS;   
                    data->UsagePage = valueCaps->UsagePage;   
                    data->ValueData.Usage = usage;   
                    data->ReportID = valueCaps->ReportID;   
                    data++;   
                }   
            }    
            else   
            {   
                data->IsButtonData = false;   
                data->Status = HIDP_STATUS_SUCCESS;   
                data->UsagePage = valueCaps->UsagePage;   
                data->ValueData.Usage = valueCaps->NotRange.Usage;   
                data->ReportID = valueCaps->ReportID;   
                data++;   
            }   
        }   
    }   
   
    return true;   
}   
   
// closes HID device handle, unregisters HID device notification,    
// frees prepased data and data/report buffers   
bool CHidDevice::CloseHidDevice()   
{   
    // close handle, if valid   
    if (m_hDevice != INVALID_HANDLE_VALUE)   
    {   
        ::CloseHandle(m_hDevice);   
        m_hDevice = INVALID_HANDLE_VALUE;   
    }   
   
    // if HID device is registered, then unregister   
    if (m_hDeviceRegistered)    
    {   
        PostMessage(m_hWnd, WM_UNREGISTER_HANDLE, 0, (LPARAM) m_hDeviceRegistered);   
        m_hDeviceRegistered = NULL;   
        m_hWnd = NULL;   
    }   
       
    // free preparsed data   
    if (m_pHidPpd)   
    {   
        ::HidD_FreePreparsedData(m_pHidPpd);   
        m_pHidPpd = NULL;   
    }   
   
    // free input report buffer   
    if (m_pInputReportBuffer)   
    {   
        delete [] m_pInputReportBuffer;   
        m_pInputReportBuffer = NULL;   
    }   
   
    // free input data buffer   
    if (m_pInputData)   
    {   
        PHID_DATA data = m_pInputData;   
   
        if (data->ButtonData.Usages)   
        {   
            delete [] data->ButtonData.Usages;   
        }   
   
        delete [] m_pInputData;   
        m_pInputData = NULL;   
    }   
   
    // free input button caps   
    if (m_pInputButtonCaps)   
    {   
        delete [] m_pInputButtonCaps;   
        m_pInputButtonCaps = NULL;   
    }   
   
    // free input value caps   
    if (m_pInputValueCaps)   
    {   
        delete [] m_pInputValueCaps;   
        m_pInputValueCaps = NULL;   
    }   
   
    // free output report buffer   
    if (m_pOutputReportBuffer)   
    {   
        delete [] m_pOutputReportBuffer;   
        m_pOutputReportBuffer = NULL;   
    }   
   
    // free output data buffer   
    if (m_pOutputData)   
    {   
        PHID_DATA data = m_pOutputData;   
   
        if (data->ButtonData.Usages)   
        {   
            delete [] data->ButtonData.Usages;   
        }   
   
        delete [] m_pOutputData;   
        m_pOutputData = NULL;   
    }   
   
    // free output button caps   
    if (m_pOutputButtonCaps)    
    {   
        delete [] m_pOutputButtonCaps;   
        m_pOutputButtonCaps = NULL;   
    }   
   
    // free output value caps   
    if (m_pOutputValueCaps)   
    {   
        delete [] m_pOutputValueCaps;   
        m_pOutputValueCaps = NULL;   
    }   
   
    // free feature report buffer   
    if (m_pFeatureReportBuffer)   
    {   
        delete [] m_pFeatureReportBuffer;   
        m_pFeatureReportBuffer = NULL;   
    }   
   
    // free feature data buffer   
    if (m_pFeatureData)    
    {   
        PHID_DATA data = m_pFeatureData;   
   
        if (data->ButtonData.Usages)   
        {   
            delete [] data->ButtonData.Usages;   
        }   
   
        delete [] m_pFeatureData;   
        m_pFeatureData = NULL;   
    }   
   
    // free feature button caps   
    if (m_pFeatureButtonCaps)    
    {   
        delete [] m_pFeatureButtonCaps;   
        m_pFeatureButtonCaps = NULL;   
    }   
   
    // free feature values caps   
    if (m_pFeatureValueCaps)    
    {   
        delete [] m_pFeatureValueCaps;   
        m_pFeatureValueCaps = NULL;   
    }   
   
    // set open to false   
    m_bOpened = false;   
   
    return true;   
}   
   
// reads an input report from the HID device, performs    
// a validity check, and unpacks the report data   
bool CHidDevice::Read()   
{   
    DWORD    bytesRead;   
   
    // read the input report data   
    if (!ReadFile (m_hDevice,   
                  m_pInputReportBuffer,   
                  m_HidCaps.InputReportByteLength,   
                  &bytesRead,   
                  NULL)) // no overlapped   
    {   
        return false;   
    }   
   
    // make sure it was valid, ensure bytes read is what   
    // is expected for this report   
    ASSERT (bytesRead == m_HidCaps.InputReportByteLength);   
    if (bytesRead != m_HidCaps.InputReportByteLength)   
    {   
        return false;   
    }   
   
    // unpack the report data into a more useful form   
    return UnpackReport (HidP_Input,   
                         m_pInputReportBuffer,   
                         m_HidCaps.InputReportByteLength,   
                         m_pInputData,   
                         m_ulInputDataLength,   
                         m_pHidPpd);   
}   
   
// for every report ID, pack a report buffer and write the report data to the HID device   
bool CHidDevice::Write()   
{   
    DWORD     bytesWritten;   
    PHID_DATA pData;   
    ULONG     Index;   
    bool      Status;   
    bool      WriteStatus;   
   
    // Begin by looping through the HID_DEVICE's HID_DATA structure and setting   
    //   the IsDataSet field to false to indicate that each structure has   
    //   not yet been set for this Write call.   
   
    pData = m_pOutputData;   
    if (!pData)   
        return false;   
   
    for (Index = 0; Index < m_ulOutputDataLength; Index++, pData++)    
    {   
        pData->IsDataSet = false;   
    }   
   
    // In setting all the data in the reports, we need to pack a report buffer   
    //   and call WriteFile for each report ID that is represented by the    
    //   device structure.  To do so, the IsDataSet field will be used to    
    //   determine if a given report field has already been set.   
   
    Status = true;   
   
    pData = m_pOutputData;   
    if (!pData)   
        return false;   
   
    for (Index = 0; Index < m_ulOutputDataLength; Index++, pData++)    
    {   
        if (!pData->IsDataSet)    
        {   
            // Package the report for this data structure.  PackReport will   
            //    set the IsDataSet fields of this structure and any other    
            //    structures that it includes in the report with this structure   
   
            PackReport (HidP_Output,   
                     m_pOutputReportBuffer,   
                     m_HidCaps.OutputReportByteLength,   
                     pData,   
                     m_ulOutputDataLength - Index,   
                     m_pHidPpd);   
   
            // Now a report has been packaged up...Send it down to the device   
   
            WriteStatus = WriteFile (m_hDevice,   
                                  m_pOutputReportBuffer,   
                                  m_HidCaps.OutputReportByteLength,   
                                  &bytesWritten,   
                                  NULL) && (bytesWritten == m_HidCaps.OutputReportByteLength);   
   
            Status = Status && WriteStatus;                            
        }   
    }   
   
    return Status;   
}   
   
// obtains the feature report from each report ID exposed by the HID device   
bool CHidDevice::GetFeature()   
{   
    ULONG     Index;   
    PHID_DATA pData;   
    BOOLEAN   bReportStatus;   
    bool      bStatus;   
   
    // Begin by looping through the HID_DEVICE's HID_DATA structure and setting   
    //   the IsDataSet field to false to indicate that each structure has   
    //   not yet been set for this SetFeature() call.   
   
    pData = m_pFeatureData;   
    if (!pData)   
        return false;   
   
    for (Index = 0; Index < m_ulFeatureDataLength; Index++, pData++)    
    {   
        pData->IsDataSet = false;   
    }   
   
    // Next, each structure in the HID_DATA buffer is filled in with a value   
    //   that is retrieved from one or more calls to HidD_GetFeature.  The    
    //   number of calls is equal to the number of reportIDs on the device   
   
    bStatus = true;   
   
    pData = m_pFeatureData;   
    if (!pData)   
        return false;   
   
    for (Index = 0; Index < m_ulFeatureDataLength; Index++, pData++)    
    {   
        // If a value has yet to have been set for this structure, build a report   
        //    buffer with its report ID as the first byte of the buffer and pass   
        //    it in the HidD_GetFeature call.  Specifying the report ID in the   
        //    first specifies which report is actually retrieved from the device.   
        //    The rest of the buffer should be zeroed before the call   
   
        if (!pData->IsDataSet)    
        {   
            memset(m_pFeatureReportBuffer, 0x00, m_HidCaps.FeatureReportByteLength);   
   
            m_pFeatureReportBuffer[0] = (UCHAR) pData->ReportID;   
   
            bReportStatus = HidD_GetFeature (m_hDevice,   
                                            m_pFeatureReportBuffer,   
                                            m_HidCaps.FeatureReportByteLength);   
   
            // If the return value is true, scan through the rest of the HID_DATA   
            //    structures and fill whatever values we can from this report   
   
            if (bReportStatus)    
            {   
                bReportStatus = UnpackReport (HidP_Feature,   
                                              m_pFeatureReportBuffer,   
                                              m_HidCaps.FeatureReportByteLength,   
                                              m_pFeatureData,   
                                              m_ulFeatureDataLength,   
                                              m_pHidPpd);   
            }   
   
            bStatus = bStatus && bReportStatus;   
        }   
   }   
   
   return bStatus;   
}   
   
// send a feature report for each report ID exposed by the HID device   
bool CHidDevice::SetFeature()   
{   
    PHID_DATA pData;   
    ULONG     Index;   
    BOOLEAN   bReportStatus;   
    bool      bStatus;   
   
    // Begin by looping through the HID_DEVICE's HID_DATA structure and setting   
    //   the IsDataSet field to false to indicate that each structure has   
    //   not yet been set for this SetFeature() call.   
   
    pData = m_pFeatureData;   
    if (!pData)   
        return false;   
   
    for (Index = 0; Index < m_ulFeatureDataLength; Index++, pData++)    
    {   
        pData->IsDataSet = false;   
    }   
   
    // In setting all the data in the reports, we need to pack a report buffer   
    //   and call WriteFile for each report ID that is represented by the    
    //   device structure.  To do so, the IsDataSet field will be used to    
    //   determine if a given report field has already been set.   
   
    bStatus = true;   
   
    pData = m_pFeatureData;   
    if (!pData)   
        return false;   
   
    for (Index = 0; Index < m_ulFeatureDataLength; Index++, pData++)    
    {   
        if (!pData->IsDataSet)    
        {   
            // Package the report for this data structure.  PackReport will   
            //    set the IsDataSet fields of this structure and any other    
            //    structures that it includes in the report with this structure   
   
            PackReport (HidP_Feature,   
                        m_pFeatureReportBuffer,   
                        m_HidCaps.FeatureReportByteLength,   
                        m_pFeatureData,   
                        m_ulFeatureDataLength - Index,   
                        m_pHidPpd);   
   
            bReportStatus =(HidD_SetFeature (m_hDevice,   
                                                m_pFeatureReportBuffer,   
                                                m_HidCaps.FeatureReportByteLength));   
   
            bStatus = bStatus && bReportStatus;   
        }   
    }   
   
    return bStatus;   
}   
   
// scans though the HID data report and fills in any values it can   
bool CHidDevice::UnpackReport(   
                              HIDP_REPORT_TYPE ReportType,   
                              PCHAR ReportBuffer,   
                              USHORT ReportBufferLength,   
                              PHID_DATA Data,   
                              ULONG DataLength,   
                              PHIDP_PREPARSED_DATA Ppd   
                             )   
{   
    ULONG       numUsages; // Number of usages returned from GetUsages.   
    ULONG       i;   
    UCHAR       reportID;   
    ULONG       Index;   
    ULONG       nextUsage;   
   
    reportID = ReportBuffer[0];   
   
    for (i = 0; i < DataLength; i++, Data++)    
    {   
        if (reportID == Data->ReportID)    
        {   
            if (Data->IsButtonData)    
            {   
                numUsages = Data->ButtonData.MaxUsageLength;   
   
                Data->Status = HidP_GetUsages (ReportType,   
                                               Data->UsagePage,   
                                               0, // All collections   
                                               Data->ButtonData.Usages,   
                                               &numUsages,   
                                               Ppd,   
                                               ReportBuffer,   
                                               ReportBufferLength);   
   
   
                // Get usages writes the list of usages into the buffer   
                // Data->ButtonData.Usages newUsage is set to the number of usages   
                // written into this array.   
                // A usage cannot not be defined as zero, so we'll mark a zero   
                // following the list of usages to indicate the end of the list of   
                // usages   
                //   
                // NOTE: One anomaly of the GetUsages function is the lack of ability   
                //        to distinguish the data for one ButtonCaps from another   
                //        if two different caps structures have the same UsagePage   
                //        For instance:   
                //          Caps1 has UsagePage 07 and UsageRange of 0x00 - 0x167   
                //          Caps2 has UsagePage 07 and UsageRange of 0xe0 - 0xe7   
                //   
                //        However, calling GetUsages for each of the data structs   
                //          will return the same list of usages.  It is the    
                //          responsibility of the caller to set in the HID_DEVICE   
                //          structure which usages actually are valid for the   
                //          that structure.    
                //         
   
                // Search through the usage list and remove those that    
                //    correspond to usages outside the define ranged for this   
                //    data structure.   
                   
                for (Index = 0, nextUsage = 0; Index < numUsages; Index++)    
                {   
                    if (Data->ButtonData.UsageMin <= Data->ButtonData.Usages[Index] &&   
                            Data->ButtonData.Usages[Index] <= Data->ButtonData.UsageMax)    
                    {   
                        Data->ButtonData.Usages[nextUsage++] = Data->ButtonData.Usages[Index];   
                    }   
                }   
   
                if (nextUsage < Data->ButtonData.MaxUsageLength)    
                {   
                    Data->ButtonData.Usages[nextUsage] = 0;   
                }   
            }   
            else    
            {   
                Data->Status = HidP_GetUsageValue (   
                                                ReportType,   
                                                Data->UsagePage,   
                                                0,               // All Collections.   
                                                Data->ValueData.Usage,   
                                                &Data->ValueData.Value,   
                                                Ppd,   
                                                ReportBuffer,   
                                                ReportBufferLength);   
   
                if (HIDP_STATUS_SUCCESS != Data->Status)   
                {   
                    return false;   
                }   
   
                Data->Status = HidP_GetScaledUsageValue (   
                                                       ReportType,   
                                                       Data->UsagePage,   
                                                       0, // All Collections.   
                                                       Data->ValueData.Usage,   
                                                       &Data->ValueData.ScaledValue,   
                                                       Ppd,   
                                                       ReportBuffer,   
                                                       ReportBufferLength);   
            }    
   
            Data->IsDataSet = true;   
        }   
    }   
   
    return true;   
}   
   
// packages the HID report based on the data in the structures   
bool CHidDevice::PackReport(   
                            HIDP_REPORT_TYPE ReportType,   
                            PCHAR ReportBuffer,   
                            USHORT ReportBufferLength,   
                            PHID_DATA Data,   
                            ULONG DataLength,   
                            PHIDP_PREPARSED_DATA Ppd   
                           )   
{   
    ULONG       numUsages; // Number of usages to set for a given report.   
    ULONG       i;   
    ULONG       CurrReportID;   
   
    // All report buffers that are initially sent need to be zero'd out   
   
    memset (ReportBuffer, (UCHAR) 0, ReportBufferLength);   
   
    // Go through the data structures and set all the values that correspond to   
    //   the CurrReportID which is obtained from the first data structure    
    //   in the list   
   
    CurrReportID = Data->ReportID;   
   
    for (i = 0; i < DataLength; i++, Data++)    
    {   
        // There are two different ways to determine if we set the current data   
        //    structure:    
        //    1) Store the report ID were using and only attempt to set those   
        //        data structures that correspond to the given report ID.  This   
        //        example shows this implementation.   
        //   
        //    2) Attempt to set all of the data structures and look for the    
        //        returned status value of HIDP_STATUS_INVALID_REPORT_ID.  This    
        //        error code indicates that the given usage exists but has a    
        //        different report ID than the report ID in the current report    
        //        buffer   
   
        if (Data->ReportID == CurrReportID)    
        {   
            if (Data->IsButtonData)    
            {   
                numUsages = Data->ButtonData.MaxUsageLength;   
                Data->Status = HidP_SetUsages (ReportType,   
                                               Data->UsagePage,   
                                               0, // All collections   
                                               Data->ButtonData.Usages,   
                                               &numUsages,   
                                               Ppd,   
                                               ReportBuffer,   
                                               ReportBufferLength);   
            }   
            else   
            {   
                Data->Status = HidP_SetUsageValue (ReportType,   
                                                   Data->UsagePage,   
                                                   0, // All Collections.   
                                                   Data->ValueData.Usage,   
                                                   Data->ValueData.Value,   
                                                   Ppd,   
                                                   ReportBuffer,   
                                                   ReportBufferLength);   
            }   
   
            if (HIDP_STATUS_SUCCESS != Data->Status)   
            {   
                return false;   
            }   
   
            Data->IsDataSet = true;   
        }   
    }      
   
    return true;   
}   
   
// obtains the manufacturer string from the HID device   
bool CHidDevice::GetManufacturerString(PCHAR szManufacturerString, ULONG ulManufacturerStringLength)   
{   
    PWCHAR str = new WCHAR[ulManufacturerStringLength];   
   
    if (str)   
    {   
        // attempt to get the manufacturer string, which is in Unicode format   
        BOOLEAN result = HidD_GetManufacturerString(m_hDevice, str, ulManufacturerStringLength*sizeof(WCHAR));   
   
        if (result)   
        {   
            // converts the wide character string (Unicode) to multi-byte string   
            SIZE_T nBytes = wcstombs(szManufacturerString, str, ulManufacturerStringLength-1);    
            if ((SIZE_T) -1 != nBytes)    
            {   
                delete str;   
                return true;   
            }   
        }   
   
        delete str;   
    }   
   
    return false;   
}   
   
// obtains the product string from the HID device   
bool CHidDevice::GetProductString(PCHAR szProductString, ULONG ulProductStringLength)   
{   
    PWCHAR str = new WCHAR[ulProductStringLength];   
   
    if (str)   
    {   
        // attempt to get the product string, which is in Unicode format   
        BOOLEAN result = HidD_GetProductString(m_hDevice, str, ulProductStringLength*sizeof(WCHAR));   
   
        if (result)   
        {   
            // converts the wide character string (Unicode) to multi-byte string   
            SIZE_T nBytes = wcstombs(szProductString, str, ulProductStringLength-1);    
            if ((SIZE_T) -1 != nBytes)    
            {   
                delete str;   
                return true;   
            }   
        }   
   
        delete str;   
    }   
   
    return false;   
}   
   
// obtains the serial numbert string from the HID device   
bool CHidDevice::GetSerialNumberString(PCHAR szSerialNumberString, ULONG ulSerialNumberStringLength)   
{   
    PWCHAR str = new WCHAR[ulSerialNumberStringLength];   
   
    if (str)   
    {   
        // attempt to get the serial number string, which is in Unicode format   
        BOOLEAN result = HidD_GetSerialNumberString(m_hDevice, str, ulSerialNumberStringLength*sizeof(WCHAR));   
   
        if (result)   
        {   
            // converts the wide character string (Unicode) to multi-byte string   
            SIZE_T nBytes = wcstombs(szSerialNumberString, str, ulSerialNumberStringLength-1);    
            if ((SIZE_T) -1 != nBytes)    
            {   
                delete str;   
                return true;   
            }   
        }   
   
        delete str;   
    }   
   
    return false;   
}   
   
// attempts to get a registry value from the registry key where the    
// device-specific configuration information is stored for the HID device   
bool CHidDevice::RegQueryValue(LPCTSTR lpValueName,   
                                LPDWORD lpType,   
                                LPBYTE lpData,   
                                LPDWORD lpcbData)   
{   
    bool ret = false;   
   
    HDEVINFO hwDeviceInfo;   
    SP_DEVICE_INTERFACE_DATA devInterfaceData;   
    GUID hidGuid;   
   
   
    // get a copy of the HID GUID to be used below   
    HidD_GetHidGuid (&hidGuid);   
   
    // get a list of all HID devices present in the system   
    hwDeviceInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);   
    if (hwDeviceInfo != INVALID_HANDLE_VALUE)           
    {   
        HKEY hkEnum;               
   
        // loop through all HID devices, looking for a match for this specific HID device   
        int iEnumInfo = 0;         
        devInterfaceData.cbSize = sizeof(devInterfaceData);   
        while ( SetupDiEnumDeviceInterfaces(hwDeviceInfo, 0, (CONST LPGUID)&hidGuid, iEnumInfo++, &devInterfaceData) )   
        {   
            SP_DEVINFO_DATA devInfoData;   
            PSP_INTERFACE_DEVICE_DETAIL_DATA functionClassDeviceData;   
            ULONG requiredLength = 0;   
            ULONG predictedLength = 0;   
   
            SetupDiGetInterfaceDeviceDetail (   
                    hwDeviceInfo,   
                    &devInterfaceData,   
                    NULL,    
                    0,    
                    &requiredLength,   
                    NULL);    
   
            predictedLength = requiredLength;   
   
            // allocate the device data   
            devInfoData.cbSize = sizeof(devInfoData);   
            functionClassDeviceData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new UCHAR[predictedLength];   
            functionClassDeviceData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);   
   
            // retrieve the information from Plug and Play   
            if (SetupDiGetInterfaceDeviceDetail(hwDeviceInfo, &devInterfaceData, functionClassDeviceData, predictedLength, &requiredLength, &devInfoData))    
            {   
                // check to see if this HID device is a match   
                if (strcmpi(functionClassDeviceData->DevicePath, m_pHidDevicePath) == 0)   
                {   
                    // found a match, open a handle to the registry   
                    // where device-specific configuration information    
                    // is stored   
                    hkEnum = SetupDiOpenDevRegKey ( hwDeviceInfo,   
                        &devInfoData,   
                        DICS_FLAG_GLOBAL,   
                        0,   
                        DIREG_DEV,   
                        KEY_ALL_ACCESS );   
   
                    if (hkEnum != INVALID_HANDLE_VALUE)   
                    {   
                        // if the reg handle was open, attempt to obtain   
                        // the registry value   
                        if (RegQueryValueEx(hkEnum,    
                            lpValueName,    
                            NULL,    
                            lpType,    
                            lpData,    
                            lpcbData   
                            ) == ERROR_SUCCESS)   
                        {   
                            // indicate success   
                            ret = true;   
                        }   
   
                        // close the reg handle   
                        RegCloseKey ( hkEnum );   
                    }   
       
                    // release the allocated device data and    
                    // leave the loop   
                    delete [] functionClassDeviceData;   
                    break;   
                }   
            }   
   
            // release the allocated device data   
            delete [] functionClassDeviceData;   
        }   
   
        // destroy the list that was created above   
        SetupDiDestroyDeviceInfoList(hwDeviceInfo);   
    }   
   
    return ret;   
}   
   
// attempts to set a registry value in the registry key where the    
// device-specific configuration information is stored for the HID device   
bool CHidDevice::RegSetValue(LPCTSTR lpValueName,   
                                DWORD Type,   
                                LPBYTE lpData,   
                                DWORD cbData)   
{   
    bool ret = false;   
   
    HDEVINFO hwDeviceInfo;   
    SP_DEVICE_INTERFACE_DATA devInterfaceData;   
    GUID hidGuid;   
   
   
    // get a copy of the HID GUID to be used below   
    HidD_GetHidGuid (&hidGuid);   
   
    // get a list of all HID devices present in the system   
    hwDeviceInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);   
    if (hwDeviceInfo != INVALID_HANDLE_VALUE)           
    {   
        HKEY hkEnum;               
   
        // loop through all HID devices, looking for a match for this specific HID device   
        int iEnumInfo = 0;         
        devInterfaceData.cbSize = sizeof(devInterfaceData);   
        while ( SetupDiEnumDeviceInterfaces(hwDeviceInfo, 0, (CONST LPGUID)&hidGuid, iEnumInfo++, &devInterfaceData) )   
        {   
            SP_DEVINFO_DATA devInfoData;   
            PSP_INTERFACE_DEVICE_DETAIL_DATA functionClassDeviceData;   
            ULONG requiredLength = 0;   
            ULONG predictedLength = 0;   
   
            SetupDiGetInterfaceDeviceDetail (   
                    hwDeviceInfo,   
                    &devInterfaceData,   
                    NULL,    
                    0,    
                    &requiredLength,   
                    NULL);    
   
            predictedLength = requiredLength;   
   
            // allocate the device data   
            devInfoData.cbSize = sizeof(devInfoData);   
            functionClassDeviceData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new UCHAR[predictedLength];   
            functionClassDeviceData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);   
   
            // retrieve the information from Plug and Play   
            if (SetupDiGetInterfaceDeviceDetail(hwDeviceInfo, &devInterfaceData, functionClassDeviceData, predictedLength, &requiredLength, &devInfoData))    
            {   
                // check to see if this HID device is a match   
                if (strcmpi(functionClassDeviceData->DevicePath, m_pHidDevicePath) == 0)   
                {   
                    // found a match, open a handle to the registry   
                    // where device-specific configuration information    
                    // is stored   
                    hkEnum = SetupDiOpenDevRegKey ( hwDeviceInfo,   
                        &devInfoData,   
                        DICS_FLAG_GLOBAL,   
                        0,   
                        DIREG_DEV,   
                        KEY_ALL_ACCESS );   
   
                    if (hkEnum != INVALID_HANDLE_VALUE)   
                    {   
                        if (RegSetValueEx(hkEnum,    
                            lpValueName,    
                            NULL,    
                            Type,    
                            lpData,    
                            cbData   
                            ) == ERROR_SUCCESS)   
                        {   
                            // indicate success   
                            ret = true;   
                        }   
   
                        // close the reg handle   
                        RegCloseKey ( hkEnum );   
                    }   
   
                    // release the allocated device data and    
                    // leave the loop   
                    delete [] functionClassDeviceData;   
                    break;   
                }   
            }   
   
            // release the allocated device data   
            delete [] functionClassDeviceData;   
        }   
   
        // destroy the list that was created above   
        SetupDiDestroyDeviceInfoList(hwDeviceInfo);   
    }   
   
    return ret;   
}   
   
   
CHidManager::CHidManager()   
{   
    m_OS.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);   
    GetVersionEx( &m_OS );   
   
    InitializeHidManager();   
}   
   
CHidManager::~CHidManager(void)   
{   
    // remove all HID devices in the maintained list   
    RemoveAllHidDevices();   
   
    // destroy the invisible window, if it exists   
    if (m_hWnd)   
    {   
        ::DestroyWindow(m_hWnd);   
        m_hWnd = NULL;   
    }   
   
    // if we registered for notificiation for this device, then clean up   
    if (m_hDeviceNotification)   
    {   
        // issue callback indicating the device notification is being unregistered   
        if (m_pHidCallback) (*m_pHidCallback)(m_pHidCallbackParam, NULL, HIDCB_NOTIFY_DEVICE_UNREGISTERED);   
   
        // unregister device notification   
        // NOTE: for Windows 98x, it can become unstable if you unregister the device notification   
        if (m_OS.dwMajorVersion > 4)   
        {   
            ::UnregisterDeviceNotification(m_hDeviceNotification);   
        }   
        m_hDeviceNotification = NULL;   
    }   
   
    // NULL callback function and parameter pointers   
    m_pHidCallback = NULL;   
    m_pHidCallbackParam = NULL;   
}   
   
// standard initialization   
void CHidManager::InitializeHidManager()   
{   
    m_pThis = this;   
   
    m_bEnabled = false;   
    m_hWnd = NULL;   
    m_hDeviceNotification = NULL;   
    m_pHidCallback = NULL;   
    m_pHidCallbackParam = NULL;   
   
    m_pHidDeviceListHead = NULL;   
    m_pCurrentHidDevice = NULL;   
    m_nHidDevices = 0;   
   
    ZeroMemory(m_bUniqueIDs, HIDMGR_SUPPORTED_HID_DEVICES);   
}   
   
   
// registers for notification of events for all HID class devices and   
// calls HID callback function to indicate registration was completed   
bool CHidManager::RegisterHidNotification(HWND hWnd)   
{   
    DEV_BROADCAST_DEVICEINTERFACE broadcastInterface = {0};   
   
    broadcastInterface.dbcc_size = sizeof(broadcastInterface);   
    broadcastInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;   
   
    // get HID GUID   
    ::HidD_GetHidGuid(&broadcastInterface.dbcc_classguid);   
   
    m_hDeviceNotification = ::RegisterDeviceNotification(hWnd, (PVOID) &broadcastInterface, DEVICE_NOTIFY_WINDOW_HANDLE);   
    if (!m_hDeviceNotification) return false;   
   
    if (m_pHidCallback) (*m_pHidCallback)(m_pHidCallbackParam, NULL, HIDCB_NOTIFY_DEVICE_REGISTERED);   
   
    return true;   
}   
   
   
// registers a window class for subsequent use in calls to CreateWindow, used below   
ATOM CHidManager::RegisterClass(HINSTANCE hInstance)   
{   
    WNDCLASSEX wcex;   
   
    wcex.cbSize = sizeof(WNDCLASSEX);    
   
    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;   
    //wcex.lpfnWndProc  = (WNDPROC)WindowProc;                                  /////////////////////////////////////////////   
    wcex.cbClsExtra     = 0;   
    wcex.cbWndExtra     = 0;   
    wcex.hInstance      = hInstance;   
    wcex.hIcon          = 0;   
    wcex.hCursor        = 0;   
    wcex.hbrBackground  = 0;   
    wcex.lpszMenuName   = 0;   
    wcex.lpszClassName  = HIDMANAGER_CLASS;   
    wcex.hIconSm        = 0;   
   
    return ::RegisterClassEx(&wcex);   
}   
   
// creates an invisible window and uses the returned window handle    
// to register for HID device notification events, then it creates    
// a list of existing HID devices that will be maintained by the    
// HID manager   
bool CHidManager::Create(HINSTANCE hInst, HWND hParentWnd, PHidCallback pHidCallback, void *pHidCallbackParam)   
{   
    bool bResult = true;   
   
    m_pHidCallback = pHidCallback;   
    m_pHidCallbackParam = pHidCallbackParam;   
   
    // class so WindowProc function is called   
    RegisterClass(hInst);   
   
    // Create an invisible window   
    m_hWnd = ::CreateWindow(HIDMANAGER_CLASS, _T(""), WS_POPUP,    
                            CW_USEDEFAULT,CW_USEDEFAULT,    
                            CW_USEDEFAULT,CW_USEDEFAULT,    
                            NULL, 0,   
                            hInst, 0);   
       
    if (!RegisterHidNotification(m_hWnd))   
    {   
        bResult = false;   
    }   
       
    if (bResult)   
    {   
        // setup all existing and valid HID devices   
        RefreshHidDevices();   
   
        // everything was created successfully    
        m_bEnabled = true;   
    }   
   
    return bResult;   
}   
   
// attempts to open a handle to the HID device to determine    
// if it is present (or not) and returns the result   
bool CHidManager::IsHidDevicePresent(CHidDevice *pHidDevice)   
{   
    // attempt to open a handle to the HID device, if the device   
    // is present then this should return success   
    HANDLE h = ::CreateFile(pHidDevice->m_pHidDevicePath,   
                            GENERIC_WRITE | GENERIC_READ,   
                            FILE_SHARE_WRITE | FILE_SHARE_READ,   
                            NULL,   
                            OPEN_EXISTING,   
                            0,   
                            NULL);   
   
    if (h != INVALID_HANDLE_VALUE)   
    {   
        // close handle and indicate device is present   
        ::CloseHandle(h);   
        return true;   
    }   
   
    return false;   
}   
   
// attempts to create and maintain a unique ID for the associated HID device   
USHORT CHidManager::CreateUniqueDeviceID()   
{   
    USHORT i;   
   
    for (i=0;i<HIDMGR_SUPPORTED_HID_DEVICES;i++)   
    {   
        if (!m_bUniqueIDs[i])   
        {   
            m_bUniqueIDs[i] = true;   
            return i;   
        }   
    }   
   
    return 0xffff;   
}   
   
// frees the specified unique ID   
void CHidManager::FreeUniqueDeviceID(USHORT uID)   
{   
    if (uID < HIDMGR_SUPPORTED_HID_DEVICES)   
    {   
        m_bUniqueIDs[uID] = false;   
    }   
}   
   
// returns a pointer to the first HID device in the list   
CHidDevice* CHidManager::GetFirstHidDevice()   
{   
    m_pCurrentHidDevice = m_pHidDeviceListHead;   
    return m_pCurrentHidDevice;   
}   
   
// returns a pointer to the next HID device in the list   
CHidDevice* CHidManager::GetNextHidDevice()   
{   
    if (m_pCurrentHidDevice)   
        m_pCurrentHidDevice = m_pCurrentHidDevice->nextHidDevice;   
   
    return m_pCurrentHidDevice;   
}   
   
// returns a pointer to the current HID device in the list   
CHidDevice* CHidManager::GetCurrentHidDevice()   
{   
    return m_pCurrentHidDevice;   
}   
   
// scans the current list of HID devices and returns a pointer    
// to the HID device that matches the device path provided   
CHidDevice* CHidManager::GetHidDeviceWithPath(PCHAR pDevicePath)   
{   
    CHidDevice * pCurHidDevice = m_pHidDeviceListHead;   
   
    while (pCurHidDevice)   
    {   
        if (!lstrcmpi(pCurHidDevice->m_pHidDevicePath, pDevicePath))   
            break;   
   
        pCurHidDevice = pCurHidDevice->nextHidDevice;   
    }   
   
    return pCurHidDevice;   
}   
   
// scans the current list of HID devices and returns a pointer    
// to the HID device that matches the device handle provided   
CHidDevice* CHidManager::GetHidDeviceWithHandle(HANDLE hHidDevice)   
{   
    CHidDevice * pCurHidDevice = m_pHidDeviceListHead;   
   
    while (pCurHidDevice)   
    {   
        if (pCurHidDevice->GetDeviceHandle() == hHidDevice)   
            break;   
   
        pCurHidDevice = pCurHidDevice->nextHidDevice;   
    }   
   
    return pCurHidDevice;   
}   
   
// determines if the HID device already exists in the list   
bool CHidManager::HidDeviceAlreadyExist(CHidDevice *pHidDevice)   
{   
    CHidDevice * pCurHidDevice = m_pHidDeviceListHead;   
   
    while (pCurHidDevice)   
    {   
        if (pCurHidDevice == pHidDevice)   
            return true;   
   
        pCurHidDevice = pCurHidDevice->nextHidDevice;   
    }   
   
    return false;   
}   
   
// checks if the provided HID device already exists, and if not,    
// adds the new HID device to the end of the list, increments the    
// HID device counter, and call the HID callback function to indicate    
// a new HID device was added   
bool CHidManager::AddHidDevice(CHidDevice *pHidDevice)   
{   
    // determine if HID device already exists   
    if (HidDeviceAlreadyExist(pHidDevice))   
        return false;   
   
    // add HID device to end of list, if first device   
    // then set it as the first device in the list   
    if (m_pHidDeviceListHead == NULL)   
    {   
        m_pHidDeviceListHead = pHidDevice;   
    }   
    else   
    {   
        CHidDevice * pHidTemp = m_pHidDeviceListHead;   
   
        while (pHidTemp->nextHidDevice != NULL)   
        {   
            pHidTemp = pHidTemp->nextHidDevice;   
        }   
   
        pHidTemp->nextHidDevice = pHidDevice;   
    }   
   
    // increment HID device counter   
    m_nHidDevices++;   
   
    // call HID callback function to indicate HID device was added   
    if (m_pHidCallback) (*m_pHidCallback)(m_pHidCallbackParam, pHidDevice, HIDCB_NOTIFY_DEVICE_ADDED);   
   
    return true;   
}   
   
// closes outstanding handle to the HID device, call HID    
// callback function to indicate HID device is being removed,    
// removes the HID device from the list, and deletes the HID device   
bool CHidManager::RemoveHidDevice(CHidDevice *pHidDevice)   
{   
    // check for valid HID device pointer   
    if (pHidDevice)   
    {   
        // close handle to HID device   
        pHidDevice->CloseHidDevice();   
   
        CHidDevice * pHidTemp = m_pHidDeviceListHead;   
        CHidDevice * pHidPrev = NULL;   
   
        // loop through all HID devices in the list to find the    
        // matching HID device to be removed   
        while (pHidTemp)   
        {   
            // check if HID device was found   
            if (pHidTemp == pHidDevice)   
            {   
                // found, so decrement HID device counter   
                m_nHidDevices--;   
   
                // call HID callback function to indicate HID device is    
                // being removed   
                if (m_pHidCallback) (*m_pHidCallback)(m_pHidCallbackParam, pHidDevice, HIDCB_NOTIFY_DEVICE_REMOVED);   
   
                // remove HID device from list   
                if (pHidPrev)   
                {   
                    pHidPrev->nextHidDevice = pHidTemp->nextHidDevice;   
                }   
   
                if (pHidTemp == m_pHidDeviceListHead)   
                {   
                    m_pHidDeviceListHead = pHidTemp->nextHidDevice;   
                }   
   
                // delete HID device   
                DeleteHidDevice(pHidTemp);   
   
                // end loop   
                pHidTemp = NULL;   
            }   
            else   
            {   
                // get next HID device in list   
                pHidPrev = pHidTemp;   
                pHidTemp = pHidTemp->nextHidDevice;   
            }   
        }   
    }   
       
    return true;   
}   
   
// scans though all HID devices in the list and removes them   
void CHidManager::RemoveAllHidDevices()   
{   
    CHidDevice * pCurHidDevice = m_pHidDeviceListHead;   
   
    while (pCurHidDevice)   
    {   
        CHidDevice * pNextHidDevice = pCurHidDevice->nextHidDevice;   
   
        RemoveHidDevice(pCurHidDevice);   
   
        pCurHidDevice = pNextHidDevice;   
    }   
   
    m_pHidDeviceListHead = NULL;   
}   
   
// validates that all HID devices in the list are still present,    
// removes those from the list that are currently not present,    
// scans the list of all existing HID devices present, and then    
// attempts to add the existing HID devices to the list   
bool CHidManager::RefreshHidDevices(void)   
{   
    bool                     bRetVal = false;   
    CHidDevice               *pCurHidDevice = m_pHidDeviceListHead;   
   
    // first check that all devices are still valid   
    // remove if not   
    while (pCurHidDevice)   
    {   
        CHidDevice * pNextHidDevice = pCurHidDevice->nextHidDevice;   
   
        if (!IsHidDevicePresent(pCurHidDevice))   
        {   
            RemoveHidDevice(pCurHidDevice);   
            bRetVal = true;   
        }   
   
        pCurHidDevice = pNextHidDevice;   
    }   
   
   
    // open a handle to the plug and play dev node, SetupDiGetClassDevs()    
    // returns a device information set that contains info on all installed    
    // devices of a specified class.   
    GUID                             hidGuid;   
    HDEVINFO                         hardwareDeviceInfo;   
    SP_DEVICE_INTERFACE_DATA         deviceInfoData;   
    PSP_DEVICE_INTERFACE_DETAIL_DATA functionClassDeviceData = NULL;   
    ULONG                            predictedLength = 0;   
    ULONG                            requiredLength = 0;   
    ULONG                            i = 0;   
    bool                             bDone = false;   
   
    HidD_GetHidGuid (&hidGuid);   
   
    hardwareDeviceInfo = SetupDiGetClassDevs (&hidGuid,   
                                                NULL, // define no enumerator (all HID devices)   
                                                NULL, // define no   
                                                (DIGCF_PRESENT | // only devices present   
                                                DIGCF_INTERFACEDEVICE)); // function class devices.   
   
    if (hardwareDeviceInfo == INVALID_HANDLE_VALUE)   
    {   
        // return true to indicate something may have changed   
        return true;   
    }   
   
    deviceInfoData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);   
   
    while (!bDone)   
    {   
        if (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,   
                                            0, // don't care about specific PDOs   
                                            &hidGuid,   
                                            i,   
                                            &deviceInfoData))   
        {   
            //   
            // allocate a function class device data structure to receive the   
            // goods about this particular device.   
   
            SetupDiGetDeviceInterfaceDetail (   
                    hardwareDeviceInfo,   
                    &deviceInfoData,   
                    NULL, // probing so no output buffer yet   
                    0, // probing so output buffer length of zero   
                    &requiredLength,   
                    NULL); // not interested in the specific dev-node   
   
   
            predictedLength = requiredLength;   
   
            functionClassDeviceData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new UCHAR[predictedLength];   
            if (functionClassDeviceData)   
            {   
                ZeroMemory (functionClassDeviceData, predictedLength);   
                functionClassDeviceData->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);   
            }   
            else   
            {   
                SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);   
   
                // return true to indicate something may have changed   
                return true;   
            }   
   
            //   
            // retrieve the information from Plug and Play.   
            //   
   
            if (! SetupDiGetDeviceInterfaceDetail (   
                        hardwareDeviceInfo,   
                        &deviceInfoData,   
                        functionClassDeviceData,   
                        predictedLength,   
                        &requiredLength,   
                        NULL))    
            {   
                SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);   
                delete [] functionClassDeviceData;   
   
                // return true to indicate something may have changed   
                return true;   
            }   
   
            HidDeviceArrival(functionClassDeviceData->DevicePath);    
   
            delete [] functionClassDeviceData;   
        }   
        else    
        {   
            if (GetLastError() == ERROR_NO_MORE_ITEMS)   
            {   
                bDone = true;   
            }   
        }   
           
        i++;   
    }   
   
    return bRetVal;   
}   
   
// makes sure the HID device does not already exist in the list,    
// and then creates a new HID device device, opens a handle to    
// the device, adds the new HID device to the list, and registers    
// event notification for this new HID device   
void CHidManager::HidDeviceArrival(PCHAR pDevicePath)   
{   
    // attempt to match a HID device in the list   
    // based on the device path provided, if not then   
    // it is okay to add this HID device to the list   
    // otherwise the HID device already exists in our   
    // list   
    if (!GetHidDeviceWithPath(pDevicePath))   
    {   
        // this HID device is not in our list, so create    
        // a new one and attempt to add it to the list   
        CHidDevice *pNewHidDevice = NewHidDevice(pDevicePath);   
   
        if (pNewHidDevice)    
        {   
            // make sure we can open a handle to   
            // the HID device before we add it    
            // to the list   
            if (pNewHidDevice->OpenHidDevice())   
            {   
                // handle to the HID device open, so   
                // now add it to the list and register   
                // it for event notification   
                AddHidDevice(pNewHidDevice);   
                pNewHidDevice->RegisterHidDevice(m_hWnd);   
            }   
            else   
            {   
                // could not open handle, so just delete    
                // the HID device craeated   
                DeleteHidDevice(pNewHidDevice);   
            }   
        }   
    }   
}   
   
// readies the HID device for removal by making sure the handle is closed,    
// this version expects the device path   
void CHidManager::HidDeviceQueryRemoval(PCHAR pDevicePath)   
{   
    CHidDevice *pHidDevice = GetHidDeviceWithPath(pDevicePath);   
   
    if (pHidDevice)   
    {   
        pHidDevice->CloseHidDevice();   
    }   
}   
   
// readies the HID device for removal by making sure the handle is closed,    
// this version expects a handle to the device   
void CHidManager::HidDeviceQueryRemoval(HANDLE hHidDevice)   
{   
    CHidDevice *pHidDevice = GetHidDeviceWithHandle(hHidDevice);   
   
    if (pHidDevice)   
    {   
        pHidDevice->CloseHidDevice();   
    }   
}   
   
// removes the HID device, this version expects the    
// device path   
void CHidManager::HidDeviceRemoval(PCHAR pDevicePath)   
{   
    CHidDevice *pHidDevice = GetHidDeviceWithPath(pDevicePath);   
   
    if (pHidDevice)   
    {   
        RemoveHidDevice(pHidDevice);   
    }   
}   
   
// removes the HID device, this version expects a   
// handle to the device   
void CHidManager::HidDeviceRemoval(HANDLE hHidDevice)   
{   
    CHidDevice *pHidDevice = GetHidDeviceWithHandle(hHidDevice);   
   
    if (pHidDevice)   
    {   
        RemoveHidDevice(pHidDevice);   
    }   
}   
   
/*  
// the global (static) callback function for the CHidManager window  
LRESULT PASCAL CHidManager::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)  
{  
    CHidManager* pHidManager = m_pThis;  
  
    // make sure our window is the target for this message  
    if (pHidManager->GetSafeHwnd() == hWnd)  
    {   
        if (message == WM_DEVICECHANGE)  
        {  
            PDEV_BROADCAST_HDR broadcastHdr = (PDEV_BROADCAST_HDR) lParam;  
  
            if (wParam == DBT_DEVICEARRIVAL)  
            {  
                if (broadcastHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)  
                {  
                    PDEV_BROADCAST_DEVICEINTERFACE pBroadcastInterface = (PDEV_BROADCAST_DEVICEINTERFACE) lParam;  
  
                    // HID device arrival  
                    pHidManager->HidDeviceArrival(pBroadcastInterface->dbcc_name);  
                }  
            }  
            else if (wParam == DBT_DEVICEQUERYREMOVE)  
            {  
                // if this message is received, the device is either  
                // being disabled or removed through device manager.  
                // To properly handle this request, we need to close  
                // the handle to the device.  
  
                if (broadcastHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)  
                {  
                    PDEV_BROADCAST_DEVICEINTERFACE pBroadcastInterface = (PDEV_BROADCAST_DEVICEINTERFACE) lParam;  
  
                    // HID device query removal  
                    pHidManager->HidDeviceQueryRemoval(pBroadcastInterface->dbcc_name);  
                }  
                else if (broadcastHdr->dbch_devicetype == DBT_DEVTYP_HANDLE)  
                {  
                    PDEV_BROADCAST_HANDLE pBroadcastDevice = (PDEV_BROADCAST_HANDLE) lParam;  
  
                    // HID device query removal  
                    pHidManager->HidDeviceQueryRemoval(pBroadcastDevice->dbch_handle);  
                }  
            }  
            else if ((wParam == DBT_DEVICEREMOVEPENDING) || (wParam == DBT_DEVICEREMOVECOMPLETE))  
            {  
                // do the same steps for DBT_DEVICEREMOVEPENDING and   
                // DBT_DEVICEREMOVECOMPLETE    
                  
                // the DBT_DEVICEREMOVECOMPLETE request is not called  
                // for a device if it is disabled or removed via   
                // the Device Manager, however, in this case it will   
                // receive the DBT_DEVICEREMOVEPENDING    
  
                // remove the device from our currently displayed  
                // list of devices and unregister notification  
  
                if (broadcastHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)  
                {  
                    PDEV_BROADCAST_DEVICEINTERFACE pBroadcastInterface = (PDEV_BROADCAST_DEVICEINTERFACE) lParam;  
  
                    // HID device removal  
                    pHidManager->HidDeviceRemoval(pBroadcastInterface->dbcc_name);  
                }  
                else if (broadcastHdr->dbch_devicetype == DBT_DEVTYP_HANDLE)  
                {  
                    PDEV_BROADCAST_HANDLE pBroadcastDevice = (PDEV_BROADCAST_HANDLE) lParam;  
  
                    // HID device removal  
                    pHidManager->HidDeviceRemoval(pBroadcastDevice->dbch_handle);  
                }  
            }  
        }  
        else if (message == WM_UNREGISTER_HANDLE)  
        {  
            ::UnregisterDeviceNotification ( (HDEVNOTIFY) lParam );   
        }  
    }  
  
    // now the process the message, just default it  
    return ::DefWindowProc(hWnd, message, wParam, lParam);  
}  
*/   