#pragma once 

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/arithmetic/Negate.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/access/Insert.h>
#include <src/raze/math/BitTestAndSet.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u32 _Size_, raw_mask_type _Mask_, arithmetic_type _Type_>
struct _Load_mask {
	template <class _AlignPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _Mask_ operator()(
		const bool* __mem, _AlignPolicy_&& __policy = _AlignPolicy_{}) raze_const_operator noexcept 
	{
		using _Signed = typename IntegerForSizeof<_Type_>::Signed;

		if constexpr (intrin_type<_Mask_>)
			return _Negate<_ISA_, _Signed>()(_Load<_ISA_, _Mask_>()(__mem, __policy));
		else if constexpr (std::is_same_v<std::remove_cvref_t<_Mask_>, bool>)
			return *__mem;
		else if constexpr (_Size_ >= 16)
			return _To_mask<_ISA_, byte>()(_Negate<_ISA_, byte>()(_Load<_ISA_, 
				type_traits::__deduce_simd_vector_type<_Signed, _Size_ * 8>>()(__mem, __policy)));
		else {
			_Mask_ __mask = 0;

			for (auto __i = 0; __i < _Size_; ++__i)
				if (*__mem++)
					math::__bit_test_and_set(__mask, __i);

			return __mask;
		}
	}
};

__RAZE_VX_NAMESPACE_END
