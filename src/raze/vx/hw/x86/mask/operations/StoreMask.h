#pragma once 

#include <src/raze/vx/hw/x86/memory/Store.h>
#include <src/raze/vx/hw/x86/arithmetic/Negate.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u32 _Size_, arithmetic_type _Type_>
struct _Store_mask {
	template <raw_mask_type _Mask_, class _AlignPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline void operator()(
		bool* __mem, _Mask_ __mask, _AlignPolicy_&& __policy = _AlignPolicy_{}) raze_const_operator noexcept
	{
		using _Signed = typename IntegerForSizeof<_Type_>::Signed;

		if constexpr (intrin_type<_Mask_>)
			_Store<_ISA_>()(__mem, _Negate<_ISA_, _Signed>()(__mask), __policy);
		else if constexpr (std::is_same_v<std::remove_cvref_t<_Mask_>, bool>)
			*__mem = __mask;
		else if constexpr (_Size_ >= 16)
			_Store<_ISA_>()(__mem, _Negate<_ISA_, byte>()(_To_vector<_ISA_, 
				type_traits::__deduce_simd_vector_type<_Signed, _Size_ * 8>, byte>()(__mask)), __policy);
		else {
			//using _StoreType = typename IntegerForSize<_Size_>::Unsigned;
			//auto __vec = _Negate<_ISA_, byte>()(_To_vector<_ISA_, __m128i, byte>()(__mask));
			//*reinterpret_cast<_StoreType*>(__mem) = _Extract<_ISA_, _StoreType>()(__vec, 0);
		}
	}
};

__RAZE_VX_NAMESPACE_END
