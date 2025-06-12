#pragma once
//+-------------------------------------------------------------------------
//
//  StobjectEx - Windows XP Systray shell service object
//  Copyright (C) Microsoft
//
//  File:       shundoc.h
//
//  History:    Jun-11-25   kfh83     Created
//
//--------------------------------------------------------------------------

#include <Windows.h>

//
//  Macros
//

#define ARGUMENT_PRESENT(a)     (a != NULL)

//
//  Enums
//

enum {
    SSOCMDID_OPEN        = 2,
    SSOCMDID_CLOSE       = 3,
};

//
//  Structs
//

typedef struct _BATTERY_STATE{
    ULONG                  ulSize;                 // Size of structure.
    struct _BATTERY_STATE  *bsNext;                // Next in list
    struct _BATTERY_STATE  *bsPrev;                // Previous in list
    ULONG                  ulBatNum;               // Display battery number.
    ULONG                  ulTag;                  // Zero implies no battery.
    HANDLE                 hDevice;                // Handle to the battery device.
#ifdef WINNT
    HDEVNOTIFY             hDevNotify;             // Device notification handle.
#endif
    UINT                   uiIconIDcache;          // Cache of the last Icon ID.
    HICON                  hIconCache;             // Cache of the last Icon handle.
    HICON                  hIconCache16;           // As above but 16x16.
    LPTSTR                 lpszDeviceName;         // The name of the battery device
    ULONG                  ulFullChargedCapacity;  // Same as PBATTERY_INFORMATION->FullChargedCapacity.
    ULONG                  ulPowerState;           // Same flags as PBATTERY_STATUS->PowerState.
    ULONG                  ulBatLifePercent;       // Battery life remaining as percentage.
    ULONG                  ulBatLifeTime;          // Battery life remaining as time in seconds.
    ULONG                  ulLastTag;              // Previous value of ulTag.
    ULONG                  ulLastPowerState;       // Previous value of ulPowerState.
    ULONG                  ulLastBatLifePercent;   // Previous value of ulBatLifePercent.
    ULONG                  ulLastBatLifeTime;      // Previous value of ulBatLifeTime.
} BATTERY_STATE, *PBATTERY_STATE;

//
//  Typedefs
//

typedef HANDLE LPSHChangeNotificationLock;

