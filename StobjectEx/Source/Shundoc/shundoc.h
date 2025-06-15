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
#include <wmistr.h>

//
//  Macros
//

#define ARGUMENT_PRESENT(a)     (a != NULL)

#define ALL     &g_bs
#define DEVICES g_bs.bsNext

#define NUM_BAT 8

#define IDI_BATFULL     200
#define IDI_BATHALF     201
#define IDI_BATLOW      202
#define IDI_BATDEAD     203
#define IDI_UNKNOWN     204
#define IDI_BATGONE     205
#define IDI_PLUG        206
#define IDI_CHARGE      207
#define IDI_BATTPLUG    208
#define IDI_BATTERY     209

#define IDB_BATTS                       300
#define IDB_BATTS16                     301

#define FIRST_ICON_IMAGE        IDI_BATFULL


#define WM_DESTROYBATMETER WM_USER+100

#define UPDATESTATUS_NOUPDATE        0
#define UPDATESTATUS_UPDATE          1
#define UPDATESTATUS_UPDATE_CHARGE   2

//  Control identifiers of IDD_BATMETER
#define IDC_BATTERYLEVEL                1001
#define IDC_REMAINING                   1002
#define IDC_POWERSTATUSICON             1003
#define IDC_POWERSTATUSBAR              1004
#define IDC_BARPERCENT                  1005
#define IDC_CHARGING                    1006
#define IDC_MOREINFO                    1007
#define IDC_BATNUM0                     1008
#define IDC_TOTALTIME                   1067
#define IDC_TIMEREMAINING               1068
#define IDC_CURRENTPOWERSOURCE          1069
#define IDC_TOTALBATPWRREMAINING        1070

// next eight must be consecutive...
#define IDC_POWERSTATUSICON1            1010
#define IDC_POWERSTATUSICON2            1011
#define IDC_POWERSTATUSICON3            1012
#define IDC_POWERSTATUSICON4            1013
#define IDC_POWERSTATUSICON5            1014
#define IDC_POWERSTATUSICON6            1015
#define IDC_POWERSTATUSICON7            1016
#define IDC_POWERSTATUSICON8            1017

// next eight must be consecutive...
#define IDC_REMAINING1                  1020
#define IDC_REMAINING2                  1021
#define IDC_REMAINING3                  1022
#define IDC_REMAINING4                  1023
#define IDC_REMAINING5                  1024
#define IDC_REMAINING6                  1025
#define IDC_REMAINING7                  1026
#define IDC_REMAINING8                  1027

// next eight must be consecutive...
#define IDC_STATUS1                     1030
#define IDC_STATUS2                     1031
#define IDC_STATUS3                     1032
#define IDC_STATUS4                     1033
#define IDC_STATUS5                     1034
#define IDC_STATUS6                     1035
#define IDC_STATUS7                     1036
#define IDC_STATUS8                     1037

// next eight must be consecutive...
#define IDC_BATNUM1                     1040
#define IDC_BATNUM2                     1041
#define IDC_BATNUM3                     1042
#define IDC_BATNUM4                     1043
#define IDC_BATNUM5                     1044
#define IDC_BATNUM6                     1045
#define IDC_BATNUM7                     1046
#define IDC_BATNUM8                     1047

// String identifiers of IDD_BATMETER.
#define IDS_ACLINEONLINE                        100
#define IDS_BATTERYLEVELFORMAT                  101
#define IDS_UNKNOWN                             102
#define IDS_PERCENTREMAININGFORMAT              104
#define IDS_TIMEREMFORMATHOUR                   105
#define IDS_TIMEREMFORMATMIN                    106
#define IDS_BATTERIES                           109
#define IDS_NOT_PRESENT                         110
#define IDS_BATTCHARGING                        111
#define IDS_BATNUM                              112
#define IDS_BATTERYNUMDETAILS                   113
#define IDS_BATTERY_POWER_ON_LINE               114
#define IDS_BATTERY_DISCHARGING                 115
#define IDS_BATTERY_CHARGING                    116
#define IDS_BATTERY_CRITICAL                    117

// DisplayFreeStr bFree parameters:
#define FREE_STR    TRUE
#define NO_FREE_STR FALSE

