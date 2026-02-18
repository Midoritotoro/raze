#pragma once 

#include <raze/arch/ProcessorInformation.h>

#if defined(raze_os_windows)
#  include <src/raze/concurrency/WindowsThreadPool.h>
#endif // defined(raze_os_windows)


__RAZE_CONCURRENCY_NAMESPACE_BEGIN

class _ThreadPoolExecutor {
#if defined(raze_os_windows)
	using implementation = WindowsThreadPool;
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
	_ThreadPoolWork* _work = nullptr;
};

template <class _Task_>
_ThreadPoolExecutor::_ThreadPoolExecutor(_Task_& task) noexcept {
	createTask(task);
}

template <class _Task_>
void _ThreadPoolExecutor::createTask(_Task_& task) noexcept {
	_work = implementation::createWork(task);
}

void _ThreadPoolExecutor::submit() noexcept {
	implementation::submit(_work);
}

void _ThreadPoolExecutor::submitForChunks(uint32 submissions) noexcept {
	for (uint32 current = 0; current < submissions; ++current)
		implementation::submit(_work);
}

_ThreadPoolExecutor::~_ThreadPoolExecutor() noexcept {
	implementation::closeWork(_work);
}

void _ThreadPoolExecutor::join() noexcept {
	implementation::waitFor(_work);
}

__RAZE_CONCURRENCY_NAMESPACE_END
