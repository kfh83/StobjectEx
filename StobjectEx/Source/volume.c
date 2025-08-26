/*******************************************************************************
*
*  (C) COPYRIGHT MICROSOFT CORP., 1993-1994
*
*  TITLE:       VOLUME.C
*
*  VERSION:     1.0
*
*  AUTHOR:      RAL
*
*  DATE:        11/01/94
*
********************************************************************************
*
*  CHANGE LOG:
*
*  DATE        REV DESCRIPTION
*  ----------- --- -------------------------------------------------------------
*  Nov. 11, 94 RAL Original
*  Oct. 24, 95 Shawnb UNICODE enabled
*
*******************************************************************************/
#include "stdafx.h"
#include "systray.h"
#include "audiocontroller.h"

#define VOLUMEMENU_PROPERTIES               100
#define VOLUMEMENU_SNDVOL                   101

extern HINSTANCE g_hInstance;

static BOOL    g_bVolumeEnabled = FALSE;
static BOOL    g_bVolumeIconShown = FALSE;
static HICON   g_hVolumeIcon = NULL;
static HICON   g_hMuteIcon = NULL;
static HMENU   g_hVolumeMenu = NULL;

static AudioController g_pAudioController = { 0 };

void Volume_UpdateStatus(HWND hWnd, BOOL bShowIcon, BOOL bKillSndVol32);
void Volume_VolumeControl();
void Volume_ControlPanel(HWND hwnd);
void Volume_UpdateIcon(HWND hwnd, DWORD message);
BOOL FileExists (LPCTSTR pszFileName);
BOOL FindSystemFile (LPCTSTR pszFileName, LPTSTR pszFullPath, UINT cchSize);
void Volume_WakeUpOrClose(BOOL fClose);

HMENU Volume_CreateMenu()
{
        HMENU  hmenu;
        LPTSTR lpszMenu1;
        LPTSTR lpszMenu2;

        lpszMenu1 = LoadDynamicString(IDS_VOLUMEMENU1);
        if (!lpszMenu1)
                return NULL;

        lpszMenu2 = LoadDynamicString(IDS_VOLUMEMENU2);
        if (!lpszMenu2)
        {
                DeleteDynamicString(lpszMenu1);
                return NULL;
        }

        hmenu = CreatePopupMenu();
        if (!hmenu)
        {
                DeleteDynamicString(lpszMenu1);
                DeleteDynamicString(lpszMenu2);
                return NULL;
        }

        AppendMenu(hmenu,MF_STRING,VOLUMEMENU_SNDVOL,lpszMenu2);
        AppendMenu(hmenu,MF_STRING,VOLUMEMENU_PROPERTIES,lpszMenu1);

        SetMenuDefaultItem(hmenu,VOLUMEMENU_SNDVOL,FALSE);

        DeleteDynamicString(lpszMenu1);
        DeleteDynamicString(lpszMenu2);

        return hmenu;
}

BOOL Volume_Init(HWND hWnd, PBOOL *bShowIcon)
{
    const TCHAR szVolApp[] = TEXT("SNDVOL32.EXE");

    // We still depend on sndvol32
    if (!FindSystemFile(szVolApp, NULL, 0))
    {
        EnableService(STSERVICE_VOLUME, FALSE);
        return FALSE;
    }

    // We release in case the system tray gets updated
    AudioController_Dispose(&g_pAudioController);

    HRESULT hr = AudioController_Initialize(&g_pAudioController, hWnd);
    if (FAILED(hr))
    {
        if (HRESULT_FROM_WIN32(ERROR_NOT_FOUND) != hr)
        {
            EnableService(STSERVICE_VOLUME, FALSE);
            return FALSE;
        }
        // Even if a device is not found, we initialize and hide the icon
        *bShowIcon = FALSE;
    }

    return TRUE;
}

//
//  Called at init time and whenever services are enabled/disabled.
//  Returns false if mixer services are not active.
//
BOOL Volume_CheckEnable(HWND hWnd, BOOL bSvcEnabled)
{
        BOOL bShowIcon = TRUE;
        BOOL bEnable = bSvcEnabled && Volume_Init(hWnd, &bShowIcon);

        if (bEnable != g_bVolumeEnabled) {
                //
                // state change
                //
                g_bVolumeEnabled = bEnable;
                Volume_UpdateStatus(hWnd, bShowIcon, TRUE);
        }
        return(bEnable);
}

