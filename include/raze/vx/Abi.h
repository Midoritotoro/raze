#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <raze/arch/CpuFeature.h>
#include <src/raze/vx/hw/Cast.h>
#include <raze/vx/SimdConfig.h>


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
 *  SIMD code. For GCC/Clang, use the regular `fixed_abi` with compile‑time ISA
 *  selection instead.
 *
 *  In summary:
 *    - Use `runtime_abi` on **MSVC** together with a runtime dispatcher.
 *    - Use `fixed_abi` on **GCC/Clang** for compile‑time ISA selection.
 *
 *  This ABI descriptor exposes basic architectural properties such as register
 *  width and register count, enabling the dispatcher to construct and manage
 *  ISA‑specific SIMD backends efficiently.
*/
template <
	arch::ISA	_ISA_,
	sizetype	_Elements_>
struct runtime_abi {
	static constexpr auto dynamic = true;
	static constexpr auto size = _Elements_;
	static constexpr auto isa = _ISA_;
};

template <sizetype _Elements_>
struct x86_abi {
	static constexpr auto dynamic = false;
	static constexpr auto size = _Elements_;
	static constexpr auto isa = __best_isa_compile_time();
};

#if defined(raze_processor_x86_64) && defined(raze_cpp_msvc)
  using default_abi = x86_abi<4>;
#endif // defined(raze_processor_x86_64) && defined(raze_cpp_msvc)

template <
	class		_Abi_, 
	sizetype	_Elements_>
struct resize_abi {
	using type = std::conditional_t<_Abi_::dynamic, runtime_abi<_Abi_::isa, _Elements_>, x86_abi<_Elements_>>;
};

template <
	class		_Abi_,
	sizetype	_Elements_>
using resize_abi_t = typename resize_abi<_Abi_, _Elements_>::type;

__RAZE_VX_NAMESPACE_END