// RemoveMissingProc lParam2 parameters:
#define REMOVE_MISSING  0
#define REMOVE_ALL      1

// Macro to compute the actual size of a WCHAR or DBCS string

#define WSTRSIZE(str) (ULONG) ( (str) ? ((PCHAR) &str[wcslen(str)] - (PCHAR)str) + sizeof(UNICODE_NULL) : 0 )
#define STRSIZE(str)  (ULONG) ( (str) ? ((PCHAR) &str[strlen(str)] - (PCHAR)str) + 1 : 0 )

//  Dialog box control identifiers.
#define IDD_BATMETER                    100
#define IDD_BATDETAIL                   126
#define IDD_MOREINFO                    127


#define IDC_PARENT           (FCIDM_BROWSERFIRST + 1)
#define IDC_NEWFOLDER        (FCIDM_BROWSERFIRST + 2)
#define IDC_VIEWLIST         (FCIDM_BROWSERFIRST + 3)
#define IDC_VIEWDETAILS      (FCIDM_BROWSERFIRST + 4)
#define IDC_DROPDRIVLIST     (FCIDM_BROWSERFIRST + 5)
#define IDC_REFRESH          (FCIDM_BROWSERFIRST + 6)
#define IDC_PREVIOUSFOLDER   (FCIDM_BROWSERFIRST + 7)
#define IDC_JUMPDESKTOP      (FCIDM_BROWSERFIRST + 9)
#define IDC_VIEWMENU         (FCIDM_BROWSERFIRST + 10)
#define IDC_BACK             (FCIDM_BROWSERFIRST + 11)

#define PWRMANHLP TEXT("PWRMN.HLP")

//  Control identifiers of IDD_BATDETAIL
#define IDC_BAT_NUM_GROUP               1100
#define IDC_STATE                       1101
#define IDC_CHEM                        1102
#define IDC_DEVNAME                     1103
#define IDC_BATMANDATE                  1104
#define IDC_BATID                       1105
#define IDC_BATMANNAME                  1106
#define IDC_REFRESH                     1107
#define IDC_BATMETERGROUPBOX            1108
#define IDC_BATMETERGROUPBOX1           1109
#define IDC_BATTERYNAME                 1110
#define IDC_UNIQUEID                    1111
#define IDC_MANUFACTURE                 1112
#define IDC_DATEMANUFACTURED            1113
#define IDC_CHEMISTRY                   1114
#define IDC_POWERSTATE                  1115

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
typedef LRESULT (CALLBACK *WALKENUMPROC)(PBATTERY_STATE, HWND, LPARAM, LPARAM);
typedef PVOID WMIHANDLE, *PWMIHANDLE, MOFHANDLE, *PMOFHANDLE;
typedef void (
#ifndef MIDL_PASS
WINAPI
#endif
*NOTIFICATIONCALLBACK)(
    PWNODE_HEADER Wnode,
    UINT_PTR NotificationContext
    );

//
//  Functions
//

HWND DestroyBatMeter(HWND hWnd);
BOOL BatMeterCapabilities(PUINT* ppuiBatCount);
BOOL UpdateBatMeter(HWND hWnd, BOOL bShowMulti, BOOL bForceUpdate, PBATTERY_STATE pbsComposite);
HWND CreateBatMeter(HWND hwndParent, HWND hwndFrame, BOOL bShowMulti, PBATTERY_STATE pbsComposite);
LPTSTR CDECL LoadDynamicString(UINT StringID, ...);

//
//  Load module stuff
//

extern BOOL(WINAPI* PowerCapabilities)();
extern ULONG(WINAPI* WmiCloseBlock)(IN WMIHANDLE DataBlockHandle);
extern ULONG(WINAPI* WmiOpenBlock)(IN GUID *Guid, IN ULONG DesiredAccess, OUT WMIHANDLE *DataBlockHandle);
extern ULONG(WINAPI* WmiReceiveNotificationsW)(IN ULONG HandleCount, IN HANDLE *HandleList, IN NOTIFICATIONCALLBACK Callback, IN ULONG_PTR DeliveryContext);


