#ifndef _SHSRVOBJ_H
#define _SHSRVOBJ_H

#include "Bringovers/dpa.h"

//
// class to manage shell service objects
//

typedef struct
{
    CLSID              clsid;
    IOleCommandTarget* pct;
}
SHELLSERVICEOBJECT, *PSHELLSERVICEOBJECT;


class CShellServiceObjectMgr
{
public:
    HRESULT Init();
    void Destroy();
    HRESULT LoadObjects();
    HRESULT EnableObject(const CLSID *pclsid, DWORD dwFlags);

    virtual ~CShellServiceObjectMgr();

private:
    static int WINAPI DestroyItemCB(SHELLSERVICEOBJECT *psso, CShellServiceObjectMgr *pssomgr);
    HRESULT _LoadObject(REFCLSID rclsid, DWORD dwFlags);
    int _FindItemByCLSID(REFCLSID rclsid);
    BOOL _IsSSOApproved(REFCLSID rclsid);

    CDSA<SHELLSERVICEOBJECT> _dsaSSO;
};

extern CShellServiceObjectMgr* g_pssomgr;

#endif  // _SHSRVOBJ_H