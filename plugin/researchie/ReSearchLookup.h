// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#pragma once

#include <string>
#include <vector>
#include <map>
#include <regex>
#include <cpprest/json.h>

struct SearchEngine
{
	using TermsMap = std::map<std::wstring, std::wstring>;
	using TermsMapPtr = std::shared_ptr<TermsMap>;
	std::wstring m_Name;
	std::wstring m_Url;
	std::wstring m_TermName;
	std::vector<std::wstring> m_Matches;
	std::shared_ptr<TermsMap> m_Terms;
};

class ReSearchLookup
{
public:
	explicit ReSearchLookup( const web::json::value& input );
	~ReSearchLookup( ) = default;

	bool IsEmpty( ) const;
	/// Finds the first engine with that matches an url
	const SearchEngine* MatchEngineFromUrl( const std::wstring& url ) const;
	/// Find an alternate term from an url using an engine
	const wchar_t* FindAlternateTerm( const SearchEngine* engine, const std::wstring& in_url ) const;
	/// Retrieve original json data
	const std::string& GetJson( ) const;
private:
private:
	std::vector<SearchEngine> m_Engines;
	std::wregex m_SearchMatcher;
	std::string m_Json;
};

