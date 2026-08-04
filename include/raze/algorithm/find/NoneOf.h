#pragma once 

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _None_of : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	struct __kernel {
		static constexpr auto vectorizable = std::contiguous_iterator<_Iterator_> &&
			vectorizable_unary_predicate<_Predicate_, _Iterator_>&&
			vectorizable_projection<_Projection_, _Iterator_>;

		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Predicate_ _predicate;
		_Projection_ _proj;
		bool _result = true;

		constexpr explicit __kernel(_Iterator_ __it, _Sentinel_ __sent, _Predicate_ __pred, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _predicate(__pred), _proj(__proj)
		{}

		template <scalar_tag _Tag_>
		raze_always_inline constexpr void operator()(_Tag_) noexcept {
			raze_disable_unrolling
			for (; _iterator != _sentinel; ++_iterator) {
				if (_predicate(_proj(*_iterator))) {
					_result = false;
					return;
				}
			}
		}

		template <vectorizable_tag _Tag_>
		raze_nodiscard raze_always_inline constexpr bool operator()(_Tag_, sizetype __aligned_size) noexcept {
			auto* __ptr = std::to_address(_iterator);
			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			raze_disable_unrolling
			do {
				if (vx::any_of(_predicate(_proj(raze::vx::load<_Tag_>(__ptr))))) 
					return _result = false;

				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			__seek_iter(_iterator, __ptr);
			return true;
		}

		raze_nodiscard constexpr raze_always_inline bool result() const noexcept {
			return _result;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline bool operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		return __raze_kernel_dispatch_call(std::move(__first), std::move(__last),
			traits::__fwd_fn(__pred), traits::__fwd_fn(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline bool operator()(_Range_&& __r, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return __raze_kernel_dispatch_call(std::forward<_Range_>(__r),
			traits::__fwd_fn(__pred), traits::__fwd_fn(__proj));
	}
private:
	__raze_define_kernel_dispatch()
};

constexpr inline auto none_of = raze::options::function_with_traits<_None_of>;

__RAZE_ALGORITHM_NAMESPACE_END
