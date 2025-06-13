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

#include "shundoc.h"

#include <commctrl.h>
#include <dbt.h>
#include <poclass.h>
#include <wchar.h>

#include <powerbase.h>
#include <shlobj_core.h>

HINSTANCE   g_hInstance;        // Global instance handle of this DLL.
HWND        g_hwndParent;       // Parent of the battery meter.
HWND        g_hwndBatMeter;     // Battery meter.

// Global battery state list. This list has the composite system battery state
// as it's always present head. individual battery devices are linked to this
// head. Use WalkBatteryState(ALL, ... to walk the entire list, including the
// head. Use WalkBatteryState(DEVICES, ... to walk just the device list. If a
// battery is in this list, it's displayable. g_uiBatCount is the count of
// battery devices in this list. The composite battery is not counted. The
// g_pbs array provides a handy UI battery number to pbs conversion. The
// following three variables are only changed during DeviceChanged.

BATTERY_STATE   g_bs;
UINT            g_uiBatCount;
PBATTERY_STATE  g_pbs[NUM_BAT+1];
LPTSTR          g_lpszDriverNames[NUM_BAT];
UINT            g_uiDriverCount;
BOOL            g_bShowingMulti;

// The following constant global array is used to walk through the
// control ID's in the battery metter dialog box. It makes getting
// a control ID from a battery number easy.

#define BAT_ICON      0
#define BAT_STATUS    1
#define BAT_REMAINING 2
#define BAT_NUM       3
#define BAT_LAST      BAT_NUM+1

UINT g_iMapBatNumToID [NUM_BAT+1][4]={
    {IDC_POWERSTATUSICON,  IDC_POWERSTATUSBAR, IDC_REMAINING, IDC_BATNUM0},
    {IDC_POWERSTATUSICON1, IDC_STATUS1, IDC_REMAINING1, IDC_BATNUM1},
    {IDC_POWERSTATUSICON2, IDC_STATUS2, IDC_REMAINING2, IDC_BATNUM2},
    {IDC_POWERSTATUSICON3, IDC_STATUS3, IDC_REMAINING3, IDC_BATNUM3},
    {IDC_POWERSTATUSICON4, IDC_STATUS4, IDC_REMAINING4, IDC_BATNUM4},
    {IDC_POWERSTATUSICON5, IDC_STATUS5, IDC_REMAINING5, IDC_BATNUM5},
    {IDC_POWERSTATUSICON6, IDC_STATUS6, IDC_REMAINING6, IDC_BATNUM6},
    {IDC_POWERSTATUSICON7, IDC_STATUS7, IDC_REMAINING7, IDC_BATNUM7},
    {IDC_POWERSTATUSICON8, IDC_STATUS8, IDC_REMAINING8, IDC_BATNUM8}
};

extern const DWORD g_ContextMenuHelpIDs[];  //Help ID's.

UINT MapBatInfoToIconID(PBATTERY_STATE pbs)
{
    UINT uIconID = IDI_BATDEAD;

    if (!pbs->ulBatNum) {
        if (pbs->ulPowerState & BATTERY_POWER_ON_LINE) {
            return IDI_PLUG;
        }
    }
    else {
        if (pbs->ulTag == BATTERY_TAG_INVALID) {
            return IDI_BATGONE;
        }
    }

    if  (pbs->ulPowerState & BATTERY_CRITICAL) {
        return IDI_BATDEAD;
    }

    if (pbs->ulBatLifePercent > 66) {
        uIconID = IDI_BATFULL;
    }
    else {
        if (pbs->ulBatLifePercent > 33) {
            uIconID = IDI_BATHALF;
        }
        else {
            if (pbs->ulBatLifePercent > 9) {
                uIconID = IDI_BATLOW;
            }
        }
    }

    return uIconID;
}

void SystemPowerStatusToBatteryState(LPSYSTEM_POWER_STATUS lpsps, PBATTERY_STATE pbs)
{
    pbs->ulPowerState = 0;
    if (lpsps->ACLineStatus == AC_LINE_ONLINE) {
        pbs->ulPowerState |= BATTERY_POWER_ON_LINE;
    }
    if (lpsps->BatteryFlag & BATTERY_FLAG_CHARGING) {
        pbs->ulPowerState |= BATTERY_CHARGING;
    }
    if (lpsps->BatteryFlag & BATTERY_FLAG_CRITICAL) {
        pbs->ulPowerState |= BATTERY_CRITICAL;
    }
    pbs->ulBatLifePercent = lpsps->BatteryLifePercent;
    pbs->ulBatLifeTime    = lpsps->BatteryLifeTime;
}

BOOL WalkBatteryState(PBATTERY_STATE pbsStart, WALKENUMPROC pfnWalkEnumProc, HWND hWnd, LPARAM lParam1, LPARAM lParam2)
{
    PBATTERY_STATE pbsTmp;

    while (pbsStart) {
        // Save the next entry in case the current one is deleted.
        pbsTmp = pbsStart->bsNext;
        if (!pfnWalkEnumProc(pbsStart, hWnd, lParam1, lParam2)) {
            return FALSE;
        }
        pbsStart = pbsTmp;
    }

    return TRUE;
}

