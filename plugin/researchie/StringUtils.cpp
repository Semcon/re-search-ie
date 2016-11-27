// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#include "stdafx.h"
#include "StringUtils.h"
#include <codecvt>
#include <cwctype>
#include <Shlwapi.h>

using StringConverter = std::wstring_convert<std::codecvt_utf8<wchar_t>>;

std::wstring ToLower( std::wstring str )
{
	std::transform( begin( str ), end( str ), begin( str ), ::towlower );
	return str;
}
std::wstring UnescapeUrl( std::wstring str )
{
	if (str.empty( ))
		return str;
	DWORD length = static_cast<DWORD>(str.length( ));
	UrlUnescapeW( (wchar_t*)str.c_str( ), NULL, &length, URL_UNESCAPE_INPLACE );
	// If string is escaped it will become shorter, so calculate real length here
	const size_t new_len = wcslen( str.c_str( ) );
	if (new_len < str.size( )) {
		str.resize( new_len );
	}
	// Handle special characters used 
	std::replace( begin( str ), end( str ), L'+', L' ' );

	return str;
}

std::string FromWide( const std::wstring& str )
{
	StringConverter converter;
	return converter.to_bytes( str );
}

std::wstring ToWide( const std::string& str )
{
	StringConverter converter;
	return converter.from_bytes( str );
}