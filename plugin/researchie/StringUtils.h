// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#pragma once

#include <string>

/// Convert a mixed case string to lowercase
std::wstring ToLower( std::wstring str );
/// Unescape url characters
std::wstring UnescapeUrl( std::wstring str );
/// Convert wstring to string
std::string FromWide( const std::wstring& str );
/// Convert string wstring
std::wstring ToWide( const std::string& str );