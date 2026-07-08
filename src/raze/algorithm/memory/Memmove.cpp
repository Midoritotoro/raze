#include <src/raze/algorithm/memory/Memmove.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_unmangled raze_declare_const_function void* raze_cdecl __raze_memmove(
	void* __dst, const void* __src, sizetype __bytes) noexcept
{	
	return memmove(__dst, __src, __bytes);
}

__RAZE_ALGORITHM_NAMESPACE_END
