#pragma once 

#include <src/raze/datapar/Compare.h>
#include <src/raze/datapar/SimdCompareAdapters.h>

#include <src/raze/type_traits/TypeTraits.h>
#include <src/raze/datapar/SimdMaskTypeCheck.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN


template <
	class	_Simd_,
	class   _DesiredType_>
using __simd_native_compare_return_type = __native_compare_return_type_helper<_Simd_,
	type_traits::invoke_result_type<_Simd_equal<_Simd_::__isa, _Simd_::__width, _DesiredType_>,
		typename _Simd_::vector_type, typename _Simd_::vector_type>, 
	_DesiredType_>;

template <
	arch::ISA	_ISA_,
	class		_Type_,
	uint32		_Width_>
class simd_compare_result {
	friend __as_index_mask_t;
	friend __as_mask_t;
	friend __as_native_t;
	friend __as_simd_t;

	using __native_type = __simd_native_compare_return_type<simd<_ISA_, _Type_, _Width_>, _Type_>;
public:
	static inline constexpr auto __isa = _ISA_;
	static inline constexpr auto __width = _Width_;

	using element_type		= _Type_;
	using native_type		= std::conditional_t<std::is_integral_v<__native_type>, simd_mask<_ISA_, element_type, _Width_>, __native_type>;

	simd_compare_result(const native_type& __result) noexcept;

	simd_compare_result(const simd_compare_result&) = delete;
	simd_compare_result& operator=(const simd_compare_result&) = delete;
	
	simd_compare_result(simd_compare_result&&) noexcept = default; 
	simd_compare_result& operator=(simd_compare_result&&) noexcept = default;

	template <class _Type_>
	raze_always_inline friend simd_compare_result operator& <>(simd_compare_result __compare_result, _Type_ __other) noexcept
		requires std::is_convertible_v<_Type_, native_type> /*|| std::is_integral_v<_Type_> || __is_simd_mask_v<_Type_>*/
	{
		return __compare_result._compare_result & __other;
	}

	template <class _Type_>
	raze_always_inline friend simd_compare_result operator| <>(simd_compare_result __compare_result, _Type_ __other) noexcept 
		requires std::is_convertible_v<_Type_, native_type> /*|| std::is_integral_v<_Type_> || __is_simd_mask_v<_Type_>*/
	{
		return __compare_result._compare_result | __other;
	}
	
	template <class _Type_>
	raze_always_inline friend simd_compare_result operator^ <>(simd_compare_result __compare_result, _Type_ __other) noexcept 
		requires std::is_convertible_v<_Type_, native_type> /*|| std::is_integral_v<_Type_> || __is_simd_mask_v<_Type_>*/
	{
		return __compare_result._compare_result ^ __other;
	}

	raze_always_inline simd_compare_result operator~() noexcept;
	raze_always_inline operator bool() const noexcept;
private:
	native_type _compare_result;
};

template <
	class _SimdCompareResult_,
	class = void>
struct __is_valid_simd_compare_result :
	std::false_type
{};

template <class _SimdCompareResult_>
struct __is_valid_simd_compare_result<
	_SimdCompareResult_,
    std::void_t<simd_compare_result<
        _SimdCompareResult_::__isa,
        typename _SimdCompareResult_::value_type,
        _SimdCompareResult_::__width>>>
    : std::bool_constant<
        type_traits::is_virtual_base_of_v<
			simd_compare_result<_SimdCompareResult_::__isa,
                typename _SimdCompareResult_::value_type,
                _SimdCompareResult_::__width>,
            _SimdCompareResult_> ||
        std::is_same_v<
            simd_compare_result<_SimdCompareResult_::__isa,
				typename _SimdCompareResult_::value_type,
				_SimdCompareResult_::__width>,
            _SimdCompareResult_>> 
{};

template <class _SimdCompareResult_>
constexpr bool __is_valid_simd_compare_result_v = __is_valid_simd_compare_result<_SimdCompareResult_>::value;

__RAZE_DATAPAR_NAMESPACE_END

#include <src/raze/datapar/SimdCompareResult.inl>
