#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <raze/options/Options.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <src/raze/vx/Concepts.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_no_stack_protector raze_declare_const_function void* raze_stdcall __memcpy_scalar(void* raze_restrict __dst,
	const void* raze_restrict __src, sizetype __bytes) noexcept
{
	char* __dst_ch = static_cast<char*>(__dst);
	const char* __src_ch = static_cast<const char*>(__src);

	for (sizetype __i = 0; __i < __bytes; ++__i)
		*__dst_ch++ = *__src_ch++;

	return __dst_ch;
}

template <vx::simd_type _Simd_>
raze_no_stack_protector raze_always_inline void* __general_memcpy_vectorized(void* raze_restrict __dst,
	const void* raze_restrict __src, sizetype __aligned_bytes, sizetype __tail_bytes) noexcept
{
	raze_assume(__aligned_bytes >= sizeof(_Simd_));

	do {
		vx::store(__dst, vx::load<_Simd_>(__src));
		__advance_bytes(__dst, __src, sizeof(_Simd_));
		__aligned_bytes -= sizeof(_Simd_);
	} while (__aligned_bytes != 0);

	char* __dst_ch = static_cast<char*>(__dst);
	const char* __src_ch = static_cast<const char*>(__src);

	for (sizetype __i = 0; __i < __tail_bytes; ++__i)
		*__dst_ch++ = *__src_ch++;

	return __dst_ch;
}

raze_no_stack_protector raze_never_inline raze_declare_const_function void* raze_stdcall __memcpy_sse2(void* raze_restrict __dst,
	const void* raze_restrict __src, sizetype __aligned_bytes, sizetype __tail_bytes) noexcept
{
	using _Simd_ = vx::simd<i32, vx::runtime_abi<arch::ISA::SSE2, 4>>;
	return __general_memcpy_vectorized<_Simd_>(__dst, __src, __aligned_bytes, __tail_bytes);
}

raze_no_stack_protector raze_never_inline raze_declare_const_function void* raze_stdcall __memcpy_avx(void* raze_restrict __dst,
	const void* raze_restrict __src, sizetype __aligned_bytes, sizetype __tail_bytes) noexcept
{
	using _Simd_ = vx::simd<i32, vx::runtime_abi<arch::ISA::AVX, 8>>;
	return __general_memcpy_vectorized<_Simd_>(__dst, __src, __aligned_bytes, __tail_bytes);
}

raze_no_stack_protector raze_never_inline raze_declare_const_function void* raze_stdcall __memcpy_avx512f(void* raze_restrict __dst,
	const void* raze_restrict __src, sizetype __aligned_bytes, sizetype __tail_bytes) noexcept
{
	using _Simd_ = vx::simd<i32, vx::runtime_abi<arch::ISA::AVX512F, 16>>;
	return __general_memcpy_vectorized<_Simd_>(__dst, __src, __aligned_bytes, __tail_bytes);
}

template <template <class> class _Func_, class _Traits_>
raze_no_stack_protector raze_always_inline raze_declare_const_function void* raze_stdcall __dispatch_memcpy(void* raze_restrict __dst,
	const void* raze_restrict __src, sizetype __bytes) noexcept
{
	return vx::__dispatch_sized_impl<_Func_, i32, void*, options::__get_forced_isa<_Traits_>()>(__bytes, __dst, __src, __bytes);
}

template <class _Traits_>
struct _Memcpy : _Traits_ {
	template <class _Tag_>
	struct __vectorized_memcpy {
		raze_always_inline void* operator()(void* raze_restrict __dst, const void* raze_restrict __src,
			sizetype __bytes) const noexcept
		{
			return __memcpy_scalar(__dst, __src, __bytes);
		}

		raze_always_inline void* operator()(sizetype __aligned_size, sizetype __tail_size,
			void* raze_restrict __dst, const void* raze_restrict __src, sizetype __bytes) const noexcept
		{
			using _Abi_ = vx::abi_t<_Tag_>;
			if constexpr (vx::__has_avx512f_support_v<_Abi_::isa>) return __memcpy_avx512f(__dst, __src, __aligned_size, __tail_size);
			else if constexpr (vx::__has_avx_support_v<_Abi_::isa>) return __memcpy_avx(__dst, __src, __aligned_size, __tail_size);
			else return __memcpy_sse2(__dst, __src, __aligned_size, __tail_size);
		}
	};

	raze_always_inline void* operator()(void* raze_restrict __dst,
		const void* raze_restrict __src, sizetype __bytes) const noexcept
	{
		using _TraitsType_ = decltype(_Traits_::traits());
		if constexpr (options::__get_forced_isa<_TraitsType_>() == arch::ISA::None) {
			return __dispatch_memcpy<__vectorized_memcpy, _TraitsType_>(__dst, __src, __bytes);
		}
		else {
			return vx::__dispatch_sized_impl<__vectorized_memcpy, i32, void*,
				options::__get_forced_isa<_TraitsType_>()>(__bytes, __dst, __src, __bytes);
		}
	}

	template <sizetype _Bytes_>
	raze_always_inline void* operator()(void* raze_restrict __dst,
		const void* raze_restrict __src, std::integral_constant<sizetype, _Bytes_> __bytes) const noexcept
	{
		using _TraitsType_ = decltype(_Traits_::traits());
		return vx::__dispatch_sized_impl<__vectorized_memcpy, i32, void*, 
			options::__get_forced_isa<_TraitsType_>()>(__bytes, __dst, __src, __bytes);
	}
};

static inline constexpr auto __memcpy = raze::options::function_with_traits<_Memcpy>;

__RAZE_ALGORITHM_NAMESPACE_END
