// Global Headers
//#include <nt.h>
//#include <ntrtl.h>
//#include <nturtl.h>

#include ".\Shundoc\shundoc.h"
#include <windows.h>
#include <windowsx.h>
#include <regstr.h>

#define NOPOWERSTATUSDEFINES

#include <mmsystem.h>
#include <shellapi.h>
//#include <shlapip.h>
#include <commctrl.h>
#include ".\Bringovers\WinUserP.h"
#include "pccrdapi.h"     
#include ".\Bringovers\systrayp.h"
//#include <help.h>         
#include <dbt.h>
//#include <ntpoapi.h>
#include <poclass.h>
//#include <cscuiext.h>

#include <objbase.h>
#include <docobj.h>
#include <shlwapi.h>
#include <shlobj.h>
//#include <shlobjp.h>

#include <dbt.h>
//#include <shfusion.h>

// Global vars
extern long g_cLocks;
extern long g_cComponents;
extern HINSTANCE g_hinstDll;

// Macros
#ifndef ARRAYSIZE
#define ARRAYSIZE(x)   (sizeof((x))/sizeof((x)[0]))
#endif