HICON PASCAL GetBattIcon(HWND hWnd, UINT uIconID, HICON hIconCache, BOOL bWantBolt, UINT uiRes)
{
    static HIMAGELIST hImgLst32, hImgLst16;
    HIMAGELIST hImgLst;

    // Destroy the old cached icon.
    if (hIconCache) {
        DestroyIcon(hIconCache);
    }

    // Don't put the charging bolt over the top of IDI_BATGONE.
    if (uIconID == IDI_BATGONE) {
        bWantBolt = FALSE;
    }

    // Use the transparency color must match that in the bit maps.
    if (!hImgLst32 || !hImgLst16) {
        hImgLst32 = ImageList_LoadImage(g_hInstance,
                                        MAKEINTRESOURCE(IDB_BATTS),
                                        32, 0, RGB(255, 0, 255), IMAGE_BITMAP, 0);
        hImgLst16 = ImageList_LoadImage(g_hInstance,
                                        MAKEINTRESOURCE(IDB_BATTS16),
                                        16, 0, RGB(255, 0, 255), IMAGE_BITMAP, 0);
        ImageList_SetOverlayImage(hImgLst32, IDI_CHARGE-FIRST_ICON_IMAGE, 1);
        ImageList_SetOverlayImage(hImgLst16, IDI_CHARGE-FIRST_ICON_IMAGE, 1);
    }

    if (uiRes == 32) {
        hImgLst = hImgLst32;
    }
    else {
        hImgLst = hImgLst16;
    }

    int ImageIndex = uIconID - FIRST_ICON_IMAGE;

    if (bWantBolt) {
        return ImageList_GetIcon(hImgLst, ImageIndex, INDEXTOOVERLAYMASK(1));
    }
    else {
        return ImageList_GetIcon(hImgLst, ImageIndex, ILD_NORMAL);
    }
}

UINT CheckUpdateBatteryState(PBATTERY_STATE pbs, BOOL bForceUpdate)
{
    UINT uiRetVal = UPDATESTATUS_NOUPDATE;

    // Check to see if anything in the battery status has changed
    // since last time.  If not then we have no work to do!

    if ((bForceUpdate) ||
        !((pbs->ulTag            == pbs->ulLastTag) &&
          (pbs->ulBatLifePercent == pbs->ulLastBatLifePercent) &&
          (pbs->ulBatLifeTime    == pbs->ulLastBatLifeTime) &&
          (pbs->ulPowerState     == pbs->ulLastPowerState))) {

        uiRetVal = UPDATESTATUS_UPDATE;

        //  Check for the special case where the charging state has changed.
        if ((pbs->ulPowerState     & BATTERY_CHARGING) !=
            (pbs->ulLastPowerState & BATTERY_CHARGING)) {
            uiRetVal |= UPDATESTATUS_UPDATE_CHARGE;
            }

        // Copy current battery state to last.
        pbs->ulLastTag            = pbs->ulTag;
        pbs->ulLastBatLifePercent = pbs->ulBatLifePercent;
        pbs->ulLastBatLifeTime    = pbs->ulBatLifeTime;
        pbs->ulLastPowerState     = pbs->ulPowerState;
          }
    return uiRetVal;
}

BOOL ShowHideItem(HWND hWnd, UINT uID, BOOL bShow)
{
    ShowWindow(GetDlgItem(hWnd, uID), (bShow)  ? SW_SHOWNOACTIVATE : SW_HIDE);
    return bShow;
}

void ShowItem(HWND hWnd, UINT uID)
{
    ShowWindow(GetDlgItem(hWnd, uID), SW_SHOWNOACTIVATE);
}

void HideItem(HWND hWnd, UINT uID)
{
    ShowWindow(GetDlgItem(hWnd, uID), SW_HIDE);
}

void DisplayIcon(HWND hWnd, UINT uIconID, PBATTERY_STATE  pbs, ULONG ulUpdateStatus)
{
    UINT    uiMsg;

    // Only redraw the icon if it has changed OR
    // if it has gone from charging to not charging.
    if ((uIconID != pbs->uiIconIDcache) ||
        (ulUpdateStatus != UPDATESTATUS_NOUPDATE)) {

        pbs->uiIconIDcache = uIconID;
        BOOL bBolt = (pbs->ulPowerState & BATTERY_CHARGING);

        pbs->hIconCache   = GetBattIcon(hWnd, uIconID, pbs->hIconCache, bBolt, 32);
        pbs->hIconCache16 = GetBattIcon(hWnd, uIconID, pbs->hIconCache16, bBolt, 16);

        if (pbs->ulBatNum) {
            uiMsg = BM_SETIMAGE;
        }
        else {
            uiMsg = STM_SETIMAGE;
        }
        SendDlgItemMessage(hWnd, g_iMapBatNumToID[pbs->ulBatNum][BAT_ICON],
                           uiMsg, IMAGE_ICON, (LPARAM) pbs->hIconCache);
        ShowItem(hWnd, g_iMapBatNumToID[pbs->ulBatNum][BAT_ICON]);
        }
}

LPTSTR CDECL LoadDynamicString(UINT uiStringID, ... )
{
    va_list Marker;
    TCHAR szBuf[256];
    LPTSTR lpsz;
    int   iLen;

    // va_start is a macro...it breaks when you use it as an assign...on ALPHA.
    va_start(Marker, uiStringID);

    iLen = LoadString(g_hInstance, uiStringID, szBuf, ARRAYSIZE(szBuf));

    if (iLen == 0) {
        wprintf(L"LoadDynamicString: LoadString on: 0x%X failed", uiStringID);
        return NULL;
    }

    FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                  (LPVOID) szBuf, 0, 0, (LPTSTR)&lpsz, 0, &Marker);

    return lpsz;
}

LPTSTR DisplayFreeStr(HWND hWnd, UINT uID, LPTSTR  lpsz, BOOL bFree)
{
    if (lpsz) {
        SetDlgItemText(hWnd, uID, lpsz);
        ShowWindow(GetDlgItem(hWnd, uID), SW_SHOWNOACTIVATE);
        if (bFree) {
            LocalFree(lpsz);
            return NULL;
        }
    }
    else {
        ShowWindow(GetDlgItem(hWnd, uID), SW_HIDE);
    }
    return lpsz;
}

