#pragma once 

#include <raze/RazeNamespace.h>

#include <raze/arch/ProcessorDetection.h>
#include <raze/compatibility/SystemDetection.h>

#include <functional>
#include <numeric>


__RAZE_NAMESPACE_BEGIN

template <typename Signature>
using Fn = std::function<Signature>;

using uchar     = unsigned char;
using ushort    = unsigned short;

using uint      = unsigned int;
using ulong     = unsigned long;

using i8      = signed char;			
using u8     = unsigned char;		

using i16     = short;			
using u16    = unsigned short;		

using i32     = int;					
using u32    = unsigned int;		

using i64     = long long;		
using u64    = unsigned long long;

using longlong  = i64;
using ulonglong = u64;

using ulong32   = unsigned long;
using long32    = long;

using f32 = float;
using f64 = double;

template <int>      struct IntegerForSize;

template <>         struct IntegerForSize<1> { typedef u8  Unsigned; typedef i8  Signed; };
template <>         struct IntegerForSize<2> { typedef u16 Unsigned; typedef i16 Signed; };

template <>         struct IntegerForSize<4> { typedef u32 Unsigned; typedef i32 Signed; };
template <>         struct IntegerForSize<8> { typedef u64 Unsigned; typedef i64 Signed; };

template <class T>  struct IntegerForSizeof : IntegerForSize<sizeof(T)> { };

using registerint   = IntegerForSize<raze_processor_wordsize>::Signed;
using registeruint  = IntegerForSize<raze_processor_wordsize>::Unsigned;

using uintptr       = IntegerForSizeof<void*>::Unsigned;
using ptrdiff       = IntegerForSizeof<void*>::Signed;

using intptr        = ptrdiff;
using sizetype      = IntegerForSizeof<std::size_t>::Unsigned;

using byte_t        = u8;
using sbyte_t       = i8;

using word_t        = u16;
using sword_t       = i16;

using dword_t       = u32;
using sdword_t      = i32;

using qword_t       = u64;
using sqword_t      = i64;

#if defined(raze_os_windows) 
  using system_bool_t = BOOL;
#else 
  using system_bool_t = bool;
#endif
  
#if defined(raze_os_windows)
  using system_handle_t = void*;
#else
  using system_handle_t = void*;
#endif // defined(raze_os_windows)

__RAZE_NAMESPACE_END
