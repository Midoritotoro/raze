#pragma once 

#include <raze/concurrency/Execution.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Find_last_if : _Traits_ {
    template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
struct __impl {
    _Iterator_ _iterator;
    _Iterator_ _iterator_last;
    _Sentinel_ _sentinel;
    _Predicate_ _predicate;
    _Projection_ _proj;

    constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Predicate_ __pred, _Projection_ __proj) noexcept:
        _iterator(__it), _iterator_last(__it), _sentinel(__sent), _predicate(__pred), _proj(__proj)
    {
        std::advance(_iterator_last, std::ranges::distance(__it, __sent));
    }

    template <class _Tag_>
    raze_always_inline raze_nodiscard constexpr bool operator()(_Tag_) noexcept {
        if constexpr (std::bidirectional_iterator<_Iterator_>) {
            if (_iterator_last == _iterator) { 
                _iterator = _sentinel;
                return true;
            }
            if (_predicate(_proj(*(--_iterator_last)))) { 
                _iterator = _iterator_last; 
                return true; 
            }
            return false;
        }
        else {
            if (_iterator == _sentinel) {
                _iterator = _iterator_last;
                return true;
            }
            if (_predicate(_proj(*_iterator))) return true;
            ++_iterator;
            return false;
        }
    }

    template <vx::simd_type _Tag_>
    raze_nodiscard raze_always_inline void operator()(_Tag_, sizetype __aligned_size, sizetype __tail_size) noexcept {
        auto __result_it = _sentinel;
        auto* __ptr = std::to_address(_iterator_last);
        raze_assume(__ptr != __nullptr);

        const auto __stop_at = __bytes_pointer_offset(__ptr, -i64(__aligned_size));

        do {
            __rewind_bytes(__ptr, sizeof(_Tag_));
            const auto __mask = _predicate(_proj(raze::vx::load<_Tag_>(__ptr)));

            if (raze::vx::any_of(__mask)) {
                __seek_possibly_wrapped_iterator(__result_it, __ptr + _Tag_::size() - vx::find_last_set[vx::not_null](__mask) - 1);
                _iterator = __result_it;
                return;
            }
        } while (__ptr != __stop_at);
        
        auto __tail_it = _iterator_last;
        __seek_possibly_wrapped_iterator(__tail_it, __ptr);

        while (true) {
            if (_predicate(_proj(*__tail_it))) {
                _iterator = __tail_it;
                return;
            }
            if (__tail_it == _iterator) break;
            --__tail_it;
        }

        _iterator = _sentinel;
    }

    template <vx::simd_type _Tag_, sizetype _AlignedSize_, sizetype _TailSize_>
    raze_nodiscard raze_always_inline void operator()(_Tag_,
        std::integral_constant<sizetype, _AlignedSize_>,
        std::integral_constant<sizetype, _TailSize_>) noexcept
    {
        auto __result_it = _sentinel;
        auto* __ptr = std::to_address(_iterator_last);
        raze_assume(__ptr != __nullptr);

        constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);
        auto __left = __iterations_aligned;

        do {
            __rewind_bytes(__ptr, sizeof(_Tag_));
            const auto __mask = _predicate(_proj(raze::vx::load<_Tag_>(__ptr)));

            if (raze::vx::any_of(__mask)) {
                __seek_possibly_wrapped_iterator(__result_it, __ptr + _Tag_::size() - vx::find_last_set[vx::not_null](__mask) - 1);
                _iterator = __result_it;
                return;
            }
        } while (--__left);

        if constexpr (_TailSize_ != 0) {
            auto __tail_it = _iterator_last;
            __seek_possibly_wrapped_iterator(__tail_it, __ptr);

            do {
                if (_predicate(_proj(*__tail_it))) {
                    _iterator = __tail_it;
                    return;
                }
                if (__tail_it == _iterator) break;
                --__tail_it;
            } while (true);
        }

        _iterator = _sentinel;
    }

    raze_nodiscard constexpr raze_always_inline _Iterator_ result() const noexcept {
        return _iterator;
    }
};

    template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
        class _Predicate_, class _Projection_ = std::identity>
    raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
        _Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
            requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
    {
        auto __unwrapped_first = type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first));

        __seek_possibly_wrapped_iterator(__first, __find_last_unchecked(__unwrapped_first,
            type_traits::__find_final_unwrapped_iterator<_Iterator_>(__unwrapped_first, std::move(__last)),
            type_traits::__pass_function(__pred), type_traits::__pass_function(__proj)));

        return __first;
    }

    template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
    constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
        _Range_&& __range, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
            requires(!constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
                _Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
    {
        auto __first = std::ranges::begin(__range);
        __seek_possibly_wrapped_iterator(__first, __find_last_unchecked(
            type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__first)), 
            type_traits::__unchecked_end(__range), type_traits::__pass_function(__pred),
            type_traits::__pass_function(__proj)));
        return __first;
    }

    template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
    constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(_Range_&& __range,
        _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
            requires(constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
                _Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
    {
        auto __first = std::ranges::begin(__range);
        __seek_possibly_wrapped_iterator(__first, __find_last_unchecked(
            type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__first)),
            type_traits::__unchecked_end(__range), type_traits::__pass_function(__pred),
            type_traits::__pass_function(__proj), std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{}));
        return __first;
    }
