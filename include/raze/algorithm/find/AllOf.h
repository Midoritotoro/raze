#pragma once 


#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _All_of : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	struct __kernel {
		static constexpr auto can_enable_vectorization = std::contiguous_iterator<_Iterator_> &&
			vectorizable_unary_predicate<_Predicate_, _Iterator_> && vectorizable_projection<_Projection_, _Iterator_>;

		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Predicate_ _predicate;
		_Projection_ _proj;
		bool _result = true;

		constexpr explicit __kernel(_Iterator_ __it, _Sentinel_ __sent, _Predicate_ __pred, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _predicate(__pred), _proj(__proj)
		{}

		template <class _Tag_>
		raze_always_inline constexpr void operator()(_Tag_) noexcept
			requires(options::concepts::same_as<_Tag_, vx::scalar_tag>)
		{
			raze_disable_unrolling
			for (; _iterator != _sentinel; ++_iterator) {
				if (!_predicate(_proj(*_iterator))) {
					_result = false;
					return;
				}
			}
		}

		template <class _Tag_>
		raze_nodiscard raze_always_inline constexpr bool operator()(_Tag_, sizetype __aligned_size) noexcept
			requires(!options::concepts::same_as<_Tag_, vx::scalar_tag>)
		{
			auto* __ptr = std::to_address(_iterator);
			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			raze_disable_unrolling
			do {
				if (!vx::all_of(_predicate(_proj(vx::load<_Tag_>(__ptr))))) return false;
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
		auto __size = __bytes_distance(__first, __last);
		return __raze_unchecked_call(traits::__uiter<_Sentinel_>(std::move(__first)),
			traits::__usent<_Iterator_>(std::move(__last)), traits::__fwd_fn(__pred),
			traits::__fwd_fn(__proj), __size);
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline bool operator()(_Range_&& __range, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return __raze_unchecked_call(traits::__ubegin(__range), traits::__uend(__range),
			traits::__fwd_fn(__pred), traits::__fwd_fn(__proj), __bytes_distance(__range));
	}
private:
	__raze_define_unchecked(bool);
};

constexpr inline auto all_of = raze::options::function_with_traits<_All_of>;

__RAZE_ALGORITHM_NAMESPACE_END
