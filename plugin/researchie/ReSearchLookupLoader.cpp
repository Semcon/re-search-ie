// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#include "stdafx.h"
#include "ReSearchLookupLoader.h"
#include "ReSearchConfig.h"
#include <cpprest/http_client.h>

ReSearchLookupLoader::StaticStuff ReSearchLookupLoader::s_Static;

void ReSearchLookupLoader::LoadAsync( )
{
	std::call_once( s_Static.m_OnceFlag, [&]
	{
		try {
			using namespace web;
			using namespace web::http;
			using namespace web::http::client;

			DebugMessage( "Loading ReSearch lookup" );

			http_client client( config::GetJsonURL( ) );
			s_Static.m_AsyncLoad =
				client.request( methods::GET ).then( []( http_response response )
			{
				DebugMessage( "Got lookup load response {} {}", response.status_code( ), response.reason_phrase() );

				if (response.status_code( ) == status_codes::OK)
				{
					return response.extract_json( true );
				}
				DebugMessage( "Failed to retrieve json data" );
				// Handle error cases, for now return empty json value... 
				return pplx::task_from_result( json::value( ) );
			} )
				.then( [&]( pplx::task<json::value> previousTask )
			{
				try
				{
					const json::value& v = previousTask.get( );
					DebugMessage( "Parsing lookup data" );
					auto lookup = std::make_shared<ReSearchLookup>( v );
					DebugMessage( "Storing in static storage" );
					s_Static.m_Lookup = lookup;
				}
				catch (const http_exception& e)
				{
					DebugMessage( "Failed to parse json data '{}'", e.what( ) );
				}
			} );
		}
		catch (...)
		{
			DebugMessage( "Exception during load async" );
		}
		s_Static.m_Initialized = true;
	} );
}

std::shared_ptr<ReSearchLookup> ReSearchLookupLoader::GetLookup( )
{
	try {
		if (s_Static.m_Lookup || s_Static.m_Initialized) {
			if (!s_Static.m_Lookup && !s_Static.m_AsyncLoad.is_done( ))
			{
				s_Static.m_AsyncLoad.wait( );
			}
			return s_Static.m_Lookup;
		}
	}
	catch (...)
	{
		DebugMessage( "Exception during async load wait" );
	}
	return nullptr;
}
