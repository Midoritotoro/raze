#pragma once 

#include <raze/RazeNamespace.h>
#include <raze/compatibility/CxxVersionDetection.h>

#include <type_traits>
#include <xutility>
#include <ranges>
#include <concepts>


__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN


#if raze_has_cxx20
	template <class _Iterator_>
	using iterator_reference_type	= std::iter_reference_t<_Iterator_>;

	template <class _Iterator_>
	using iterator_value_type		= std::iter_value_t<_Iterator_>;

	template <class _Iterator_>
	using iterator_difference_type	= std::iter_difference_t<_Iterator_>;
#else
	template <class _Iterator_>
	using iterator_reference_type   = typename std::iterator_traits<_Iterator_>::reference;

	template <class _Iterator_>
	using iterator_value_type       = typename std::iterator_traits<_Iterator_>::value_type;

	template <class _Iterator_>
	using iterator_difference_type  = typename std::iterator_traits<_Iterator_>::difference_type;
#endif // raze_has_cxx20


#if raze_has_cxx20
	template <class _Iterator_>
	constexpr bool is_iterator_contiguous_v = std::contiguous_iterator<_Iterator_>;
#else
	template <class _Iterator_>
	constexpr bool is_iterator_contiguous_v = std::is_pointer_v<_Iterator_>;
#endif

template <
	class _FirstIterator_,
	class _SecondIterator_>
constexpr bool are_iterators_contiguous = is_iterator_contiguous_v<_FirstIterator_> 
	&& is_iterator_contiguous_v<_SecondIterator_>;

template <class _Iterator_>
constexpr inline bool is_iterator_volatile_v = std::is_volatile_v<std::remove_reference_t<std::iter_reference_t<_Iterator_>>>;

template <
	class _Type_,
	class = void>
constexpr inline bool is_iterator_v = false;

template <class _Type_>
constexpr inline bool is_iterator_v<_Type_, std::void_t<
	typename std::iterator_traits<_Type_>::iterator_category>> = true;

template <class _Iterator_>
constexpr inline bool is_iterator_input_cxx17_v = std::is_convertible_v<
	typename std::iterator_traits<_Iterator_>::iterator_category, std::input_iterator_tag>;

template <class _Iterator_>
constexpr inline bool is_iterator_input_ranges_v =
#if raze_has_cxx20
    (std::input_iterator<_Iterator_> && std::sentinel_for<_Iterator_, _Iterator_>) ||
#endif // raze_has_cxx20
	is_iterator_input_cxx17_v<_Iterator_>;


template <class _Iterator_>
constexpr inline bool is_iterator_bidirectional_cxx17_v = std::is_convertible_v<
	typename std::iterator_traits<_Iterator_>::iterator_category, std::bidirectional_iterator_tag>;

template <class _Iterator_>
constexpr inline bool is_iterator_bidirectional_ranges_v =
#if raze_has_cxx20
    std::bidirectional_iterator<_Iterator_> ||
#endif // raze_has_cxx20
	is_iterator_bidirectional_cxx17_v<_Iterator_>;


template <class _Iterator_>
constexpr inline bool is_iterator_random_cxx17_v = std::is_convertible_v<
	typename std::iterator_traits<_Iterator_>::iterator_category, std::random_access_iterator_tag>;

template <class _Iterator_>
constexpr inline bool is_iterator_random_ranges_v =
#if raze_has_cxx20
    std::random_access_iterator<_Iterator_> ||
#endif // raze_has_cxx20
	is_iterator_random_cxx17_v<_Iterator_>;


template <class _Iterator_>
constexpr inline bool is_iterator_forward_cxx17_v = std::is_convertible_v<
    typename std::iterator_traits<_Iterator_>::iterator_category, std::forward_iterator_tag>;

template <class _Iterator_>
constexpr inline bool is_iterator_forward_ranges_v =
#if raze_has_cxx20
    std::forward_iterator<_Iterator_> ||
#endif // raze_has_cxx20
	is_iterator_forward_cxx17_v<_Iterator_>;


template <class _Iterator_>
constexpr inline bool is_iterator_parallel_v = is_iterator_forward_ranges_v<_Iterator_>;

