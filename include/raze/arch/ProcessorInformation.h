#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/utility/Assert.h>

#include <raze/math/BitMath.h>

__RAZE_ARCH_NAMESPACE_BEGIN

class ProcessorInformation {
public:
    raze_nodiscard raze_always_inline static uint32 hardware_concurrency() noexcept {
        return _processor_information_internal._logical_processors;
    }
private:
    class ProcessorInformationInternal
    {
    public:
        uint32 _logical_processors = 0;

        ProcessorInformationInternal() noexcept {
#if defined(raze_os_win)
#  if defined(raze_os_win64)
            constexpr auto __length = 48;
#  else 
            constexpr auto __length = 44;
#  endif

            uint8 __buffer[__length];
            uint8* __information_buffer = reinterpret_cast<uint8*>(&__buffer);
            
            dword_t __buffer_length = __length;

            raze_assert(GetLogicalProcessorInformationEx(RelationProcessorPackage, 
                reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(__information_buffer), &__buffer_length));

            while (__buffer_length > 0) {
                const auto __information  = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(__information_buffer);
                const auto __information_size = __information->Size;

                for (int16 __current = 0; __current != __information->Processor.GroupCount; ++__current)
                    _logical_processors += math::population_count(__information->Processor.GroupMask[__current].Mask);

                __information_buffer += __information_size;
                __buffer_length -= __information_size;
            }
        }
#endif // defined(raze_os_win)
     
    };

    static inline ProcessorInformationInternal _processor_information_internal;
};

__RAZE_ARCH_NAMESPACE_END
