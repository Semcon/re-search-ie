// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#pragma once

namespace config {

void Initialize( );

const wchar_t* GetJsonURL( );

const char* GetToolbarCSS( );
const char* GetInjectToolbarJS( );

bool IsEnabled( );
void SaveIsEnabled( bool enable );

} // namespace config