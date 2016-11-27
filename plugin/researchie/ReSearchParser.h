// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#pragma once

#include "ReSearchLookup.h"
#include <cpprest/json.h>

std::vector<SearchEngine> ParseLookups( const web::json::value& input );
