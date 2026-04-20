#pragma once 

#include <raze/Types.h>
#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/hw/Arithmetic.h>
#include <src/raze/math/Fma.h>
#include <src/raze/math/Fms.h>
#include <src/raze/math/Fnma.h>
#include <src/raze/math/Fnms.h>
#include <src/raze/math/Sin.h>
#include <src/raze/math/Cos.h>


__RAZE_MATH_NAMESPACE_BEGIN

template <class _Options_>
struct _Abs : vx::_Configurable_abs<_Options_> {
	using mask_type = raze::options::fetch_t<raze::options::condition_key, _Options_>;

	template <class _Type_>
	raze_nodiscard raze_always_inline _Type_ operator()(_Type_ __x) const noexcept
		requires(std::integral<_Type_> || std::floating_point<_Type_>)
	{
		return raze::options::__dispatch_call(*this, __x);
	}

	template <vx::simd_type _Simd_>
	raze_nodiscard raze_always_inline auto operator()(const _Simd_& __x) const noexcept {
		return raze::options::__dispatch_call(*this, __x);
	}

	template <vx::simd_type _Simd_>
	static raze_always_inline auto deferred_call(
		auto            __options,
		const _Simd_& __x) noexcept -> decltype(vx::_Configurable_abs<_Options_>::deferred_call(__options, __x))
	{
		return vx::_Configurable_abs<_Options_>::deferred_call(__options, __x);
	}

	// using vx::_Configurable_abs<_Options_>::operator();

	template <class _Type_>
	static raze_always_inline _Type_ deferred_call(
		auto            __options,
		const _Type_&	__x) noexcept
			requires(std::integral<_Type_> || std::floating_point<_Type_>)
	{
		const auto __do_abs = [=]() {
			if constexpr (std::is_unsigned_v<_Type_>)
				return __x;
			else
				return (__x < 0) ? -__x : __x;
		};

		if constexpr (options::concepts::same_as<mask_type, options::unknown_key>) {
			return __do_abs();
		}
		else {
			auto __condition = __options[raze::options::condition_key];
            const auto __mask = __condition.mask(raze::options::as<typename mask_type::condition_type>{});

            if constexpr (mask_type::has_alternative)
                return __mask ? __do_abs() : __condition.alternative();
            else
                return __mask ? __do_abs() : 0;
		}
	}
};

constexpr inline auto abs = raze::options::functor<_Abs>;

template <class _Type_>
raze_nodiscard raze_always_inline _Type_ sin(_Type_ __x) noexcept {
	return __sin(__x);
}

template <class _Type_>
raze_nodiscard raze_always_inline _Type_ cos(_Type_ __x) noexcept {
	return __cos(__x);
}

__RAZE_MATH_NAMESPACE_END