void Volume_UpdateStatus(HWND hWnd, BOOL bShowIcon, BOOL bKillSndVol32)
{
    // Don't show icon if not enabled
    if (!g_bVolumeEnabled)
        bShowIcon = FALSE;

        if (bShowIcon != g_bVolumeIconShown) {
                g_bVolumeIconShown = bShowIcon;
                if (bShowIcon) {
                        g_hVolumeIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_VOLUME),
                                                IMAGE_ICON, 16, 16, 0);
                        g_hMuteIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MUTE),
                                                IMAGE_ICON, 16, 16, 0);
                        Volume_UpdateIcon(hWnd, NIM_ADD);
                } else {
                        SysTray_NotifyIcon(hWnd, STWM_NOTIFYVOLUME, NIM_DELETE, NULL, NULL);
                        if (g_hVolumeIcon) {
                                DestroyIcon(g_hVolumeIcon);
                                g_hVolumeIcon = NULL;
                        }
                        if (g_hMuteIcon) {
                                DestroyIcon(g_hMuteIcon);
                                g_hMuteIcon = NULL;
                        }

                        //
                        // SNDVOL32 may have a TRAYMASTER window open,
                        // sitting on a timer before it closes (so multiple
                        // l-clicks on the tray icon can bring up the app
                        // quickly after the first hit).  Close that app
                        // if it's around.
                        //
                        if (bKillSndVol32)
                        {
                                Volume_WakeUpOrClose (TRUE);
                        }
                }
    }
}

void Volume_UpdateIcon(
    HWND hWnd,
    DWORD message)
{
    BOOL        fMute;
    LPTSTR      lpsz;
    HICON       hVol;

    fMute   = AudioController_GetMute(&g_pAudioController);
    hVol    = fMute?g_hMuteIcon:g_hVolumeIcon;
    lpsz    = LoadDynamicString(fMute?IDS_MUTED:IDS_VOLUME);
    SysTray_NotifyIcon(hWnd, STWM_NOTIFYVOLUME, message, hVol, lpsz);
    DeleteDynamicString(lpsz);
}


void Volume_AudioChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    Volume_UpdateIcon(hWnd, NIM_MODIFY);
}

// Maybe not needed
void Volume_HandlePowerBroadcast(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
        case PBT_APMQUERYSUSPENDFAILED:
        case PBT_APMRESUMESUSPEND:
        {
            Volume_DeviceChange(hWnd, wParam, lParam);
        }
        break;
    }
}

void Volume_DeviceChange(HWND hWnd,WPARAM wParam,LPARAM lParam)
{
    AudioController_Reattach(&g_pAudioController);
    if (!AudioController_AttachStatus(&g_pAudioController))
    {
        Volume_UpdateStatus(hWnd, FALSE, FALSE);
        return;
    }
    g_bVolumeIconShown ? Volume_UpdateIcon(hWnd, NIM_MODIFY) : Volume_UpdateStatus(hWnd, TRUE, TRUE);
}

void Volume_WmDestroy(
   HWND hDlg
   )
{
    AudioController_Dispose(&g_pAudioController);
}

void Volume_Shutdown(
    HWND hWnd)
{
    Volume_UpdateStatus(hWnd, FALSE, FALSE);
}

void Volume_Menu(HWND hwnd, UINT uMenuNum, UINT uButton)
{
    POINT   pt;
    UINT    iCmd;
    HMENU   hmenu;

    GetCursorPos(&pt);

    hmenu = Volume_CreateMenu();
    if (!hmenu)
                return;

    SetForegroundWindow(hwnd);
    iCmd = TrackPopupMenu(hmenu, uButton | TPM_RETURNCMD | TPM_NONOTIFY,
        pt.x, pt.y, 0, hwnd, NULL);

    DestroyMenu(hmenu);
    switch (iCmd) {
        case VOLUMEMENU_PROPERTIES:
            Volume_ControlPanel(hwnd);
            break;

        case VOLUMEMENU_SNDVOL:
            Volume_VolumeControl();
            break;
    }

    SetIconFocus(hwnd, STWM_NOTIFYVOLUME);

}

void Volume_Notify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    switch (lParam)
    {
        case WM_RBUTTONUP:
            Volume_Menu(hwnd, 1, TPM_RIGHTBUTTON);
            break;

        case WM_LBUTTONDOWN:
            SetTimer(hwnd, VOLUME_TIMER_ID, GetDoubleClickTime()+100, NULL);
            break;

        case WM_LBUTTONDBLCLK:
            KillTimer(hwnd, VOLUME_TIMER_ID);
            Volume_VolumeControl();
            break;
    }
}


