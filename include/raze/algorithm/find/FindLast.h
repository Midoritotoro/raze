#pragma once 

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <raze/math/Math.h>
#include <utility>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Find_last_if : _Traits_, dispatchable<_Find_last_if<_Traits_>> {
    template <source _Source_, class _Predicate_, class _Projection_>
    struct __impl {
        using source_type = std::remove_cvref_t<_Source_>;
        using iterator_type = typename source_type::iterator_type;

        using iterator_type = typename source_type::iterator_type;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using vector_value_type = std::iter_value_t<iterator_type>;

        _Source_ _source;

        unchecked_iterator_type _iterator;
        unchecked_sentinel_type _sentinel;

        _Predicate_ _predicate;
        _Projection_ _proj;

        std::ranges::subrange<iterator_type> _result;

        constexpr explicit __impl(_Source_&& __src, _Predicate_ __pred, _Projection_ __proj) noexcept:
            _source(std::forward<_Source_>(__src)), _predicate(__pred), _proj(__proj)
        {
            _iterator = _source.ubegin();
            _sentinel = _source.uend();
        }

        raze_always_inline constexpr void operator()() noexcept {
            if constexpr (std::bidirectional_iterator<unchecked_iterator_type> && std::same_as<unchecked_iterator_type, unchecked_sentinel_type>) {
                for (auto __r = _sentinel; __r != _iterator;) {
                    if (_predicate(_proj(*--__r))) {
                        _result = std::ranges::subrange<iterator_type>{ std::move(__r), std::move(_sentinel) };
                        break;
                    }
                }
                
                _result = std::ranges::subrange<iterator_type> { _sentinel, _sentinel };
            }
            else if constexpr (std::same_as<unchecked_iterator_type, unchecked_sentinel_type>) {
                auto __r = _sentinel;

                for (; _iterator != _sentinel; ++_iterator)
                    if (_predicate(_proj(*_iterator)))
                        __r = _iterator;
                
                _result = std::ranges::subrange<iterator_type> { std::move(__r), std::move(_sentinel) };
            }
            else {
                auto __r = _iterator;
                bool __found = false;

                for (;; ++_iterator) {
                    if (_iterator == _sentinel) {
                        if (!__found) __r = _iterator;
                        break;
                    }

                    if (_predicate(_proj(*_iterator))) {
                        __r = _iterator;
                        __found = true;
                    }
                }
                
                _result = std::ranges::subrange<iterator_type> { std::move(__r), std::move(_iterator) };
            }
        }

        template <vectorizable_tag _Tag_>
        raze_always_inline bool operator()(_Tag_, sizetype __aligned_size) noexcept {
            unchecked_iterator_type __iterator_last;

            if constexpr (std::same_as<unchecked_iterator_type, unchecked_sentinel_type>) __iterator_last = _sentinel;
            else { __iterator_last = _iterator; std::ranges::advance(__iterator_last, std::ranges::distance(_iterator, _sentinel)); };

            auto* __ptr = std::to_address(__iterator_last);
            const auto __stop_at = __bytes_pointer_offset(__ptr, -i64(__aligned_size));

            do {
                __rewind_bytes(__ptr, sizeof(_Tag_));
                const auto __mask = _predicate(_proj(vx::load<_Tag_>(__ptr)));

                if (vx::any_of(__mask)) {
                    _Source_::from_ptr(_iterator, __ptr + _Tag_::size() - vx::find_last_set[vx::not_null](__mask) - 1);
                    _result = std::ranges::subrange<iterator_type> { _iterator, __iterator_last };
                    return false;
                }
            } while (__ptr != __stop_at);

            auto __tail_it = __iterator_last;
            __seek_iter(__tail_it, __ptr);

            while (true) {
                if (_predicate(_proj(*__tail_it))) {
                    _iterator = __tail_it;
                    _result = { _iterator, __iterator_last };
                    return false;
                }
                if (__tail_it == _iterator) break;
                --__tail_it;
            }

            _result = std::ranges::subrange<iterator_type>{ __iterator_last, __iterator_last };
            return false;
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() noexcept requires(constexpr_sized_source<_Source_>) {
            return _Source_::static_size();
        }

        raze_nodiscard constexpr raze_always_inline auto size() const noexcept {
            return _source.size();
        }

        static consteval bool vectorizable() noexcept {
            return std::contiguous_iterator<unchecked_iterator_type> &&
                vectorizable_unary_predicate<_Predicate_, unchecked_iterator_type>&&
                vectorizable_projection<_Projection_, unchecked_iterator_type>;
        }

        constexpr raze_always_inline auto result() const noexcept {
            return _result;
        }
    };

    template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
        class _Predicate_, class _Projection_ = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::subrange<_Iterator_> operator()(_Iterator_ __first,
        _Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
            requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
    {
        auto __unwrapped_first = traits::__uiter<_Sentinel_>(std::move(__first));

        if constexpr (std::bidirectional_iterator<_Iterator_>) {
            auto __unwrapped_last = traits::__last_uiter<_Iterator_>(__unwrapped_first, std::move(__last));
            auto __unwrapped_result = __find_last_unchecked(
                std::move(__unwrapped_first), std::move(__unwrapped_last),
                traits::__fwd_fn(__pred), traits::__fwd_fn(__proj));

            return traits::__rewrap_subrange<std::ranges::subrange<_Iterator_>>(__first, std::move(__unwrapped_result));
        }
        else {
            auto __unwrapped_result = __find_last_unchecked(std::move(__unwrapped_first),
                traits::__usent<_Iterator_>(std::move(__last)),
                traits::__fwd_fn(__pred), traits::__fwd_fn(__proj));

            return traits::__rewrap_subrange<std::ranges::subrange<_Iterator_>>(__first, std::move(__unwrapped_result));
        }
    }

    template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
    constexpr raze_always_inline std::ranges::borrowed_subrange_t<_Range_> operator()(
        _Range_&& __range, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
            requires(!constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
                _Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
    {
        if constexpr (std::ranges::bidirectional_range<_Range_>) {
            return traits::__rewrap_subrange<std::ranges::borrowed_subrange_t<_Range_>>(__range, __find_last_unchecked(
                traits::__ubegin(__range), traits::__last_uiter(__range),
                traits::__fwd_fn(__pred), traits::__fwd_fn(__proj)));
        }
        else {
            return traits::__rewrap_subrange<std::ranges::borrowed_subrange_t<_Range_>>(__range, __find_last_unchecked(
                traits::__ubegin(__range), traits::__uend(__range),
                traits::__fwd_fn(__pred), traits::__fwd_fn(__proj)));
        }

    }

    template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
    constexpr raze_always_inline std::ranges::borrowed_subrange_t<_Range_> operator()(_Range_&& __range,
        _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
            requires(constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
                _Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
    {
        if constexpr (std::ranges::bidirectional_range<_Range_>) {
            return traits::__rewrap_subrange<std::ranges::borrowed_subrange_t<_Range_>>(__range, __find_last_unchecked(
                traits::__ubegin(__range), traits::__last_uiter(__range),
                traits::__fwd_fn(__pred), traits::__fwd_fn(__proj),
                std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{}));
        }
        else {
            return traits::__rewrap_subrange<std::ranges::borrowed_subrange_t<_Range_>>(__range, __find_last_unchecked(
                traits::__ubegin(__range), traits::__uend(__range),
                traits::__fwd_fn(__pred), traits::__fwd_fn(__proj),
                std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{}));
        }
    }
};

constexpr inline auto find_last_if = raze::options::function_with_traits<_Find_last_if>;

template <class _Traits_>
struct _Find_last : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Value_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::subrange<_Iterator_> operator()(_Iterator_ __first,
		_Sentinel_ __last, const _Value_& __v, _Projection_ __proj = {}) const noexcept
	{
		return find_last_if[_Traits_::traits()](std::move(__first), std::move(__last), algorithm::equal_to(
			function_return_type<_Projection_, std::iter_value_t<_Iterator_>>(__v)),
			traits::__fwd_fn(__proj));
	}

	template <std::ranges::input_range _Range_, class _Value_,
		class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<_Range_> operator()(
		_Range_&& __r, const _Value_& __v, _Projection_ __proj = {}) const noexcept
	{
		return find_last_if[_Traits_::traits()](std::forward<_Range_>(__r), algorithm::equal_to(
			function_return_type<_Projection_, std::ranges::range_value_t<_Range_>>(__v)),
			traits::__fwd_fn(__proj));
	}
};

constexpr inline auto find_last = raze::options::function_with_traits<_Find_last>;

template <class _Traits_>
struct _Find_last_if_not : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline std::ranges::subrange<_Iterator_> operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		return find_last_if[_Traits_::traits()](std::move(__first), std::move(__last), make_not_fn(__pred), traits::__fwd_fn(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<_Range_> operator()(
		_Range_&& __r, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<_Predicate_, 
                std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return find_last_if[_Traits_::traits()](std::forward<_Range_>(__r), make_not_fn(__pred), traits::__fwd_fn(__proj));
	}
};

constexpr inline auto find_last_if_not = raze::options::function_with_traits<_Find_last_if_not>;


__RAZE_ALGORITHM_NAMESPACE_END
