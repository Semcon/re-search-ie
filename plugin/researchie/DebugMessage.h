// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#pragma once
#if defined(_DEBUG) || defined(FORCE_DEBUG_MESSAGE)
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <string>
#include <ostream>

std::wostream& operator<<( std::wostream& os, const VARIANT& variant );
template<typename T>
std::wostream& operator<<( std::wostream& os, const ATL::CComPtr<T>& com_type )
{
	if (com_type == nullptr)
		return (os << L"nullptr");
	return (os << (const void*)com_type);
}

namespace detail {
void DebugMessageExImpl( const wchar_t* msg );
template <typename... Args>
void DebugMessageEx( const wchar_t *file, int line,
					 const wchar_t *format,
					 const Args& ... args )
{
	fmt::WMemoryWriter writer;
	writer << L"Re-search: " << fmt::format( format, args... ) << " " << fmt::format( L"{}({})\n", file, line );
	DebugMessageExImpl( writer.str( ).c_str( ) );
}
} // namespace detail

#define DEBUG_WIDEN2(x) L ## x
#define DEBUG_WIDEN(x) DEBUG_WIDEN2(x)

#define DebugMessage(msg, ...) detail::DebugMessageEx(DEBUG_WIDEN(__FUNCTION__), __LINE__, DEBUG_WIDEN(msg), __VA_ARGS__)

#else
#define DebugMessage(msg, ...) (void)0
#endif
