#pragma once 

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/arithmetic/Negate.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/access/Insert.h>


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
			using _LoadType = typename IntegerForSize<_Size_>::Unsigned;
			__m128i __vec;
			_LoadType __loaded = *reinterpret_cast<const _LoadType*>(__mem);
			_Insert<_ISA_>()(__vec, 0, __loaded);
			return _To_mask<_ISA_, byte>()(__vec)& ((1u << _Size_) - 1);
		}
	}
};

__RAZE_VX_NAMESPACE_END