/* WARNING - WARNING - DANGER - DANGER - WARNING - WARNING - DANGER - DANGER */
/* WARNING - WARNING - DANGER - DANGER - WARNING - WARNING - DANGER - DANGER */
/* WARNING - WARNING - DANGER - DANGER - WARNING - WARNING - DANGER - DANGER */
/*
 * MYWM_WAKEUP and the "Tray Volume" window are defined by the SNDVOL32.EXE
 * application.  Changing these values or changing the values in SNDVOL32.EXE
 * without mirroring them here will break the tray volume dialog.
 */
/* WARNING - WARNING - DANGER - DANGER - WARNING - WARNING - DANGER - DANGER */
/* WARNING - WARNING - DANGER - DANGER - WARNING - WARNING - DANGER - DANGER */
/* WARNING - WARNING - DANGER - DANGER - WARNING - WARNING - DANGER - DANGER */

#define MYWM_WAKEUP             (WM_APP+100+6)

void Volume_Timer(HWND hwnd)
{
        KillTimer(hwnd, VOLUME_TIMER_ID);

        Volume_WakeUpOrClose (FALSE);
}

void Volume_WakeUpOrClose(BOOL fClose)
{
        const TCHAR szVolWindow [] = TEXT ("Tray Volume");
        HWND hApp;

        if (hApp = FindWindow(szVolWindow, NULL))
        {
                SendMessage(hApp, MYWM_WAKEUP, (WPARAM)fClose, 0);
        }
        else if (!fClose)
        {
                const TCHAR szOpen[]    = TEXT ("open");
                const TCHAR szVolApp[]  = TEXT ("SNDVOL32.EXE");
                const TCHAR szParamsWakeup[]  = TEXT ("/t");

                ShellExecute (NULL, szOpen, szVolApp, szParamsWakeup, NULL, SW_SHOWNORMAL);
        }
}


/*
 * Volume_ControlPanel
 *
 * Launch "Audio" control panel/property sheet upon request.
 *
 * */
void Volume_ControlPanel(HWND hwnd)
{
        const TCHAR szOpen[]    = TEXT ("open");
        const TCHAR szRunDLL[]  = TEXT ("RUNDLL32.EXE");
        const TCHAR szParams[]  = TEXT ("shell32.dll,Control_RunDLL MMSYS.CPL,,0"); // Playback tab

        ShellExecute(NULL, szOpen, szRunDLL, szParams, NULL, SW_SHOWNORMAL);
}

/*
 * Volume_VolumeControl
 *
 * Launch Volume Control App
 *
 * */
void Volume_VolumeControl()
{
        const TCHAR szOpen[]    = TEXT ("open");
        const TCHAR szVolApp[]  = TEXT ("SNDVOL32.EXE");

        ShellExecute(NULL, szOpen, szVolApp, NULL, NULL, SW_SHOWNORMAL);
}



/*
 * FileExists
 *
 * Does a file exist
 *
 * */

BOOL FileExists(LPCTSTR pszPath)
{
        return (GetFileAttributes(pszPath) != (DWORD)-1);
} // End FileExists


/*
 * FindSystemFile
 *
 * Finds full path to specified file
 *
 * */

BOOL FindSystemFile(LPCTSTR pszFileName, LPTSTR pszFullPath, UINT cchSize)
{
        TCHAR       szPath[MAX_PATH];
        LPTSTR      pszName;
        DWORD       cchLen;

        if ((pszFileName == NULL) || (pszFileName[0] == 0))
                return FALSE;

        cchLen = SearchPath(NULL, pszFileName, NULL, MAX_PATH,
                                                szPath,&pszName);
        if (cchLen == 0)
                return FALSE;
        
        if (cchLen >= MAX_PATH)
                cchLen = MAX_PATH - 1;

        if (! FileExists (szPath))
                return FALSE;

        if ((pszFullPath == NULL) || (cchSize == 0))
                return TRUE;

           // Copy full path into buffer
        if (cchLen >= cchSize)
                cchLen = cchSize - 1;
        
        lstrcpyn (pszFullPath, szPath, cchLen);
        
        pszFullPath[cchLen] = 0;

        return TRUE;
} // End FindSystemFile
