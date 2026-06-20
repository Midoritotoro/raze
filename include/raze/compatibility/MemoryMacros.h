#pragma once 

#include <raze/compatibility/CompilerDetection.h>
#include <raze/compatibility/Nodiscard.h>


#if !defined(raze_restrict)
#  define raze_restrict __restrict  
#endif // !defined(raze_restrict)

#if !defined(raze_sizeof_in_bits)
#  define raze_sizeof_in_bits(type) (sizeof(type) * 8)
#endif // !defined(raze_sizeof_in_bits)