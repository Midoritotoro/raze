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

using int8      = signed char;			
using uint8     = unsigned char;		

using int16     = short;			
using uint16    = unsigned short;		

using int32     = int;					
using uint32    = unsigned int;		

using int64     = long long;		
using uint64    = unsigned long long;

using longlong  = int64;
using ulonglong = uint64;

using ulong32   = unsigned long;
using long32    = long;

using float32 = float;
using float64 = double;

template <int>      struct IntegerForSize;

template <>         struct IntegerForSize<1> { typedef uint8  Unsigned; typedef int8  Signed; };
template <>         struct IntegerForSize<2> { typedef uint16 Unsigned; typedef int16 Signed; };

template <>         struct IntegerForSize<4> { typedef uint32 Unsigned; typedef int32 Signed; };
template <>         struct IntegerForSize<8> { typedef uint64 Unsigned; typedef int64 Signed; };

template <class T>  struct IntegerForSizeof : IntegerForSize<sizeof(T)> { };

using registerint   = IntegerForSize<raze_processor_wordsize>::Signed;
using registeruint  = IntegerForSize<raze_processor_wordsize>::Unsigned;

using uintptr       = IntegerForSizeof<void*>::Unsigned;
using ptrdiff       = IntegerForSizeof<void*>::Signed;

using intptr        = ptrdiff;
using sizetype      = IntegerForSizeof<std::size_t>::Unsigned;

using byte_t        = uint8;
using sbyte_t       = int8;

using word_t        = uint16;
using sword_t       = int16;

using dword_t       = ulong32;
using sdword_t      = long32;

using qword_t       = uint64;
using sqword_t      = int64;

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
