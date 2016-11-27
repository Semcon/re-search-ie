// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#include "stdafx.h"
#include "ReSearchBHO.h"
#include "ReSearchConfig.h"
#include <cassert>
#include <comutil.h>

// Status bar pane name
#define PANE_CLASS_NAME L"ReSearchIEPaneClass"
#define ABOUT_BLANK L"about:blank"

using ScopedLock = std::lock_guard<std::mutex>;

using namespace ATL;
extern CComModule _Module;

std::wstring GetLastErrorMessage( )
{
	const auto error = GetLastError( );
	LPWSTR errMsg{ nullptr };
	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		(LPWSTR)&errMsg,
		0, NULL );
	if (errMsg)
	{
		std::wstring msg_str = errMsg;
		LocalFree( errMsg );
		return errMsg;
	}
	return L"Failed to retrieve message";
}

bool IsBlankUrl(const VARIANT* variant)
{
	if (variant->vt != VT_BSTR || variant->bstrVal == nullptr)
		return true;
	return wcsncmp( variant->bstrVal, ABOUT_BLANK, ARRAYSIZE( ABOUT_BLANK ) ) == 0 || wcsstr( variant->bstrVal, L"blank.html" ) != nullptr;
}

template<typename T>
void PutProperty( IWebBrowser2* wnd, const wchar_t* name, const T& value )
{
	assert( wnd != nullptr );
	_variant_t variant( value );
	if (FAILED( wnd->PutProperty( _bstr_t( name ), variant ) ))
		throw std::runtime_error( "Failed to put property" );
}

template<typename T>
bool GetProperty( IWebBrowser2* wnd, const wchar_t* name, T& value )
{
	assert( wnd != nullptr );
	_variant_t variant;
	if (FAILED( wnd->GetProperty( _bstr_t( name ), &variant ) ))
		return false;
	value = variant;
	return true;
}

template<typename T>
T GetProperty( IWebBrowser2* wnd, const wchar_t* name )
{
	assert( wnd != nullptr );
	_variant_t variant;
	if (FAILED( wnd->GetProperty( _bstr_t( name ), &variant ) ))
		throw std::runtime_error( "Failed to get property" );
	return variant;
}

void MarkSearch( IWebBrowser2* wnd )
{
	assert( wnd != nullptr );
	PutProperty( wnd, L"RS_Marked", true );
}

bool IsMarked( IWebBrowser2* wnd )
{
	bool marked{ false };
	if (!GetProperty( wnd, L"RS_Marked", marked ))
		return false;
	wnd->PutProperty( L"RS_Marked", _variant_t{} );
	return marked;
}

void RemoveProperty( IWebBrowser2* wnd, const wchar_t* name )
{
	assert( wnd != nullptr );
	const auto str = _bstr_t( name );
	_variant_t variant;
	if (SUCCEEDED( wnd->GetProperty( str, &variant ) ) && variant.vt != VT_EMPTY) {
		if (variant.vt == VT_DISPATCH) {
			variant.pdispVal->Release( );
		}
		wnd->PutProperty( str, _variant_t{} );
	}
}

HWND GetHwnd( IWebBrowser2* left )
{
	if (!left) {
		DebugMessage( "Can not retrieve hwnd without browser" );
		return NULL;
	}
	SHANDLE_PTR handle;
	if (FAILED( left->get_HWND( &handle ) )) {
		DebugMessage( "Failed to retrieve HWND from browser ({})", (const void*)left );
		return NULL;
	}
	DebugMessage( "Got HWND {} from browser ", (const void*)handle, (const void*)left );
	return (HWND)handle;
}

HMONITOR GetMonitor( IWebBrowser2* left )
{
	auto wnd = GetHwnd( left );
	if (wnd) {
		DebugMessage( "Retrieving monitor using HWND ({})", (const void*)left );
		return MonitorFromWindow( wnd, MONITOR_DEFAULTTOPRIMARY );
	} else {
		DebugMessage( "Retrieving monitor using point (0,0) ({})", (const void*)left );
		const POINT ptZero = { 0, 0 };
		return MonitorFromPoint( ptZero, MONITOR_DEFAULTTOPRIMARY );
	}
}

