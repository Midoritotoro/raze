#pragma once 

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _All_of : _Traits_ {
	template <class _Source_, class _Predicate_, class _Projection_>
	struct __kernel {
		using source_type = std::remove_cvref_t<_Source_>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		_Source_ _source;
		unchecked_iterator_type _iterator;
		unchecked_sentinel_type _sentinel;
		_Predicate_ _predicate;
		_Projection_ _proj;
		bool _result = true;

		constexpr explicit __kernel(_Source_&& __src, _Predicate_ __pred, _Projection_ __proj) noexcept :
			_source(std::forward<_Source_>(__src)), _predicate(__pred), _proj(__proj)
		{
			_iterator = _source.ubegin();
			_sentinel = _source.uend();
		}

		template <scalar_tag _Tag_>
		raze_always_inline constexpr void operator()(_Tag_) noexcept {
			raze_disable_unrolling
			for (; _iterator != _sentinel; ++_iterator) {
				if (!_predicate(_proj(*_iterator))) {
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
				if (!vx::all_of(_predicate(_proj(vx::load<_Tag_>(__ptr)))))
					return _result = false;

				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			source_type::from_ptr(_iterator, __ptr);
			return true;
		}

		raze_nodiscard constexpr raze_always_inline auto get_size() const noexcept {
			return _source.size();
		}

		raze_nodiscard constexpr raze_always_inline bool result() const noexcept {
			return _result;
		}

		static consteval bool vectorizable() noexcept {
			return std::contiguous_iterator<unchecked_iterator_type> &&
				vectorizable_unary_predicate<_Predicate_, unchecked_iterator_type> &&
				vectorizable_projection<_Projection_, unchecked_iterator_type>;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline bool operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		return __raze_kernel_dispatch_call(get_source(std::move(__first), std::move(__last)),
			traits::__fwd_fn(__pred), traits::__fwd_fn(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline bool operator()(_Range_&& __r, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return __raze_kernel_dispatch_call(get_source(std::forward<_Range_>(__r)), 
			traits::__fwd_fn(__pred), traits::__fwd_fn(__proj));
	}
private:
	__raze_define_kernel_dispatch()
};

constexpr inline auto all_of = raze::options::function_with_traits<_All_of>;

__RAZE_ALGORITHM_NAMESPACE_END
