#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <raze/arch/CpuFeature.h>
#include <src/raze/datapar/SimdIntegralTypesCheck.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	sizetype	_Width_ = __default_width<_ISA_>>
struct x86_abi {
	static_assert(type_traits::__is_generation_supported_v<_ISA_>);

	static constexpr auto width = _Width_;
	static constexpr auto isa = _ISA_;
};

template <sizetype _Width_>
struct arm_neon_abi {
	// Not supported
};

struct arm_sve_abi {
	// Not supported
};

struct riscv_abi {
	// Not supported
};

__RAZE_DATAPAR_NAMESPACE_END