bool ResizeWindowAlt( HWND wnd, int x, int y, int width, int height )
{
	if (!MoveWindow( wnd, x, y, width, height, TRUE )) {
		DebugMessage( "Failed to resize window with error: {}", GetLastErrorMessage( ) );
		return false;
	}
	return true;
}

bool ResizeBrowser2( IWebBrowser2* wnd, int x, int y, int width, int height )
{
	assert( wnd != nullptr );
	DebugMessage( "Resizing window ({}) ({}, {}, {}, {}", (const void*)wnd, x, y, width, height );
	auto hwnd = GetHwnd( wnd );
	if (hwnd) {
		RECT rcClient, rcWindow;
		GetClientRect( hwnd, &rcClient );
		GetWindowRect( hwnd, &rcWindow );
		width += std::max( ((rcWindow.right - rcWindow.left) - rcClient.right) / 2 - 1, 0l );
		height += std::max( ((rcWindow.bottom - rcWindow.top) - rcClient.bottom) / 2, 0l );
	}
	const auto r1 = wnd->put_Left( x );
	const auto r2 = wnd->put_Width( width );
	const auto r3 = wnd->put_Top( y );
	const auto r4 = wnd->put_Height( height );

	if (SUCCEEDED( r1 ) && SUCCEEDED( r2 ) && SUCCEEDED( r3 ) && SUCCEEDED( r4 )) {
		return true;
	}
	DebugMessage( "Failed to resize browser ({}) {} {} {} {}", (const void*)wnd, r1, r2, r3, r4 );
	return ResizeWindowAlt( GetHwnd( wnd ), x, y, width, height );
}

void MaximizeWindows( IWebBrowser2* left, IWebBrowser2* right )
{
	auto focus = GetFocus( );
	auto monitor = GetMonitor( left );
	if (monitor == NULL) {
		DebugMessage( "Failed to retrieve monitor handle ({})", (const void*)left );
		return;
	}
	MONITORINFO info;
	ZeroMemory( &info, sizeof( info ) );
	info.cbSize = sizeof( MONITORINFO );
	if (!GetMonitorInfo( monitor, &info )) {
		DebugMessage( "Failed to retrieve monitor info ({})", (const void*)right );
		return;
	}
	const auto width = info.rcWork.right - info.rcWork.left;
	const auto height = info.rcWork.bottom - info.rcWork.top;

	ResizeBrowser2( left, info.rcWork.left - 8, info.rcWork.top, width / 2 + 16, height );
	ResizeBrowser2( right, info.rcWork.left + width / 2, info.rcWork.top, width / 2, height );

	if (focus) {
		DebugMessage( "Restoring focus to {}", (const void*)focus );
		SetFocus( focus );
	}
}

struct Dimensions
{
	long m_Width{ 200 }, m_Height{ 200 };
	long m_Left{ 0 }, m_Top{ 0 };
	bool m_Fullscreen{ false };
};

void StoreDimensions( const Dimensions& dims, IWebBrowser2* wnd )
{
	PutProperty( wnd, L"RS_Width", dims.m_Width );
	PutProperty( wnd, L"RS_Height", dims.m_Height );
	PutProperty( wnd, L"RS_Left", dims.m_Left );
	PutProperty( wnd, L"RS_Top", dims.m_Top );
	PutProperty( wnd, L"RS_Fullscreen", dims.m_Fullscreen );
}

Dimensions GetStoredDimensions( IWebBrowser2* wnd )
{
	Dimensions dims;
	GetProperty( wnd, L"RS_Fullscreen", dims.m_Fullscreen );
	GetProperty( wnd, L"RS_Width", dims.m_Width );
	GetProperty( wnd, L"RS_Height", dims.m_Height );
	GetProperty( wnd, L"RS_Left", dims.m_Left );
	GetProperty( wnd, L"RS_Top", dims.m_Top );
	return dims;
}

