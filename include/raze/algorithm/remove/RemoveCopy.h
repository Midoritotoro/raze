#pragma once 

#include <raze/algorithm/find/Find.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Remove_copy_if : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Output_, class Predicate, class Projection>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Output_ _out;
		Predicate _predicate;
		Projection _proj;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Output_ __output, Predicate __pred,
			Projection __proj) noexcept : _iterator(__it), _sentinel(__sent), _out(__output),
			_predicate(__pred), _proj(__proj)
		{}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;
			if (!_predicate(_proj(*_iterator))) {
				*_out = std::ranges::iter_move(_iterator);
				++_out;
			}
			++_iterator;
			return false;
		}

		raze_nodiscard raze_always_inline constexpr _Output_ result() const noexcept {
			return _out;
		}
	};

	template <class _Tag_>
	struct __vectorized_remove_copy {
		template <class _Iterator_, class _Sentinel_, class _Output_, class Predicate, class Projection>
		raze_always_inline _Output_ operator()(_Iterator_ __first, _Sentinel_ __sentinel, _Output_ __output,
			Predicate __predicate, Projection __proj) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			for (; __first != __sentinel; ++__first) {
				if (!std::invoke(__predicate, std::invoke(__proj, *__first))) {
					*__output = std::ranges::iter_move(__first);
					++__output;
				}
			}

			return __output;
		}

		template <class _Iterator_, class _Sentinel_, class _Output_, class Predicate, class Projection>
		raze_always_inline _Output_ operator()(sizetype __aligned_size, sizetype __tail_size,
			_Iterator_ __first, _Sentinel_ __sentinel, _Output_ __output, Predicate __predicate,
			Projection __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __ptr = std::to_address(__first);
			auto* __out_ptr = std::to_address(__output);
			raze_assume(__ptr != nullptr);
			raze_assume(__out_ptr != nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			do {
				const auto __loaded = vx::load<_Tag_>(__ptr);
				__out_ptr = vx::compress_store(__out_ptr, __loaded, __predicate(__proj(__loaded)));
				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			__seek_iter(__output, __out_ptr);
			__seek_iter(__first, __ptr);

			for (; __first != __sentinel; ++__first) {
				if (!__predicate(__proj(*__first))) {
					*__output = std::ranges::iter_move(__first);
					++__output;
				}
			}

			return __output;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		std::weakly_incrementable _Output_, class Predicate, class Projection = std::identity>
	constexpr raze_always_inline std::pair<_Iterator_, _Output_> operator()(_Iterator_ __first, _Sentinel_ __last,
		_Output_ __output, Predicate __pred, Projection __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<Predicate, std::projected<_Iterator_, Projection>> &&
			std::indirectly_copyable<_Iterator_, _Output_>)
	{
		auto __result_out = __remove_copy_if_unchecked(traits::__uiter<_Sentinel_>(std::move(__first)),
			traits::__usent<_Iterator_>(__last), std::move(__output),
			traits::fwd_fn(__pred), traits::fwd_fn(__proj));

		__seek_iter(__output, std::move(__result_out));
		return { __last, __output };
	}

	template <std::ranges::input_range Range, std::weakly_incrementable _Output_,
		class Predicate, class Projection = std::identity>
	constexpr raze_always_inline std::pair<std::ranges::iterator_t<Range>, _Output_> operator()(
		Range&& __range, _Output_ __output, Predicate __pred, Projection __proj = {}) const noexcept
		requires(!constexpr_sized_range<Range> && std::indirect_unary_predicate<
			Predicate, std::projected<std::ranges::iterator_t<Range>, Projection>> &&
			std::indirectly_copyable<std::ranges::iterator_t<Range>, _Output_>)
	{
		auto __end = std::ranges::end(__range);
		auto __result_out = __remove_copy_if_unchecked(traits::__ubegin(__range),
			traits::__r_usent<Range>(__end), std::move(__output),
			traits::fwd_fn(__pred), traits::fwd_fn(__proj));

		__seek_iter(__output, std::move(__result_out));
		return { __end, __result_out };
	}

	template <std::ranges::input_range Range, std::weakly_incrementable _Output_,
		class Predicate, class Projection = std::identity>
	constexpr raze_always_inline std::pair<std::ranges::iterator_t<Range>, _Output_> operator()(
		Range&& __range, _Output_ __output, Predicate __pred, Projection __proj = {}) const noexcept
		requires(constexpr_sized_range<Range> && std::indirect_unary_predicate<
			Predicate, std::projected<std::ranges::iterator_t<Range>, Projection>> &&
			std::indirectly_copyable<std::ranges::iterator_t<Range>, _Output_>)
	{
		auto __end = std::ranges::end(__range);
		auto __result_out = __remove_copy_if_unchecked(traits::__ubegin(__range),
			traits::__r_usent<Range>(__end), std::move(__output),
			traits::fwd_fn(__pred), traits::fwd_fn(__proj),
			std::integral_constant<sizetype, __range_constexpr_size<Range>()>{});

		__seek_iter(__output, std::move(__result_out));
		return { __end, __result_out };
	}

private:
	template <class _Iterator_, class _Sentinel_, class _Output_, class Predicate, class Projection>
	constexpr raze_always_inline _Output_ __remove_copy_if_unchecked(
		_Iterator_ __first, _Sentinel_ __last, _Output_ __output, Predicate __pred, Projection __proj) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_>
			&& std::contiguous_iterator<_Output_>
			&& vectorizable_unary_predicate<Predicate, _Iterator_> &&
			vectorizable_projection<Projection, _Iterator_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_remove_copy, _Value_, _Output_>(
					algorithm::distance(__first, __last) * sizeof(_Value_),
					__first, __last, __output, __pred, __proj);
			}
		}

		return options::_unroller_t<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __output, __pred, __proj));
	}

	template <class _Iterator_, class _Sentinel_, class _Output_, class Predicate, class Projection, sizetype _Size_>
	constexpr raze_always_inline _Output_ __remove_copy_if_unchecked(_Iterator_ __first,
		_Sentinel_ __last, _Output_ __output, Predicate __pred, Projection __proj,
		std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_>
			&& std::contiguous_iterator<_Output_> && vectorizable_unary_predicate<Predicate, _Iterator_>
			&& vectorizable_projection<Projection, _Iterator_>)
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{};
				return vx::__dispatch_sized_impl<__vectorized_remove_copy, _Value_, _Output_>(
					__bytes, __first, __last, __output, __pred, __proj);
			}
		}

		return options::_unroller_t<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __output, __pred, __proj));
	}
};

