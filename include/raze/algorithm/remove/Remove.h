#pragma once 

#include <raze/algorithm/find/Find.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Remove_if : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	struct __impl {
		_Iterator_ _current;
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Predicate_ _predicate;
		_Projection_ _proj;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Predicate_ __pred,
			_Projection_ __proj) noexcept : _iterator(__it), _sentinel(__sent),
			_predicate(__pred), _proj(__proj), _current(__it)
		{
			_current = algorithm::find_if[raze::options::scalar](_iterator, _sentinel, __pred, __proj);
			if (_current != _sentinel) _iterator = std::ranges::next(_current);
			else _iterator = _current;
		}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;
			if (!_predicate(_proj(*_iterator))) {
				*_current = std::ranges::iter_move(_iterator); 
				++_current;
				++_iterator;
				return false;
			}
			++_iterator;
			return false;
		}

		raze_nodiscard raze_always_inline constexpr _Iterator_ result() const noexcept {
			return _current;
		}
	};

	template <class _Tag_>
	struct __vectorized_remove {
		template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
		raze_always_inline _Iterator_ operator()(_Iterator_ __first, _Sentinel_ __sentinel,
			_Predicate_ __predicate, _Projection_ __proj) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			__first = algorithm::find_if[raze::options::scalar](std::move(__first), __sentinel, __predicate, __proj);

			if (__first != __sentinel) {
				for (_Iterator_ __it = std::ranges::next(__first); __it != __sentinel; ++__it) {
					if (!std::invoke(__predicate, std::invoke(__proj, *__it))) {
						*__first = std::ranges::iter_move(__it);
						++__first;
					}
				}
			}

			return __first;
		}

		template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
		raze_always_inline _Iterator_ operator()(sizetype __aligned_size, sizetype __tail_size,
			_Iterator_ __first, _Sentinel_ __sentinel, _Predicate_ __predicate,
			_Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __ptr = std::to_address(__first);
			auto* __current_ptr = __ptr;
			raze_assume(__ptr != nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			do {
				const auto __loaded = vx::load<_Tag_>(__ptr);
				const auto __mask = __predicate(__proj(__loaded));
				__current_ptr = vx::compress_store(__current_ptr, __loaded, __mask);
				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			_Iterator_ __current_it;
			__seek_possibly_wrapped_iterator(__first, __ptr);
			__seek_possibly_wrapped_iterator(__current_it, __current_ptr);

			for (; __first != __sentinel; ++__first )
				if (!__predicate(__proj(*__first))) 
					*__current_it++ = *__first;

			return __current_it;
		}
	};

	template <std::permutable _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::subrange<_Iterator_> operator()(_Iterator_ __first, _Sentinel_ __last,
		_Predicate_ __pred, _Projection_ __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		auto __r = __remove_if_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(__last),
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj));

		__seek_possibly_wrapped_iterator(__first, std::move(__r));
		return { __first, __last };
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<_Range_> operator()(_Range_&& __range, _Predicate_ __pred,
		_Projection_ __proj = {}) const noexcept requires(!constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
			_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>
				&& std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);
		auto __r = __remove_if_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(__end), type_traits::__pass_function(__pred),
			type_traits::__pass_function(__proj));

		__seek_possibly_wrapped_iterator(__begin, std::move(__r));
		return { __begin, __end };
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<_Range_> operator()(_Range_&& __range, _Predicate_ __pred,
		_Projection_ __proj = {}) const noexcept requires(constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
			_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>
				&& std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);
		auto __r = __remove_if_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(__end), type_traits::__pass_function(__pred),
			type_traits::__pass_function(__proj), std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});

		__seek_possibly_wrapped_iterator(__begin, std::move(__r));
		return { __begin, __end };
	}

private:
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	constexpr raze_always_inline _Iterator_ __remove_if_unchecked(
		_Iterator_ __first, _Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		auto __real_first = algorithm::find_if(__first, __last, __pred, __proj);
		if (__real_first == __last) return __real_first;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_>
			&& vectorizable_unary_predicate<_Predicate_, _Iterator_> &&
			vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_remove, _Value_, _Iterator_,
					arch::ISA::None, arch::ISA::AVX512VBMI2, arch::ISA::AVX2, arch::ISA::SSSE3>(
					algorithm::distance(__real_first, __last) * sizeof(_Value_),
					__real_first, __last, __pred, __proj);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__real_first, __last, __pred, __proj));
	}

	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_, sizetype _Size_>
	constexpr raze_always_inline _Iterator_ __remove_if_unchecked(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj,
		std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		auto __real_first = algorithm::find_if(__first, __last, __pred, __proj);
		if (__real_first == __last) return __real_first;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_>
			&& vectorizable_unary_predicate<_Predicate_, _Iterator_>
			&& vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{};
				return vx::__dispatch_sized_impl<__vectorized_remove, _Value_, _Iterator_, arch::ISA::None, 
					arch::ISA::AVX512VBMI2, arch::ISA::AVX2, arch::ISA::SSSE3>(
					__bytes, __real_first, __last, __pred, __proj);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__real_first, __last, __pred, __proj));
	}
};

constexpr inline auto remove_if = raze::options::function_with_traits<_Remove_if>;

template <class _Traits_>
struct _Remove : _Traits_ {
	template <std::permutable _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _ValueType_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::subrange<_Iterator_> operator()(
		_Iterator_ __first, _Sentinel_ __last, const _ValueType_& __v, _Projection_ __proj = {}) const noexcept
	{
		return remove_if[_Traits_::traits()](std::move(__first), std::move(__last), algorithm::equal_to(
			function_return_type<_Projection_, std::iter_value_t<_Iterator_>>(__v)), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _ValueType_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<_Range_> operator()(
		_Range_&& __range, const _ValueType_& __v, _Projection_ __proj = {}) const noexcept
		requires(std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		return remove_if[_Traits_::traits()](std::forward<_Range_>(__range), algorithm::equal_to(
			function_return_type<_Projection_, std::ranges::range_value_t<_Range_>>(__v)), type_traits::__pass_function(__proj));
	}
};

constexpr inline auto remove = raze::options::function_with_traits<_Remove>;

__RAZE_ALGORITHM_NAMESPACE_END