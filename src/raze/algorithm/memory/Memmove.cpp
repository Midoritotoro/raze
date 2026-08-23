#include <src/raze/algorithm/memory/Memmove.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_unmangled raze_declare_const_function void* raze_cdecl raze_memmove(
	void* dst, const void* src, sizetype bytes) noexcept
{	
	return memmove(dst, src, bytes);
}

__RAZE_ALGORITHM_NAMESPACE_END
