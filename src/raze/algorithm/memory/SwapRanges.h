#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <raze/options/Options.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <src/raze/vx/Concepts.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_unmangled raze_no_stack_protector raze_declare_const_function void raze_stdcall raze_swap_ranges_scalar(
	void* first1, void* first2,  sizetype total) noexcept
{
	char* ch_first1 = static_cast<char*>(first1);
	char* ch_first2 = static_cast<char*>(first2);

	for (; total != 0; ++ch_first1, ++ch_first2, --total)
		std::ranges::iter_swap(ch_first1, ch_first2);
}

template <vx::simd_type V>
raze_no_stack_protector raze_always_inline void general_swap_ranges_vectorized(void* first1,
	void* first2, sizetype aligned_bytes, sizetype tail_bytes) noexcept
{
	raze_assume(aligned_bytes >= sizeof(V));
	const auto first1_aligned_end = bytes_pointer_offset(first1, aligned_bytes);

	do {
		auto v1 = vx::load<V>(first1);
		auto v2 = vx::load<V>(first2);
		vx::store(first1, v2);
		vx::store(first2, v1);
		advance_bytes(first1, first2, sizeof(V));
	} while (first1 != first1_aligned_end);

	char* ch_first1 = static_cast<char*>(first1);
	char* ch_first2 = static_cast<char*>(first2);

	for (; tail_bytes != 0; ++ch_first1, ++ch_first2, --tail_bytes)
		std::ranges::iter_swap(ch_first1, ch_first2);
}

raze_unmangled raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall raze_swap_ranges_sse2(
	void* first1, void* first2, sizetype aligned_bytes, sizetype tail_bytes) noexcept
{
	using V = vx::simd<i32, vx::runtime_abi<arch::ISA::SSE2, 4>>;
	return general_swap_ranges_vectorized<V>(first1, first2, aligned_bytes, tail_bytes);
}

raze_unmangled raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall raze_swap_ranges_avx(void* first1,
	void* first2, sizetype aligned_bytes, sizetype tail_bytes) noexcept
{
	using V = vx::simd<i32, vx::runtime_abi<arch::ISA::AVX, 8>>;
	return general_swap_ranges_vectorized<V>(first1, first2, aligned_bytes, tail_bytes);
}

raze_unmangled raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall raze_swap_ranges_avx512f(void* first1,
	void* first2, sizetype aligned_bytes, sizetype tail_bytes) noexcept
{
	using V = vx::simd<i32, vx::runtime_abi<arch::ISA::AVX512F, 16>>;
	return general_swap_ranges_vectorized<V>(first1, first2, aligned_bytes, tail_bytes);
}

template <class V>
struct vectorized_swap_ranges_internal {
	raze_always_inline void operator()(void* first1, void* first2, sizetype total) const noexcept {
		return raze_swap_ranges_scalar(first1, first2, total);
	}

	raze_always_inline void operator()(sizetype aligned_size, sizetype tail_size,
		void* first1, void* first2, sizetype) const noexcept
	{
		using Abi = vx::abi_t<V>;
		if constexpr (vx::has_avx512f<Abi::isa>) return raze_swap_ranges_avx512f(first1, first2, aligned_size, tail_size);
		else if constexpr (vx::has_avx<Abi::isa>) return raze_swap_ranges_avx(first1, first2, aligned_size, tail_size);
		else return raze_swap_ranges_sse2(first1, first2, aligned_size, tail_size);
	}
};

raze_unmangled raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall raze_swap_ranges(
	void* first1, void* first2, sizetype bytes) noexcept
{
	return vx::dispatch_sized_impl<vectorized_swap_ranges_internal, i32, void>(bytes, first1, first2, bytes);
}

template <class Traits>
struct _Swap_ranges_internal : Traits {
	raze_always_inline sizetype operator()(void* first1, void* last1, void* first2, void* last2) const noexcept {
		using TraitsType = decltype(Traits::traits());
		const auto min_dist = std::min(byte_length(first1, last1), byte_length(first2, last2));

		if constexpr (options::get_forced_isa<TraitsType>() == arch::ISA::None) raze_swap_ranges(first1, first2, min_dist);
		else vx::dispatch_sized_impl<vectorized_swap_ranges_internal, i32, void,
				options::get_forced_isa<TraitsType>()>(min_dist, first1, first2, min_dist);

		return min_dist;
	}

	template <sizetype _Size_>
	raze_always_inline sizetype operator()(void* first1, void* last1, void* first2, void* last2,
		std::integral_constant<sizetype, _Size_> size) const noexcept
	{
		using TraitsType = decltype(_Traits_::traits());

		vx::dispatch_sized_impl<vectorized_swap_ranges_internal, i32, void,
			options::get_forced_isa<TraitsType>()>(size, first1, first2, size);

		return _Size_;
	}
};

static inline constexpr auto swap_ranges_impl = raze::options::function_with_traits<_Swap_ranges_internal>;

__RAZE_ALGORITHM_NAMESPACE_END
