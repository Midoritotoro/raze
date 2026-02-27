#pragma once 

#include <raze/arch/ProcessorInformation.h>

#if defined(raze_os_windows)
#  include <src/raze/concurrency/WindowsThreadPool.h>
#endif // defined(raze_os_windows)


__RAZE_CONCURRENCY_NAMESPACE_BEGIN

class _ThreadPoolExecutor {
#if defined(raze_os_windows)
	using implementation = __windows_thread_pool;
#endif // defined(raze_os_windows)
public:
	~_ThreadPoolExecutor() noexcept;

	template <class _Task_>
	_ThreadPoolExecutor(_Task_& task) noexcept;

	void submit() noexcept;
	void submitForChunks(uint32 submissions) noexcept;

	template <class _Task_> 
	void createTask(_Task_& task) noexcept;

	void join() noexcept;
private:
	__thread_pool_work* _work = nullptr;
};

template <class _Task_>
_ThreadPoolExecutor::_ThreadPoolExecutor(_Task_& task) noexcept {
	createTask(task);
}

template <class _Task_>
void _ThreadPoolExecutor::createTask(_Task_& __task) noexcept {
	_work = implementation::__create_work(__task);
}

void _ThreadPoolExecutor::submit() noexcept {
	implementation::__submit(_work);
}

void _ThreadPoolExecutor::submitForChunks(uint32 __submissions) noexcept {
	for (auto __current = uint32(0); __current < __submissions; ++__current)
		implementation::__submit(_work);
}

_ThreadPoolExecutor::~_ThreadPoolExecutor() noexcept {
	implementation::__close_work(_work);
}

void _ThreadPoolExecutor::join() noexcept {
	implementation::__wait_for(_work);
}

__RAZE_CONCURRENCY_NAMESPACE_END