#if !defined(__raze_require_parallel_iterator)
#define __raze_require_parallel_iterator(_Iterator_) \
    static_assert(raze::type_traits::is_iterator_parallel_v<_Iterator_>, "Parallel algorithms require forward iterators or stronger.")
#endif // !defined(__raze_require_parallel_iterator)

template <class _Iterator_, class = void>
constexpr bool __allow_inheriting_unwrap_v = true;

template <class _Iterator_>
constexpr bool __allow_inheriting_unwrap_v<_Iterator_, std::void_t<typename _Iterator_::_Prevent_inheriting_unwrap>> =
    std::is_same_v<_Iterator_, typename _Iterator_::_Prevent_inheriting_unwrap>;

template <class _Iterator_, class _Sentinel_ = _Iterator_, class = void>
constexpr bool __is_range_verifiable_v = false;

template <class _Iterator_, class _Sentinel_>
constexpr bool __is_range_verifiable_v<_Iterator_, _Sentinel_, std::void_t<decltype(
	__verify_range__(std::declval<const _Iterator_&>(), std::declval<const _Sentinel_&>()))>> 
        = __allow_inheriting_unwrap_v<_Iterator_>;

template <class _Iterator_, class = void>
constexpr bool __is_iterator_unwrappable_v = false;

template <class _Iterator_>
constexpr bool __is_iterator_unwrappable_v<_Iterator_,
    std::void_t<decltype(std::declval<std::remove_cvref_t<_Iterator_>&>()._Seek_to(std::declval<_Iterator_>()._Unwrapped()))>> =
    __allow_inheriting_unwrap_v<std::remove_cvref_t<_Iterator_>>;

template <class _Iterator_> 
constexpr bool __is_iterator_unwrapped_v = !__is_iterator_unwrappable_v<_Iterator_>;

template <class _Iterator_, class = void>
constexpr bool __is_nothrow_unwrappable_v = false;

template <class _Iterator_>
constexpr bool __is_nothrow_unwrappable_v<_Iterator_, std::void_t<decltype(std::declval<_Iterator_>()._Unwrapped())>> =
    noexcept(std::declval<_Iterator_>()._Unwrapped());

template <class _Iterator_, class = bool>
constexpr bool __can_unwrap_when_unverified_v = false;

template <class _Iterator_>
constexpr bool __can_unwrap_when_unverified_v<_Iterator_, decltype(static_cast<bool>(_Iterator_::_Unwrap_when_unverified))> =
    static_cast<bool>(_Iterator_::_Unwrap_when_unverified);

template <class _Iterator_>
constexpr bool __is_possibly_unverified_iterator_unwrappable_v =
    __is_iterator_unwrappable_v<_Iterator_> && __can_unwrap_when_unverified_v<std::remove_cvref_t<_Iterator_>>;

template <class _Iterator_, class = void>
constexpr bool __is_offset_verifiable_v = false;

template <class _Iterator_>
constexpr bool __is_offset_verifiable_v
    <_Iterator_, std::void_t<decltype(std::declval<const _Iterator_&>()._Verify_offset(type_traits::iterator_difference_type<_Iterator_>{}))>> = true;

template <class _Iterator_, class = void>
constexpr bool __is_offset_nothrow_verifiable_v = false;

template <class _Iterator_>
constexpr bool __is_offset_nothrow_verifiable_v
    <_Iterator_, std::void_t<decltype(std::declval<const _Iterator_&>()._Verify_offset(type_traits::iterator_difference_type<_Iterator_>{}))>> = 
        noexcept(std::declval<const _Iterator_&>()._Verify_offset(type_traits::iterator_difference_type<_Iterator_>{}));

template <class _Iterator_>
constexpr bool __is_iterator_unwrappable_for_offset_v = 
    __is_iterator_unwrappable_v<_Iterator_> && __is_offset_verifiable_v<std::remove_cvref_t<_Iterator_>>;

template <class _Iterator_>
constexpr bool __is_iterator_nothrow_unwrappable_for_offset_v = 
    __is_nothrow_unwrappable_v<_Iterator_> && __is_offset_nothrow_verifiable_v<std::remove_cvref_t<_Iterator_>>;

template <class _Iterator_, class _UnwrappedIterator_, class = void>
constexpr bool __is_wrapped_iterator_seekable_v = false;

