#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <chrono>
#include <src/raze/type_traits/Invoke.h>

#if defined(raze_os_win) 

#if !defined(_DLL)
#  include <process.h> // _beginthreadex && _endthreadex
#  include <stdnoreturn.h>
#else
   raze_disable_warning_msvc(6258)
#endif // !defined(_DLL)

__RAZE_CONCURRENCY_NAMESPACE_BEGIN

struct __thread_type {
    void* __handle = nullptr;
    dword_t __id = 0;
};

enum class __thread_result: uint8 {
    __error,
    __success
};

enum __thread_creation_flags : dword_t {
    __run_after_creation        = 0,
    __suspend_after_creation    = CREATE_SUSPENDED
};

raze_nodiscard dword_t __current_thread_id() noexcept {
	return GetCurrentThreadId();
}

raze_nodiscard dword_t __thread_id(void* __handle) noexcept {
	return GetThreadId(__handle);
}

raze_nodiscard void* __current_thread() noexcept {
	return GetCurrentThread();
}

__thread_result __wait_for_thread(void* __handle) noexcept {
    if (WaitForSingleObjectEx(__handle, INFINITE, FALSE) == WAIT_FAILED)
        return __thread_result::__error;

    return __thread_result::__success;
}

int __thread_priority(void* __handle) noexcept {
    return GetThreadPriority(__handle);
}

void __current_thread_sleep(dword_t __milliseconds) noexcept {
	Sleep(__milliseconds);
}

template <
    class           _Tuple_,
    sizetype ...    _Indices_>
uint32 raze_stdcall __thread_task_invoke(void* __raw) noexcept {
    const std::unique_ptr<_Tuple_> __args(static_cast<_Tuple_*>(__raw));

    _Tuple_& __tuple = *__args.get();
    type_traits::invoke(std::move(std::get<_Indices_>(__tuple))...);

    return 0;
}

template <
    class       _Tuple_,
    size_t...   _Indices_>
static constexpr auto __get_thread_task_invoker(std::index_sequence<_Indices_...>) noexcept {
    return &__thread_task_invoke<_Tuple_, _Indices_...>;
}

template <
    class       _Task_,
    class...    _Args_>
__thread_type raze_stdcall __create_thread(
    __thread_creation_flags     __creation,
    dword_t                     __stack_size,
    _Task_&&                    __task,
    _Args_&& ...                __args) noexcept
{
    __thread_type __result;
    using _Tuple = std::tuple<std::decay_t<_Task_>, std::decay_t<_Args_>...>;

    auto __decay_copied         = std::make_unique<_Tuple>(std::forward<_Task_>(__task),  std::forward<_Args_>(__args)...);
    constexpr auto __invoker    = __get_thread_task_invoker<_Tuple>(std::make_index_sequence<1 + sizeof...(_Args_)>{});

    auto __thread_id = dword_t(0);

#if defined(raze_cpp_msvc) && !defined(_DLL)
    // -MT || -MTd 

    __result.__handle = reinterpret_cast<HANDLE>(
        _beginthreadex(
            nullptr, __stack_size, __invoker, __decay_copied.get(), __creation,
            reinterpret_cast<uint32*>(&__thread_id)
        )
    );
#else
    // -MD || -MDd

    __result.__handle = CreateThread(
        nullptr, __stack_size, reinterpret_cast<LPTHREAD_START_ROUTINE>(__invoker),
        reinterpret_cast<LPVOID>(__decay_copied.get()),
        __creation, reinterpret_cast<LPDWORD>(&__thread_id));

#endif // defined(raze_cpp_msvc) && !defined(_DLL)

    if (raze_likely(__result.__handle != nullptr)) {
        __result.__id = __thread_id;
        raze_unused(__decay_copied.release());
    }
    else {
        __result.__id = 0;
    }

    return __result;
}

bool __resume_suspended_thread(void* __handle) noexcept {
    return ResumeThread(__handle) != -1;
}

dword_t __thread_exit_code(void* __handle) {
    dword_t __exit_code = 0;
    GetExitCodeThread(__handle, &__exit_code);

    return __exit_code;
}

void raze_stdcall __detach_thread(void* __handle) noexcept {
    CloseHandle(__handle);
}   

dword_t raze_stdcall __terminate_thread(void* __handle) noexcept {
    return TerminateThread(__handle, __thread_exit_code(__handle));
}

void raze_stdcall __set_thread_priority(
    void*   __handle,
    int     __priority) noexcept 
{
    if (!SetThreadPriority(__handle, __priority)) {
        raze_assert_log(false, "raze::concurrency::__set_thread_priority: Failed to set thread priority.");
    }
}

template <
    class _TickCountType_, 
    class _Period_>
raze_always_inline auto __to_absolute_time(const std::chrono::duration<_TickCountType_, _Period_>& __relative_time) noexcept {
    constexpr auto __zero = std::chrono::duration<_TickCountType_, _Period_>::zero();
    const auto __now      = std::chrono::steady_clock::now();

    decltype(__now + __relative_time) __absolute_time = __now; 

    if (__relative_time > __zero) {
        constexpr auto __forever = (decltype(__absolute_time)::max)();

        if (__absolute_time < __forever - __relative_time)
            __absolute_time += __relative_time;
        else
            __absolute_time = __forever;
    }

    return __absolute_time;
}

__RAZE_CONCURRENCY_NAMESPACE_END

#endif // defined(raze_os_win)