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
