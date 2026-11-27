#pragma once 

#if !defined(raze_no_stack_protector)
#  if defined(raze_cpp_msvc)
#    define raze_no_stack_protector __declspec(safebuffers)
#  else
#    define raze_no_stack_protector
#  endif
#endif // !defined(raze_no_stack_protector)
