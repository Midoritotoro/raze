#pragma once 

#include <src/raze/vx/hw/x86/bitwise/Andnot.h>
#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>
#include <src/raze/vx/hw/x86/bitwise/BitOr.h>
#include <src/raze/vx/hw/x86/bitwise/BitXor.h>
#include <src/raze/vx/hw/x86/bitwise/BitNot.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>


__RAZE_VX_NAMESPACE_BEGIN

struct _Ternarylogic_emulated {
    template <class _Tp_, u8 _Op_, class _Or_, class _Xor_,
        class _And_, class _Andnot_, class _Not_, class _Zero_>
    raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Tp_ __z,
        std::integral_constant<u8, _Op_> __imm8, _Or_ __or, _Xor_ __xor, _And_ __and,
        _Andnot_ __andnot, _Not_ __not, _Zero_ __zero) raze_const_operator noexcept
    {
        if constexpr (__imm8 == 0x00) return __zero();
        if constexpr (__imm8 == 0x01) return __not(__or(__x, __or(__y, __z)));
        if constexpr (__imm8 == 0x02) return __andnot(__or(__x, __y), __z);
        if constexpr (__imm8 == 0x03) return __not(__or(__x, __y));
        if constexpr (__imm8 == 0x04) return __andnot(__or(__x, __z), __y);
        if constexpr (__imm8 == 0x05) return __not(__or(__x, __z));
        if constexpr (__imm8 == 0x06) return __andnot(__x, __xor(__y, __z));
        if constexpr (__imm8 == 0x07) return __not(__or(__x, __and(__y, __z)));
        if constexpr (__imm8 == 0x08) return __and(__andnot(__x, __y), __z);
        if constexpr (__imm8 == 0x09) return __not(__or(__x, __xor(__y, __z)));
        if constexpr (__imm8 == 0x0a) return __andnot(__x, __z);
        if constexpr (__imm8 == 0x0b) return __andnot(__x, __or(__not(__y), __z));
        if constexpr (__imm8 == 0x0c) return __andnot(__x, __y);
        if constexpr (__imm8 == 0x0d) return __andnot(__x, __or(__y, __not(__z)));
        if constexpr (__imm8 == 0x0e) return __andnot(__x, __or(__y, __z));
        if constexpr (__imm8 == 0x0f) return __not(__x);

        if constexpr (__imm8 == 0x10) return __andnot(__or(__y, __z), __x);
        if constexpr (__imm8 == 0x11) return __not(__or(__y, __z));
        if constexpr (__imm8 == 0x12) return __andnot(__y, __xor(__x, __z));
        if constexpr (__imm8 == 0x13) return __not(__or(__y, __and(__x, __z)));
        if constexpr (__imm8 == 0x14) return __andnot(__z, __xor(__x, __y));
        if constexpr (__imm8 == 0x15) return __not(__or(__z, __and(__x, __y)));
        if constexpr (__imm8 == 0x16) return __or(__andnot(__or(__y, __z), __x), __andnot(__x, __xor(__y, __z)));
        if constexpr (__imm8 == 0x17) return __or(__not(__or(__y, __z)), __andnot(__x, __xor(__y, __z)));
        if constexpr (__imm8 == 0x18) return __and(__xor(__x, __z), __xor(__x, __y));
        if constexpr (__imm8 == 0x19) return __andnot(__and(__x, __y), __not(__xor(__y, __z)));
        if constexpr (__imm8 == 0x1a) return __andnot(__and(__x, __y), __xor(__x, __z));
        if constexpr (__imm8 == 0x1b) return __not(__or(__and(__z, __x), __andnot(__z, __y)));
        if constexpr (__imm8 == 0x1c) return __andnot(__and(__x, __z), __xor(__x, __y));
        if constexpr (__imm8 == 0x1d) return __not(__or(__and(__y, __x), __andnot(__y, __z)));
        if constexpr (__imm8 == 0x1e) return __xor(__x, __or(__y, __z));
        if constexpr (__imm8 == 0x1f) return __not(__and(__x, __or(__y, __z)));

        if constexpr (__imm8 == 0x20) return __and(__andnot(__y, __x), __z);
        if constexpr (__imm8 == 0x21) return __not(__or(__y, __xor(__x, __z)));
        if constexpr (__imm8 == 0x22) return __andnot(__y, __z);
        if constexpr (__imm8 == 0x23) return __andnot(__y, __or(__not(__x), __z));
        if constexpr (__imm8 == 0x24) return __and(__xor(__x, __y), __xor(__y, __z));
        if constexpr (__imm8 == 0x25) return __andnot(__and(__x, __y), __xor(__x, __not(__z)));
        if constexpr (__imm8 == 0x26) return __andnot(__and(__x, __y), __xor(__y, __z));
        if constexpr (__imm8 == 0x27) return __not(__or(__and(__z, __y), __andnot(__z, __x)));
        if constexpr (__imm8 == 0x28) return __and(__z, __xor(__x, __y));
        if constexpr (__imm8 == 0x29) return __or(__and(__z, __xor(__y, __x)), __andnot(__z, __not(__or(__y, __x))));
        if constexpr (__imm8 == 0x2a) return __andnot(__and(__y, __x), __z);
        if constexpr (__imm8 == 0x2b) return __or(__andnot(__and(__y, __x), __z), __andnot(__z, __not(__or(__y, __x))));
        if constexpr (__imm8 == 0x2c) return __and(__or(__y, __z), __xor(__x, __y));
        if constexpr (__imm8 == 0x2d) return __xor(__x, __or(__y, __not(__z)));
        if constexpr (__imm8 == 0x2e) return __or(__and(__y, __not(__x)), __andnot(__y, __z));
        if constexpr (__imm8 == 0x2f) return __or(__andnot(__y, __z), __not(__x));

        if constexpr (__imm8 == 0x30) return __andnot(__y, __x);
        if constexpr (__imm8 == 0x31) return __andnot(__y, __or(__x, __not(__z)));
        if constexpr (__imm8 == 0x32) return __andnot(__y, __or(__x, __z));
        if constexpr (__imm8 == 0x33) return __not(__y);
        if constexpr (__imm8 == 0x34) return __andnot(__and(__y, __z), __xor(__x, __y));
        if constexpr (__imm8 == 0x35) return __not(__or(__and(__x, __y), __andnot(__x, __z)));
        if constexpr (__imm8 == 0x36) return __xor(__y, __or(__x, __z));
        if constexpr (__imm8 == 0x37) return __not(__and(__y, __or(__x, __z)));
        if constexpr (__imm8 == 0x38) return __and(__or(__x, __z), __xor(__x, __y));
        if constexpr (__imm8 == 0x39) return __xor(__y, __or(__x, __not(__z)));
        if constexpr (__imm8 == 0x3a) return __or(__and(__x, __not(__y)), __andnot(__x, __z));
        if constexpr (__imm8 == 0x3b) return __or(__andnot(__x, __z), __not(__y));
        if constexpr (__imm8 == 0x3c) return __xor(__y, __x);
        if constexpr (__imm8 == 0x3d) return __or(__xor(__x, __y), __not(__or(__x, __z)));
        if constexpr (__imm8 == 0x3e) return __or(__andnot(__x, __z), __xor(__x, __y));
        if constexpr (__imm8 == 0x3f) return __not(__and(__y, __x));

        if constexpr (__imm8 == 0x40) return __and(__andnot(__z, __x), __y);
        if constexpr (__imm8 == 0x41) return __not(__or(__z, __xor(__y, __x)));
        if constexpr (__imm8 == 0x42) return __and(__xor(__x, __z), __xor(__y, __z));
        if constexpr (__imm8 == 0x43) return __andnot(__and(__x, __z), __xor(__x, __not(__y)));
        if constexpr (__imm8 == 0x44) return __andnot(__z, __y);
        if constexpr (__imm8 == 0x45) return __andnot(__z, __or(__not(__x), __y));
        if constexpr (__imm8 == 0x46) return __andnot(__and(__x, __z), __xor(__y, __z));
        if constexpr (__imm8 == 0x47) return __not(__or(__and(__y, __z), __andnot(__y, __x)));
        if constexpr (__imm8 == 0x48) return __and(__y, __xor(__x, __z));
        if constexpr (__imm8 == 0x49) return __or(__and(__y, __xor(__x, __z)), __andnot(__y, __not(__or(__x, __z))));
        if constexpr (__imm8 == 0x4a) return __and(__or(__y, __z), __xor(__x, __z));
        if constexpr (__imm8 == 0x4b) return __xor(__x, __or(__not(__y), __z));
        if constexpr (__imm8 == 0x4c) return __andnot(__and(__x, __z), __y);
        if constexpr (__imm8 == 0x4d) return __or(__andnot(__and(__x, __z), __y), __andnot(__y, __not(__or(__x, __z))));
        if constexpr (__imm8 == 0x4e) return __or(__and(__z, __not(__x)), __andnot(__z, __y));
        if constexpr (__imm8 == 0x4f) return __or(__not(__x), __andnot(__z, __y));

        if constexpr (__imm8 == 0x50) return __andnot(__z, __x);
        if constexpr (__imm8 == 0x51) return __andnot(__z, __or(__x, __not(__y)));
        if constexpr (__imm8 == 0x52) return __andnot(__and(__y, __z), __xor(__x, __z));
        if constexpr (__imm8 == 0x53) return __not(__or(__and(__x, __z), __andnot(__x, __y)));
        if constexpr (__imm8 == 0x54) return __andnot(__z, __or(__x, __y));
        if constexpr (__imm8 == 0x55) return __not(__z);
        if constexpr (__imm8 == 0x56) return __xor(__z, __or(__y, __x));
        if constexpr (__imm8 == 0x57) return __not(__and(__z, __or(__y, __x)));
        if constexpr (__imm8 == 0x58) return __and(__or(__x, __y), __xor(__x, __z));
        if constexpr (__imm8 == 0x59) return __xor(__z, __or(__x, __not(__y)));
        if constexpr (__imm8 == 0x5a) return __xor(__z, __x);
        if constexpr (__imm8 == 0x5b) return __or(__xor(__x, __z), __not(__or(__x, __y)));
        if constexpr (__imm8 == 0x5c) return __or(__and(__x, __not(__z)), __andnot(__x, __y));
        if constexpr (__imm8 == 0x5d) return __or(__andnot(__x, __y), __not(__z));
        if constexpr (__imm8 == 0x5e) return __or(__andnot(__z, __y), __xor(__x, __z));
        if constexpr (__imm8 == 0x5f) return __not(__and(__z, __x));

        if constexpr (__imm8 == 0x60) return __and(__x, __xor(__y, __z));
        if constexpr (__imm8 == 0x61) return __or(__and(__x, __xor(__y, __z)), __andnot(__x, __not(__or(__y, __z))));
        if constexpr (__imm8 == 0x62) return __and(__or(__x, __z), __xor(__y, __z));
        if constexpr (__imm8 == 0x63) return __xor(__y, __or(__not(__x), __z));
        if constexpr (__imm8 == 0x64) return __and(__or(__x, __y), __xor(__y, __z));
        if constexpr (__imm8 == 0x65) return __xor(__z, __or(__not(__x), __y));
        if constexpr (__imm8 == 0x66) return __xor(__z, __y);
        if constexpr (__imm8 == 0x67) return __or(__xor(__y, __z), __not(__or(__x, __y)));
        if constexpr (__imm8 == 0x68) return __or(__and(__x, __xor(__y, __z)), __andnot(__x, __and(__y, __z)));
        if constexpr (__imm8 == 0x69) return __not(__xor(__x, __xor(__y, __z)));
        if constexpr (__imm8 == 0x6a) return __xor(__z, __and(__y, __x));
        if constexpr (__imm8 == 0x6b) return __or(__andnot(__x, __z), __xor(__not(__x), __xor(__y, __z)));
        if constexpr (__imm8 == 0x6c) return __xor(__y, __and(__x, __z));
        if constexpr (__imm8 == 0x6d) return __or(__andnot(__x, __y), __xor(__not(__x), __xor(__y, __z)));
        if constexpr (__imm8 == 0x6e) return __or(__andnot(__x, __y), __xor(__y, __z));
        if constexpr (__imm8 == 0x6f) return __or(__xor(__y, __z), __not(__x));

        if constexpr (__imm8 == 0x70) return __andnot(__and(__y, __z), __x);
        if constexpr (__imm8 == 0x71) return __or(__not(__or(__y, __z)), __and(__x, __xor(__y, __z)));
        if constexpr (__imm8 == 0x72) return __or(__and(__z, __not(__y)), __andnot(__z, __x));
        if constexpr (__imm8 == 0x73) return __or(__andnot(__z, __x), __not(__y));
        if constexpr (__imm8 == 0x74) return __or(__and(__y, __not(__z)), __andnot(__y, __x));
        if constexpr (__imm8 == 0x75) return __or(__andnot(__y, __x), __not(__z));
        if constexpr (__imm8 == 0x76) return __or(__andnot(__y, __x), __xor(__y, __z));
        if constexpr (__imm8 == 0x77) return __not(__and(__y, __z));
        if constexpr (__imm8 == 0x78) return __xor(__x, __and(__y, __z));
        if constexpr (__imm8 == 0x79) return __or(__andnot(__y, __x), __xor(__not(__y), __xor(__x, __z)));
        if constexpr (__imm8 == 0x7a) return __or(__andnot(__y, __x), __xor(__x, __z));
        if constexpr (__imm8 == 0x7b) return __or(__xor(__x, __z), __not(__y));
        if constexpr (__imm8 == 0x7c) return __or(__andnot(__z, __x), __xor(__x, __y));
        if constexpr (__imm8 == 0x7d) return __or(__xor(__x, __y), __not(__z));
        if constexpr (__imm8 == 0x7e) return __or(__xor(__x, __y), __xor(__x, __z));
        if constexpr (__imm8 == 0x7f) return __not(__and(__x, __and(__y, __z)));

        if constexpr (__imm8 == 0x80) return __and(__x, __and(__y, __z));
        if constexpr (__imm8 == 0x81) return __andnot(__xor(__x, __z), __xor(__x, __not(__y)));
        if constexpr (__imm8 == 0x82) return __andnot(__xor(__y, __x), __z);
        if constexpr (__imm8 == 0x83) return __andnot(__xor(__x, __y), __or(__not(__x), __z));
        if constexpr (__imm8 == 0x84) return __andnot(__xor(__x, __z), __y);
        if constexpr (__imm8 == 0x85) return __andnot(__xor(__x, __z), __or(__y, __not(__z)));
        if constexpr (__imm8 == 0x86) return __and(__or(__y, __z), __xor(__z, __xor(__x, __y)));
        if constexpr (__imm8 == 0x87) return __not(__xor(__x, __and(__y, __z)));
        if constexpr (__imm8 == 0x88) return __and(__z, __y);
        if constexpr (__imm8 == 0x89) return __andnot(__xor(__y, __z), __or(__not(__x), __y));
        if constexpr (__imm8 == 0x8a) return __andnot(__andnot(__y, __x), __z);
        if constexpr (__imm8 == 0x8b) return __or(__and(__y, __z), __andnot(__y, __not(__x)));
        if constexpr (__imm8 == 0x8c) return __andnot(__andnot(__z, __x), __y);
        if constexpr (__imm8 == 0x8d) return __or(__and(__z, __y), __andnot(__z, __not(__x)));
        if constexpr (__imm8 == 0x8e) return __or(__and(__y, __z), __andnot(__x, __xor(__y, __z)));
        if constexpr (__imm8 == 0x8f) return __or(__and(__y, __z), __not(__x));

        if constexpr (__imm8 == 0x90) return __andnot(__xor(__y, __z), __x);
        if constexpr (__imm8 == 0x91) return __andnot(__xor(__y, __z), __or(__x, __not(__y)));
        if constexpr (__imm8 == 0x92) return __and(__or(__x, __z), __xor(__z, __xor(__x, __y)));
        if constexpr (__imm8 == 0x93) return __not(__xor(__y, __and(__x, __z)));
        if constexpr (__imm8 == 0x94) return __and(__or(__x, __y), __xor(__y, __xor(__x, __z)));
        if constexpr (__imm8 == 0x95) return __not(__xor(__z, __and(__y, __x)));
        if constexpr (__imm8 == 0x96) return __xor(__x, __xor(__y, __z));
        if constexpr (__imm8 == 0x97) return __or(__andnot(__xor(__y, __z), __x), __andnot(__x, __not(__and(__y, __z))));
        if constexpr (__imm8 == 0x98) return __andnot(__xor(__y, __z), __or(__x, __y));
        if constexpr (__imm8 == 0x99) return __not(__xor(__z, __y));
        if constexpr (__imm8 == 0x9a) return __xor(__andnot(__y, __x), __z);
        if constexpr (__imm8 == 0x9b) return __or(__andnot(__x, __z), __xor(__y, __not(__z)));
        if constexpr (__imm8 == 0x9c) return __xor(__andnot(__z, __x), __y);
        if constexpr (__imm8 == 0x9d) return __or(__andnot(__x, __y), __xor(__y, __not(__z)));
        if constexpr (__imm8 == 0x9e) return __or(__and(__y, __z), __xor(__z, __xor(__x, __y)));
        if constexpr (__imm8 == 0x9f) return __not(__and(__x, __xor(__y, __z)));

        if constexpr (__imm8 == 0xa0) return __and(__z, __x);
        if constexpr (__imm8 == 0xa1) return __andnot(__xor(__x, __z), __or(__x, __not(__y)));
        if constexpr (__imm8 == 0xa2) return __andnot(__andnot(__x, __y), __z);
        if constexpr (__imm8 == 0xa3) return __or(__and(__x, __z), __andnot(__x, __not(__y)));
        if constexpr (__imm8 == 0xa4) return __andnot(__xor(__x, __z), __or(__x, __y));
        if constexpr (__imm8 == 0xa5) return __not(__xor(__z, __x));
        if constexpr (__imm8 == 0xa6) return __xor(__andnot(__x, __y), __z);
        if constexpr (__imm8 == 0xa7) return __or(__andnot(__y, __z), __xor(__x, __not(__z)));
        if constexpr (__imm8 == 0xa8) return __and(__z, __or(__x, __y));
        if constexpr (__imm8 == 0xa9) return __not(__xor(__z, __or(__y, __x)));
        if constexpr (__imm8 == 0xaa) return __z;
        if constexpr (__imm8 == 0xab) return __or(__z, __not(__or(__y, __x)));
        if constexpr (__imm8 == 0xac) return __or(__and(__x, __z), __andnot(__x, __y));
        if constexpr (__imm8 == 0xad) return __or(__and(__y, __z), __xor(__x, __not(__z)));
        if constexpr (__imm8 == 0xae) return __or(__z, __andnot(__x, __y));
        if constexpr (__imm8 == 0xaf) return __or(__z, __not(__x));

        if constexpr (__imm8 == 0xb0) return __andnot(__andnot(__z, __y), __x);
        if constexpr (__imm8 == 0xb1) return __or(__and(__z, __x), __andnot(__z, __not(__y)));
        if constexpr (__imm8 == 0xb2) return __or(__and(__y, __and(__x, __z)), __andnot(__y, __or(__x, __z)));
        if constexpr (__imm8 == 0xb3) return __or(__and(__x, __z), __not(__y));
        if constexpr (__imm8 == 0xb4) return __xor(__andnot(__z, __y), __x);
        if constexpr (__imm8 == 0xb5) return __or(__andnot(__y, __x), __xor(__x, __not(__z)));
        if constexpr (__imm8 == 0xb6) return __or(__and(__x, __z), __xor(__x, __xor(__y, __z)));
        if constexpr (__imm8 == 0xb7) return __not(__and(__y, __xor(__x, __z)));
        if constexpr (__imm8 == 0xb8) return __or(__and(__y, __z), __andnot(__y, __x));
        if constexpr (__imm8 == 0xb9) return __or(__and(__x, __z), __xor(__y, __not(__z)));
        if constexpr (__imm8 == 0xba) return __or(__andnot(__y, __x), __z);
        if constexpr (__imm8 == 0xbb) return __or(__z, __not(__y));
        if constexpr (__imm8 == 0xbc) return __or(__and(__x, __z), __xor(__x, __y));
        if constexpr (__imm8 == 0xbd) return __or(__xor(__x, __y), __xor(__x, __not(__z)));
        if constexpr (__imm8 == 0xbe) return __or(__z, __xor(__y, __x));
        if constexpr (__imm8 == 0xbf) return __or(__z, __not(__and(__y, __x)));

        if constexpr (__imm8 == 0xc0) return __and(__y, __x);
        if constexpr (__imm8 == 0xc1) return __andnot(__xor(__x, __y), __or(__x, __not(__z)));
        if constexpr (__imm8 == 0xc2) return __andnot(__xor(__y, __x), __or(__x, __z));
        if constexpr (__imm8 == 0xc3) return __not(__xor(__y, __x));
        if constexpr (__imm8 == 0xc4) return __andnot(__andnot(__x, __z), __y);
        if constexpr (__imm8 == 0xc5) return __or(__and(__x, __y), __andnot(__x, __not(__z)));
        if constexpr (__imm8 == 0xc6) return __xor(__andnot(__x, __z), __y);
        if constexpr (__imm8 == 0xc7) return __or(__andnot(__z, __y), __xor(__x, __not(__y)));
        if constexpr (__imm8 == 0xc8) return __and(__y, __or(__x, __z));
        if constexpr (__imm8 == 0xc9) return __not(__xor(__y, __or(__x, __z)));
        if constexpr (__imm8 == 0xca) return __or(__and(__x, __y), __andnot(__x, __z));
        if constexpr (__imm8 == 0xcb) return __or(__and(__y, __z), __xor(__x, __not(__y)));
        if constexpr (__imm8 == 0xcc) return __y;
        if constexpr (__imm8 == 0xcd) return __or(__y, __not(__or(__x, __z)));
        if constexpr (__imm8 == 0xce) return __or(__andnot(__x, __z), __y);
        if constexpr (__imm8 == 0xcf) return __or(__y, __not(__x));

        if constexpr (__imm8 == 0xd0) return __andnot(__andnot(__y, __z), __x);
        if constexpr (__imm8 == 0xd1) return __or(__and(__y, __x), __andnot(__y, __not(__z)));
        if constexpr (__imm8 == 0xd2) return __xor(__andnot(__y, __z), __x);
        if constexpr (__imm8 == 0xd3) return __or(__andnot(__z, __x), __xor(__x, __not(__y)));
        if constexpr (__imm8 == 0xd4) return __or(__andnot(__z, __y), __andnot(__xor(__y, __z), __x));
        if constexpr (__imm8 == 0xd5) return __or(__and(__x, __y), __not(__z));
        if constexpr (__imm8 == 0xd6) return __or(__and(__x, __y), __xor(__y, __xor(__x, __z)));
        if constexpr (__imm8 == 0xd7) return __not(__and(__z, __xor(__y, __x)));
        if constexpr (__imm8 == 0xd8) return __or(__and(__z, __y), __andnot(__z, __x));
        if constexpr (__imm8 == 0xd9) return __or(__and(__x, __y), __xor(__y, __not(__z)));
        if constexpr (__imm8 == 0xda) return __or(__and(__x, __y), __xor(__x, __z));
        if constexpr (__imm8 == 0xdb) return __or(__xor(__x, __z), __xor(__x, __not(__y)));
        if constexpr (__imm8 == 0xdc) return __or(__andnot(__z, __x), __y);
        if constexpr (__imm8 == 0xdd) return __or(__y, __not(__z));
        if constexpr (__imm8 == 0xde) return __or(__y, __xor(__x, __z));
        if constexpr (__imm8 == 0xdf) return __or(__y, __not(__and(__x, __z)));

        if constexpr (__imm8 == 0xe0) return __and(__x, __or(__y, __z));
        if constexpr (__imm8 == 0xe1) return __not(__xor(__x, __or(__y, __z)));
        if constexpr (__imm8 == 0xe2) return __or(__and(__y, __x), __andnot(__y, __z));
        if constexpr (__imm8 == 0xe3) return __or(__and(__x, __z), __xor(__x, __not(__y)));
        if constexpr (__imm8 == 0xe4) return __or(__and(__z, __x), __andnot(__z, __y));
        if constexpr (__imm8 == 0xe5) return __or(__and(__x, __y), __xor(__x, __not(__z)));
        if constexpr (__imm8 == 0xe6) return __or(__and(__x, __y), __xor(__y, __z));
        if constexpr (__imm8 == 0xe7) return __or(__xor(__y, __z), __xor(__x, __not(__y)));
        if constexpr (__imm8 == 0xe8) return __or(__and(__y, __z), __and(__x, __xor(__y, __z)));
        if constexpr (__imm8 == 0xe9) return __or(__and(__x, __y), __xor(__y, __xor(__x, __not(__z))));
        if constexpr (__imm8 == 0xea) return __or(__z, __and(__y, __x));
        if constexpr (__imm8 == 0xeb) return __or(__z, __not(__xor(__y, __x)));
        if constexpr (__imm8 == 0xec) return __or(__y, __and(__x, __z));
        if constexpr (__imm8 == 0xed) return __or(__y, __not(__xor(__x, __z)));
        if constexpr (__imm8 == 0xee) return __or(__z, __y);
        if constexpr (__imm8 == 0xef) return __or(__y, __or(__not(__x), __z));

        if constexpr (__imm8 == 0xf0) return __x;
        if constexpr (__imm8 == 0xf1) return __or(__x, __not(__or(__y, __z)));
        if constexpr (__imm8 == 0xf2) return __or(__andnot(__y, __z), __x);
        if constexpr (__imm8 == 0xf3) return __or(__x, __not(__y));
        if constexpr (__imm8 == 0xf4) return __or(__andnot(__z, __y), __x);
        if constexpr (__imm8 == 0xf5) return __or(__x, __not(__z));
        if constexpr (__imm8 == 0xf6) return __or(__x, __xor(__y, __z));
        if constexpr (__imm8 == 0xf7) return __or(__x, __not(__and(__y, __z)));
        if constexpr (__imm8 == 0xf8) return __or(__x, __and(__y, __z));
        if constexpr (__imm8 == 0xf9) return __or(__x, __not(__xor(__y, __z)));
        if constexpr (__imm8 == 0xfa) return __or(__z, __x);
        if constexpr (__imm8 == 0xfb) return __or(__x, __or(__not(__y), __z));
        if constexpr (__imm8 == 0xfc) return __or(__y, __x);
        if constexpr (__imm8 == 0xfd) return __or(__or(__x, __y), __not(__z));
        if constexpr (__imm8 == 0xfe) return __or(__or(__x, __y), __z);
        if constexpr (__imm8 == 0xff) return __not(__zero());
    }

    template <class _Tp_, u8 _Op_, raw_mask_type _Mask_, class _Select_, class _Or_,
        class _Xor_, class _And_, class _Andnot_, class _Not_, class _Zero_>
    raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Tp_ __z,
        std::integral_constant<u8, _Op_> __imm8, _Mask_ __mask, _Select_ __select, _Or_ __or,
        _Xor_ __xor, _And_ __and, _Andnot_ __andnot, _Not_ __not, _Zero_ __zero) raze_const_operator noexcept
    {
        return __select((*this)(__x, __y, __z, __imm8, __or, __xor, __and, __andnot, __not, __zero), __mask);
    }


    template <class _Tp_, u8 _Op_, raw_mask_type _Mask_, class _Select_, class _Or_,
        class _Xor_, class _And_, class _Andnot_, class _Not_, class _Zero_>
    raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Tp_ __z,
        std::integral_constant<u8, _Op_> __imm8, _Mask_ __mask, _Select_ __select, _Tp_ __src, _Or_ __or,
        _Xor_ __xor, _And_ __and, _Andnot_ __andnot, _Not_ __not, _Zero_ __zero) raze_const_operator noexcept
    {
        return __select((*this)(__x, __y, __z, __imm8, __or, __xor, __and, __andnot, __not, __zero), __src, __mask);
    }
};

__RAZE_VX_NAMESPACE_END
