#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <raze/options/Options.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <src/raze/vx/Concepts.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_unmangled raze_no_stack_protector raze_declare_const_function void raze_stdcall raze_swap_3_ranges_scalar(void* first1,
	void* last1, void* first2, void* first3) noexcept
{
	char* ch_first1 = static_cast<char*>(first1);
	char* ch_first2 = static_cast<char*>(first2);
	char* ch_first3 = static_cast<char*>(first3);

	for (; ch_first1 != last1; ++ch_first1, ++ch_first2, ++ch_first3) {
		char v1 = *ch_first1;
		char v2 = *ch_first2;
		char v3 = *ch_first3;

		*ch_first1 = v2;
		*ch_first2 = v3;
		*ch_first3 = v1;
	}
}

template <vx::simd_type V>
raze_no_stack_protector raze_always_inline void general_swap_3_ranges_vectorized(void* first1,
	void* last1, void* first2, void* first3, sizetype aligned_bytes, sizetype tail_bytes) noexcept
{
	raze_assume(aligned_bytes >= sizeof(V));
	const auto first1_aligned_end = bytes_pointer_offset(first1, aligned_bytes);

	do {
		auto v1 = vx::load<V>(first1);
		auto v2 = vx::load<V>(first2);
		auto v3 = vx::load<V>(first3);
		vx::store(first1, v2);
		vx::store(first2, v3);
		vx::store(first3, v1);
		advance_bytes(first1, first3, sizeof(V));
		advance_bytes(first2, sizeof(V));
	} while (first1 != first1_aligned_end);

	char* ch_first1 = static_cast<char*>(first1);
	char* ch_first2 = static_cast<char*>(first2);
	char* ch_first3 = static_cast<char*>(first3);

	for (; ch_first1 != last1; ++ch_first1, ++ch_first2, ++ch_first3) {
		char v1 = *ch_first1;
		char v2 = *ch_first2;
		char v3 = *ch_first3;

		*ch_first1 = v2;
		*ch_first2 = v3;
		*ch_first3 = v1;
	}
}

raze_unmangled raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall
raze_swap_3_ranges_sse2(void* first1, void* last1, void* first2, 
	void* first3, sizetype aligned_bytes, sizetype tail_bytes) noexcept
{
	using V = vx::simd<i32, vx::runtime_abi<arch::ISA::SSE2, 4>>;
	return general_swap_3_ranges_vectorized<V>(first1, last1, first2, first3, aligned_bytes, tail_bytes);
}

raze_unmangled raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall raze_swap_3_ranges_avx(void* first1,
	void* last1, void* first2, void* first3, sizetype aligned_bytes, sizetype tail_bytes) noexcept
{
	using V = vx::simd<i32, vx::runtime_abi<arch::ISA::AVX, 8>>;
	return general_swap_3_ranges_vectorized<V>(first1, last1, first2, first3, aligned_bytes, tail_bytes);
}

raze_unmangled raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall raze_swap_3_ranges_avx512f(void* first1,
	void* last1, void* first2, void* first3, sizetype aligned_bytes, sizetype tail_bytes) noexcept
{
	using V = vx::simd<i32, vx::runtime_abi<arch::ISA::AVX512F, 16>>;
	return general_swap_3_ranges_vectorized<V>(first1, last1, first2, first3, aligned_bytes, tail_bytes);
}

template <class V>
struct vectorized_swap_3_ranges_internal {
	raze_always_inline void operator()(void* first1, void* last1, void* first2, void* first3) const noexcept {
		return raze_swap_3_ranges_scalar(first1, last1, first2, first3);
	}

	raze_always_inline void operator()(sizetype aligned_size, sizetype tail_size,
		void* first1, void* last1, void* first2, void* first3) const noexcept
	{
		using Abi = vx::abi_t<V>;
		if constexpr (vx::has_avx512f<Abi::isa>) return raze_swap_3_ranges_avx512f(first1, last1, first2, first3, aligned_size, tail_size);
		else if constexpr (vx::has_avx<Abi::isa>) return raze_swap_3_ranges_avx(first1, last1, first2, first3, aligned_size, tail_size);
		else return raze_swap_3_ranges_sse2(first1, last1, first2, first3, aligned_size, tail_size);
	}
};

raze_unmangled raze_no_stack_protector raze_never_inline raze_declare_const_function void raze_stdcall raze_swap_3_ranges(
	void* first1, void* last1, void* first2, void* first3) noexcept
{
	return vx::dispatch<vectorized_swap_3_ranges_internal, i32, void>(byte_length(first1, last1), first1, last1, first2, first3);
}

template <class Traits>
struct swap_3_ranges_t : Traits {
	raze_always_inline void operator()(void* first1, void* last1, void* first2, void* first3) const noexcept {
		using TraitsType = decltype(Traits::traits());
		if constexpr (options::get_forced_isa<TraitsType>() == arch::ISA::None) return raze_swap_3_ranges(first1, last1, first2, first3);
		else return vx::dispatch<vectorized_swap_3_ranges_internal, i32, void,
			options::get_forced_isa<TraitsType>()>(byte_length(first1, last1), first1, last1, first2, first3);
	}
};

static inline constexpr auto swap_3_ranges_impl = raze::options::function_with_traits<swap_3_ranges_t>;

__RAZE_ALGORITHM_NAMESPACE_END
