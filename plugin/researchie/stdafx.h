// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#pragma once

#ifndef STRICT
#define STRICT
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

#include <shlguid.h>

#include <string>
#include <memory>
#include <map>
#include <vector>

#include <cpprest/json.h>
#include <cpprest/http_client.h>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "DebugMessage.h"