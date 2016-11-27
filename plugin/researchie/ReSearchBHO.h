// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#pragma once
#include "resource.h"
#include "ReSearchLookupLoader.h"
#include "generated/ReSearchIE_i.h"
#include <shlguid.h>     // IID_IWebBrowser2, DIID_DWebBrowserEvents2, etc.
#include <ExDisp.h> // DIID_DWebBrowserEvents2
#include <ExDispid.h>
#include <DocObj.h>
#include <mutex>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

enum class BrowserMode
{
	Detached,
	Attached_Main,
	Attached_Alternate,
};

using WebBrowser2Ptr = ATL::CComPtr<IWebBrowser2>;
using OleTargetPtr = ATL::CComQIPtr<IOleCommandTarget, &IID_IOleCommandTarget>;

// CReSearchBHO
class ATL_NO_VTABLE CReSearchBHO :
	public ATL::CComObjectRootEx<ATL::CComSingleThreadModel>,
	public ATL::CComCoClass<CReSearchBHO, &CLSID_ReSearchBHO>,
	public ATL::IObjectWithSiteImpl<CReSearchBHO>,
	public IOleCommandTarget,
	public ATL::IDispEventImpl<1, CReSearchBHO, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 1>
{
public:
	CReSearchBHO( );
	~CReSearchBHO( ) override;

	DECLARE_REGISTRY_RESOURCEID( IDR_RESEARCHBHO )
	DECLARE_NOT_AGGREGATABLE( CReSearchBHO )
	BEGIN_COM_MAP( CReSearchBHO )
		COM_INTERFACE_ENTRY( IObjectWithSite )
		COM_INTERFACE_ENTRY( IOleCommandTarget )
	END_COM_MAP( )
	BEGIN_SINK_MAP( CReSearchBHO )
		SINK_ENTRY_EX( 1, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, OnBeforeNavigate2 )
		SINK_ENTRY_EX( 1, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete )
		SINK_ENTRY_EX( 1, DIID_DWebBrowserEvents2, DISPID_ONQUIT, OnQuit )
	END_SINK_MAP( )
	DECLARE_PROTECT_FINAL_CONSTRUCT( )

	STDMETHOD( SetSite )(IUnknown* unknown_site) override;
	STDMETHOD( QueryStatus )(const GUID* pguidCmdGroup, ULONG cCmds,
							  OLECMD prgCmds[], OLECMDTEXT* pCmdText) override;
	STDMETHOD( Exec )(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
					   VARIANTARG* pvaIn, VARIANTARG* pvaOut) override;

	void STDMETHODCALLTYPE OnBeforeNavigate2( IDispatch* pDisp,
											  VARIANT* URL,
											  VARIANT* Flags,
											  VARIANT* TargetFrameName,
											  VARIANT* PostData,
											  VARIANT* Headers,
											  VARIANT_BOOL* Cancel );

	void STDMETHODCALLTYPE OnDocumentComplete( IDispatch *pDisp, VARIANT *pvarURL );
	void STDMETHODCALLTYPE OnQuit( );
private:
	void Unadvise( );
	HMENU CreateToolbarMenu( ) const;
	void DisplayToolbarMenu( HMENU hMenu, int nToolbarCmdID, POINT pt, UINT nMenuFlags );

	void SetProperties( IWebBrowser2* main, IWebBrowser2* alternate ) const;
	
	static void RemoveProperties( IWebBrowser2* main );
	static BrowserMode GetBrowserMode( IWebBrowser2* browser );
	static WebBrowser2Ptr GetOtherBrowser( IWebBrowser2* browser );
	static WebBrowser2Ptr GetMainBrowser( IWebBrowser2* browser );
	static WebBrowser2Ptr GetAlternateBrowser( IWebBrowser2* browser );
	void Detach( );

	const SearchEngine* FindEngine(const std::wstring& url) const;

	static ATOM GetToolbarMenuClass( );
private:
	ReSearchLookupLoader m_LookupLoader;
	std::shared_ptr<ReSearchLookup> m_Lookup;
	WebBrowser2Ptr m_WebBrowser2;
	std::mutex m_Mutex;
	bool m_IsAdvised{ false };
	bool m_IsEnabled{ true };
	
	// Cached values
	mutable std::wstring m_LastUrl;
	mutable const wchar_t* m_LastSearch{ nullptr };
};

OBJECT_ENTRY_AUTO( __uuidof(ReSearchBHO), CReSearchBHO )
