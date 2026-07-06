#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <raze/options/Options.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <src/raze/vx/Concepts.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_no_stack_protector raze_declare_const_function void raze_stdcall __swap_3_ranges_scalar(void* __first1,
	void* __last1, void* __first2, void* __first3) noexcept
{
	char* __ch_first1 = static_cast<char*>(__first1);
	char* __ch_first2 = static_cast<char*>(__first2);
	char* __ch_first3 = static_cast<char*>(__first3);

	for (; __ch_first1 != __last1; ++__ch_first1, ++__ch_first2, ++__ch_first3) {
		char __v1 = *__ch_first1;
		char __v2 = *__ch_first2;
		char __v3 = *__ch_first3;

		*__ch_first1 = __v2;
		*__ch_first2 = __v3;
		*__ch_first3 = __v1;
	}
}

template <vx::simd_type _Simd_>
raze_no_stack_protector raze_always_inline void __general_swap_3_ranges_vectorized(void* __first1,
	void* __last1, void* __first2, void* __first3, sizetype __aligned_bytes, sizetype __tail_bytes) noexcept
{
	raze_assume(__aligned_bytes >= sizeof(_Simd_));

	const auto __first1_aligned_end = __bytes_pointer_offset(__first1, __aligned_bytes);

	do {
		auto __v1 = vx::load<_Simd_>(__first1);
		auto __v2 = vx::load<_Simd_>(__first2);
		auto __v3 = vx::load<_Simd_>(__first3);
		vx::store(__first1, __v2);
		vx::store(__first2, __v3);
		vx::store(__first3, __v1);
		__advance_bytes(__first1, __first3, sizeof(_Simd_));
		__advance_bytes(__first2, sizeof(_Simd_));
	} while (__first1 != __first1_aligned_end);

	char* __ch_first1 = static_cast<char*>(__first1);
	char* __ch_first2 = static_cast<char*>(__first2);
	char* __ch_first3 = static_cast<char*>(__first3);

	for (; __ch_first1 != __last1; ++__ch_first1, ++__ch_first2, ++__ch_first3) {
		char __v1 = *__ch_first1;
		char __v2 = *__ch_first2;
		char __v3 = *__ch_first3;

		*__ch_first1 = __v2;
		*__ch_first2 = __v3;
		*__ch_first3 = __v1;
	}
}

raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall __swap_3_ranges_sse2(void* __first1,
	void* __last1, void* __first2, void* __first3, sizetype __aligned_bytes, sizetype __tail_bytes) noexcept
{
	using _Simd_ = vx::simd<i32, vx::runtime_abi<arch::ISA::SSE2, 4>>;
	return __general_swap_3_ranges_vectorized<_Simd_>(__first1, __last1, __first2, __first3, __aligned_bytes, __tail_bytes);
}

raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall __swap_3_ranges_avx(void* __first1,
	void* __last1, void* __first2, void* __first3, sizetype __aligned_bytes, sizetype __tail_bytes) noexcept
{
	using _Simd_ = vx::simd<i32, vx::runtime_abi<arch::ISA::AVX, 8>>;
	return __general_swap_3_ranges_vectorized<_Simd_>(__first1, __last1, __first2, __first3, __aligned_bytes, __tail_bytes);
}

raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall __swap_3_ranges_avx512f(void* __first1,
	void* __last1, void* __first2, void* __first3, sizetype __aligned_bytes, sizetype __tail_bytes) noexcept
{
	using _Simd_ = vx::simd<i32, vx::runtime_abi<arch::ISA::AVX512F, 16>>;
	return __general_swap_3_ranges_vectorized<_Simd_>(__first1, __last1, __first2, __first3, __aligned_bytes, __tail_bytes);
}

template <template <class> class _Func_, class _Traits_>
raze_no_stack_protector raze_always_inline raze_declare_const_function void raze_stdcall __dispatch_swap_3_ranges(
	void* __first1, void* __last1, void* __first2, void* __first3, sizetype __bytes) noexcept
{
	return vx::__dispatch_sized_impl<_Func_, i32, void, options::__get_forced_isa<_Traits_>()>(__bytes, __first1, __last1, __first2, __first3);
}

template <class _Traits_>
struct _Swap_3_ranges : _Traits_ {
	template <class _Tag_>
	struct __vectorized_swap_3_ranges {
		raze_always_inline void operator()(void* __first1, void* __last1, void* __first2, void* __first3) const noexcept {
			return __swap_3_ranges_scalar(__first1, __last1, __first2, __first3);
		}

		raze_always_inline void operator()(sizetype __aligned_size, sizetype __tail_size,
			void* __first1, void* __last1, void* __first2, void* __first3) const noexcept
		{
			using _Abi_ = vx::abi_t<_Tag_>;
			if constexpr (vx::__has_avx512f_support_v<_Abi_::isa>) return __swap_3_ranges_avx512f(__first1, __last1, __first2, __first3, __aligned_size, __tail_size);
			else if constexpr (vx::__has_avx_support_v<_Abi_::isa>) return __swap_3_ranges_avx(__first1, __last1, __first2, __first3, __aligned_size, __tail_size);
			else return __swap_3_ranges_sse2(__first1, __last1, __first2, __first3, __aligned_size, __tail_size);
		}
	};

	raze_always_inline void operator()(void* __first1, void* __last1, void* __first2, void* __first3) const noexcept {
		using _TraitsType_ = decltype(_Traits_::traits());
		if constexpr (options::__get_forced_isa<_TraitsType_>() == arch::ISA::None) {
			return __dispatch_swap_3_ranges<__vectorized_swap_3_ranges, _TraitsType_>(__first1, __last1, __first2, __first3, __byte_length(__first1, __last1));
		}
		else {
			return vx::__dispatch_sized_impl<__vectorized_swap_3_ranges, i32, void,
				options::__get_forced_isa<_TraitsType_>()>(__byte_length(__first1, __last1), __first1, __last1, __first2, __first3);
		}
	}
};

static inline constexpr auto __swap_3_ranges = raze::options::function_with_traits<_Swap_3_ranges>;

__RAZE_ALGORITHM_NAMESPACE_END
