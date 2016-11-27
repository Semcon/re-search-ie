// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#include "stdafx.h"
#include "ReSearchBHO.h"
#include "generated/ReSearchIE_i.h"
#include "resource.h"

using namespace ATL;

CComModule _Module;

BEGIN_OBJECT_MAP( ObjectMap )
	OBJECT_ENTRY( CLSID_ReSearchBHO, CReSearchBHO )
END_OBJECT_MAP( )

// DLL Entry Point
extern "C" BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved )
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		_Module.Init( ObjectMap, _Module.GetModuleInstance( ), &LIBID_ReSearchIELib );
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

// Used to determine whether the DLL can be unloaded by OLE.
STDAPI DllCanUnloadNow( void )
{
	return(_Module.GetLockCount( ) == 0) ? S_OK : S_FALSE;
}

// Returns a class factory to create an object of the requested type.
_Check_return_
STDAPI DllGetClassObject( _In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv )
{
	return _Module.GetClassObject( rclsid, riid, ppv );
}

// DllRegisterServer - Adds entries to the system registry.
STDAPI DllRegisterServer( void )
{
	// registers object, typelib and all interfaces in typelib
	return _Module.RegisterServer( TRUE );
}

// DllUnregisterServer - Removes entries from the system registry.
STDAPI DllUnregisterServer( void )
{
	return _Module.UnregisterServer( TRUE );
}

// DllInstall - Adds/Removes entries to the system registry per user per machine.
STDAPI DllInstall( BOOL bInstall, _In_opt_  LPCWSTR pszCmdLine )
{
	HRESULT hr = E_FAIL;
	static const wchar_t szUserSwitch[] = L"user";

	if (pszCmdLine != NULL)
	{
		if (_wcsnicmp( pszCmdLine, szUserSwitch, _countof( szUserSwitch ) ) == 0)
		{
			ATL::AtlSetPerUserRegistration( true );
		}
	}

	if (bInstall)
	{
		hr = DllRegisterServer( );
		if (FAILED( hr ))
		{
			DllUnregisterServer( );
		}
	} else
	{
		hr = DllUnregisterServer( );
	}

	return hr;
}
