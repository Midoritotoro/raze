#pragma once 

#include <src/raze/algorithm/unchecked/find/FindUnchecked.h>
#include <src/raze/algorithm/unchecked/find/FindIfUnchecked.h>
#include <src/raze/algorithm/unchecked/find/FindIfNotUnchecked.h>

#include <raze/concurrency/Execution.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Find : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Value_, class _Projection_>
	struct __fallback_impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Value_ _value;
		_Projection_ _proj;

		raze_no_stack_protector constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Value_ __v, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _value(__v), _proj(__proj)
		{}

		raze_no_stack_protector raze_always_inline raze_nodiscard constexpr bool operator()() noexcept {
			//if constexpr (std::is_same_v<_Projection_, std::identity> && std::sized_sentinel_for<_Sentinel_, _Iterator_>
			//	&& type_traits::__is_vectorized_find_algorithm_safe_v<_Iterator_, _Value_>) 
			//{
			//	if not consteval {
			//		const auto __first_address = std::to_address(__first);
			//		const auto __position = __find_vectorized(__first_address, std::to_address(__last), __value);

			//		if constexpr (std::is_pointer_v<_Iterator_>) return __position;
			//		else return __first + (__position - __first_address);
			//	}
			//}

			if (_iterator == _sentinel) return true;
			else if (_proj(*_iterator) == _value) return true;
			else {
				++_iterator; 
				return false;
			}
		}

		raze_nodiscard constexpr raze_always_inline _Iterator_ result() const noexcept {
			return _iterator;
		}
	};

	template <class _Simd_, class _Iterator_, class _Sentinel_, class _Value_, class _Projection_>
	struct __simd_impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Value_ _value;
		_Projection_ _proj;

		raze_no_stack_protector constexpr explicit __impl(_Simd_, _Iterator_ __it, _Sentinel_ __sent, _Value_ __v, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _value(__v), _proj(__proj)
		{}

		raze_no_stack_protector raze_always_inline raze_nodiscard constexpr bool operator()() noexcept {
			if (_iterator == _sentinel) return true;
			else if (_proj(*_iterator) == _value) return true;
			else {
				++_iterator;
				return false;
			}
		}

		raze_nodiscard constexpr raze_always_inline _Iterator_ result() const noexcept {
			return _iterator;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Value_ = std::iter_value_t<_Iterator_>, class _Projection_ = std::identity>
	raze_no_stack_protector constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, const std::type_identity_t<_Value_>& __value, _Projection_ __proj = {}) const noexcept
	{
		const auto __first_unwrapped = type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first));
		const auto __last_unwrapped = type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last));

		__verify_range(__first_unwrapped, __last_unwrapped);

		auto __fallback_work = __fallback_impl(__first_unwrapped, __last_unwrapped, __value, __proj);
		using _Work = decltype(__fallback_work);

		auto __dispatched = vx::__dispatch_sized_impl<typename options::_Unroller<_Traits_>::__impl, _Value_>(
			std::ranges::distance(__first, __last), options::__unroller<_Traits_, void>, __first_unwrapped,
			__last_unwrapped, __value, __proj);

		__seek_possibly_wrapped_iterator(__first, __dispatched);
		return __first;
	}

	//template <std::ranges::input_range _Range_, class _Value_ = std::ranges::range_value_t<_Range_>, class _Projection_ = std::identity>
	//raze_no_stack_protector constexpr raze_always_inline std::ranges::iterator_t<_Range_> operator()(
	//	_Range_&& __range, const std::type_identity_t<_Value_>& __value, _Projection_ __proj = {}) const noexcept
	//{
	//	return __find_unchecked(std::ranges::begin(__range), std::ranges::end(__range),
	//		__value, type_traits::__pass_function(__proj));
	//}
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