BOOL UpdateBatMeterProc(PBATTERY_STATE pbs, HWND hWnd, LPARAM bShowMulti, LPARAM bForceUpdate)
{
    LPTSTR lpsz, lpszRemaining;

    ULONG ulUpdateStatus = CheckUpdateBatteryState(pbs, (BOOL)bForceUpdate);

    // Make sure there is work to do.
    if (ulUpdateStatus == UPDATESTATUS_NOUPDATE) {
       return TRUE;
    }

    // Determine which icon to display.
    UINT uIconID = MapBatInfoToIconID(pbs);
    DisplayIcon(hWnd, uIconID, pbs, ulUpdateStatus);

    // Are we looking for system power status ?
    if (!pbs->ulBatNum) {

        // Display the Current Power Source text
        lpsz = LoadDynamicString(((pbs->ulPowerState & BATTERY_POWER_ON_LINE) ?
                                   IDS_ACLINEONLINE : IDS_BATTERIES));
        DisplayFreeStr(hWnd, IDC_BATTERYLEVEL, lpsz, FREE_STR);

        if (pbs->ulBatLifePercent <= 100) {
            lpsz = LoadDynamicString(IDS_PERCENTREMAININGFORMAT,
                                        pbs->ulBatLifePercent);
        }
        else {
            lpsz = LoadDynamicString(IDS_UNKNOWN);
        }
        DisplayFreeStr(hWnd, IDC_REMAINING, lpsz, NO_FREE_STR);

        ShowHideItem(hWnd, IDC_CHARGING, pbs->ulPowerState & BATTERY_CHARGING);

        // Show and Update the PowerStatusBar only if in single battery mode and
        // there is al least one battery installed.
        if (!bShowMulti && g_uiBatCount) {
            SendDlgItemMessage(hWnd, IDC_POWERSTATUSBAR, PBM_SETPOS,
                               (WPARAM) pbs->ulBatLifePercent, 0);
            lpsz = DisplayFreeStr(hWnd, IDC_BARPERCENT, lpsz, FREE_STR);
        }

        if (lpsz) {
            LocalFree(lpsz);
        }

        if (pbs->ulBatLifeTime != (UINT) -1) {
            UINT uiHour = pbs->ulBatLifeTime / 3600;
            UINT uiMin = (pbs->ulBatLifeTime % 3600) / 60;
            if (uiHour) {
                lpsz = LoadDynamicString(IDS_TIMEREMFORMATHOUR, uiHour, uiMin);
            }
            else {
                lpsz = LoadDynamicString(IDS_TIMEREMFORMATMIN, uiMin);
            }
            DisplayFreeStr(hWnd, IDC_TIMEREMAINING, lpsz, FREE_STR);
            ShowHideItem(hWnd, IDC_TOTALTIME, TRUE);
        }
        else {
            ShowHideItem(hWnd, IDC_TOTALTIME, FALSE);
            ShowHideItem(hWnd, IDC_TIMEREMAINING, FALSE);
        }
    }
    else {

        // Here when getting the power status of each individual battery
        // when in multi-battery display mode.
        lpsz = LoadDynamicString(IDS_BATNUM, pbs->ulBatNum);
        DisplayFreeStr(hWnd, g_iMapBatNumToID[pbs->ulBatNum][BAT_NUM],
                       lpsz, FREE_STR);

        if (pbs->ulTag != BATTERY_TAG_INVALID) {
            if (pbs->ulPowerState & BATTERY_CHARGING) {
                lpsz = LoadDynamicString(IDS_BATTCHARGING);
            }
            else {
                lpsz = NULL;
            }
            lpszRemaining  = LoadDynamicString(IDS_PERCENTREMAININGFORMAT,
                                               pbs->ulBatLifePercent);
        }
        else {
            lpsz = LoadDynamicString(IDS_NOT_PRESENT);
            lpszRemaining  = NULL;
        }
        DisplayFreeStr(hWnd, g_iMapBatNumToID[pbs->ulBatNum][BAT_STATUS],
                       lpsz, FREE_STR);

        DisplayFreeStr(hWnd, g_iMapBatNumToID[pbs->ulBatNum][BAT_REMAINING],
                       lpszRemaining, FREE_STR);
    }
    return TRUE;
}

BOOL UpdateBatInfoProc(PBATTERY_STATE pbs, HWND hWnd, LPARAM lParam1, LPARAM lParam2)
{
    DWORD                       dwByteCount, dwWait;
    BATTERY_STATUS              bs;
    BATTERY_WAIT_STATUS         bws;
    BATTERY_INFORMATION         bi;
    BATTERY_QUERY_INFORMATION   bqi;

    if (pbs->hDevice == INVALID_HANDLE_VALUE) {
        wprintf(L"UpdateBatInfoProc, Bad battery driver handle, LastError: 0x%X", GetLastError());
        return FALSE;
    }

    // If no tag, then don't update the battery info.
    DWORD dwIOCTL = IOCTL_BATTERY_QUERY_TAG;
    dwWait = 0;
    if (DeviceIoControl(pbs->hDevice, dwIOCTL,
                        &dwWait, sizeof(dwWait),
                        &(pbs->ulTag), sizeof(ULONG),
                        &dwByteCount, NULL)) {

        bqi.BatteryTag = pbs->ulTag;
        bqi.InformationLevel = BatteryInformation;
        bqi.AtRate = 0;
        
        dwIOCTL = IOCTL_BATTERY_QUERY_INFORMATION;
        if (DeviceIoControl(pbs->hDevice, dwIOCTL,
                            &bqi, sizeof(bqi),
                            &bi,  sizeof(bi),
                            &dwByteCount, NULL)) {

            if (bi.FullChargedCapacity != UNKNOWN_CAPACITY) {
                pbs->ulFullChargedCapacity = bi.FullChargedCapacity;
            }
            else {
                pbs->ulFullChargedCapacity = bi.DesignedCapacity;
            }

            memset(&bws, 0, sizeof(BATTERY_WAIT_STATUS));
            bws.BatteryTag = pbs->ulTag;
            dwIOCTL = IOCTL_BATTERY_QUERY_STATUS;
            if (DeviceIoControl(pbs->hDevice, dwIOCTL,
                                &bws, sizeof(BATTERY_WAIT_STATUS),
                                &bs,  sizeof(BATTERY_STATUS),
                                &dwByteCount, NULL)) {

                pbs->ulPowerState = bs.PowerState;
                if (pbs->ulFullChargedCapacity < bs.Capacity) {
                    pbs->ulFullChargedCapacity = bs.Capacity;
                    wprintf(L"UpdateBatInfoProc, unable to calculate ulFullChargedCapacity");
                }
                if (pbs->ulFullChargedCapacity == 0) {
                    pbs->ulBatLifePercent = 0;
                }
                else {
                    pbs->ulBatLifePercent =
                        (100 * bs.Capacity) / pbs->ulFullChargedCapacity;
                }
                return TRUE;
            }
        }
    }
    else {
        pbs->ulTag = BATTERY_TAG_INVALID;

        // No battery tag, that's ok, the user may have removed the battery.
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            return TRUE;
        }
    }
    wprintf(L"UpdateBatInfoProc, IOCTL: %X Failure, BatNum: %d, LastError: %d\n", dwIOCTL, pbs->ulBatNum, GetLastError());
    return FALSE;
}

