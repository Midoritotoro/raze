#pragma once

#include <raze/compatibility/CompilerDetection.h>

#include <vector>
#include <raze/Types.h>

#include <raze/arch/CpuFeature.h>

#if defined(raze_cpp_msvc) && !defined(_M_ARM64EC)
#  include <intrin.h>
#endif

#if defined(raze_processor_x86) && defined(__SSE2__)
#  include <emmintrin.h>
#endif // defined(raze_processor_x86) && defined(__SSE2__)

__RAZE_ARCH_NAMESPACE_BEGIN

using xmmint    = __m128i;
using ymmint    = __m256i;
using zmmint    = __m512i;

using xmmdouble = __m128d;
using ymmdouble = __m256d;
using zmmdouble = __m512d;

using xmmfloat  = __m128;
using ymmfloat  = __m256;
using zmmfloat  = __m512;

__RAZE_ARCH_NAMESPACE_END
