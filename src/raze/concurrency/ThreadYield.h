#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <raze/arch/ProcessorDetection.h>


__RAZE_CONCURRENCY_NAMESPACE_BEGIN

raze_always_inline void __yield() noexcept {
#if __has_builtin(__yield)
        __yield();

#elif defined(_YIELD_PROCESSOR) && defined(raze_cpp_msvc)
        _YIELD_PROCESSOR();

#elif __has_builtin(__builtin_ia32_pause)
        __builtin_ia32_pause();

#elif defined(raze_processor_x86) && defined(raze_cpp_gnu)
        __builtin_ia32_pause();

#elif defined(raze_processor_x86) && defined(raze_cpp_msvc)
        _mm_pause();

#elif defined(raze_processor_x86)
        asm("pause");

#elif __has_builtin(__builtin_arm_yield)
        __builtin_arm_yield();

#elif defined(raze_processor_arm) && raze_processor_arm >= 7 && defined(raze_cpp_gnu)
        asm("yield");        
#endif
}

__RAZE_CONCURRENCY_NAMESPACE_END
