#pragma once 

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <raze/math/Math.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

// Although GCC does not vectorize find/find_if quite as efficiently as
// our manual SIMD library, its autovectorizer performs well enough to
// safely enable `target_clones` by default. This provides excellent
// dynamic dispatch across multiple ISAs without a significant drop in
// performance. Users can override this behavior via [scalar] or by
// explicitly requesting manual SIMD dispatch.
constexpr auto find_strategy = options::strategy<strategy<>().for_gcc<strategy_mode::autovec>()>;

template <class _Traits_>
struct _Find_if : _Traits_, dispatchable<_Find_if<_Traits_>> {
	template <source _Source_, class _Predicate_, class _Projection_>
	struct __kernel {
		using source_type = std::remove_cvref_t<_Source_>;
		using iterator_type = typename source_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = std::iter_value_t<iterator_type>;

		_Source_ _source;
		unchecked_iterator_type _iterator;
		unchecked_sentinel_type _sentinel;
		_Predicate_ _predicate;
		_Projection_ _proj;

		constexpr explicit __kernel(_Source_&& __src, _Predicate_ __pred, _Projection_ __proj) noexcept:
			_source(std::forward<_Source_>(__src)), _predicate(__pred), _proj(__proj)
		{
			_iterator = _source.ubegin();
			_sentinel = _source.uend();
		}

		raze_always_inline constexpr void operator()() noexcept {
			for (; _iterator != _sentinel; ++_iterator) 
				if (_predicate(_proj(*_iterator))) 
					break;
		}

		template <vectorizable_tag _Tag_>
		raze_always_inline bool operator()(_Tag_, sizetype __aligned_size) noexcept {
			auto* __ptr = std::to_address(_iterator);
			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			raze_disable_unrolling
			do {
				const auto __mask = _predicate(_proj(vx::load<_Tag_>(__ptr)));

				if (vx::any_of(__mask)) {
					source_type::from_ptr(_iterator, __ptr + vx::find_first_set[vx::not_null](__mask));
					return false;
				}

				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			source_type::from_ptr(_iterator, __ptr);
			return true;
		}

		template <vectorizable_tag _Tag_>
		raze_always_inline void operator()(_Tag_, tail_mask_type auto const& __ignore) noexcept {
			auto* __ptr = std::to_address(_iterator);
			const auto __mask = _predicate(_proj(vx::load<_Tag_>[__ignore](__ptr)));
			
			const auto __offset = math::min(vx::find_first_set[__ignore](__mask), __ignore.tail_elements());
			source_type::from_ptr(_iterator, __ptr + __offset);
		}

		constexpr raze_always_inline iterator_type result() const noexcept {
			return _source.wrap(_iterator);
		}

		static constexpr decltype(auto) static_size() noexcept requires(constexpr_sized_source<_Source_>) {
			return _Source_::static_size();
		}

		constexpr raze_always_inline auto size() const noexcept {
			return _source.size();
		}

		static consteval bool vectorizable() noexcept {
			return std::contiguous_iterator<unchecked_iterator_type> &&
				vectorizable_unary_predicate<_Predicate_, unchecked_iterator_type> &&
				vectorizable_projection<_Projection_, unchecked_iterator_type>;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __sent, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		return this->dispatch(get_source(std::move(__first), std::move(__sent)),
			traits::__fwd_fn(__pred), traits::__fwd_fn(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __r, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return this->dispatch(get_source(std::forward<_Range_>(__r)),
			traits::__fwd_fn(__pred), traits::__fwd_fn(__proj));
	}
};

constexpr inline auto find_if = raze::options::function_with_traits<_Find_if>[raze::options::unroll<4>][find_strategy];

template <class _Traits_>
struct _Find : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Value_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, const _Value_& __v, _Projection_ __proj = {}) const noexcept
	{
		return find_if[_Traits_::traits()](std::move(__first), std::move(__last), algorithm::equal_to(
			function_return_type<_Projection_, std::iter_value_t<_Iterator_>>(__v)),
			traits::__fwd_fn(__proj));
	}

	template <std::ranges::input_range _Range_, class _Value_,
		class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __range, const _Value_& __v, _Projection_ __proj = {}) const noexcept
	{
		return find_if[_Traits_::traits()](std::forward<_Range_>(__range), algorithm::equal_to(
			function_return_type<_Projection_, std::ranges::range_value_t<_Range_>>(__v)),
			traits::__fwd_fn(__proj));
	}
};

constexpr inline auto find = raze::options::function_with_traits<_Find>[raze::options::unroll<4>][find_strategy];

template <class _Traits_>
struct _Find_if_not : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		return find_if[_Traits_::traits()](std::move(__first), std::move(__last), make_not_fn(__pred), traits::__fwd_fn(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __range, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return find_if[_Traits_::traits()](std::forward<_Range_>(__range), make_not_fn(__pred), traits::__fwd_fn(__proj));
	}
};

constexpr inline auto find_if_not = raze::options::function_with_traits<_Find_if_not>[raze::options::unroll<4>][find_strategy];

__RAZE_ALGORITHM_NAMESPACE_END
