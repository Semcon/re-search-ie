// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#include "stdafx.h"
#include "ReSearchConfig.h"
#include "DebugMessage.h"
#include "Resource.h"
#include <atlbase.h>
#include <string>
#include <fstream>
#include <mutex>
#include <cpprest/json.h>

//#define CHECK_LOCAL_FOLDER 1

#define REGISTRY_KEY_PATH L"SOFTWARE\\ReSearchIE"

extern ATL::CComModule _Module;

namespace config {
namespace {
struct SettingsStorage
{
	std::mutex m_StorageMutex;
	bool m_Initialized;
	std::wstring m_JsonURL;
	std::string m_ToolbarCSS;
	std::string m_InjectToolbarJS;
};
static SettingsStorage s_StaticSettings;

const char* LoadFileFromResource( int name, int type )
{
	HINSTANCE handle = _Module.m_hInst;
	if (handle == nullptr) {
		DebugMessage( "instance not initialized" );
		return nullptr;
	}
	HRSRC rc = ::FindResource( handle, MAKEINTRESOURCE( name ), MAKEINTRESOURCE( type ) );
	if (rc == nullptr) {
		DebugMessage( "Failed to find resource {}", name );
		return nullptr;
	}
	HGLOBAL rcData = ::LoadResource( handle, rc );
	if (rcData == nullptr) {
		DebugMessage( "Failed to load resource {}", name );
		return nullptr;
	}
	const char* data = static_cast<const char*>(::LockResource( rcData ));
	DWORD size = ::SizeofResource( handle, rc );
	if (data == nullptr || size == 0)
		return nullptr;
	assert( data[size] == '\0' );
	return data;
}

DWORD GetRegistrySetting( HKEY root, const wchar_t* key_path, const wchar_t* key_name, DWORD default_value )
{
	HKEY hKey;
	LSTATUS lres = ::RegOpenKeyExW( root, key_path, 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &hKey );
	DWORD value = default_value;
	if (lres == ERROR_SUCCESS) {
		DWORD dwBufferSize( sizeof( DWORD ) );
		DWORD nResult( 0 );
		LSTATUS nError = ::RegQueryValueExW( hKey,
											 key_name,
											 0,
											 nullptr,
											 reinterpret_cast<LPBYTE>(&nResult),
											 &dwBufferSize );
		if (ERROR_SUCCESS == nError)
		{
			DebugMessage( "Retrieved registry value {}\\{} = {}", key_path, key_name, nResult );
			value = nResult;
		} else {
			DebugMessage( "Failed to retrieve registry value" );
		}
		RegCloseKey( hKey );
	}
	return value;
}

std::wstring GetRegistryKeyString( HKEY root, const wchar_t* key_path, const wchar_t* key_name )
{
	HKEY hKey;
	LSTATUS lres = ::RegOpenKeyExW( root, key_path, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey );
	if (lres != ERROR_SUCCESS)
		lres = ::RegOpenKeyExW( root, key_path, 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &hKey );
	if (lres != ERROR_SUCCESS)
		return std::wstring{};
	DWORD dwBufferSize( sizeof( DWORD ) );
	DWORD dwType{ 0 };
	LSTATUS nError = ::RegQueryValueExW( hKey,
										 key_name,
										 0,
										 &dwType,
										 nullptr,
										 &dwBufferSize );
	std::wstring value;
	if (ERROR_SUCCESS == nError && dwBufferSize > 0 && dwType == REG_SZ) {
		value.resize( dwBufferSize );
		nError = ::RegQueryValueExW( hKey,
									 key_name,
									 0,
									 nullptr,
									 (LPBYTE)value.data( ),
									 &dwBufferSize );
		if (nError != ERROR_SUCCESS)
			value.clear( );
	}
	RegCloseKey( hKey );
	while (value.back( ) == L'\0')
		value.pop_back( );
	return value;
}

bool SetRegistrySetting( HKEY root, const wchar_t* key_path, const wchar_t* key_name, DWORD value )
{
	HKEY hKey;
	LRESULT lRes = ::RegCreateKeyExW( root, key_path, 0, NULL, 0, KEY_WRITE | KEY_SET_VALUE | KEY_WOW64_32KEY, NULL, &hKey, NULL );
	if (lRes != ERROR_SUCCESS) {
		DebugMessage( "Failed to create registry key" );
		return false;
	}
	DebugMessage( "Saving registry {}\\{} = {}", key_path, key_name, value );
	lRes = RegSetValueExW( hKey, key_name, 0, REG_DWORD, (LPBYTE)&value, sizeof( value ) );
	RegCloseKey( hKey );
	if (lRes != ERROR_SUCCESS) {
		DebugMessage( "Failed to set registry key value" );
		return false;
	}
	return true;
}

template<typename TChar>
std::basic_string<TChar> ReadFile( const std::wstring& filename )
{
	std::basic_ifstream<TChar> t( filename );
	if (!t.is_open( )) {
		DebugMessage( "Failed to open file '{}'", filename );
		return std::basic_string<TChar>{};
	}
	return std::basic_string<TChar>( (std::istreambuf_iterator<TChar>( t )), std::istreambuf_iterator<TChar>( ) );
}

} // namespace

const wchar_t* GetJsonURL( )
{
	Initialize( );
	if (s_StaticSettings.m_JsonURL.empty( ))
		return L"http://raw.githubusercontent.com/Semcon/re-search-config/master/data.json";
	return s_StaticSettings.m_JsonURL.c_str( );
}

const char* GetToolbarCSS( )
{
	Initialize( );
	if (s_StaticSettings.m_ToolbarCSS.empty( ))
		return LoadFileFromResource( IDR_RESEARCH_CSS, TEXTFILE );
	return s_StaticSettings.m_ToolbarCSS.c_str( );
}

const char* GetInjectToolbarJS( )
{
	Initialize( );
	if (s_StaticSettings.m_InjectToolbarJS.empty( ))
		return LoadFileFromResource( IDR_JS_TOOLBAR, TEXTFILE );
	return s_StaticSettings.m_InjectToolbarJS.c_str( );
}

bool IsEnabled( )
{
	return GetRegistrySetting( HKEY_CURRENT_USER, REGISTRY_KEY_PATH, L"Enabled", 1 ) != 0;
}

void SaveIsEnabled( bool enable )
{
	SetRegistrySetting( HKEY_CURRENT_USER, REGISTRY_KEY_PATH, L"Enabled", enable ? 1 : 0 );
}

void Initialize( )
{
	std::unique_lock<std::mutex> lock( s_StaticSettings.m_StorageMutex );
	if (s_StaticSettings.m_Initialized)
		return;
	s_StaticSettings.m_Initialized = true;

#if defined(CHECK_LOCAL_FOLDER) && CHECK_LOCAL_FOLDER
	const auto install_path = GetRegistryKeyString( HKEY_LOCAL_MACHINE, REGISTRY_KEY_PATH, L"InstallFolder" );
	if (install_path.empty( ))
		return;
	try {
		// Try to read settings json file
		auto file_data = ReadFile<wchar_t>( install_path + L"\\settings.json" );
		if (!file_data.empty( )) {
			auto&& settings = web::json::value::parse( file_data );
			auto json_url = settings[L"data_url"];
			auto readmore_url = settings[L"readmore_url"];
			if (json_url.is_string( )) {
				s_StaticSettings.m_JsonURL = settings[L"data_url"].as_string( );
			}
		}
	}
	catch (const std::exception& e)
	{
		DebugMessage( "Failed to parse settings file '{}'", e.what( ) );
	}
	s_StaticSettings.m_InjectToolbarJS = ReadFile<char>( install_path + L"\\toolbar.js" );
	s_StaticSettings.m_ToolbarCSS = ReadFile<char>( install_path + L"\\research.css" );
#endif
}

} // namespace config