BOOL SwitchDisplayMode(HWND hWnd, BOOL bShowMulti)
{
    ULONG i, j;

    // Override request if multi-battery display is not possible.
    if ((bShowMulti) && (!g_uiBatCount)) {
        bShowMulti = FALSE;
    }

    if (!g_uiBatCount) {

        //
        // Hide all info if no batteries are installed
        //
        HideItem(hWnd, IDC_POWERSTATUSBAR);
        HideItem(hWnd, IDC_BARPERCENT);
        HideItem(hWnd, IDC_MOREINFO);

    } else if (bShowMulti) {
        HideItem(hWnd, IDC_POWERSTATUSBAR);
        HideItem(hWnd, IDC_BARPERCENT);
        ShowItem(hWnd, IDC_MOREINFO);

        for (i = 1; i <= g_uiBatCount; i++) {
            for (j = 0; j < BAT_LAST; j++) {
                ShowItem(hWnd, g_iMapBatNumToID[i][0]);
            }
        }
    }
    else {
        for (i = 1; i <= g_uiBatCount; i++) {
            for (j = 0; j < BAT_LAST; j++) {
                HideItem(hWnd, g_iMapBatNumToID[i][j]);
            }
        }

        ShowItem(hWnd, IDC_POWERSTATUSBAR);
        ShowItem(hWnd, IDC_BARPERCENT);
        HideItem(hWnd, IDC_MOREINFO);
    }
    return bShowMulti;
}

BOOL UpdateBatMeter(HWND hWnd, BOOL bShowMulti, BOOL bForceUpdate, PBATTERY_STATE pbsComposite)
{
    BOOL bRet = FALSE;
    SYSTEM_POWER_STATUS sps;
    UINT uIconID;

    // Update the composite battery state.
    if (GetSystemPowerStatus(&sps) && hWnd) {
        if (sps.BatteryLifePercent > 100) {
            wprintf(L"GetSystemPowerStatus, set BatteryLifePercent: %d", sps.BatteryLifePercent);
        }

        // Fill in the composite battery state.
        SystemPowerStatusToBatteryState(&sps, &g_bs);

        // Update the information in the battery state list if we have a battery.
        if (g_hwndBatMeter) {

#ifndef SIM_BATTERY
           WalkBatteryState(DEVICES,
                            (WALKENUMPROC)UpdateBatInfoProc,
                            NULL,
                            (LPARAM)NULL,
                            (LPARAM)NULL);
#else
           WalkBatteryState(DEVICES,
                            (WALKENUMPROC)SimUpdateBatInfoProc,
                            NULL,
                            (LPARAM)NULL,
                            (LPARAM)NULL);
#endif

           // See if the current display mode matches the requested mode.
           if ((g_bShowingMulti != bShowMulti) || (bForceUpdate)) {
               g_bShowingMulti = SwitchDisplayMode(hWnd, bShowMulti);
               bForceUpdate  = TRUE;
           }

           if (g_bShowingMulti) {
               // Walk the bs list, and update all battery displays.
               WalkBatteryState(ALL,
                                (WALKENUMPROC)UpdateBatMeterProc,
                                hWnd,
                                (LPARAM)g_bShowingMulti,
                                (LPARAM)bForceUpdate);
           }
           else {
               // Display only the composite battery information.
               UpdateBatMeterProc(&g_bs,
                                  hWnd,
                                  (LPARAM)g_bShowingMulti,
                                  (LPARAM)bForceUpdate);
           }
           bRet = TRUE;
        }
    }
    else {
        // Fill in default composite info.
        g_bs.ulPowerState     = BATTERY_POWER_ON_LINE;
        g_bs.ulBatLifePercent = (UINT) -1;
        g_bs.ulBatLifeTime    = (UINT) -1;

        uIconID = MapBatInfoToIconID(&g_bs);
        g_bs.hIconCache = GetBattIcon(hWnd, uIconID, g_bs.hIconCache, FALSE, 32);
        g_bs.hIconCache16 = GetBattIcon(hWnd, uIconID, g_bs.hIconCache16, FALSE, 16);
    }

    // If a pointer is provided, copy the composite battery state data.
    if (pbsComposite) {
        if (pbsComposite->ulSize == sizeof(BATTERY_STATE)) {
            memcpy(pbsComposite, &g_bs, sizeof(BATTERY_STATE));
        }
        else {
           wprintf(L"UpdateBatMeter, passed BATTERY_STATE size is invalid");
        }
    }
    return bRet;
}

BOOL RemoveBatteryStateDevice(PBATTERY_STATE pbs)
{
    // Unlink
    if (pbs->bsNext) {
        pbs->bsNext->bsPrev = pbs->bsPrev;
    }
    if (pbs->bsPrev) {
        pbs->bsPrev->bsNext = pbs->bsNext;
    }

#ifdef winnt
    UnregisterForDeviceNotification(pbs);
#endif
    
    // Free the battery driver handle if one was opened.
    if (pbs->hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(pbs->hDevice);
    }

    // Free the device name.
    LocalFree(pbs->lpszDeviceName);

    // Destroy any icons.
    if (pbs->hIconCache) {
        DestroyIcon(pbs->hIconCache);
    }
    if (pbs->hIconCache16) {
        DestroyIcon(pbs->hIconCache16);
    }

    // Free the associated storage.
    LocalFree(pbs);

    return TRUE;
}