private:
    template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
    raze_nodiscard constexpr raze_always_inline _Iterator_ __find_last_unchecked(
        _Iterator_ __first, _Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj) const noexcept
    {
        using _Value_ = std::iter_value_t<_Iterator_>;

        __verify_range(__first, __last);
        auto __work = __impl(__first, __last, __pred, __proj);
        
        if constexpr (std::contiguous_iterator<_Iterator_> && vectorizable_unary_predicate<_Predicate_, _Iterator_> &&
            vectorizable_projection<_Projection_, _Iterator_>)
        {
            if not consteval {
                __seek_possibly_wrapped_iterator(__first, vx::__dispatch_sized_impl<
                    typename options::_Unroller<decltype(this->traits())>::__impl,
                    _Value_, _Iterator_>(algorithm::distance(__first, __last) * sizeof(_Value_), std::move(__work)));
                return __first;
            }
        }
        
        __seek_possibly_wrapped_iterator(__first, options::__unroller<decltype(this->traits()), vx::scalar_tag>(std::move(__work)));
        return __first;
    }

    template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_, sizetype _Size_>
    raze_nodiscard constexpr raze_always_inline _Iterator_ __find_last_unchecked(_Iterator_ __first, 
        _Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj,
        std::integral_constant<sizetype, _Size_> __size) const noexcept
    {
        using _Value_ = std::iter_value_t<_Iterator_>;

        __verify_range(__first, __last);
        auto __work = __impl(__first, __last, __pred, __proj);

        if constexpr (std::contiguous_iterator<_Iterator_> && vectorizable_unary_predicate<_Predicate_, _Iterator_>
            && vectorizable_projection<_Projection_, _Iterator_>)
        {
            if not consteval {
                __seek_possibly_wrapped_iterator(__first, vx::__dispatch_sized_impl<
                    typename options::_Unroller<decltype(this->traits())>::__impl,
                    _Value_, _Iterator_>(std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{}, std::move(__work)));
                return __first;
            }
        }
        
        __seek_possibly_wrapped_iterator(__first, options::__unroller<decltype(this->traits()), vx::scalar_tag>(std::move(__work)));
        return __first;
    }
};

constexpr inline auto find_last_if = raze::options::function_with_traits<_Find_last_if>;

template <class _Traits_>
struct _Find_last : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Value_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, const _Value_& __v, _Projection_ __proj = {}) const noexcept
	{
		return find_last_if(std::move(__first), std::move(__last), algorithm::equal_to(
			function_return_type<_Projection_, std::iter_value_t<_Iterator_>>(__v)),
			type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Value_,
		class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __range, const _Value_& __v, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_>)
	{
		return find_last_if(std::forward<_Range_>(__range), algorithm::equal_to(
			function_return_type<_Projection_, std::ranges::range_value_t<_Range_>>(__v)),
			type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Value_, 
		class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(_Range_&& __range,
		const _Value_& __v, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_>)
	{
		return find_last_if(std::forward<_Range_>(__range), algorithm::equal_to(
			function_return_type<_Projection_, std::ranges::range_value_t<_Range_>>(__v)),
			type_traits::__pass_function(__proj));
	}
};

constexpr inline auto find_last = raze::options::function_with_traits<_Find_last>;

template <class _Traits_>
struct _Find_last_if_not : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		return find_last_if(std::move(__first), std::move(__last), make_not_fn(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __range, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return find_last_if(std::forward<_Range_>(__range), make_not_fn(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(_Range_&& __range,
		_Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return find_last_if(std::forward<_Range_>(__range), make_not_fn(__pred), type_traits::__pass_function(__proj));
	}
};

constexpr inline auto find_last_if_not = raze::options::function_with_traits<_Find_last_if_not>;


__RAZE_ALGORITHM_NAMESPACE_END
