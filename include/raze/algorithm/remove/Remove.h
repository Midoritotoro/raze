#pragma once 

#include <raze/algorithm/find/Find.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Remove_if : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class Predicate, class Projection>
	struct __impl {
		_Iterator_ _current;
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		Predicate _predicate;
		Projection _proj;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, Predicate __pred,
			Projection __proj) noexcept : _iterator(__it), _sentinel(__sent),
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
		template <class _Iterator_, class _Sentinel_, class Predicate, class Projection>
		raze_always_inline _Iterator_ operator()(_Iterator_ __first, _Sentinel_ __sentinel,
			Predicate __predicate, Projection __proj) const noexcept requires(!vx::simd_type<_Tag_>)
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

		template <class _Iterator_, class _Sentinel_, class Predicate, class Projection>
		raze_always_inline _Iterator_ operator()(sizetype __aligned_size, sizetype __tail_size,
			_Iterator_ __first, _Sentinel_ __sentinel, Predicate __predicate,
			Projection __proj) const noexcept requires(vx::simd_type<_Tag_>)
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
			__seek_iter(__first, __ptr);
			__seek_iter(__current_it, __current_ptr);

			for (; __first != __sentinel; ++__first )
				if (!__predicate(__proj(*__first))) 
					*__current_it++ = *__first;

			return __current_it;
		}
	};

	template <std::permutable _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class Predicate, class Projection = std::identity>
	constexpr raze_always_inline std::ranges::subrange<_Iterator_> operator()(_Iterator_ __first, _Sentinel_ __last,
		Predicate __pred, Projection __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<Predicate, std::projected<_Iterator_, Projection>>)
	{
		auto __r = __remove_if_unchecked(traits::__uiter<_Sentinel_>(std::move(__first)),
			traits::__usent<_Iterator_>(__last),
			traits::fwd_fn(__pred), traits::fwd_fn(__proj));

		__seek_iter(__first, std::move(__r));
		return { __first, __last };
	}

	template <std::ranges::input_range Range, class Predicate, class Projection = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<Range> operator()(Range&& __range, Predicate __pred,
		Projection __proj = {}) const noexcept requires(!constexpr_sized_range<Range> && std::indirect_unary_predicate<
			Predicate, std::projected<std::ranges::iterator_t<Range>, Projection>>
				&& std::permutable<std::ranges::iterator_t<Range>>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);
		auto __r = __remove_if_unchecked(traits::__r_uiter<Range>(std::move(__begin)),
			traits::__r_usent<Range>(__end), traits::fwd_fn(__pred),
			traits::fwd_fn(__proj));

		__seek_iter(__begin, std::move(__r));
		return { __begin, __end };
	}

	template <std::ranges::input_range Range, class Predicate, class Projection = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<Range> operator()(Range&& __range, Predicate __pred,
		Projection __proj = {}) const noexcept requires(constexpr_sized_range<Range> && std::indirect_unary_predicate<
			Predicate, std::projected<std::ranges::iterator_t<Range>, Projection>>
				&& std::permutable<std::ranges::iterator_t<Range>>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);
		auto __r = __remove_if_unchecked(traits::__r_uiter<Range>(std::move(__begin)),
			traits::__r_usent<Range>(__end), traits::fwd_fn(__pred),
			traits::fwd_fn(__proj), std::integral_constant<sizetype, __range_constexpr_size<Range>()>{});

		__seek_iter(__begin, std::move(__r));
		return { __begin, __end };
	}

private:
	template <class _Iterator_, class _Sentinel_, class Predicate, class Projection>
	constexpr raze_always_inline _Iterator_ __remove_if_unchecked(
		_Iterator_ __first, _Sentinel_ __last, Predicate __pred, Projection __proj) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		auto __real_first = algorithm::find_if(__first, __last, __pred, __proj);
		if (__real_first == __last) return __real_first;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_>
			&& vectorizable_unary_predicate<Predicate, _Iterator_> &&
			vectorizable_projection<Projection, _Iterator_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_remove, _Value_, _Iterator_,
					arch::ISA::None, arch::ISA::AVX512VBMI2, arch::ISA::AVX2, arch::ISA::SSSE3>(
					algorithm::distance(__real_first, __last) * sizeof(_Value_),
					__real_first, __last, __pred, __proj);
			}
		}

		return options::_unroller_t<_TraitsType, vx::scalar_tag>(__impl(__real_first, __last, __pred, __proj));
	}

	template <class _Iterator_, class _Sentinel_, class Predicate, class Projection, sizetype _Size_>
	constexpr raze_always_inline _Iterator_ __remove_if_unchecked(_Iterator_ __first,
		_Sentinel_ __last, Predicate __pred, Projection __proj,
		std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		auto __real_first = algorithm::find_if(__first, __last, __pred, __proj);
		if (__real_first == __last) return __real_first;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_>
			&& vectorizable_unary_predicate<Predicate, _Iterator_>
			&& vectorizable_projection<Projection, _Iterator_>)
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{};
				return vx::__dispatch_sized_impl<__vectorized_remove, _Value_, _Iterator_, arch::ISA::None, 
					arch::ISA::AVX512VBMI2, arch::ISA::AVX2, arch::ISA::SSSE3>(
					__bytes, __real_first, __last, __pred, __proj);
			}
		}

		return options::_unroller_t<_TraitsType, vx::scalar_tag>(__impl(__real_first, __last, __pred, __proj));
	}
};

constexpr inline auto remove_if = raze::options::function_with_traits<_Remove_if>;

template <class _Traits_>
struct _Remove : _Traits_ {
	template <std::permutable _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _ValueType_, class Projection = std::identity>
	constexpr raze_always_inline std::ranges::subrange<_Iterator_> operator()(
		_Iterator_ __first, _Sentinel_ __last, const _ValueType_& __v, Projection __proj = {}) const noexcept
	{
		return remove_if[_Traits_::traits()](std::move(__first), std::move(__last), algorithm::equal_to(
			function_return_type<Projection, std::iter_value_t<_Iterator_>>(__v)), traits::fwd_fn(__proj));
	}

	template <std::ranges::input_range Range, class _ValueType_, class Projection = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<Range> operator()(
		Range&& __range, const _ValueType_& __v, Projection __proj = {}) const noexcept
		requires(std::permutable<std::ranges::iterator_t<Range>>)
	{
		return remove_if[_Traits_::traits()](std::forward<Range>(__range), algorithm::equal_to(
			function_return_type<Projection, std::ranges::range_value_t<Range>>(__v)), traits::fwd_fn(__proj));
	}
};

constexpr inline auto remove = raze::options::function_with_traits<_Remove>;

__RAZE_ALGORITHM_NAMESPACE_END