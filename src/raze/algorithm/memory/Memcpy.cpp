#include <src/raze/algorithm/memory/Memcpy.h>
#include <raze/arch/ProcessorFeatures.h>
#include <src/raze/algorithm/AdvanceBytes.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_unmangled raze_never_inline raze_declare_const_function void* raze_stdcall __raze_memcpy(void* raze_restrict __dst,
	const void* raze_restrict __src, sizetype __bytes) noexcept
{
	std::memcpy(__dst, __src, __bytes);
	return __bytes_pointer_offset(__dst, __bytes);
}

__RAZE_ALGORITHM_NAMESPACE_END
