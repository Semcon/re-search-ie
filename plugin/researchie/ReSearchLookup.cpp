// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#include "stdafx.h"
#include "ReSearchLookup.h"
#include "ReSearchParser.h"
#include "StringUtils.h"

ReSearchLookup::ReSearchLookup( const web::json::value& input )
	: m_Engines( ParseLookups( input ) )
	, m_SearchMatcher( LR"rgx([(\?|\&)]q\=([^&#]+))rgx" )
	, m_Json( FromWide( input.serialize( ) ) )
{
}

bool ReSearchLookup::IsEmpty( ) const
{
	return !m_Engines.empty( );
}

const wchar_t* ReSearchLookup::FindAlternateTerm( const SearchEngine* engine, const std::wstring& in_url ) const
{
	if (engine == nullptr)
		return nullptr;
	std::wsmatch matches;
	// Find search term
	if (!std::regex_search( in_url, matches, m_SearchMatcher ))
		return nullptr;
	std::wstring search_term = matches[1];
	{
		std::wstring str = matches.suffix( );
		while (std::regex_search( str, matches, m_SearchMatcher )) {
			search_term = matches[1];
			str = matches.suffix( );
		}
	}
	// Cleanup
	search_term = ToLower( UnescapeUrl( std::move( search_term ) ) );
	auto it = engine->m_Terms->find( search_term );
	if (it == engine->m_Terms->end( ))
		return nullptr;
	return it->second.c_str( );
}

const SearchEngine* ReSearchLookup::MatchEngineFromUrl( const std::wstring& url ) const
{
	for (auto& engine : m_Engines)
	{
		bool matched = true;
		for (auto& match : engine.m_Matches)
		{
			if (url.find( match ) == std::wstring::npos) {
				matched = false;
				break;
			}
		}
		if (matched)
			return &engine;
	}
	return nullptr;
}

const std::string& ReSearchLookup::GetJson( ) const
{
	return m_Json;
}