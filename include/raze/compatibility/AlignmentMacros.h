#pragma once 

#include <raze/compatibility/CompilerDetection.h>
#include <raze/arch/ProcessorDetection.h>


#if !defined(raze_unaligned)
#  if defined(raze_processor_x86_64) || defined(raze_processor_arm) || (raze_processor_arm == 8) // x64 ARM
#    if defined(raze_os_windows) && defined(raze_cpp_msvc)
#      define raze_unaligned __unaligned
#    else
#      define raze_unaligned
#    endif // defined(raze_os_windows) && defined(raze_cpp_msvc)
#  else 
#    define raze_unaligned
#  endif // defined(raze_processor_x86_64) || defined(raze_processor_arm) || (raze_processor_arm == 8)
#endif // !defined(raze_unaligned)


#if !defined(raze_aligned_type)
#  if defined(raze_cpp_gnu)
#    define raze_aligned_type(alignment) __attribute__((aligned(alignment)))
#  elif defined(raze_cpp_msvc) || defined(raze_cpp_clang) 
#    define raze_aligned_type(alignment) __declspec(align(alignment))
#  elif raze_has_cxx11
#    define raze_aligned_type(alignment) alignas(alignment)
#  endif // defined(raze_cpp_gnu) || defined(raze_cpp_msvc) || defined(raze_cpp_clang) || raze_has_cxx11
#endif // !defined(raze_aligned_type)