Dimensions GetDimensions( IWebBrowser2* wnd )
{
	assert( wnd != nullptr );
	Dimensions dims;

	VARIANT_BOOL is_fullscreen{ VARIANT_FALSE };
	if (FAILED( wnd->get_FullScreen( &is_fullscreen ) ))
		throw std::runtime_error( "Failed to retrieve fullscreen" );
	dims.m_Fullscreen = is_fullscreen == VARIANT_TRUE;

	if (FAILED( wnd->get_Width( &dims.m_Width ) ))
		throw std::runtime_error( "Failed to retrieve width" );
	if (FAILED( wnd->get_Height( &dims.m_Height ) ))
		throw std::runtime_error( "Failed to retrieve height" );
	if (FAILED( wnd->get_Left( &dims.m_Left ) ))
		throw std::runtime_error( "Failed to retrieve left" );
	if (FAILED( wnd->get_Top( &dims.m_Top ) ))
		throw std::runtime_error( "Failed to retrieve top" );
	return dims;
}

void PutDimensions( const Dimensions& dims, IWebBrowser2* wnd )
{
	assert( wnd != nullptr );
	if (FAILED( wnd->put_Width( dims.m_Width ) ))
		throw std::runtime_error( "Failed to put width" );
	if (FAILED( wnd->put_Height( dims.m_Height ) ))
		throw std::runtime_error( "Failed to put height" );
	if (FAILED( wnd->put_Left( dims.m_Left ) ))
		throw std::runtime_error( "Failed to put left" );
	if (FAILED( wnd->put_Top( dims.m_Top ) ))
		throw std::runtime_error( "Failed to put top" );
	if (FAILED( wnd->put_FullScreen( dims.m_Fullscreen ? VARIANT_TRUE : VARIANT_FALSE ) ))
		throw std::runtime_error( "Failed to put fullscreen" );
}

CReSearchBHO::CReSearchBHO( )
{
	const HRESULT result = ::CoInitialize( nullptr );
	if (!SUCCEEDED( result ))
		throw std::runtime_error( "Failed to initialize COM" );
	m_IsEnabled = config::IsEnabled( );
}

CReSearchBHO::~CReSearchBHO( )
{
	::CoUninitialize( );
}

void ToWstring( std::wstring& out, const BSTR b )
{
	if (b) {
		const auto len = ::SysStringLen( b );
		out.assign( b, len );
	} else {
		out.clear( );
	}
}

std::wstring ToWstring( const BSTR b )
{
	std::wstring str;
	ToWstring( str, b );
	return str;
}

STDMETHODIMP CReSearchBHO::SetSite( IUnknown* unknown_site )
{
	DebugMessage( "SetSite ({})", (const void*)unknown_site );
	if (unknown_site != nullptr)
	{
		// Fetch webbrowser
		m_WebBrowser2 = CComQIPtr<IWebBrowser2>( unknown_site );
		if (m_WebBrowser2 != nullptr) {
			if (m_IsEnabled) {
				// Trigger async load
				m_LookupLoader.LoadAsync( );
			}
			// Advise events
			auto hr = DispEventAdvise( m_WebBrowser2 );
			m_IsAdvised = SUCCEEDED( hr );
			DebugMessage( "Initialized browser ({})", m_WebBrowser2 );
		} else {
			DebugMessage( "SetSite called without valid WebBrowser2 ({})", m_WebBrowser2 );
		}
	} else
	{
		if (m_WebBrowser2) {
			m_WebBrowser2 = nullptr;
		}
	}

	// Return the base class implementation
	return IObjectWithSiteImpl<CReSearchBHO>::SetSite( unknown_site );
}

STDMETHODIMP CReSearchBHO::QueryStatus( const GUID* pguidCmdGroup, ULONG cCmds,
										OLECMD prgCmds[], OLECMDTEXT* pCmdText )
{
	if (cCmds == 0)
		return E_INVALIDARG;
	if (prgCmds == nullptr)
		return E_POINTER;

	prgCmds[0].cmdf = OLECMDF_ENABLED;
	return S_OK;
}

