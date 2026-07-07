#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <raze/options/Options.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <src/raze/vx/Concepts.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_unmangled raze_no_stack_protector raze_declare_const_function void raze_stdcall __raze_swap_ranges_scalar(
	void* __first1, void* __first2,  sizetype __total) noexcept
{
	char* __ch_first1 = static_cast<char*>(__first1);
	char* __ch_first2 = static_cast<char*>(__first2);

	for (; __total != 0; ++__ch_first1, ++__ch_first2, --__total)
		std::ranges::iter_swap(__ch_first1, __ch_first2);
}

template <vx::simd_type _Simd_>
raze_no_stack_protector raze_always_inline void __general_swap_ranges_vectorized(void* __first1,
	void* __first2, sizetype __aligned_bytes, sizetype __tail_bytes) noexcept
{
	raze_assume(__aligned_bytes >= sizeof(_Simd_));
	const auto __first1_aligned_end = __bytes_pointer_offset(__first1, __aligned_bytes);

	do {
		auto __v1 = vx::load<_Simd_>(__first1);
		auto __v2 = vx::load<_Simd_>(__first2);
		vx::store(__first1, __v2);
		vx::store(__first2, __v1);
		__advance_bytes(__first1, __first2, sizeof(_Simd_));
	} while (__first1 != __first1_aligned_end);

	char* __ch_first1 = static_cast<char*>(__first1);
	char* __ch_first2 = static_cast<char*>(__first2);

	for (; __tail_bytes != 0; ++__ch_first1, ++__ch_first2, --__tail_bytes)
		std::ranges::iter_swap(__ch_first1, __ch_first2);
}

raze_unmangled raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall __raze_swap_ranges_sse2(
	void* __first1, void* __first2, sizetype __aligned_bytes, sizetype __tail_bytes) noexcept
{
	using _Simd_ = vx::simd<i32, vx::runtime_abi<arch::ISA::SSE2, 4>>;
	return __general_swap_ranges_vectorized<_Simd_>(__first1, __first2, __aligned_bytes, __tail_bytes);
}

raze_unmangled raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall __raze_swap_ranges_avx(void* __first1,
	void* __first2, sizetype __aligned_bytes, sizetype __tail_bytes) noexcept
{
	using _Simd_ = vx::simd<i32, vx::runtime_abi<arch::ISA::AVX, 8>>;
	return __general_swap_ranges_vectorized<_Simd_>(__first1, __first2, __aligned_bytes, __tail_bytes);
}

raze_unmangled raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall __raze_swap_ranges_avx512f(void* __first1,
	void* __first2, sizetype __aligned_bytes, sizetype __tail_bytes) noexcept
{
	using _Simd_ = vx::simd<i32, vx::runtime_abi<arch::ISA::AVX512F, 16>>;
	return __general_swap_ranges_vectorized<_Simd_>(__first1, __first2, __aligned_bytes, __tail_bytes);
}

template <class _Tag_>
struct __vectorized_swap_ranges_internal {
	raze_always_inline void operator()(void* __first1, void* __first2, sizetype __total) const noexcept {
		return __raze_swap_ranges_scalar(__first1, __first2, __total);
	}

	raze_always_inline void operator()(sizetype __aligned_size, sizetype __tail_size,
		void* __first1, void* __first2, sizetype) const noexcept
	{
		using _Abi_ = vx::abi_t<_Tag_>;
		if constexpr (vx::__has_avx512f_support_v<_Abi_::isa>) return __raze_swap_ranges_avx512f(__first1, __first2, __aligned_size, __tail_size);
		else if constexpr (vx::__has_avx_support_v<_Abi_::isa>) return __raze_swap_ranges_avx(__first1, __first2, __aligned_size, __tail_size);
		else return __raze_swap_ranges_sse2(__first1, __first2, __aligned_size, __tail_size);
	}
};

raze_unmangled raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall __raze_swap_ranges(
	void* __first1, void* __first2, sizetype __bytes) noexcept
{
	return vx::__dispatch_sized_impl<__vectorized_swap_ranges_internal, i32, void>(__bytes, __first1, __first2, __bytes);
}

template <class _Traits_>
struct _Swap_ranges_internal : _Traits_ {
	raze_always_inline sizetype operator()(void* __first1, void* __last1, void* __first2, void* __last2) const noexcept {
		using _TraitsType_ = decltype(_Traits_::traits());
		const auto __min_dist = std::min(__byte_length(__first1, __last1), __byte_length(__first2, __last2));

		if constexpr (options::__get_forced_isa<_TraitsType_>() == arch::ISA::None) __raze_swap_ranges(__first1, __first2, __min_dist);
		else vx::__dispatch_sized_impl<__vectorized_swap_ranges_internal, i32, void,
				options::__get_forced_isa<_TraitsType_>()>(__min_dist, __first1, __first2, __min_dist);

		return __min_dist;
	}

	template <sizetype _Size_>
	raze_always_inline sizetype operator()(void* __first1, void* __last1, void* __first2, void* __last2,
		std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		using _TraitsType_ = decltype(_Traits_::traits());

		vx::__dispatch_sized_impl<__vectorized_swap_ranges_internal, i32, void,
			options::__get_forced_isa<_TraitsType_>()>(__size, __first1, __first2, __size);

		return _Size_;
	}
};

static inline constexpr auto __swap_ranges = raze::options::function_with_traits<_Swap_ranges_internal>;

__RAZE_ALGORITHM_NAMESPACE_END
