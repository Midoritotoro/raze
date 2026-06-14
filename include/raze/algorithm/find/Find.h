#pragma once 

#include <src/raze/algorithm/unchecked/find/FindUnchecked.h>
#include <src/raze/algorithm/unchecked/find/FindIfUnchecked.h>
#include <src/raze/algorithm/unchecked/find/FindIfNotUnchecked.h>

#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Find : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Value_, class _Projection_>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Value_ _value;
		_Projection_ _proj;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Value_ __v, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _value(__v), _proj(__proj)
		{}

		template <class _Tag_>
		raze_always_inline raze_nodiscard constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;
			else if (_proj(*_iterator) == _value) return true;
			else {
				++_iterator;
				return false;
			}
		}

		template <vx::simd_type _Tag_>
		raze_always_inline raze_nodiscard constexpr bool operator()(_Tag_) noexcept {
			if (std::ranges::distance(_iterator, _sentinel) < _Tag_::size())
				return true;

			auto __addr = std::to_address(_iterator);

			const auto __loaded = raze::vx::load<_Tag_>(__addr);
			const auto __mask = _value == __loaded;

			if (raze::vx::any_of(__mask)) {
				_iterator = __addr + raze::vx::find_first_set(__mask);
				return true;
			}

			__advance_bytes(__addr, sizeof(_Tag_));
			_iterator = __addr;

			return false;
		}

		raze_nodiscard constexpr raze_always_inline _Iterator_ result() const noexcept {
			return _iterator;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Value_ = std::iter_value_t<_Iterator_>, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, const std::type_identity_t<_Value_>& __v, _Projection_ __proj = {}) const noexcept
	{
		__seek_possibly_wrapped_iterator(__first, __find_unchecked(
			type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)), __v, __proj));

		return __first;
	}

	template <std::ranges::input_range _Range_, class _Value_ = std::ranges::range_value_t<_Range_>, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::iterator_t<_Range_> operator()(
		_Range_&& __range, const std::type_identity_t<_Value_>& __v, _Projection_ __proj = {}) const noexcept
	{
		return __find_unchecked(std::ranges::begin(__range), std::ranges::end(__range),
			__v, type_traits::__pass_function(__proj));
	}
private:
	template <class _Iterator_, class _Sentinel_, class _Value_, class _Projection_>
	raze_nodiscard constexpr raze_always_inline _Iterator_ __find_unchecked(
		_Iterator_ __first, _Sentinel_ __last, const _Value_& __v, _Projection_ __proj) const noexcept
	{
		__verify_range(__first, __last);

		auto __work_tuple = std::make_tuple(__impl(__first, __last, __v, __proj));
		return vx::__dispatch_sized_impl<typename options::_Unroller<decltype(this->traits())>::__impl, 
			_Value_, _Iterator_>(std::ranges::distance(__first, __last), __work_tuple, __work_tuple);
	}
};

constexpr inline auto find = raze::options::function_with_traits<_Find>;

template <
	class _InputIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _InputIterator_ find_if_not(
	_InputIterator_	__first, 
	_InputIterator_	__last, 
	_Predicate_		__predicate) noexcept(
		std::is_nothrow_invocable_v<
			_Predicate_, std::iter_value_t<_InputIterator_>>)
{
	__verify_range(__first, __last);

	__seek_possibly_wrapped_iterator(__first, __find_if_not_unchecked(__unwrap_iterator(__first),
		__unwrap_iterator(__last), type_traits::__pass_function(__predicate)));

	return __first;
}

template <
	class _InputIterator_,
	class _Predicate_>
__simd_nodiscard_inline_constexpr _InputIterator_ find_if(
	_InputIterator_	__first,
	_InputIterator_	__last,
	_Predicate_		__predicate) noexcept(
		std::is_nothrow_invocable_v<
			_Predicate_, std::iter_value_t<_InputIterator_>>)
{
	__verify_range(__first, __last);

	__seek_possibly_wrapped_iterator(__first, __find_if_unchecked(__unwrap_iterator(__first),
		__unwrap_iterator(__last), type_traits::__pass_function(__predicate)));

	return __first;
}

//template <
//	class _ExecutionPolicy_,
//	class _Iterator_,
//	class _Type_ = std::iter_value_t<_Iterator_>,
//	concurrency::enable_if_execution_policy<_ExecutionPolicy_> = 0>
//raze_nodiscard _Iterator_ find(
//	_ExecutionPolicy_&&,
//	_Iterator_											__first,
//	_Iterator_											__last,
//	const typename std::type_identity<_Type_>::type&	__value) noexcept;

__RAZE_ALGORITHM_NAMESPACE_END