constexpr inline auto remove_copy_if = raze::options::function_with_traits<_Remove_copy_if>;

template <class _Traits_>
struct _Remove_copy : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		std::weakly_incrementable _Output_, class _ValueType_, class Projection = std::identity>
	constexpr raze_always_inline std::pair<_Iterator_, _Output_> operator()(
		_Iterator_ __first, _Sentinel_ __last, _Output_ __output, const _ValueType_& __v, Projection __proj = {}) const noexcept
		requires(std::indirectly_copyable<_Iterator_, _Output_>)
	{
		return remove_copy_if[_Traits_::traits()](std::move(__first), std::move(__last), std::move(__output), algorithm::equal_to(
			function_return_type<Projection, std::iter_value_t<_Iterator_>>(__v)), traits::fwd_fn(__proj));
	}

	template <std::ranges::input_range Range, std::weakly_incrementable _Output_,
		class _ValueType_, class Projection = std::identity>
	constexpr raze_always_inline std::pair<std::ranges::iterator_t<Range>, _Output_> operator()(
		Range&& __range, _Output_ __output, const _ValueType_& __v, Projection __proj = {}) const noexcept
		requires(std::indirectly_copyable<std::ranges::iterator_t<Range>, _Output_>)
	{
		return remove_copy_if[_Traits_::traits()](std::forward<Range>(__range), std::move(__output), algorithm::equal_to(
			function_return_type<Projection, std::ranges::range_value_t<Range>>(__v)), traits::fwd_fn(__proj));
	}
};

constexpr inline auto remove_copy = raze::options::function_with_traits<_Remove_copy>;

__RAZE_ALGORITHM_NAMESPACE_END