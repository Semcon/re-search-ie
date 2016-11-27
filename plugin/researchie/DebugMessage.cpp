// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#include "stdafx.h"
#include "DebugMessage.h"
#if defined(_DEBUG) || defined(FORCE_DEBUG_MESSAGE)
#include <Windows.h>

std::wostream& operator<<( std::wostream& os, const VARIANT& variant )
{
	switch (variant.vt)
	{
	case VT_BSTR:
		os << L"(bstr)";
		if (variant.bstrVal)
			os << variant.bstrVal;
		else
			os << L"null";
		return os;
	case VT_BOOL:
		return (os << L"(bool)" << (variant.boolVal == VARIANT_TRUE ? L"true" : L"false"));
	case VT_I4:
		return (os << L"(i4)" << variant.intVal);
	case VT_EMPTY:
	case VT_NULL:
		return (os << L"(null)");
	case VT_DISPATCH:
		os << L"(dispatch)";
		os << variant.pdispVal;
		return os;
	default:
		return (os << L"(other)" << variant.vt);
	}
}

void detail::DebugMessageExImpl( const wchar_t* msg )
{
	if (msg)
		::OutputDebugStringW( msg );
}
#endif