STDMETHODIMP CReSearchBHO::Exec( const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
								 VARIANTARG* pvaIn, VARIANTARG* pvaOut )
{
	try
	{
		DebugMessage( "Got command ({}) {} {}", m_WebBrowser2, nCmdID, nCmdexecopt );
		if (nCmdID == 0) {
			HMENU hMenu = CreateToolbarMenu( );
			if (!hMenu)
				return E_FAIL;

			int nIDCommand = -1;
			POINT pt;
			GetCursorPos( &pt );
			UINT nFlags = 0;

			DisplayToolbarMenu( hMenu, nIDCommand, pt, nFlags );
		}
		return S_OK;
	}
	catch (...) {
		DebugMessage( "Exception during Exec ({})", m_WebBrowser2 );
		return E_FAIL;
	}
}

HMENU CReSearchBHO::CreateToolbarMenu( ) const
{
	HINSTANCE hInstance = ATL::_AtlBaseModule.GetModuleInstance( );
	HMENU hMenu = ::LoadMenu( hInstance, MAKEINTRESOURCE( IDR_TOOLBAR_MENU ) );
	HMENU hMenuTrackPopup = GetSubMenu( hMenu, 0 );
	CheckMenuItem( hMenu, ID_MENU_ENABLE_RESEARCH, m_IsEnabled ? MF_CHECKED : MF_UNCHECKED );
	return hMenuTrackPopup;
}

INT_PTR CALLBACK AboutBoxProc( HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	switch (Msg)
	{
	case WM_INITDIALOG:
	// Position at center of parent
	// https://msdn.microsoft.com/en-gb/library/windows/desktop/ms644996(v=vs.85).aspx
	{
		auto hwndOwner = GetParent( hWndDlg );

		RECT rcOwner, rcDlg, rc;
		GetWindowRect( hwndOwner, &rcOwner );
		GetWindowRect( hWndDlg, &rcDlg );
		CopyRect( &rc, &rcOwner );

		OffsetRect( &rcDlg, -rcDlg.left, -rcDlg.top );
		OffsetRect( &rc, -rc.left, -rc.top );
		OffsetRect( &rc, -rcDlg.right, -rcDlg.bottom );

		SetWindowPos( hWndDlg,
					  HWND_TOP,
					  rcOwner.left + (rc.right / 2),
					  rcOwner.top + (rc.bottom / 2),
					  0, 0,          // Ignores size arguments. 
					  SWP_NOSIZE );
		return TRUE;
	}
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			EndDialog( hWndDlg, 0 );
			return TRUE;
		}
		break;
	}

	return FALSE;
}

void CReSearchBHO::DisplayToolbarMenu( HMENU hMenu, int nToolbarCmdID, POINT pt, UINT nMenuFlags )
{
	// Create menu parent window
	HWND hMenuWnd = ::CreateWindowEx(
		NULL,
		MAKEINTATOM( GetToolbarMenuClass( ) ),
		L"",
		0,
		0, 0, 0, 0,
		NULL,
		NULL,
		_Module.m_hInst,
		NULL );

	if (!hMenuWnd)
	{
		DestroyMenu( hMenu );
		return;
	}

	// Display menu
	nMenuFlags |= TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON;

	int nCommand = ::TrackPopupMenu( hMenu, nMenuFlags, pt.x, pt.y, 0, hMenuWnd, 0 );

	::DestroyMenu( hMenu );
	::DestroyWindow( hMenuWnd );

	switch (nCommand)
	{
	case ID_MENU_ENABLE_RESEARCH:
		DebugMessage( "Toggle 'Enable' setting ({})", m_WebBrowser2 );
		m_IsEnabled = !m_IsEnabled;
		if (m_IsEnabled) {
			m_LookupLoader.LoadAsync( );
		}
		config::SaveIsEnabled( m_IsEnabled );
		break;
	case ID_MENU_ABOUT:
		DebugMessage( "Opening about box ({})", m_WebBrowser2 );
		DialogBox( _Module.m_hInst, MAKEINTRESOURCE( IDD_ABOUT_DIALOG ), NULL, (DLGPROC)&AboutBoxProc );
	default:
		DebugMessage( "Unknown command ({}) {}", m_WebBrowser2, nCommand );
		break;
	}
}

