#pragma once 

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Equal : _Traits_ {
	template <source _Source1_, source _Source2_,
		class _Predicate_, class _Projection1_, class _Projection2_>
	struct __kernel {
		using source1_type = std::remove_cvref_t<_Source1_>;
		using source2_type = std::remove_cvref_t<_Source2_>;

		using unchecked_iterator1_type = typename source1_type::unchecked_iterator_type;
		using unchecked_iterator2_type = typename source2_type::unchecked_iterator_type;

		using unchecked_sentinel1_type = typename source1_type::unchecked_sentinel_type;
		using vector_value_type = std::iter_value_t<unchecked_iterator1_type>;

		_Source1_ _source1;
		_Source2_ _source2;

		unchecked_iterator1_type _iterator1;
		unchecked_iterator2_type _iterator2;
		unchecked_sentinel1_type _sentinel1;

		_Predicate_ _predicate;
		_Projection1_ _proj1;
		_Projection2_ _proj2;

		bool _result = true;

		constexpr explicit __kernel(_Source1_&& __src1, _Source2_&& __src2, _Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2) noexcept:
			_source1(std::forward<_Source1_>(__src1)), _source2(std::forward<_Source2_>(__src2)), _predicate(__pred), _proj1(__proj1), _proj2(__proj2)
		{
			_iterator1 = _source1.ubegin();
			_iterator2 = _source2.ubegin();
			_sentinel1 = _source1.uend();
		}

		template <scalar_tag _Tag_>
		constexpr raze_always_inline void operator()(_Tag_) noexcept {
			raze_disable_unrolling
			for (; _iterator1 != _sentinel1; ++_iterator1, ++_iterator2) {
				if (!_predicate(_proj1(*_iterator1), _proj2(*_iterator2))) {
					_result = false;
					break;
				}
			}
		}

		template <vectorizable_tag _Tag_>
		constexpr raze_always_inline bool operator()(_Tag_, sizetype __aligned_size) noexcept {
			auto* __ptr1 = std::to_address(_iterator1);
			auto* __ptr2 = std::to_address(_iterator2);

			const auto __aligned_end = __bytes_pointer_offset(__ptr1, __aligned_size);
			
			raze_disable_unrolling
			do {
				const auto __mask = _predicate(_proj1(vx::load<_Tag_>(__ptr1)), _proj2(vx::load<_Tag_>(__ptr2)));

				if (!vx::all_of(__mask))
					return _result = false;

				__advance_bytes(__ptr1, __ptr2, sizeof(_Tag_));
			} while (__ptr1 != __aligned_end);

			_Source1_::from_ptr(_iterator1, __ptr1);
			_Source2_::from_ptr(_iterator2, __ptr2);

			return true;
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() noexcept 
			requires(constexpr_sized_source<source1_type> || constexpr_sized_source<source2_type>)
		{
			if constexpr (constexpr_sized_source<source1_type>) return source1_type::static_size();
			else return source2_type::static_size();
		}

		raze_nodiscard constexpr raze_always_inline auto size() const noexcept {
			return _source1.size();
		}

		raze_nodiscard constexpr raze_always_inline bool exit() const noexcept {
			return _source1.size() != _source2.size();
		}

		raze_nodiscard constexpr raze_always_inline bool result() const noexcept {
			return _result;
		}

		raze_nodiscard constexpr raze_always_inline bool default_result() const noexcept {
			return false;
		}

		static consteval bool vectorizable() noexcept {
			return options::concepts::same_as<std::iter_value_t<unchecked_iterator1_type>, std::iter_value_t<unchecked_iterator2_type>> &&
				std::contiguous_iterator<unchecked_iterator1_type> && std::contiguous_iterator<unchecked_iterator2_type> &&
				vectorizable_binary_predicate<_Predicate_, unchecked_iterator1_type, unchecked_iterator2_type> &&
				vectorizable_projection<_Projection1_, unchecked_iterator1_type> && vectorizable_projection<_Projection2_, unchecked_iterator2_type>;
		}
	};

	template <std::input_iterator _Iterator1_, std::sentinel_for<_Iterator1_> _Sentinel1_,
		std::input_iterator _Iterator2_, std::sentinel_for<_Iterator2_> _Sentinel2_,
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline bool operator()(_Iterator1_ __first1,
		_Sentinel1_ __sent1, _Iterator2_ __first2, _Sentinel2_ __sent2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(std::indirectly_comparable<_Iterator1_, _Sentinel1_, _Predicate_, _Projection1_, _Projection2_>)
	{
		return __raze_kernel_dispatch_call(get_source(std::move(__first1), std::move(__sent1)),
			get_source(std::move(__first2), std::move(__sent2)), traits::__fwd_fn(__pred),
			traits::__fwd_fn(__proj1), traits::__fwd_fn(__proj2));
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_, 
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline bool operator()(_Range1_&& __r1, _Range2_&& __r2, 
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(std::indirectly_comparable<std::ranges::iterator_t<_Range1_>,
				std::ranges::iterator_t<_Range2_>, _Predicate_, _Projection1_, _Projection2_>)
	{
		return __raze_kernel_dispatch_call(get_source(std::forward<_Range1_>(__r1)),
			get_source(std::forward<_Range2_>(__r2)), traits::__fwd_fn(__pred),
			traits::__fwd_fn(__proj1), traits::__fwd_fn(__proj2));
	}
private:
	__raze_define_kernel_dispatch()
};

constexpr inline auto equal = raze::options::function_with_traits<_Equal>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
