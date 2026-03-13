#pragma once 

#include <raze/compatibility/Compatibility.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN


__raze_msvc_intrin_workaround void __vmaskmovps(
	float* __a, __m256i __m, __m256 __v) noexcept
{
	_mm256_maskstore_ps(__a, __m, __v);
}

__raze_msvc_intrin_workaround void __vmaskmovpd(
	double* __a, __m256i __m, __m256d __v) noexcept
{
	_mm256_maskstore_pd(__a, __m, __v);
}

__raze_msvc_intrin_workaround void __vpmaskmovd(
	int* __a, __m256i __m, __m256i __v) noexcept
{
	_mm256_maskstore_epi32(__a, __m, __v);
}

__raze_msvc_intrin_workaround void __vpmaskmovq(
	int64* __a, __m256i __m, __m256i __v) noexcept
{
	_mm256_maskstore_epi64(__a, __m, __v);
}

__raze_msvc_intrin_workaround void __vmaskmovps(
	float* __a, __m128i __m, __m128 __v) noexcept
{
	_mm_maskstore_ps(__a, __m, __v);
}

__raze_msvc_intrin_workaround void __vmaskmovpd(
	double* __a, __m128i __m, __m128d __v) noexcept
{
	_mm_maskstore_pd(__a, __m, __v);
}

__raze_msvc_intrin_workaround void __vpmaskmovd(
	int* __a, __m128i __m, __m128i __v) noexcept
{
	_mm_maskstore_epi32(__a, __m, __v);
}

__raze_msvc_intrin_workaround void __vpmaskmovq(
	int64* __a, __m128i __m, __m128i __v) noexcept
{
	_mm_maskstore_epi64(__a, __m, __v);
}

__RAZE_DATAPAR_NAMESPACE_END
