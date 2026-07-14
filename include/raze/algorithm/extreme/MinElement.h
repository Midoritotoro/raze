#pragma once 

#include <raze/concurrency/Execution.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>
#include <optional>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Min_element : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Projection_>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Projection_ _proj;
		_Iterator_ _lowest_value_iterator;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent),  _proj(__proj), _lowest_value_iterator(__it)
		{
			++_iterator;
		}

		template <class _Tag_>
		raze_always_inline raze_nodiscard constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;
			else {
				if (_proj(*_lowest_value_iterator) > _proj(*_iterator))
					_lowest_value_iterator = _iterator;
				++_iterator;
				return false;
			}
		}

		raze_nodiscard constexpr raze_always_inline _Iterator_ result() const noexcept {
			return _lowest_value_iterator;
		}
	};

	template <class _Tag_>
	struct __vectorized_min_element {
		template <class _Iterator_, class _Sentinel_, class _Projection_>
		raze_nodiscard raze_always_inline _Iterator_ operator()(_Iterator_ __first, 
			_Sentinel_ __sentinel, _Projection_ __proj) const noexcept
		{
			if (__first == __sentinel) return __first;
			_Iterator_ __lowest_value_iterator = __first;

			for (; ++__first != __sentinel;)
				if (__proj(*__lowest_value_iterator) > __proj(*__first))
					__lowest_value_iterator = __first;

			return __lowest_value_iterator;
		}

		template <class _Iterator_, class _Sentinel_, class _Projection_>
		raze_nodiscard raze_always_inline auto operator()(sizetype __aligned_size,
			sizetype __tail_size, _Iterator_ __first, _Sentinel_ __sentinel,
			_Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			using _Value_ = std::iter_value_t<_Iterator_>;

			auto* __ptr = const_cast<_Value_*>(std::to_address(__first));
			raze_assume(__ptr != nullptr);

			using _UnsignedValueType = IntegerForSizeof<_Value_>::Unsigned;
			using _IndexSimdType = vx::simd<_UnsignedValueType, vx::abi_t<_Tag_>>;

			auto __min_element = static_cast<_Value_*>(__ptr);

			auto __current_indices_min = _IndexSimdType::zero();
			auto __current_indices = _IndexSimdType::zero();
			auto __current_values = __proj(vx::load<_Tag_>(__ptr));
			auto __current_values_min = __current_values;

			constexpr auto __integer_max = math::__maximum_integral_limit<_UnsignedValueType>();
			constexpr auto __max_portion_size = std::max(static_cast<u64>(__integer_max)
				+ 1, static_cast<u64>(__integer_max)) * sizeof(_Tag_);

			constexpr auto __has_portion_max_value = (math::__maximum_integral_limit<u64>() != __max_portion_size);
			auto __aligned_portion_size = std::min(__max_portion_size, __aligned_size);

			auto __portion_begin = __min_element;
			auto __stop_at = __bytes_pointer_offset(__ptr, __aligned_portion_size);
			__aligned_size -= __aligned_portion_size;

			while (true) {
				__advance_bytes(__ptr, sizeof(_Tag_));
				++__current_indices;

				if (__ptr != __stop_at) {
					__current_values = __proj(vx::load<_Tag_>(__ptr));
					__current_values_min = __proj(__current_values_min);

					const auto __less_mask = (__current_values < __current_values_min);

					__current_indices_min = vx::select[__less_mask, __current_indices_min](__current_indices);
					__current_values_min = vx::select[__less_mask, __current_values_min](__current_values);
				}
				else {
					const auto __min = vx::horizontal_min(__current_values_min);

					if (__min < *__min_element) {
						auto __min_mask = (__min == __current_values_min);
						
						const auto __min_values_indices = vx::select[__min_mask, ~_IndexSimdType::zero()](__current_indices_min);
						const auto __all_min_indices = vx::horizontal_min(__min_values_indices);
						
						const auto __horizontal_position = vx::find_first_set(math::bit_cast<decltype(__min_mask)>(__all_min_indices == __min_values_indices) & __min_mask);
						const auto __vertical_position = sizetype(__current_indices_min[__horizontal_position]);
						
						const auto __maybe_min_element = __bytes_pointer_offset(__portion_begin,
							__vertical_position * sizeof(_Tag_) + __horizontal_position * sizeof(_Value_));

						if (*__maybe_min_element < *__min_element) __min_element = __maybe_min_element;
					}

					if constexpr (__has_portion_max_value) {
						__aligned_portion_size = std::min(__max_portion_size, __aligned_size);

						if (__aligned_portion_size == 0) break;

						__aligned_size -= __aligned_portion_size;
						__advance_bytes(__stop_at, __aligned_portion_size);
						__portion_begin = static_cast<_Value_*>(__first);

						__current_values = __proj(vx::load<_Tag_>(__first));
						__current_values_min = __current_values;
						__current_indices_min = _IndexSimdType::zero();
					}
					else {
						break;
					}
				}
			}

			_Iterator_ __lowest_value_iterator;

			__seek_possibly_wrapped_iterator(__first, __ptr);
			__seek_possibly_wrapped_iterator(__lowest_value_iterator, __min_element);

			for (; __first != __sentinel; ++__first)
				if (__proj(*__lowest_value_iterator) > __proj(*__first)) 
					__lowest_value_iterator = __first;

			return __lowest_value_iterator;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, _Projection_ __proj = {}) const noexcept
	{
		__seek_possibly_wrapped_iterator(__first, __min_element_unchecked(
			type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)),
			type_traits::__pass_function(__proj)));

		return __first;
	}

	template <std::ranges::input_range _Range_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __range, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_>)
	{
		auto __begin = std::ranges::begin(__range);
		__seek_possibly_wrapped_iterator(__begin, __min_element_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__proj)));
		return __begin;
	}

	template <std::ranges::input_range _Range_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(_Range_&& __range,
		_Projection_ __proj = {}) const noexcept requires(constexpr_sized_range<_Range_>)
	{
		auto __begin = std::ranges::begin(__range);
		__seek_possibly_wrapped_iterator(__begin, __min_element_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__proj),
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{}));
		return __begin;
	}
private:
	template <class _Iterator_, class _Sentinel_, class _Projection_>
	raze_nodiscard constexpr raze_always_inline _Iterator_ __min_element_unchecked(
		_Iterator_ __first, _Sentinel_ __last, _Projection_ __proj) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_>
			&& vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_min_element, _Value_, _Iterator_>(
					algorithm::distance(__first, __last) * sizeof(_Value_), __first, __last, __proj);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __proj));
	}

	template <class _Iterator_, class _Sentinel_, class _Projection_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline _Iterator_ __min_element_unchecked(_Iterator_ __first,
		_Sentinel_ __last, _Projection_ __proj, std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_>
			&& vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{};
				return vx::__dispatch_sized_impl<__vectorized_min_element,
					_Value_, _Iterator_>(__bytes, __first, __last, __proj);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __proj));
	}
};

constexpr inline auto min_element = raze::options::function_with_traits<_Min_element>;

__RAZE_ALGORITHM_NAMESPACE_END