BOOL RemoveMissingProc(PBATTERY_STATE pbs, HWND hWnd, LPARAM lParam1, LPARAM lParam2)
{
    LPTSTR  *pszDeviceNames;

    if (lParam2 == REMOVE_MISSING) {
        if ((pszDeviceNames = (LPTSTR *)lParam1) != NULL) {
            for (UINT i = 0; i < NUM_BAT; i++) {
                if (pszDeviceNames[i]) {
                    if (!lstrcmp(pbs->lpszDeviceName, pszDeviceNames[i])) {
                        // Device found in device list, leave it alone.
                        return TRUE;
                    }
                }
                else {
                    continue;
                }
            }
        }
    }

    // Device not in the device names list, remove it.
    RemoveBatteryStateDevice(pbs);
    return TRUE;
}

BOOL FindNameProc(PBATTERY_STATE pbs, HWND hWnd, LPARAM lParam1, LPARAM lParam2)
{
    if (lParam1) {
        if (!lstrcmp(pbs->lpszDeviceName, (LPTSTR)lParam1)) {
            // Device found in device list.
            return FALSE;
        }
    }
    return TRUE;
}

PBATTERY_STATE AddBatteryStateDevice(LPTSTR lpszName, ULONG ulBatNum)
{
    PBATTERY_STATE  pbs, pbsTemp = &g_bs;
    LPTSTR          lpsz = NULL;

    if (!lpszName) {
        return NULL;
    }

    // Append to end of list
    while (pbsTemp->bsNext) {
        pbsTemp = pbsTemp->bsNext;
    }

    // Allocate storage for new battery device state.
    if (pbs = LocalAlloc(LPTR, sizeof(BATTERY_STATE))) {
        if (lpsz = LocalAlloc(0, STRSIZE(lpszName))) {
            lstrcpy(lpsz, lpszName);
            pbs->lpszDeviceName = lpsz;
            pbs->ulSize = sizeof(BATTERY_STATE);
            pbs->ulBatNum = ulBatNum;

            // Open a handle to the battery driver.
            pbs->hDevice = CreateFile(lpszName,
                                      GENERIC_READ | GENERIC_WRITE,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                      NULL, OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL, NULL);
#ifdef WINNT
            // Setup for notification by PNP when battery goes away. 
            RegisterForDeviceNotification(pbs);
#endif
            // Get the current battery info from the battery driver.
            if (UpdateBatInfoProc(pbs, NULL, 0, 0)) {

                // Link the new battery device state into the list.
                pbsTemp->bsNext = pbs;
                pbs->bsPrev = pbsTemp;
                return pbs;
            }
            LocalFree(lpsz);
        }
        LocalFree(pbs);
    }
    return NULL;
}

BOOL UpdateDriverList(LPTSTR *lpszDriverNames, UINT uiDriverCount)
{
    UINT            i;
    PBATTERY_STATE  pbs;

    // Walk the bs list, and remove any devices which aren't in pszDeviceNames.
    WalkBatteryState(DEVICES,
                     (WALKENUMPROC)RemoveMissingProc,
                     NULL,
                     (LPARAM)g_lpszDriverNames,
                     (LPARAM)REMOVE_MISSING);

    // Scan the pszDeviceNames list and add any devices which aren't in bs.
    for (i = 0; i < uiDriverCount; i++) {

        if (WalkBatteryState(DEVICES,
                             (WALKENUMPROC)FindNameProc,
                             NULL,
                             (LPARAM)g_lpszDriverNames[i],
                             (LPARAM)NULL)) {

#ifndef SIM_BATTERY
            if (!AddBatteryStateDevice(g_lpszDriverNames[i], i + 1)) {
                // We weren't able get minimal info from driver, dec the
                // battery counts. g_uiBatCount should always be > 0.
                if (--g_uiDriverCount) {;
                    g_uiBatCount--;
                }
            }
#else
            SimAddBatteryStateDevice(g_lpszDriverNames[i], i + 1);
#endif
                             }
    }

    // Clear and rebuild g_pbs, the handy batttery number to pbs array.
    memset(&g_pbs, 0, sizeof(g_pbs));
    pbs = &g_bs;
    for (i = 0; i <= g_uiBatCount; i++) {
        if (pbs) {
            g_pbs[i] = pbs;
            pbs = pbs->bsNext;
        }
    }
    return TRUE;
}


BOOL GetBatQueryInfo(PBATTERY_STATE pbs, PBATTERY_QUERY_INFORMATION pbqi, PULONG pulData, ULONG ulSize)
{
    DWORD dwByteCount;

    if (DeviceIoControl(pbs->hDevice, IOCTL_BATTERY_QUERY_INFORMATION,
                        pbqi, sizeof(BATTERY_QUERY_INFORMATION),
                        pulData,  ulSize,
                        &dwByteCount, NULL)) {
        return TRUE;
                        }
    return FALSE;
}

BOOL GetAndSetBatQueryInfoText(
    HWND                        hWnd,
    PBATTERY_STATE              pbs,
    PBATTERY_QUERY_INFORMATION  pbqi,
    UINT                        uiIDS,
    UINT                        uiLabelID
)
{
    WCHAR szBatStr[MAX_BATTERY_STRING_SIZE];

    memset(szBatStr, 0, sizeof(szBatStr));
    if (GetBatQueryInfo(pbs, pbqi, (PULONG)szBatStr, sizeof(szBatStr))) {
#ifdef UNICODE
        if (lstrcmp(szBatStr, TEXT(""))) {
            SetDlgItemText(hWnd, uiIDS, szBatStr);
            return TRUE;
        }
#else
        CHAR szaBatStr[MAX_BATTERY_STRING_SIZE];

        szaBatStr[0] = '\0';
        WideCharToMultiByte(CP_ACP, 0, szBatStr, -1,
                            szaBatStr, MAX_BATTERY_STRING_SIZE, NULL, NULL);
        if (szaBatStr[0]) {
            SetDlgItemText(hWnd, uiIDS, szaBatStr);
            return TRUE;
        }
#endif
    }
    ShowWindow(GetDlgItem(hWnd, uiIDS), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, uiLabelID), SW_HIDE);
    return FALSE;
}