ATOM CReSearchBHO::GetToolbarMenuClass( )
{
	static ATOM AtomPaneClass = NULL;
	if (AtomPaneClass == NULL) {

		WNDCLASSEXW wcex;

		wcex.cbSize = sizeof( wcex );
		wcex.style = 0;
		wcex.lpfnWndProc = DefWindowProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = _Module.m_hInst;
		wcex.hIcon = NULL;
		wcex.hCursor = NULL;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = PANE_CLASS_NAME;
		wcex.hIconSm = NULL;

		DebugMessage( "Creating pane class" );
		AtomPaneClass = ::RegisterClassExW( &wcex );
		if (AtomPaneClass == NULL) {
			DebugMessage( "Failed to create pane class" );
		}
	}

	return AtomPaneClass;
}

void STDMETHODCALLTYPE CReSearchBHO::OnBeforeNavigate2( IDispatch* pDisp,
														VARIANT* URL,
														VARIANT* Flags,
														VARIANT* TargetFrameName,
														VARIANT* PostData,
														VARIANT* Headers,
														VARIANT_BOOL* Cancel )
{
	try {
		// Verify some arguments
		if (m_WebBrowser2 == nullptr || pDisp == nullptr || CComQIPtr<IWebBrowser2>( pDisp ) == nullptr || URL == nullptr || URL->vt != VT_BSTR || Cancel == nullptr)
		{
			DebugMessage( "Invalid arguments ({})", m_WebBrowser2 );
			return;
		}
		DebugMessage( "OnBeforeNavigate2({}, {}, {}, {})", (const void*)pDisp, *URL, *Flags, *TargetFrameName );
		if (!m_WebBrowser2.IsEqualObject( pDisp )) {
			DebugMessage( "Browser not equal?! {} != {}", m_WebBrowser2, (const void*)pDisp );
		}

		m_IsEnabled = config::IsEnabled( );
		if (!m_IsEnabled)
			return;
		if (IsBlankUrl( URL ))
			return;
		// Do we even have any lookup data?
		if (m_Lookup == nullptr) {
			m_Lookup = m_LookupLoader.GetLookup( );
			if (m_Lookup == nullptr) {
				// nope
				DebugMessage( "No ReSearch lookup data available ({})", m_WebBrowser2 );
				return;
			}
		}
		if (wcsncmp( URL->bstrVal, m_LastUrl.c_str( ), ::SysStringLen( URL->bstrVal ) ) == 0) {
			DebugMessage( "Visiting the same url ({}) {}", m_WebBrowser2, m_LastUrl );
			return;
		}
		ToWstring( m_LastUrl, URL->bstrVal );
		const wchar_t* alternate_search_term = nullptr;
		auto engine = FindEngine( m_LastUrl );
		if (engine != nullptr) {
			DebugMessage( "Matched engine ({}) {}", m_WebBrowser2, engine->m_Name );
			alternate_search_term = m_Lookup->FindAlternateTerm( engine, m_LastUrl );
		} else {
			DebugMessage( "Failed to find matching engine from url ({})", m_WebBrowser2 );
		}
		const bool marked = IsMarked( m_WebBrowser2 );
		const auto browser_mode = GetBrowserMode( m_WebBrowser2 );
		DebugMessage( "Browser: {} Mode: {} Marked: {}", m_WebBrowser2, (int)browser_mode, marked );

		// Detect if we are primary or alternative browser window
		if (browser_mode == BrowserMode::Attached_Alternate) {
			if (!marked) {
				// Check the header if this is a search generated by the original browser window?
				auto main = GetMainBrowser( m_WebBrowser2 );
				// If we have an alternate term for the search, forward the sarch to the original browser window instead
				if (alternate_search_term == nullptr) {
					DebugMessage( "Detaching main browser ({})", m_WebBrowser2 );
					// Nope, the user probably printed something else,
					if (main != nullptr) {
						RemoveProperties( main );
					}
					// so release this browser from its parents grip
					RemoveProperties( m_WebBrowser2 );
					assert( GetBrowserMode( m_WebBrowser2 ) == BrowserMode::Detached );
				} else {
					DebugMessage( "Forwarding search to main browser ({}) {}", m_WebBrowser2, (const void*)main );
					if (main != nullptr) {
						main->Navigate2( URL, Flags, TargetFrameName, PostData, Headers );
						*Cancel = VARIANT_TRUE;
					}
				}
			}
		} else {
			if (alternate_search_term == nullptr) {
				if (browser_mode != BrowserMode::Detached) {
					// If there is no alternate term, kill any alternate window if opened
					auto alternate = GetAlternateBrowser( m_WebBrowser2 );
					if (alternate) {
						DebugMessage( "Closing down alternate window ({}) {}", m_WebBrowser2, alternate );
						RemoveProperties( alternate );
						alternate->put_Visible( VARIANT_FALSE );
						alternate->Stop( );
						alternate->ExecWB( OLECMDID_CLOSE, OLECMDEXECOPT_DONTPROMPTUSER, 0, 0 );
						alternate.Release( );
						alternate = nullptr;
					}
					// Restore size of window
					PutDimensions( GetStoredDimensions( m_WebBrowser2 ), m_WebBrowser2 );
					RemoveProperties( m_WebBrowser2 );
					// Remove local properties
					assert( GetBrowserMode( m_WebBrowser2 ) == BrowserMode::Detached );
				}
			} else {
				if (m_LastSearch != alternate_search_term) {
					WebBrowser2Ptr alternate_browser;
					// Create a new browser window for alternate search if we do not already have one ready
					if (browser_mode == BrowserMode::Attached_Main) {
						alternate_browser = GetAlternateBrowser( m_WebBrowser2 );
					} else {
						assert( browser_mode == BrowserMode::Detached );
						DebugMessage( "Created new alternate window ({}) {}", m_WebBrowser2, alternate_browser );
						alternate_browser.CoCreateInstance( __uuidof(InternetExplorer) );
						if (alternate_browser != nullptr) {
							SetProperties( m_WebBrowser2, alternate_browser );
							assert( GetBrowserMode( m_WebBrowser2 ) == BrowserMode::Attached_Main );
						} else {
							DebugMessage( "Failed to create alternate browser window ({})", m_WebBrowser2 );
						}
					}
					if (alternate_browser != nullptr) {
						// Navigate the alternate browser window to the alternate url
						_variant_t flags = navUntrustedForDownload;
						_variant_t target = L"_top";
						DebugMessage( "Navigating alternate window ({}) {}", m_WebBrowser2, alternate_browser );
						MarkSearch( alternate_browser );
						assert( engine != nullptr );
						alternate_browser->Navigate( _bstr_t( (engine->m_Url + alternate_search_term).c_str( ) ), &flags, &target, nullptr, nullptr );
						MaximizeWindows( m_WebBrowser2, alternate_browser );
						alternate_browser->put_Visible( VARIANT_TRUE );
					} else {
						DebugMessage( "No alternate browser window ({})", m_WebBrowser2 );
					}
				}
			}
		}
		m_LastSearch = alternate_search_term;
	}
	catch (const std::exception& e)
	{
		DebugMessage( "Exception during OnBeforeNavigate2 '{}' ({})", m_WebBrowser2, e.what( ) );
	}
}

