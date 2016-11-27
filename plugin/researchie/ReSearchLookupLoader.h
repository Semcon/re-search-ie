// Copyright (c) 2016, Semcon Sweden AB. All rights reserved.
#pragma once

#include "ReSearchLookup.h"
#include <memory>
#include <mutex>
#include <atomic>
#include <pplx/pplxtasks.h>

class ReSearchLookupLoader
{
public:
	void LoadAsync( );

	/// Function will block until lookup is ready
	std::shared_ptr<ReSearchLookup> GetLookup( );
private:
	struct StaticStuff
	{
		std::once_flag m_OnceFlag;
		pplx::task<void> m_AsyncLoad;
		std::atomic_bool m_Initialized{ false };
		std::shared_ptr<ReSearchLookup> m_Lookup;
	};
	static StaticStuff s_Static;
};