BOOL GetBatOptionalDetails(HWND hWnd, PBATTERY_STATE pbs)
{
    BATTERY_QUERY_INFORMATION   bqi;
    ULONG                       ulData;
    LPTSTR                      lpsz = NULL;
    BATTERY_MANUFACTURE_DATE    bmd;
    TCHAR                       szDateBuf[128];
    SYSTEMTIME                  stDate;

    bqi.BatteryTag = pbs->ulTag;
    bqi.InformationLevel = BatteryManufactureDate;
    bqi.AtRate = 0;
    
    if (GetBatQueryInfo(pbs, &bqi, (PULONG)&bmd,
                        sizeof(BATTERY_MANUFACTURE_DATE))) {

        memset(&stDate, 0, sizeof(SYSTEMTIME));
        stDate.wYear  = (WORD) bmd.Year;
        stDate.wMonth = (WORD) bmd.Month;
        stDate.wDay   = (WORD) bmd.Day;

        GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE,
                      &stDate, NULL, szDateBuf, 128);
        SetDlgItemText(hWnd, IDC_BATMANDATE, szDateBuf);
                        }
    else {
        ShowWindow(GetDlgItem(hWnd, IDC_BATMANDATE), SW_HIDE);
        ShowWindow(GetDlgItem(hWnd, IDC_DATEMANUFACTURED), SW_HIDE);
    }
    bqi.InformationLevel = BatteryDeviceName;
    GetAndSetBatQueryInfoText(hWnd, pbs, &bqi, IDC_DEVNAME, IDC_BATTERYNAME);

    bqi.InformationLevel = BatteryManufactureName;
    GetAndSetBatQueryInfoText(hWnd, pbs, &bqi, IDC_BATMANNAME, IDC_MANUFACTURE);

    bqi.InformationLevel = BatteryUniqueID;
    GetAndSetBatQueryInfoText(hWnd, pbs, &bqi, IDC_BATID, IDC_UNIQUEID);

    return TRUE;
}

