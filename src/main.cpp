#include "stdafx.h"
#include "main.hpp"
#include "resource.hpp"
#include "3dsmaxport.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

static BOOL showPrompts;
static BOOL exportSelected;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved) 
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        hInstance = hinstDLL;
        DisableThreadLibraryCalls(hInstance);
    }

    if (!controlsInit)
    {
        controlsInit = true;
        InitCommonControls();
    }

    return TRUE;
}


__declspec( dllexport ) const TCHAR* LibDescription() 
{
    return _T("EG file exporter");
}

__declspec( dllexport ) int LibNumberClasses() 
{
    return 4;
}


__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
    switch(i)
    {
        case 0: return GetParObjModClassDesc();
        case 1: return GetParGroupModClassDesc();
        case 2: return GetParCenterModClassDesc();
        case 3: return GetParMatrixModClassDesc();
        default: return 0;
    }
}

__declspec( dllexport ) ULONG LibVersion() 
{
    return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
    return TRUE;
}

TCHAR *GetString(int id)
{
    static TCHAR buf[256];

    if (hInstance)
        return LoadString(hInstance, id, buf, sizeof(buf) / sizeof(TCHAR)) ? buf : NULL;

    return NULL;
}

static INT_PTR CALLBACK AboutBoxDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_INITDIALOG:
            CenterWindow(hWnd, GetParent(hWnd)); 
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    EndDialog(hWnd, 1);
                    break;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;
}

// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
    return(0);
}

/****************************************************************************

 Configuration.
 To make all options "sticky" across sessions, the options are read and
 written to a configuration file every time the exporter is executed.

 ****************************************************************************/

BOOL MtlKeeper::AddMtl(Mtl *mtl)
{
    if (!mtl)
        return FALSE;

    int numMtls = mtlTab.Count();
    for (int i = 0; i < numMtls; i++)
        if (mtlTab[i] == mtl)
            return FALSE;

    mtlTab.Append(1, &mtl, 25);

    return TRUE;
}

int MtlKeeper::GetMtlID(Mtl *mtl)
{
    int numMtls = mtlTab.Count();
    for (int i = 0; i < numMtls; i++)
        if (mtlTab[i] == mtl)
            return i;

    return -1;
}

int MtlKeeper::Count()
{
    return mtlTab.Count();
}

Mtl *MtlKeeper::GetMtl(int id)
{
    return mtlTab[id];
}
