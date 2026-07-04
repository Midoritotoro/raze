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
struct _Is_sorted: _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Comp_, class _Projection_>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Comp_ _comp;
		_Projection_ _proj;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Comp_ __comp, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _comp(__comp), _proj(__proj)
		{}

		template <class _Tag_>
		raze_always_inline raze_nodiscard constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;

			auto __next = _iterator;

			if (++__next == _sentinel) {
				_iterator = __next;
				return true;
			}

			if (_comp(_proj(*__next), _proj(*_iterator))) {
				_iterator = __next;
				return true;
			}

			_iterator = __next;
			return false;
		}

		raze_nodiscard constexpr raze_always_inline bool result() const noexcept {
			return _iterator == _sentinel;
		}
	};

	template <class _Tag_>
	struct __vectorized_is_sorted {
		template <class _Iterator_, class _Sentinel_, class _Comp_, class _Projection_>
		raze_nodiscard raze_always_inline bool operator()(_Iterator_ __first, _Sentinel_ __sentinel, _Comp_ __comp,
			_Projection_ __proj) const noexcept
		{
			if (__first == __sentinel) return true;

			for (auto __next = __first; ++__next != __sentinel; __first = __next)
				if (__comp(__proj(*__next), __proj(*__first)))
					return false;

			return true;
		}

		template <class _Iterator_, class _Sentinel_, class _Comp_, class _Projection_>
		raze_nodiscard raze_always_inline bool operator()(sizetype __aligned_size,
			sizetype __tail_size, _Iterator_ __first, _Sentinel_ __sentinel, _Comp_ __comp,
			_Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __ptr = std::to_address(__first);
			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			if (__aligned_size >= sizeof(_Tag_) * 2) {
#if defined(raze_cpp_msvc_only)
				do {
					const auto __current = __proj(vx::load<_Tag_>(__ptr));
					const auto __next_block = __proj(vx::load<_Tag_>(__ptr + _Tag_::size()));

					const auto __next = vx::slide_left_merge(__current, __next_block, std::integral_constant<sizetype, 1>{});

					if (vx::any_of(__comp(__next, __current)))
						return false;

					__advance_bytes(__ptr, sizeof(_Tag_));
				} while (__ptr + _Tag_::size() != __aligned_end);
#else
				auto __first_loaded = __proj(vx::load<_Tag_>(__ptr));
				do {
					const auto __second_loaded = __proj(vx::load<_Tag_>(__ptr + _Tag_::size()));
					const auto __current = __first_loaded;
					const auto __next = vx::slide_left_merge(__current, __second_loaded, std::integral_constant<sizetype, 1>{});

					if (vx::any_of(__comp(__next, __current))) return false;

					__first_loaded = __second_loaded;
					__advance_bytes(__ptr, sizeof(_Tag_));
				} while (__ptr + _Tag_::size() != __aligned_end);
#endif
			}

			__seek_possibly_wrapped_iterator(__first, __ptr);
			return (*this)(__first, __sentinel, __comp, __proj);
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Comp_ = std::less<>, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline bool operator()(_Iterator_ __first,
		_Sentinel_ __last, _Comp_ __comp = {}, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_strict_weak_order<_Comp_, std::projected<_Iterator_, _Projection_>>)
	{
		return __is_sorted_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)),
			type_traits::__pass_function(__comp), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Comp_ = std::less<>, class _Projection_ = std::identity>
	constexpr raze_always_inline bool operator()(
		_Range_&& __range, _Comp_ __comp = {}, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_> && std::indirect_strict_weak_order<_Comp_,
				std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return __is_sorted_unchecked(type_traits::__unchecked_begin(__range),
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__comp),
			type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Comp_ = std::less<>, class _Projection_ = std::identity>
	constexpr raze_always_inline bool operator()(_Range_&& __range,
		_Comp_ __comp, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_> && std::indirect_strict_weak_order<_Comp_,
				std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return __is_sorted_unchecked(type_traits::__unchecked_begin(__range),
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__comp),
			type_traits::__pass_function(__proj), std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});
	}
private:
	template <class _Iterator_, class _Sentinel_, class _Comp_, class _Projection_>
	raze_nodiscard constexpr raze_always_inline bool __is_sorted_unchecked(
		_Iterator_ __first, _Sentinel_ __last, _Comp_ __comp, _Projection_ __proj) const noexcept
	{
		__verify_range(__first, __last);
		
		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (std::contiguous_iterator<_Iterator_> && vectorizable_binary_predicate<_Comp_, _Iterator_> &&
			vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_is_sorted, _Value_, bool>(
					algorithm::distance(__first, __last) * sizeof(_Value_), __first, __last, __comp, __proj);
			}
		}
		
		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(__first, __last, __comp, __proj));
	}

	template <class _Iterator_, class _Sentinel_, class _Comp_, class _Projection_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline bool __is_sorted_unchecked(_Iterator_ __first,
		_Sentinel_ __last, _Comp_ __comp, _Projection_ __proj, std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (std::contiguous_iterator<_Iterator_> && vectorizable_binary_predicate<_Comp_, _Iterator_>
			&& vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{};
				return vx::__dispatch_sized_impl<__vectorized_is_sorted,
					_Value_, bool>(__bytes, __first, __last, __comp, __proj);
			}
		}
		
		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __comp, __proj));
	}
};

constexpr inline auto is_sorted = raze::options::function_with_traits<_Is_sorted>;

__RAZE_ALGORITHM_NAMESPACE_END
