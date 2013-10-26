#pragma once

#include "../config.h"
#include "document_thread_pool.h"
#include "query_thread_pool.h"

START_NAMESPACE

class ThreadPools {
  public:
	static QueryThreadPool queryThreadPool;
	static DocumentThreadPool documentThreadPool;
};

END_NAMESPACE
