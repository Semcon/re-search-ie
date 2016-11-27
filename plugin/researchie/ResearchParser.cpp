// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#include "stdafx.h"
#include "ReSearchParser.h"
#include "StringUtils.h"
#include <algorithm>
#include <map>

std::vector<SearchEngine> ParseLookups( const web::json::value& input )
{
	std::vector<SearchEngine> engines;
	// Parse engines
	for (auto&& engine : input.at( L"engines" ).as_array( ))
	{
		try {
			SearchEngine e;
			e.m_Name = engine.at( L"name" ).as_string( );
			e.m_Url = engine.at( L"url" ).as_string( );
			e.m_TermName = engine.at( L"terms" ).as_string( );
			for (auto&& match : engine.at( L"match" ).as_array( ))
			{
				e.m_Matches.push_back( match.as_string( ) );
			}
			e.m_Matches.shrink_to_fit( );
			engines.push_back( std::move( e ) );
		}
		catch (const web::json::json_exception&)
		{
		}
	}
	engines.shrink_to_fit( );
	std::map<std::wstring, SearchEngine::TermsMapPtr> terms_collection;
	// Parse terms
	for (auto& engine : engines)
	{
		auto terms_it = terms_collection.find( engine.m_TermName );
		if (terms_it != terms_collection.end( )) {
			engine.m_Terms = terms_it->second;
		} else {
			auto terms = std::make_shared<SearchEngine::TermsMap>( );
			try {
				auto&& node = input.at( L"terms" ).at( engine.m_TermName ).as_object( );
				for (auto iter = node.cbegin( ); iter != node.cend( ); ++iter)
				{
					auto term = ToLower( iter->first );
					auto alternate = ToLower( iter->second.at( L"updated" ).as_string( ) );
					terms->emplace( std::move( term ), std::move( alternate ) );
				}
				engine.m_Terms = terms;
				terms_collection[engine.m_TermName] = terms;
			}
			catch (const web::json::json_exception& e)
			{
				DebugMessage( "Failed to parse terms {}", e.what( ) );
			}
		}
	}
	// Remove unusable engines
	engines.erase(
		std::remove_if( begin( engines ), end( engines ), []( auto& eng )
	{
		return eng.m_Terms == nullptr || eng.m_Terms->empty( ) || eng.m_Matches.empty( );
	} ), end( engines ) );
	return engines;
}