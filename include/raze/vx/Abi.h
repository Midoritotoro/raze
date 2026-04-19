#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <raze/arch/CpuFeature.h>
#include <src/raze/vx/hw/Cast.h>


__RAZE_VX_NAMESPACE_BEGIN

/**
 *  @brief  ABI descriptor enabling runtime‑dispatched SIMD execution on MSVC.
 *
 *  @tparam _ISA_     Maximum instruction set architecture that the dispatcher
 *                    is permitted to use.
 *  @tparam _Width_   SIMD register width in bits. Defaults to the natural width
 *                    of @p _ISA_.
 *
 *  This ABI model is intended **exclusively for MSVC compiler. 
 *	It does not perform instruction selection by itself. Instead,
 *  it is designed to be used together with an external runtime dispatcher that
 *  selects the most capable SIMD implementation available on the current CPU.
 *
 *  The purpose of this ABI is to allow MSVC builds to store **all SIMD
 *  implementations in a single binary** without incurring the performance
 *  penalties normally associated with dynamic dispatch. Since these compilers
 *  do not support compile‑time multiversioning (such as `target_clones`,
 *  `target` attributes), a dedicated runtime dispatcher
 *  is required to route SIMD operations to the appropriate ISA‑specific code
 *  path.
 *
 *  On GCC and Clang, this ABI model is **not applicable**. True runtime ISA
 *  dispatch on those compilers requires compiling multiple translation units
 *  with different simd flags and loading them dynamically (e.g., via
 *  `dlopen`), which is significantly slower and unsuitable for high‑performance
 *  SIMD code. For GCC/Clang, use the regular `x86_abi` with compile‑time ISA
 *  selection instead.
 *
 *  In summary:
 *    - Use `x86_runtime_abi` on **MSVC** together with a runtime dispatcher.
 *    - Use `x86_abi` on **GCC/Clang** for compile‑time ISA selection.
 *
 *  This ABI descriptor exposes basic architectural properties such as register
 *  width and register count, enabling the dispatcher to construct and manage
 *  ISA‑specific SIMD backends efficiently.
*/
template <
	arch::ISA	_ISA_,
	sizetype	_Width_ = __default_width<_ISA_>>
struct x86_runtime_abi {
private:
	static constexpr auto __calculate_registers_count() noexcept {
		if constexpr (arch::__is_xmm_v<_ISA_>)
			return 16;
		else if constexpr (arch::__is_ymm_v<_ISA_>)
			return 16;
		else if constexpr (arch::__is_zmm_v<_ISA_>)
			return 32;
	}
public:
	static_assert(type_traits::__is_generation_supported_v<_ISA_>);

	static constexpr auto width = _Width_;
	static constexpr auto isa = _ISA_;
	static constexpr auto registers_count = __calculate_registers_count();
};

#if defined(raze_processor_x86_64) && defined(raze_cpp_msvc)
  using default_abi = x86_runtime_abi<arch::ISA::SSE2>;
#endif // defined(raze_processor_x86_64) && defined(raze_cpp_msvc)

template <sizetype _Width_>
struct x86_abi {
	// Not supported
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

__RAZE_VX_NAMESPACE_END
