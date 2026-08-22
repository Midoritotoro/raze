#pragma once 


#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

constexpr auto replace_strategy = options::strategy<strategy<>()
	.for_gcc<strategy_mode::autovec>()
	.for_clang<strategy_mode::autovec>()>;

template <class Traits>
struct replace_if_t : Traits, dispatchable<replace_if_t<Traits>>{
	template <source Source, class Predicate, class Projection, class Value>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;

		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = std::iter_value_t<iterator_type>;

		static consteval bool vectorizable() noexcept {
			return std::contiguous_iterator<unchecked_iterator_type> &&
				vectorizable_unary_predicate<Predicate, unchecked_iterator_type>&&
				vectorizable_projection<Projection, unchecked_iterator_type>;
		}

		Source _source;
		unchecked_iterator_type _iterator;
		unchecked_sentinel_type _sentinel;
		Predicate _predicate;
		Projection _proj;
		Value _new_value;

		constexpr __kernel(Source&& src, Predicate pred,
			Projection proj, Value new_val) noexcept :
				_source(std::forward<Source>(src)), _predicate(pred), _proj(proj), _new_value(new_val)
		{
			_iterator = _source.ubegin();
			_sentinel = _source.uend();
		}
	
		void operator()(autovectorizable) noexcept requires(vectorizable()) {
			auto* raze_restrict b = std::to_address(_iterator);
			auto* raze_restrict e = std::to_address(_sentinel);

			for (; b != e; ++b)
				*b = _predicate(_proj(*b)) ? _new_value : *b;

			source_type::from_ptr(_iterator, b);
		}

		raze_always_inline constexpr void operator()() noexcept {
			raze_disable_unrolling
			for (; _iterator != _sentinel; ++_iterator) 
				*_iterator = (_predicate(_proj(*_iterator))) ? _new_value : *_iterator;
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) noexcept {
			auto* ptr = std::to_address(_iterator);

			const auto aligned_end = __bytes_pointer_offset(ptr, aligned_size);
			const auto new_value = Tag(_new_value);

			raze_disable_unrolling
			do {
				vx::store[_predicate(_proj(vx::load<Tag>(ptr)))](ptr, new_value);
				__advance_bytes(__ptr, sizeof(Tag));
			} while (ptr != aligned_end);

			source_type::from_ptr(_iterator, ptr);
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) noexcept {
			auto* ptr = std::to_address(_iterator);
			vx::store[_predicate(_proj(vx::load<Tag>[ignore](ptr))) & ignore()](ptr, Tag(_new_value));
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() noexcept requires(constexpr_sized_source<Source>) {
			return Source::static_size();
		}

		raze_nodiscard constexpr raze_always_inline auto size() const noexcept {
			return _source.size();
		}
	};

	template <std::permutable Iterator, std::sentinel_for<Iterator> Sentinel,
		class Predicate, class Value, class Projection = std::identity>
	constexpr raze_always_inline void operator()(Iterator first, Sentinel sent,
		Predicate pred, Value new_value, Projection proj = {}) const noexcept
			requires(std::indirect_unary_predicate<Predicate, std::projected<Iterator, Projection>>)
	{
		this->dispatch(get_source(std::move(first), std::move(sent)),
			traits::__fwd_fn(__pred), traits::__fwd_fn(proj), new_value);
	}

	template <std::ranges::input_range _Range_, class Predicate, class Value,
		class Projection = std::identity>
	constexpr raze_always_inline void operator()(_Range_&& __r, Predicate __pred,
		Value __new_value, Projection __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<Predicate,
				std::projected<std::ranges::iterator_t<_Range_>, Projection>>
				&& std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		this->dispatch(get_source(std::forward<_Range_>(__r)),
			traits::__fwd_fn(__pred), traits::__fwd_fn(__proj), __new_value);
	}
};

constexpr inline auto replace_if = raze::options::function_with_traits<_Replace_if>[raze::options::unroll<1>][replace_strategy];

template <class _Traits_>
struct _Replace : _Traits_ {
	template <std::permutable _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _ValueType1_, class _ValueType2_, class Projection = std::identity>
	constexpr raze_always_inline void operator()(_Iterator_ __first, _Sentinel_ __last,
		const _ValueType1_& __old_value, const _ValueType2_& __new_value, Projection __proj = {}) const noexcept
	{
		replace_if[_Traits_::traits()](std::move(__first), std::move(__last), algorithm::equal_to(
			function_return_type<Projection, std::iter_value_t<_Iterator_>>(__old_value)),
			__new_value, traits::__fwd_fn(__proj));
	}

	template <std::ranges::input_range _Range_, class _ValueType1_, class _ValueType2_,
		class Projection = std::identity>
	constexpr raze_always_inline void operator()(_Range_&& __r, const _ValueType1_& __old_value,
		const _ValueType2_& __new_value, Projection __proj = {}) const noexcept
			requires(std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		replace_if[_Traits_::traits()](std::forward<_Range_>(__r), algorithm::equal_to(
			function_return_type<Projection, std::ranges::range_value_t<_Range_>>(__old_value)),
			__new_value, traits::__fwd_fn(__proj));
	}
};

constexpr inline auto replace = raze::options::function_with_traits<_Replace>[raze::options::unroll<1>][replace_strategy];

__RAZE_ALGORITHM_NAMESPACE_END