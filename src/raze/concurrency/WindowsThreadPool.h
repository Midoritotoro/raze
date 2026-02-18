#pragma once

#include <raze/Types.h>

#include <raze/compatibility/Compatibility.h>
#include <src/raze/utility/Assert.h>

#if defined(raze_os_windows)
#  include <threadpoolapiset.h>
#  include <stdnoreturn.h>

__RAZE_CONCURRENCY_NAMESPACE_BEGIN

using __thread_pool_work = TP_WORK;

class __windows_thread_pool {
public:
	template <class _Task_> 
	static __thread_pool_work* __create_work(_Task_& __task) noexcept {
		return CreateThreadpoolWork(&_Task_::thread_pool_callback, reinterpret_cast<void*>(&__task), nullptr);
	}

	static void __close_work(__thread_pool_work* __work) noexcept {
		CloseThreadpoolWork(__work);
	}

	static void __submit(__thread_pool_work* __work) noexcept {
		SubmitThreadpoolWork(__work);
	}

	static void __wait_for(__thread_pool_work* __work) noexcept {
		WaitForThreadpoolWorkCallbacks(__work, true);
	}
};

__RAZE_CONCURRENCY_NAMESPACE_END

#endif // defined(raze_os_windows)
