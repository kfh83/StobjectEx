#include "stdafx.h"
#include "stobject.h"
#include "systray.h"

/************************************************************************************
IUnknown Implementation

************************************************************************************/

HRESULT CSysTray::QueryInterface(REFIID iid, void** ppvObject)
{
    HRESULT hr = S_OK;

    if ((iid == IID_IOleCommandTarget) || (iid == IID_IUnknown))
    {
        *ppvObject = (IOleCommandTarget*) this;
    }
    else
    {
        *ppvObject = NULL;
        hr = E_NOINTERFACE;
    }

    if (hr == S_OK)
    {
        ((IUnknown*) (*ppvObject))->AddRef();
    }

    return hr;
}

ULONG CSysTray::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CSysTray::Release()
{
    //ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }    
    return cRef;
}

/************************************************************************************
IOleCommandTarget Implementation

************************************************************************************/

HRESULT CSysTray::QueryStatus(const GUID* pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT* pCmdText)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;
 
    if (*pguidCmdGroup == CGID_ShellServiceObject)
    {
        // We like Shell Service Object notifications...
        hr = S_OK;
    }

    return hr;
}

HRESULT CSysTray::Exec(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG* pvaIn, VARIANTARG* pvaOut)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    if (*pguidCmdGroup == CGID_ShellServiceObject)
    {
        // Handle Shell Service Object notifications here.
        switch (nCmdID)
        {
            case SSOCMDID_OPEN:
                hr = CreateSysTrayThread();
                break;

            case SSOCMDID_CLOSE:
                hr = DestroySysTrayWindow();
                break;

            default:
                hr = S_OK;
                break;
        }
    }

    return hr;
}

/************************************************************************************
Constructor/Destructor Implementation

************************************************************************************/
CSysTray::CSysTray(BOOL fRunTrayOnConstruct)
{
    m_cRef = 1;
    InterlockedIncrement(&g_cLocks);

    if (fRunTrayOnConstruct)
    {
        // We are being called through SHLoadInProc - Launch the systray thread immediately
        CreateSysTrayThread();
    }
}

CSysTray::~CSysTray()
{
    //ASSERT( 0 != g_cLocks );
    InterlockedDecrement(&g_cLocks);
}

/************************************************************************************
Private Function Implementation

************************************************************************************/

HRESULT CSysTray::CreateSysTrayThread()
{
    HRESULT hr = S_OK;
    HANDLE hThread;
    DWORD dwThreadId;
 
    hThread = CreateThread(NULL, 0, CSysTray::SysTrayThreadProc, NULL, 0, &dwThreadId);
    if (hThread != NULL)
    {
        CloseHandle(hThread);
    }
    else
        hr = E_FAIL;

    return hr;
}

DWORD CSysTray::SysTrayThreadProc(void* lpv)
{
    // We pass a "" for the command line to so that the tray applets don't start.
    TCHAR szModule[MAX_PATH];

    GetModuleFileName(g_hinstDll, szModule, ARRAYSIZE(szModule));

    HINSTANCE hInstThis = LoadLibrary(szModule);

    int Result = SysTrayMain(g_hinstDll, NULL, (LPTSTR)TEXT(""), SW_SHOWNORMAL);
    
    FreeLibraryAndExitThread(hInstThis, (DWORD) Result);

    // Never gets here
    return 0; 
}

HRESULT CSysTray::DestroySysTrayWindow()
{
    HWND hExistWnd = FindWindow(SYSTRAY_CLASSNAME, NULL);
    if (hExistWnd) 
    {
        // Destroy the window. Note that we can't use DestroyWindow since
        // the window is on a different thread and DestroyWindow fails.
        SendMessage(hExistWnd, WM_CLOSE, 0, 0);
    }

    return S_OK;
}
