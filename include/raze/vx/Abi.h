#pragma once 

#include <raze/arch/CpuFeature.h>
#include <src/raze/vx/hw/Cast.h>
#include <raze/vx/Config.h>


__RAZE_VX_NAMESPACE_BEGIN

/**
 *  @brief  ABI descriptor enabling runtime‑dispatched SIMD execution on MSVC.
 *
 *  @tparam ISA     Maximum instruction set architecture that the dispatcher
 *                    is permitted to use.
 *  @tparam Width   SIMD register width in bits. Defaults to the natural width
 *                    of @p ISA.
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
 *  `dlopen`). For GCC/Clang, use the regular `fixed_abi` with compile‑time ISA
 *  selection instead.
 *
 *  In summary:
 *    - Use `runtime_abi` on **MSVC** together with a runtime dispatcher.
 *    - Use `x86_abi` on **GCC/Clang** for compile‑time ISA selection.
 *
 *  This ABI descriptor exposes basic architectural properties such as register
 *  width and register count, enabling the dispatcher to construct and manage
 *  ISA‑specific SIMD backends efficiently.
*/
template <arch::ISA	ISA, sizetype Elements>
struct runtime_abi {
	static constexpr auto dynamic = true;
	static constexpr auto size = Elements;
	static constexpr auto isa = ISA;
};

template <sizetype Elements>
struct x86_abi {
	static constexpr auto dynamic = false;
	static constexpr auto size = Elements;
	static constexpr auto isa = target_isa();
};

#if defined(raze_processor_x86_64) && defined(raze_cpp_msvc)
  using default_abi = x86_abi<4>;
#endif // defined(raze_processor_x86_64) && defined(raze_cpp_msvc)

template <class Abi, sizetype Elements>
struct resize_abi {
	using type = std::conditional_t<Abi::dynamic, runtime_abi<Abi::isa, Elements>, x86_abi<Elements>>;
};

template <class	Abi, sizetype Elements>
using resize_abi_t = typename resize_abi<Abi, Elements>::type;

template <class Type>
using native_abi = x86_abi<native_size<Type>>;

__RAZE_VX_NAMESPACE_END