const SearchEngine* CReSearchBHO::FindEngine( const std::wstring& url ) const
{
	if (url.empty( ))
		return nullptr;
	return m_Lookup->MatchEngineFromUrl( url );
}

void STDMETHODCALLTYPE CReSearchBHO::OnDocumentComplete( IDispatch *pDisp, VARIANT *pvarURL )
{
	DebugMessage( "OnDocumentComplete ({}, {})", (const void*)pDisp, *pvarURL );
	if (!m_IsEnabled || m_WebBrowser2 == nullptr)
		return;
	if (pDisp == nullptr || pvarURL == nullptr || pvarURL->vt != VT_BSTR || CComQIPtr<IWebBrowser2>( pDisp ) == nullptr) {
		DebugMessage( "Invalid arguments ({}, {})", (const void*)pDisp, *pvarURL );
		return;
	}
	if (IsBlankUrl( pvarURL ))
			return;
	if (m_Lookup == nullptr) {
		DebugMessage( "No lookup ({})", m_WebBrowser2 );
		return;
	}
	auto engine = FindEngine( pvarURL->bstrVal );
	if (engine == nullptr) {
		DebugMessage( "Found no matching engine ({})", m_WebBrowser2 );
		return;
	}
	CComPtr<IDispatch> dispatch;
	auto hr = m_WebBrowser2->get_Document( &dispatch );
	if (FAILED( hr )) {
		DebugMessage( "Failed to retrieve document dispatch from browser ({}) {:x}", m_WebBrowser2, hr );
		return;
	}
	CComPtr<IHTMLDocument2> document;
	hr = dispatch->QueryInterface( &document );
	if (FAILED( hr ) || document == nullptr)
	{
		DebugMessage( "Failed to cast dispatch to document ({}) {:x}", m_WebBrowser2, hr );
		return;
	}
	CComPtr<IHTMLWindow2> win;
	hr = document->get_parentWindow( &win );
	if (FAILED( hr ) || win == nullptr) {
		DebugMessage( "Failed to retrieve parent window ({})", m_WebBrowser2 );
		return;
	}
	// http://stackoverflow.com/questions/18342200/how-do-i-call-eval-in-ie-from-c
	CComDispatchDriver dispWindow;
	hr = win->QueryInterface( &dispWindow );
	if (FAILED( hr ) || dispWindow == nullptr) {
		DebugMessage( "Failed to retrieve dispatch driver ({})", m_WebBrowser2 );
		return;
	}
	CComPtr<IDispatchEx> dispexWindow;
	hr = win->QueryInterface( &dispexWindow );
	if (FAILED( hr ) || dispexWindow == nullptr)
	{
		DebugMessage( "Failed to retrieve dispex window ({})", m_WebBrowser2 );
		return;
	}
	DISPID dispidEval = -1;
	hr = dispexWindow->GetDispID( CComBSTR( "eval" ), fdexNameCaseSensitive, &dispidEval );
	if (FAILED( hr )) {
		DebugMessage( "Failed retrieve dispid for eval ({})", m_WebBrowser2 );
		return;
	}
	// Inject the js code
	{
		CComVariant source_var( config::GetInjectToolbarJS( ) );
		hr = dispWindow.Invoke1( dispidEval, &source_var );
		if (FAILED( hr )) {
			DebugMessage( "Failed to inject js ({})", m_WebBrowser2 );
			return;
		}
	}
	// Execute the function in js
	{
		DISPID dispidAddResearchToolbar = -1;
		hr = dispexWindow->GetDispID( CComBSTR( "addResearchToolbar" ), fdexNameCaseSensitive, &dispidAddResearchToolbar );
		if (FAILED( hr )) {
			DebugMessage( "Failed retrieve dispid for addResearchToolbar ({})", m_WebBrowser2 );
			return;
		}
		CComVariant json_data( m_Lookup->GetJson( ).c_str( ) );
		CComVariant css_data( config::GetToolbarCSS( ) );
		hr = dispWindow.Invoke2( dispidAddResearchToolbar, &json_data, &css_data );
		if (FAILED( hr )) {
			DebugMessage( "Failed to inject toolbar ({})", m_WebBrowser2 );
			return;
		}
	}
	DebugMessage( "Successfully evaluated javascript ({})", m_WebBrowser2 );
}