BOOL AppendStrID(LPTSTR lpszDest, UINT uiID, BOOLEAN bUseComma)
{
    if (lpszDest) {
        LPTSTR lpsz = LoadDynamicString(uiID);
        if (lpsz) {
            if (bUseComma) {
                lstrcat(lpszDest, TEXT(", "));
            }
            lstrcat(lpszDest, lpsz);
            LocalFree(lpsz);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL GetBatStatusDetails(HWND hWnd, PBATTERY_STATE pbs)
{
    BATTERY_STATUS              bs;
    BATTERY_WAIT_STATUS         bws;
    DWORD                       dwByteCount;
    BATTERY_INFORMATION         bi;
    BATTERY_QUERY_INFORMATION   bqi;
    TCHAR                       szChem[5], szStatus[128];
    CHAR                        szaChem[5];
    LPTSTR                      lpsz;
    UINT                        uiIDS;

    bqi.BatteryTag = pbs->ulTag;
    bqi.InformationLevel = BatteryInformation;
    bqi.AtRate = 0;

    if (DeviceIoControl(pbs->hDevice, IOCTL_BATTERY_QUERY_INFORMATION,
                        &bqi, sizeof(bqi),
                        &bi,  sizeof(bi),
                        &dwByteCount, NULL)) {

        // Set chemistry.
        memcpy(szaChem, bi.Chemistry, 4);
        szaChem[4] = 0;

        if (szaChem[0]) {
#ifdef UNICODE
            MultiByteToWideChar(CP_ACP, 0, szaChem, -1, szChem, 5);
            SetDlgItemText(hWnd, IDC_CHEM, szChem);
#else
            SetDlgItemText(hWnd, IDC_CHEM, szaChem);
#endif
        }
        else {
            ShowWindow(GetDlgItem(hWnd, IDC_CHEM), SW_HIDE);
            ShowWindow(GetDlgItem(hWnd, IDC_CHEMISTRY), SW_HIDE);
        }

        // Set up BATTERY_WAIT_STATUS for immediate return.
        memset(&bws, 0, sizeof(BATTERY_WAIT_STATUS));
        bws.BatteryTag = pbs->ulTag;

        if (DeviceIoControl(pbs->hDevice, IOCTL_BATTERY_QUERY_STATUS,
                            &bws, sizeof(BATTERY_WAIT_STATUS),
                            &bs,  sizeof(BATTERY_STATUS),
                            &dwByteCount, NULL)) {

            szStatus[0] = '\0';
            BOOLEAN bUseComma = FALSE;
            if (bs.PowerState & BATTERY_POWER_ON_LINE) {
                AppendStrID(szStatus, IDS_BATTERY_POWER_ON_LINE, bUseComma);
                bUseComma = TRUE;
            }
            if (bs.PowerState & BATTERY_DISCHARGING) {
                AppendStrID(szStatus, IDS_BATTERY_DISCHARGING, bUseComma);
                bUseComma = TRUE;
            }
            if (bs.PowerState & BATTERY_CHARGING) {
                AppendStrID(szStatus, IDS_BATTERY_CHARGING, bUseComma);
                bUseComma = TRUE;
            }
            if (bs.PowerState & BATTERY_CRITICAL) {
                AppendStrID(szStatus, IDS_BATTERY_CRITICAL, bUseComma);
                bUseComma = TRUE;
            }
            SetDlgItemText(hWnd, IDC_STATE, szStatus);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL InitBatDetailDialogs(HWND hWnd, PBATTERY_STATE pbs)
{
    LPTSTR                      lpsz;
    DWORD                       dwByteCount;

    lpsz = LoadDynamicString(IDS_BATTERYNUMDETAILS, pbs->ulBatNum);
    if (lpsz) {
        SetWindowText(hWnd, lpsz);
        LocalFree(lpsz);
    }

    if (GetBatOptionalDetails(hWnd, pbs)) {
        return GetBatStatusDetails(hWnd, pbs);
    }
    return FALSE;
}


LRESULT CALLBACK BatDetailDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UINT uiBatNum;
    static PBATTERY_STATE pbs;

    switch (uMsg) {
        case WM_INITDIALOG:
            pbs = (PBATTERY_STATE) lParam;
            return InitBatDetailDialogs(hWnd, pbs);

        case WM_COMMAND:
            switch (wParam) {
            case IDC_REFRESH:
                    GetBatStatusDetails(hWnd, pbs);
                    break;

            case IDCANCEL:
            case IDOK:
                    EndDialog(hWnd, wParam);
                    break;
            }
            break;

        case WM_HELP:             // F1
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, PWRMANHLP, HELP_WM_HELP, (ULONG_PTR)(LPTSTR)g_ContextMenuHelpIDs);
            break;

        case WM_CONTEXTMENU:      // right mouse click
            WinHelp((HWND)wParam, PWRMANHLP, HELP_CONTEXTMENU, (ULONG_PTR)(LPTSTR)g_ContextMenuHelpIDs);
            break;
    }

    return FALSE;
}

VOID FreeBatteryDriverNames(LPTSTR *lpszDriverNames)
{
    // Free any old driver names.
    for (UINT i = 0; i < NUM_BAT; i++) {
        if (lpszDriverNames[i]) {
            LocalFree(lpszDriverNames[i]);
            lpszDriverNames[i] = NULL;
        }
    }
}

void CleanupBatteryData(void)
{
    g_hwndBatMeter = NULL;

    // Mark all batteries as missing.
    memset(&g_pbs, 0, sizeof(g_pbs));

    // Walk the bs list, remove all devices and cleanup.
    WalkBatteryState(DEVICES,
                     (WALKENUMPROC)RemoveMissingProc,
                     NULL,
                     (LPARAM)NULL,
                     (LPARAM)REMOVE_ALL);

    // Free any old driver names.
    FreeBatteryDriverNames(g_lpszDriverNames);
    g_uiBatCount = 0;
}

LRESULT CALLBACK BatMeterDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef WINNT
    UINT i, j;
    PBATTERY_STATE pbsTemp;
#endif // WINNT

    UINT uiBatNum;

   switch (uMsg) {
      case WM_COMMAND:
         if ((HIWORD(wParam) == STN_CLICKED) ||
             (HIWORD(wParam) == BN_CLICKED)) {
            switch (LOWORD(wParam)) {
               case IDC_POWERSTATUSICON1:
               case IDC_POWERSTATUSICON2:
               case IDC_POWERSTATUSICON3:
               case IDC_POWERSTATUSICON4:
               case IDC_POWERSTATUSICON5:
               case IDC_POWERSTATUSICON6:
               case IDC_POWERSTATUSICON7:
               case IDC_POWERSTATUSICON8:
                  uiBatNum = LOWORD(wParam) - IDC_POWERSTATUSICON1 + 1;
                  // Allow battery details only for present batteries.
                  if ((g_pbs[uiBatNum]) &&
                      (g_pbs[uiBatNum]->ulTag != BATTERY_TAG_INVALID)) {
                     DialogBoxParam(g_hInstance,
                                    MAKEINTRESOURCE(IDD_BATDETAIL),
                                    hWnd,
                                    BatDetailDlgProc,
                                    (LPARAM)g_pbs[uiBatNum]);
                  }
                  break;
            }
         }
         break;

      case WM_DESTROYBATMETER:
         CleanupBatteryData();
         EndDialog(hWnd, wParam);
         break;

      case WM_DESTROY:
         CleanupBatteryData();
         break;

      case WM_DEVICECHANGE:
#ifdef WINNT
         if ((wParam == DBT_DEVICEQUERYREMOVE) || (wParam == DBT_DEVICEREMOVECOMPLETE)) {
            if ( ((PDEV_BROADCAST_HANDLE)lParam)->dbch_devicetype == DBT_DEVTYP_HANDLE) {

               //
               // Find Device that got removed
               //
               pbsTemp = DEVICES;
               while (pbsTemp) {
                  if (pbsTemp->hDevNotify == ((PDEV_BROADCAST_HANDLE)lParam)->dbch_hdevnotify) {
                     break;
                  }
                  pbsTemp = pbsTemp->bsNext;
               }
               if (!pbsTemp) {
                  break;
               }

               //
               // Close the handle to this device and release cached data.
               //
               RemoveBatteryStateDevice (pbsTemp);
               g_uiDriverCount--;
               g_uiBatCount = g_uiDriverCount;

               // Clear and rebuild g_pbs, the handy batttery number to pbs array.
               memset(&g_pbs, 0, sizeof(g_pbs));
               pbsTemp = &g_bs;
               for (i = 0; i <= g_uiBatCount; i++) {
                  if (pbsTemp) {
                     g_pbs[i] = pbsTemp;
                     pbsTemp->ulBatNum = i;
                     pbsTemp = pbsTemp->bsNext;
                  }
               }

               // Refresh display
               for (i = 1; i <= NUM_BAT; i++) {
                  for (j = 0; j < BAT_LAST; j++) {
                     HideItem(g_hwndBatMeter, g_iMapBatNumToID[i][j]);
                  }
               }

               g_bShowingMulti = SwitchDisplayMode (g_hwndBatMeter, g_bShowingMulti);
               if (g_bShowingMulti) {
                  // Walk the bs list, and update all battery displays.
                  WalkBatteryState(DEVICES,
                                   (WALKENUMPROC)UpdateBatMeterProc,
                                   g_hwndBatMeter,
                                   (LPARAM)g_bShowingMulti,
                                   (LPARAM)TRUE);
               }
            }
         }
#else
         if (wParam == DBT_DEVICEQUERYREMOVE) {
            if (g_hwndBatMeter) {
               // Close all of the batteries.
               CleanupBatteryData();
            }
         }
#endif
         return TRUE;

      case WM_HELP:             // F1
         WinHelp(((LPHELPINFO)lParam)->hItemHandle, PWRMANHLP, HELP_WM_HELP, (ULONG_PTR)(LPTSTR)g_ContextMenuHelpIDs);
         return TRUE;

      case WM_CONTEXTMENU:      // right mouse click
         WinHelp((HWND)wParam, PWRMANHLP, HELP_CONTEXTMENU, (ULONG_PTR)(LPTSTR)g_ContextMenuHelpIDs);
         return TRUE;
   }
   return FALSE;
}

HWND CreateBatMeter(HWND hwndParent, HWND hwndFrame, BOOL bShowMulti, PBATTERY_STATE pbsComposite)
{
    RECT rFrame = {0};

    // Build the battery devices name list if hasn't already been built.
    if (!g_uiBatCount)
    {
        BatMeterCapabilities(NULL);
    }

    // Remember if we are showing details for each battery
    g_bShowingMulti = bShowMulti;

    // Make sure we have at least one battery.
    if (g_uiBatCount)
    {
        // Create the battery meter control.
        g_hwndParent = hwndParent;
        g_hwndBatMeter = CreateDialog(g_hInstance,
                                MAKEINTRESOURCE(IDD_BATMETER),
                                hwndParent,
                                BatMeterDlgProc);

        // Place the battery meter in the passed frame window.
        if ((g_hwndBatMeter) && (hwndFrame))
        {
            // Position the BatMeter dialog in the frame.
            if (!GetWindowRect(hwndFrame, &rFrame))
            {
                wprintf(L"CreateBatMeter, GetWindowRect failed, hwndFrame: %08X", hwndFrame);
            }

            INT iWidth = rFrame.right - rFrame.left;
            INT iHeight = rFrame.bottom - rFrame.top;

            // @MOD - Skipped RTL
            /*
            if (IsBiDiLocalizedSystemEx(NULL))
            {
                // Whistler #209400: On BIDI systems, ScreenToClient() wants the right
                // coord in the left location because everything is flipped.
                rFrame.left = rFrame.right;
            }
            */

            if (!ScreenToClient(hwndParent, (LPPOINT)&rFrame))
            {
                wprintf(L"CreateBatMeter, ScreenToClient failed");
            }

            if (!MoveWindow(g_hwndBatMeter,
                         rFrame.left,
                         rFrame.top,
                         iWidth,
                         iHeight,
                         FALSE))
            {
                wprintf(L"CreateBatMeter, MoveWindow failed, %d, %d", rFrame.left, rFrame.top);
            }

            // Build the battery driver data list.
            if (!UpdateDriverList(g_lpszDriverNames, g_uiDriverCount))
            {
                return DestroyBatMeter(g_hwndBatMeter);
            }

            // Do the first update.
            UpdateBatMeter(g_hwndBatMeter, bShowMulti, TRUE, pbsComposite);
            ShowWindow(g_hwndBatMeter, SW_SHOWNOACTIVATE);
        }
    }

   return g_hwndBatMeter;
}

HWND DestroyBatMeter(HWND hWnd)
{
    SendMessage(hWnd, WM_DESTROYBATMETER, 0, 0);
    g_hwndBatMeter = NULL;
    return g_hwndBatMeter;
}

BOOL BatMeterCapabilities(PUINT* ppuiBatCount)
{
#ifndef SIM_BATTERY
    SYSTEM_POWER_CAPABILITIES   spc;
#endif // SIM_BATTERY

    if (ppuiBatCount) {
        *ppuiBatCount = &g_uiBatCount;
    }
    g_uiBatCount = 0;

#ifndef SIM_BATTERY
    // Make sure we have batteries to query.
    if (GetPwrCapabilities(&spc)) {
        if (spc.SystemBatteriesPresent) {
            // @MOD - Come back to this
            g_uiDriverCount = 0;//GetBatteryDriverNames(g_lpszDriverNames);
            if (g_uiDriverCount != 0) {
                g_uiBatCount = g_uiDriverCount;

                return TRUE;
            }
            else {
                wprintf(L"BatMeterCapabilities, no battery drivers found.");
            }
        }
    }
    return FALSE;

#else // SIM_BATTERY
    g_uiBatCount = g_uiDriverCount = GetBatteryDriverNames(g_lpszDriverNames);
    return UpdateDriverList(g_lpszDriverNames, g_uiDriverCount);
#endif // SIM_BATTERY
}

//
//  Load module stuff
//

BOOL(WINAPI* PowerCapabilities)();

//
// Function loader
//
#define MODULE_VARNAME(NAME) hMod_ ## NAME

#define LOAD_MODULE(NAME)                                        \
HMODULE MODULE_VARNAME(NAME) = LoadLibraryW(L#NAME ".dll");      \
if (!MODULE_VARNAME(NAME))                                       \
return TRUE;

#define LOAD_FUNCTION(MODULE, FUNCTION)                                      \
*(FARPROC *)&FUNCTION = GetProcAddress(MODULE_VARNAME(MODULE), #FUNCTION);   \
if (!FUNCTION)                                                               \
return FALSE;

#define LOAD_ORDINAL(MODULE, FUNCNAME, ORDINAL)                                   \
*(FARPROC *)&FUNCNAME = GetProcAddress(MODULE_VARNAME(MODULE), (LPCSTR)ORDINAL);  \
if (!FUNCNAME)                                                                    \
return TRUE;


BOOL SHUndocInit(void)
{
    LOAD_MODULE(batmeter);
    LOAD_FUNCTION(batmeter, PowerCapabilities);
    
    return TRUE;
}