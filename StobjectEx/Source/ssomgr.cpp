#include "stdafx.h"

#include "ssomgr.h"

CShellServiceObjectMgr* g_pssomgr;

HRESULT CShellServiceObjectMgr::_LoadObject(REFCLSID rclsid, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;

    if (dwFlags & LIPF_HOLDREF)
    {
        if (_dsaSSO)
        {
            SHELLSERVICEOBJECT sso = {0};
            sso.clsid = rclsid;

            hr = CoCreateInstance(rclsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER, IID_PPV_ARGS(&sso.pct));
            if (SUCCEEDED(hr))
            {
                if (_dsaSSO.AppendItem(&sso) != -1)
                {
                    sso.pct->Exec(&CGID_ShellServiceObject, SSOCMDID_OPEN, 0, NULL, NULL);
                }
                else
                {
                    sso.pct->Release();
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }
    else
    {
        // just ask for IUnknown for these dudes
        IUnknown *punk;
        hr = CoCreateInstance(rclsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&punk));
        if (SUCCEEDED(hr))
        {
            punk->Release();
        }
    }

    return hr;
}

HRESULT CShellServiceObjectMgr::LoadObjects()
{
    DWORD cchName;
    HKEY phkResult;
    HKEY hkey;
    CLSID clsid;
    WCHAR szName[39];

    HRESULT hr = ResultFromWin32(RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellServiceObjects",
        0,
        KEY_READ,
        &phkResult));
    if (hr >= 0)
    {
        if (RegOpenKeyEx(
            HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellServiceObjects",
            0,
            KEY_READ,
            &hkey) != 0)
        {
            hkey = NULL;
        }

        DWORD dwIndex = 0;
        do
        {
            cchName = ARRAYSIZE(szName);
            hr = ResultFromWin32(RegEnumKeyEx(phkResult, dwIndex, szName, &cchName, NULL, NULL, NULL, NULL));
            if (hr >= 0
                && (RegGetValue(phkResult, szName, L"AutoStart", SRRF_RT_ANY, NULL, NULL, NULL) == 0
                    || hkey != NULL && RegGetValue(hkey, szName, L"AutoStart", SRRF_RT_ANY, NULL, NULL, NULL) == 0)
                && (hkey == NULL || RegGetValue(hkey, szName, L"NoAutoStart", SRRF_RT_ANY, NULL, NULL, NULL) != 0)
                && CLSIDFromString(szName, &clsid) >= 0
                && _IsSSOApproved(clsid) != 0)
            {
                _LoadObject(clsid, LIPF_ENABLE | LIPF_HOLDREF);
            }
            ++dwIndex;
        }
        while (hr >= 0);

        hr = S_OK;

        if (hkey != NULL)
            RegCloseKey(hkey);
        RegCloseKey(phkResult);
    }

    return hr;
}

HRESULT CShellServiceObjectMgr::EnableObject(const CLSID* pclsid, DWORD dwFlags)
{
    HRESULT hr = E_INVALIDARG;

    if (_IsSSOApproved(*pclsid))
    {
        if (dwFlags & LIPF_ENABLE)
        {
            return _LoadObject(*pclsid, dwFlags);
        }

        int i = _FindItemByCLSID(*pclsid);
        if (i != -1)
        {
            PSHELLSERVICEOBJECT psso = _dsaSSO.GetItemPtr(i);
            if (psso)
            {
                DestroyItemCB(psso, this);
            }
            DSA_DeleteItem(_dsaSSO, i);
            hr = S_OK;
        }
    }

    return hr;
}

int CShellServiceObjectMgr::_FindItemByCLSID(REFCLSID rclsid)
{
    if (_dsaSSO)
    {
        for (int i = _dsaSSO.GetItemCount() - 1; i >= 0; i--)
        {
            PSHELLSERVICEOBJECT psso = _dsaSSO.GetItemPtr(i);
            if (IsEqualCLSID(psso->clsid, rclsid))
            {
                return i;
            }
        }
    }
    return -1;
}

#define INITGUID
#include <initguid.h>
DEFINE_GUID(CLSID_AltTabSSO,                    0xA1607060, 0x5D4C, 0x467A, 0xB7, 0x11, 0x2B, 0x59, 0xA6, 0xF2, 0x59, 0x57);
DEFINE_GUID(CLSID_ConnectionTray,               0x7007ACCF, 0x3202, 0x11D1, 0xAA, 0xD2, 0x00, 0x80, 0x5F, 0xC1, 0x27, 0x0E);
DEFINE_GUID(CLSID_PostBootReminder,             0x7849596A, 0x48EA, 0x486E, 0x89, 0x37, 0xA2, 0xA3, 0x00, 0x9F, 0x31, 0xA9);
DEFINE_GUID(CLSID_OfflineFilesShellSvcObject,   0xC51F0A6B, 0x2A63, 0x4CF4, 0x89, 0x38, 0x24, 0x40, 0x4E, 0xAE, 0xF4, 0x22);
DEFINE_GUID(CLSID_WebCheck,                     0xE6FB5E20, 0xDE35, 0x11CF, 0x9C, 0x87, 0x00, 0xAA, 0x00, 0x51, 0x27, 0xED);
DEFINE_GUID(stru_6D502800,                      0xFD6905CE, 0x952F, 0x41F1, 0x9A, 0x6F, 0x13, 0x5D, 0x9C, 0x66, 0x22, 0xCC);
DEFINE_GUID(stru_6D5027E0,                      0xDA67B8AD, 0xE81B, 0x4C70, 0x9B, 0x91, 0xB4, 0x17, 0xB5, 0xE3, 0x35, 0x27);
DEFINE_GUID(stru_6D502790,                      0x6FDEDD65, 0xAC51, 0x43CA, 0xB2, 0xD0, 0x9E, 0xB5, 0xD1, 0x15, 0x5D, 0x03);
DEFINE_GUID(CLSID_WPDShServiceObj,              0xAAA288BA, 0x9A4C, 0x45B0, 0x95, 0xD7, 0x94, 0xD5, 0x24, 0x86, 0x9D, 0xB5);
DEFINE_GUID(CLSID_SyncMgrShellServiceObject,    0xF20487CC, 0xFC04, 0x4B1E, 0x86, 0x3F, 0xD9, 0x80, 0x17, 0x96, 0x13, 0x0B);
DEFINE_GUID(CLSID_AudioVolumeShellService,      0x3BF043EF, 0xA974, 0x49B3, 0x83, 0x22, 0xB8, 0x53, 0xCF, 0x1E, 0x5E, 0xC5);
DEFINE_GUID(CLSID_DirtyShutdownReason,          0x68DDBB56, 0x9D1D, 0x4FD9, 0x89, 0xC5, 0xC0, 0xDA, 0x2A, 0x62, 0x53, 0x92);
DEFINE_GUID(stru_6D502810,                      0xE31004D1, 0xA431, 0x41B8, 0x82, 0x6F, 0xE9, 0x02, 0xF9, 0xD9, 0x5C, 0x81);

const CLSID* c_ClassMap[] =
{
    &CLSID_AltTabSSO,
    &CLSID_CDBurn,
    &CLSID_ConnectionTray,
    &CLSID_PostBootReminder,
    &CLSID_WebCheck,
    &CLSID_OfflineFilesShellSvcObject,
    &stru_6D502800,
    &stru_6D5027E0,
    &stru_6D502790,
    &CLSID_WPDShServiceObj,
    &CLSID_SyncMgrShellServiceObject,
    &CLSID_AudioVolumeShellService,
    &CLSID_DirtyShutdownReason,
    &stru_6D502810
};

BOOL CShellServiceObjectMgr::_IsSSOApproved(REFCLSID rclsid)
{
    BOOL fApproved = FALSE;

    for (int i = 0; i < ARRAYSIZE(c_ClassMap); ++i)
    {
        if (IsEqualCLSID(*c_ClassMap[i], rclsid))
        {
            fApproved = TRUE;
            break;
        }
    }

    return fApproved;
}

HRESULT CShellServiceObjectMgr::Init()
{
    return _dsaSSO.Create(2) ? S_OK : E_FAIL;
}

CShellServiceObjectMgr::~CShellServiceObjectMgr()
{
    Destroy();
}

int WINAPI CShellServiceObjectMgr::DestroyItemCB(SHELLSERVICEOBJECT *psso, CShellServiceObjectMgr *pssomgr)
{
    psso->pct->Exec(&CGID_ShellServiceObject, SSOCMDID_CLOSE, 0, NULL, NULL);
    psso->pct->Release();
    return 1;
}

void CShellServiceObjectMgr::Destroy()
{
    _dsaSSO.DestroyCallbackEx<CShellServiceObjectMgr*>(DestroyItemCB, this);
}