void STDMETHODCALLTYPE CReSearchBHO::OnQuit( )
{
	DebugMessage( "Browser OnQuit ({})", m_WebBrowser2 );
	Detach( );
	Unadvise( );
	m_WebBrowser2 = nullptr;
}

void CReSearchBHO::Detach( )
{
	try {
		if (m_WebBrowser2) {
			const auto browser_mode = GetBrowserMode( m_WebBrowser2 );
			DebugMessage( "Detaching browser ({}) Mode: {}", m_WebBrowser2, (int)browser_mode );
			auto other = GetOtherBrowser( m_WebBrowser2 );
			if (other) {
				if (browser_mode != BrowserMode::Detached) {
					DebugMessage( "Trying to restore dims of other ({})", m_WebBrowser2 );
					PutDimensions( GetStoredDimensions( other ), other );
				}
				RemoveProperties( other );
			}
		}
	}
	catch (const std::exception& e)
	{
		DebugMessage( "Exception during Detach ({}) '{}'", m_WebBrowser2, e.what( ) );
	}
}

void CReSearchBHO::Unadvise( )
{
	if (!m_WebBrowser2)
	{
		return;
	}
	ScopedLock lock( m_Mutex );

	if (m_IsAdvised)
	{
		auto hr = DispEventUnadvise( m_WebBrowser2 );
		hr;
		assert( SUCCEEDED( hr ) );
		m_IsAdvised = false;
	}
}

