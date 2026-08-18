#pragma once 


#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

constexpr auto replace_strategy = options::strategy<strategy<>()
	.for_gcc<strategy_mode::autovec>()
	.for_clang<strategy_mode::autovec>()>;

template <class _Traits_>
struct _Replace_if : _Traits_, dispatchable<_Replace_if<_Traits_>>{
	template <source _Source_, class _Predicate_, class _Projection_, class _ValueType_>
	struct __kernel {
		using source_type = std::remove_cvref_t<_Source_>;
		using iterator_type = typename source_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = std::iter_value_t<iterator_type>;

		static consteval bool vectorizable() noexcept {
			return std::contiguous_iterator<unchecked_iterator_type> &&
				vectorizable_unary_predicate<_Predicate_, unchecked_iterator_type>&&
				vectorizable_projection<_Projection_, unchecked_iterator_type>;
		}

		_Source_ _source;
		unchecked_iterator_type _iterator;
		unchecked_sentinel_type _sentinel;
		_Predicate_ _predicate;
		_Projection_ _proj;
		_ValueType_ _new_value;

		constexpr __kernel(_Source_&& __src, _Predicate_ __pred,
			_Projection_ __proj, _ValueType_ __new_val) noexcept :
				_source(std::forward<_Source_>(__src)), _predicate(__pred), _proj(__proj), _new_value(__new_val)
		{
			_iterator = _source.ubegin();
			_sentinel = _source.uend();
		}
	
		void operator()(autovectorizable) noexcept requires(vectorizable()) {
			auto* raze_restrict __b = std::to_address(_iterator);
			auto* raze_restrict __e = std::to_address(_sentinel);

			for (; __b != __e; ++__b)
				*__b = (_predicate(_proj(*__b))) ? _new_value : *__b;
		}

		raze_always_inline constexpr void operator()() noexcept {
			for (; _iterator != _sentinel; ++_iterator) 
				*_iterator = (_predicate(_proj(*_iterator))) ? _new_value : *_iterator;
		}

		template <vectorizable_tag _Tag_>
		raze_always_inline constexpr void operator()(_Tag_, sizetype __aligned_size) noexcept {
			auto* __ptr = std::to_address(_iterator);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);
			const auto __new_value = _Tag_(_new_value);

			raze_disable_unrolling
			do {
				vx::store[_predicate(_proj(vx::load<_Tag_>(__ptr)))](__ptr, __new_value);
				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			source_type::from_ptr(_iterator, __ptr);
		}

		template <vectorizable_tag _Tag_>
		raze_always_inline void operator()(_Tag_, tail_mask_type auto const& __ignore) noexcept {
			auto* __ptr = std::to_address(_iterator);
			vx::store[_predicate(_proj(vx::load<_Tag_>[__ignore](__ptr)))](__ptr, _Tag_(_new_value));
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() noexcept requires(constexpr_sized_source<_Source_>) {
			return _Source_::static_size();
		}

		raze_nodiscard constexpr raze_always_inline auto size() const noexcept {
			return _source.size();
		}
	};

	template <std::permutable _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _ValueType_, class _Projection_ = std::identity>
	constexpr raze_always_inline void operator()(_Iterator_ __first, _Sentinel_ __sent,
		_Predicate_ __pred, _ValueType_ __new_value, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		this->dispatch(get_source(std::move(__first), std::move(__sent)),
			traits::__fwd_fn(__pred), traits::__fwd_fn(__proj), __new_value);
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _ValueType_,
		class _Projection_ = std::identity>
	constexpr raze_always_inline void operator()(_Range_&& __r, _Predicate_ __pred,
		_ValueType_ __new_value, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<_Predicate_,
				std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>
				&& std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		this->dispatch(get_source(std::forward<_Range_>(__r)),
			traits::__fwd_fn(__pred), traits::__fwd_fn(__proj), __new_value);
	}
};

constexpr inline auto replace_if = raze::options::function_with_traits<_Replace_if>[raze::options::unroll<4>][replace_strategy];

template <class _Traits_>
struct _Replace : _Traits_ {
	template <std::permutable _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _ValueType1_, class _ValueType2_, class _Projection_ = std::identity>
	constexpr raze_always_inline void operator()(_Iterator_ __first, _Sentinel_ __last,
		const _ValueType1_& __old_value, const _ValueType2_& __new_value, _Projection_ __proj = {}) const noexcept
	{
		replace_if[_Traits_::traits()](std::move(__first), std::move(__last), algorithm::equal_to(
			function_return_type<_Projection_, std::iter_value_t<_Iterator_>>(__old_value)),
			__new_value, traits::__fwd_fn(__proj));
	}

	template <std::ranges::input_range _Range_, class _ValueType1_, class _ValueType2_,
		class _Projection_ = std::identity>
	constexpr raze_always_inline void operator()(_Range_&& __r, const _ValueType1_& __old_value,
		const _ValueType2_& __new_value, _Projection_ __proj = {}) const noexcept
			requires(std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		replace_if[_Traits_::traits()](std::forward<_Range_>(__r), algorithm::equal_to(
			function_return_type<_Projection_, std::ranges::range_value_t<_Range_>>(__old_value)),
			__new_value, traits::__fwd_fn(__proj));
	}
};

constexpr inline auto replace = raze::options::function_with_traits<_Replace>[raze::options::unroll<4>][replace_strategy];

__RAZE_ALGORITHM_NAMESPACE_END