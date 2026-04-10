#pragma once 

#include <raze/vx/SimdDataparAlgorithms.h>
#include <src/raze/vx/SizedSimdDispatcher.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN


template <class _Type_>
raze_nodiscard __raze_simd_algorithm_inline const _Type_* __find_first_of_scalar(
	const void* __first1,
	const void* __last1,
	const void* __first2,
	const void* __last2) noexcept
{
	auto __first1_current = static_cast<const _Type_*>(__first1);
	auto __first2_current = static_cast<const _Type_*>(__first2);

	for (; __first1_current != __last1; ++__first1_current)
		for (auto __mid2_unwrapped = __first2_current; __mid2_unwrapped != __last2; ++__mid2_unwrapped)
			if (*__first1_current == *__mid2_unwrapped)
				return __first1_current;

	return static_cast<const _Type_*>(__last1);
}

template <class _Simd_>
struct __find_first_of_vectorized_internal {
private:
	using _ValueType = typename _Simd_::value_type;

	raze_nodiscard static __raze_simd_algorithm_inline const _ValueType* __find_first_of_any_size(
		sizetype	__aligned_size,
		sizetype	__tail_size,
		sizetype	__values_count,
		const void* __first1,
		const void* __last1,
		const void* __first2,
		const void* __last2) noexcept
	{
		const auto __stop_at = __bytes_pointer_offset(__first1, __aligned_size);

		do {
			const auto __loaded_main = vx::load<_Simd_>(__first1);

			auto __values_begin = static_cast<const _ValueType*>(__first2);
			auto __found_address = static_cast<const _ValueType*>(__last1);

			for (auto __i = 0; __i < __values_count; ++__i) {
				const auto __comparand = _Simd_(*__values_begin++);
				const auto __mask = __comparand == __loaded_main;

				if (vx::any_of(__mask))
					__found_address = raze::algorithm::min(__found_address,
						static_cast<const _ValueType*>(__first1) + vx::find_first_set(__mask));
			}

			if (__found_address != __last1)
				return __found_address;

			__advance_bytes(__first1, sizeof(_Simd_));
		} while (__first1 != __stop_at);

		return __find_first_of_scalar<_ValueType>(__first1, __last1, __first2, __last2);
	}

	template <uint64 ... _Indices_>
	raze_nodiscard static __raze_simd_algorithm_inline const _ValueType* __find_first_of_small_size(
		sizetype	__aligned_size,
		sizetype	__tail_size,
		const void* __first1,
		const void* __last1,
		const void* __first2,
		const void* __last2,
		std::index_sequence<_Indices_...>) noexcept
	{
		auto __found = static_cast<const _ValueType*>(__last1);
		auto __values_begin = static_cast<const _ValueType*>(__first2);

		const auto __main_loaded = vx::load<_Simd_>(__first1);

		const _Simd_ __values[sizeof...(_Indices_)] = {
			 ([=, &__found]() noexcept {
				const auto __comparand = _Simd_(__values_begin[_Indices_]);
				const auto __compared = __main_loaded == __comparand;

				if (vx::any_of(__compared))
					__found = raze::algorithm::min(__found, static_cast<const _ValueType*>(__first1) + vx::find_first_set(__compared));

				return __comparand;
			} ())...
		};

		if (__found != __last1)
			return __found;

		auto __stop_at = __bytes_pointer_offset(__first1, __aligned_size);
		__advance_bytes(__first1, sizeof(_Simd_));

		while (__first1 != __stop_at) {
			const auto __current_loaded = vx::load<_Simd_>(__first1);

			([=, &__found]() noexcept {
				const auto __compared = __current_loaded == __values[_Indices_];

				if (vx::any_of(__compared))
					__found = std::min(__found, static_cast<const _ValueType*>(__first1) + vx::find_first_set(__compared));
			} (), ...);

			if (__found != __last1)
				return __found;

			__advance_bytes(__first1, sizeof(_Simd_));
		}

		return __find_first_of_scalar<_ValueType>(__first1, __last1, __first2, __last2);
	}
public:
	raze_nodiscard raze_static_operator __raze_simd_algorithm_inline const _ValueType* operator()(
		sizetype	__aligned_size,
		sizetype	__tail_size,
		const void* __first1,
		const void* __last1,
		const void* __first2,
		const void* __last2) raze_const_operator noexcept
	{
		const auto __guard = vx::make_guard<_Simd_>();
		const auto __values_count = __byte_length(__first2, __last2) / sizeof(_ValueType);

		switch (__values_count) {
			case 1:
				return __find_first_of_small_size(__aligned_size, __tail_size,
					__first1, __last1, __first2, __last2, std::make_index_sequence<1>());

			case 2:
				return __find_first_of_small_size(__aligned_size, __tail_size,
					__first1, __last1, __first2, __last2, std::make_index_sequence<2>());

			case 3:
				return __find_first_of_small_size(__aligned_size, __tail_size,                 
					__first1, __last1, __first2, __last2, std::make_index_sequence<3>()); 

			case 4:
				return __find_first_of_small_size(__aligned_size, __tail_size,
					__first1, __last1, __first2, __last2, std::make_index_sequence<4>()); 

			case 5:
				return __find_first_of_small_size(__aligned_size, __tail_size,
					__first1, __last1, __first2, __last2, std::make_index_sequence<5>());

			case 6:
				return __find_first_of_small_size(__aligned_size, __tail_size,
					__first1, __last1, __first2, __last2, std::make_index_sequence<6>());

			case 7:
				return __find_first_of_small_size(__aligned_size, __tail_size,
					__first1, __last1, __first2, __last2, std::make_index_sequence<7>());

			case 8:
				return __find_first_of_small_size(__aligned_size, __tail_size,
					__first1, __last1, __first2, __last2, std::make_index_sequence<8>());

			default:
				return __find_first_of_any_size(__aligned_size, __tail_size,
					__values_count, __first1, __last1, __first2, __last2);
		}
	}
};

template <class _Type_> 
raze_nodiscard __raze_simd_algorithm_inline const _Type_* __find_first_of_vectorized(
	const void* __first1,
	const void* __last1,
	const void* __first2,
	const void* __last2) noexcept
{
	return vx::__simd_sized_dispatcher<__find_first_of_vectorized_internal, _Type_>()(
		__byte_length(__first1, __last1), &__find_first_of_scalar<_Type_>, __first1, __last1, __first2, __last2);
}

__RAZE_ALGORITHM_NAMESPACE_END