void CReSearchBHO::SetProperties( IWebBrowser2* main, IWebBrowser2* alternate ) const
{
	DebugMessage( "Setting propertes {} {}", (const void*)main, (const void*)alternate );
	assert( main != nullptr && alternate != nullptr );
	// Retrieve current size and position of main window
	const auto dims = GetDimensions( main );
	// Store in main window
	StoreDimensions( dims, main );
	StoreDimensions( dims, alternate );
	// Store reference to alternate window in mainc
	PutProperty( main, L"RS_Alternate", alternate );
	// Store reference to main window in alternate
	PutProperty( alternate, L"RS_Main", main );
	PutProperty( main, L"RS_IsAlternate", false );
	PutProperty( alternate, L"RS_IsAlternate", true );
}

void CReSearchBHO::RemoveProperties( IWebBrowser2* main )
{
	DebugMessage( "Removing propertes ({})", (const void*)main );
	RemoveProperty( main, L"RS_Alternate" );
	RemoveProperty( main, L"RS_Main" );
	RemoveProperty( main, L"RS_Fullscreen" );
	RemoveProperty( main, L"RS_Width" );
	RemoveProperty( main, L"RS_Height" );
	RemoveProperty( main, L"RS_Left" );
	RemoveProperty( main, L"RS_Top" );
	RemoveProperty( main, L"RS_IsAlternate" );
}

BrowserMode CReSearchBHO::GetBrowserMode( IWebBrowser2* browser )
{
	if (browser != nullptr) {
		_variant_t variant;
		if (SUCCEEDED( browser->GetProperty( _bstr_t( L"RS_IsAlternate" ), &variant ) ) && variant.vt == VT_BOOL)
		{
			return (variant.boolVal) ? BrowserMode::Attached_Alternate : BrowserMode::Attached_Main;
		}
	}
	return BrowserMode::Detached;
}

WebBrowser2Ptr CReSearchBHO::GetOtherBrowser( IWebBrowser2* browser )
{
	switch (GetBrowserMode( browser ))
	{
	default:
	case BrowserMode::Detached:
		return nullptr;
	case BrowserMode::Attached_Main:
		return GetAlternateBrowser( browser );
	case BrowserMode::Attached_Alternate:
		return GetMainBrowser( browser );
	}
}

WebBrowser2Ptr CReSearchBHO::GetMainBrowser( IWebBrowser2* browser )
{
	assert( GetBrowserMode( browser ) == BrowserMode::Attached_Alternate );
	if (browser != nullptr) {
		_variant_t variant;
		if (SUCCEEDED( browser->GetProperty( _bstr_t( L"RS_Main" ), &variant ) ) && variant.vt == VT_DISPATCH)
		{
			if (variant.pdispVal)
				return CComQIPtr<IWebBrowser2>( variant.pdispVal );
		}
	}
	return nullptr;
}

WebBrowser2Ptr CReSearchBHO::GetAlternateBrowser( IWebBrowser2* browser )
{
	assert( GetBrowserMode( browser ) == BrowserMode::Attached_Main );
	if (browser != nullptr) {
		_variant_t variant;
		if (SUCCEEDED( browser->GetProperty( _bstr_t( L"RS_Alternate" ), &variant ) ) && variant.vt == VT_DISPATCH)
		{
			if (variant.pdispVal)
				return CComQIPtr<IWebBrowser2>( variant.pdispVal );
		}
	}
	return nullptr;

}