template <class _Iterator_, class _UnwrappedIterator_>
constexpr bool __is_wrapped_iterator_seekable_v
    <_Iterator_, _UnwrappedIterator_, std::void_t<decltype(std::declval<_Iterator_&>()._Seek_to(std::declval<_UnwrappedIterator_>()))>> = true;

template <class _Iterator_, class _UnwrappedIterator_, class = void>
constexpr bool __is_wrapped_iterator_nothrow_seekable_v = false;

template <class _Iterator_, class _UnwrappedIterator_>
constexpr bool __is_wrapped_iterator_nothrow_seekable_v
    <_Iterator_, _UnwrappedIterator_, std::void_t<decltype(std::declval<_Iterator_&>()._Seek_to(std::declval<_UnwrappedIterator_>()))>> = 
        noexcept(std::declval<_Iterator_&>()._Seek_to(std::declval<_UnwrappedIterator_>()));


#if !defined(__verify_unchecked)
#  define __verify_unchecked(_Iterator) static_assert(raze::type_traits::__is_iterator_unwrapped_v<_Iterator>, "Iterators in unchecked-functions must be unwrapped. ");
#endif // !defined(__verify_unchecked)

template <class _Wrapped>
concept __weakly_unwrappable = __allow_inheriting_unwrap_v<std::remove_cvref_t<_Wrapped>>
	&& requires(_Wrapped&& __wrapped) { std::forward<_Wrapped>(__wrapped)._Unwrapped(); };

template <class _Sentinel_>
concept __weakly_unwrappable_sentinel = __weakly_unwrappable<const std::remove_reference_t<_Sentinel_>&>;

template <class _Iterator_>
concept __weakly_unwrappable_iterator = __weakly_unwrappable<_Iterator_> && \
	requires(_Iterator_&& __it, std::remove_cvref_t<_Iterator_>& __mutable_it) {
		__mutable_it._Seek_to(std::forward<_Iterator_>(__it)._Unwrapped());
	};

template <class _Sentinel_, class _Iterator_>
concept __unwrappable_sentinel_for = __weakly_unwrappable_sentinel<_Sentinel_> && __weakly_unwrappable_iterator<_Iterator_> && 
	requires(_Iterator_&& __it, const std::remove_reference_t<_Sentinel_>& __sentinel) {
		{ __sentinel._Unwrapped() } -> std::sentinel_for<decltype(std::forward<_Iterator_>(__it)._Unwrapped())>;
	};

template <class _Sentinel_, class _Iterator_>
raze_nodiscard raze_always_inline constexpr decltype(auto) __ranges_unwrap_iterator(_Iterator_&& __iterator)
	noexcept(!__unwrappable_sentinel_for<_Sentinel_, _Iterator_> || __is_nothrow_unwrappable_v<_Iterator_>)
{
	static_assert(std::sentinel_for<std::remove_cvref_t<_Sentinel_>, std::remove_cvref_t<_Iterator_>>);

	if constexpr (std::is_pointer_v<std::remove_cvref_t<_Iterator_>>) return __iterator + 0;
	else if constexpr (__unwrappable_sentinel_for<_Sentinel_, _Iterator_>) return static_cast<_Iterator_&&>(__iterator)._Unwrapped();
	else return static_cast<_Iterator_&&>(__iterator);
}

template <class _Iterator_, class _Sentinel_>
raze_nodiscard raze_always_inline constexpr decltype(auto) __ranges_unwrap_sentinel(_Sentinel_&& __sentinel)
	noexcept(!__unwrappable_sentinel_for<_Sentinel_, _Iterator_> || __is_nothrow_unwrappable_v<_Sentinel_>) 
{
	static_assert(std::sentinel_for<std::remove_cvref_t<_Sentinel_>, std::remove_cvref_t<_Iterator_>>);

	if constexpr (std::is_pointer_v<std::remove_cvref_t<_Sentinel_>>) return __sentinel + 0;
	else if constexpr (__unwrappable_sentinel_for<_Sentinel_, _Iterator_>) return static_cast<_Sentinel_&&>(__sentinel)._Unwrapped();
	else return static_cast<_Sentinel_&&>(__sentinel);
}

