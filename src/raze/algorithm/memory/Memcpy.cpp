#include <src/raze/algorithm/memory/Memcpy.h>
#include <raze/arch/ProcessorFeatures.h>
#include <src/raze/algorithm/AdvanceBytes.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_declare_const_function void* raze_stdcall raze_memcpy(
	void* raze_restrict dst, const void* raze_restrict src, sizetype bytes) noexcept
{
	std::memcpy(dst, src, bytes);
	return bytes_pointer_offset(dst, bytes);
}

__RAZE_ALGORITHM_NAMESPACE_END