template <std::ranges::range _Range_, class _Iterator_>
raze_nodiscard raze_always_inline constexpr decltype(auto) __ranges_unwrap_range_iterator(_Iterator_&& __iterator)
	noexcept(noexcept(__ranges_unwrap_iterator<std::ranges::sentinel_t<_Range_>>(static_cast<_Iterator_&&>(__iterator))))
{
	static_assert(std::same_as<std::remove_cvref_t<_Iterator_>, std::ranges::iterator_t<_Range_>>);
	return __ranges_unwrap_iterator<std::ranges::sentinel_t<_Range_>>(static_cast<_Iterator_&&>(__iterator));
}

template <std::ranges::range _Range_, class _Sentinel_>
raze_nodiscard raze_always_inline constexpr decltype(auto) __ranges_unwrap_range_sentinel(_Sentinel_&& __sentinel)
	noexcept(noexcept(__ranges_unwrap_sentinel<std::ranges::iterator_t<_Range_>>(static_cast<_Sentinel_&&>(__sentinel))))
{
	static_assert(std::same_as<std::remove_cvref_t<_Sentinel_>, std::ranges::sentinel_t<_Range_>>);
	return __ranges_unwrap_sentinel<std::ranges::iterator_t<_Range_>>(static_cast<_Sentinel_&&>(__sentinel));
}

template <class _Iterator_, class _Sentinel_>
using __ranges_unwrap_iter_t = std::remove_cvref_t<decltype(__ranges_unwrap_iterator<_Sentinel_>(std::declval<_Iterator_>()))>;

template <class _Sentinel_, class _Iterator_>
using __ranges_unwrap_sent_t = std::remove_cvref_t<decltype(__ranges_unwrap_sentinel<_Iterator_>(std::declval<_Sentinel_>()))>;

template <std::ranges::range _Range_>
using __unwrapped_iterator_t = __ranges_unwrap_iter_t<std::ranges::iterator_t<_Range_>, std::ranges::sentinel_t<_Range_>>;

template <std::ranges::range _Range_>
using __unwrapped_sentinel_t = __ranges_unwrap_sent_t<std::ranges::sentinel_t<_Range_>, std::ranges::iterator_t<_Range_>>;

#if defined(raze_cpp_msvc)
  constexpr inline auto __unchecked_begin = std::ranges::_Ubegin;
  constexpr inline auto __unchecked_end = std::ranges::_Uend;
#else 
  constexpr inline auto __unchecked_begin = std::identity{};
  constexpr inline auto __unchecked_end = std::identity{};
#endif // defined(raze_cpp_msvc)

template <std::forward_iterator _Iterator_, class _Sentinel_>
raze_nodiscard raze_always_inline constexpr __ranges_unwrap_iter_t<_Iterator_, _Sentinel_> __find_final_unwrapped_iterator(
	const __ranges_unwrap_iter_t<_Iterator_, _Sentinel_>& __first, _Sentinel_&& __last) 
		requires(std::sentinel_for<std::remove_cvref_t<_Sentinel_>, _Iterator_>)
{
	  if constexpr (std::is_same_v<__ranges_unwrap_iter_t<_Iterator_, _Sentinel_>, __ranges_unwrap_sent_t<_Sentinel_, _Iterator_>>)
		  return __ranges_unwrap_sentinel<_Iterator_>(std::forward<_Sentinel_>(__last));
	  else return std::ranges::next(__first, __ranges_unwrap_sentinel<_Iterator_>(std::forward<_Sentinel_>(__last)));
  }

template <std::ranges::forward_range _Range_>
raze_nodiscard raze_always_inline constexpr auto __find_final_unwrapped_iterator(_Range_& __range) {
	  if constexpr (std::ranges::common_range<_Range_>) {
		  if constexpr (std::same_as<decltype(__unchecked_end(__range)), __unwrapped_iterator_t<_Range_>>) return __unchecked_end(__range);
		  else return __ranges_unwrap_sentinel<_Range_>(std::ranges::end(__range));
	  }
	  else if constexpr (std::ranges::sized_range<_Range_>) return std::ranges::next(__unchecked_begin(__range), std::ranges::distance(__range));
	  else return std::ranges::next(__unchecked_begin(__range), __unchecked_end(__range));
  }


__RAZE_TYPE_TRAITS_NAMESPACE_END
