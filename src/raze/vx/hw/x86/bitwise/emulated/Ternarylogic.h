#pragma once 

#include <src/raze/vx/hw/x86/bitwise/Andnot.h>
#include <src/raze/vx/hw/x86/bitwise/BitAnd.h>
#include <src/raze/vx/hw/x86/bitwise/BitOr.h>
#include <src/raze/vx/hw/x86/bitwise/BitXor.h>
#include <src/raze/vx/hw/x86/bitwise/BitNot.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class V, u8 Op, class Or, class Xor,
    class And, class Andnot, class Not, class Zero>
raze_always_inline V ternarylogic_emulated_(V x, V y, V z,
    std::integral_constant<u8, Op> op, Or or_, Xor xor_, And and_,
    Andnot andn_, Not not_, Zero zero) noexcept
{
    if constexpr (op == 0x00) return zero();
    if constexpr (op == 0x01) return not_(or_(x, or_(y, z)));
    if constexpr (op == 0x02) return andn_(or_(x, y), z);
    if constexpr (op == 0x03) return not_(or_(x, y));
    if constexpr (op == 0x04) return andn_(or_(x, z), y);
    if constexpr (op == 0x05) return not_(or_(x, z));
    if constexpr (op == 0x06) return andn_(x, xor_(y, z));
    if constexpr (op == 0x07) return not_(or_(x, and_(y, z)));
    if constexpr (op == 0x08) return and_(andn_(x, y), z);
    if constexpr (op == 0x09) return not_(or_(x, xor_(y, z)));
    if constexpr (op == 0x0a) return andn_(x, z);
    if constexpr (op == 0x0b) return andn_(x, or_(not_(y), z));
    if constexpr (op == 0x0c) return andn_(x, y);
    if constexpr (op == 0x0d) return andn_(x, or_(y, not_(z)));
    if constexpr (op == 0x0e) return andn_(x, or_(y, z));
    if constexpr (op == 0x0f) return not_(x);

    if constexpr (op == 0x10) return andn_(or_(y, z), x);
    if constexpr (op == 0x11) return not_(or_(y, z));
    if constexpr (op == 0x12) return andn_(y, xor_(x, z));
    if constexpr (op == 0x13) return not_(or_(y, and_(x, z)));
    if constexpr (op == 0x14) return andn_(z, xor_(x, y));
    if constexpr (op == 0x15) return not_(or_(z, and_(x, y)));
    if constexpr (op == 0x16) return or_(andn_(or_(y, z), x), andn_(x, xor_(y, z)));
    if constexpr (op == 0x17) return or_(not_(or_(y, z)), andn_(x, xor_(y, z)));
    if constexpr (op == 0x18) return and_(xor_(x, z), xor_(x, y));
    if constexpr (op == 0x19) return andn_(and_(x, y), not_(xor_(y, z)));
    if constexpr (op == 0x1a) return andn_(and_(x, y), xor_(x, z));
    if constexpr (op == 0x1b) return not_(or_(and_(z, x), andn_(z, y)));
    if constexpr (op == 0x1c) return andn_(and_(x, z), xor_(x, y));
    if constexpr (op == 0x1d) return not_(or_(and_(y, x), andn_(y, z)));
    if constexpr (op == 0x1e) return xor_(x, or_(y, z));
    if constexpr (op == 0x1f) return not_(and_(x, or_(y, z)));

    if constexpr (op == 0x20) return and_(andn_(y, x), z);
    if constexpr (op == 0x21) return not_(or_(y, xor_(x, z)));
    if constexpr (op == 0x22) return andn_(y, z);
    if constexpr (op == 0x23) return andn_(y, or_(not_(x), z));
    if constexpr (op == 0x24) return and_(xor_(x, y), xor_(y, z));
    if constexpr (op == 0x25) return andn_(and_(x, y), xor_(x, not_(z)));
    if constexpr (op == 0x26) return andn_(and_(x, y), xor_(y, z));
    if constexpr (op == 0x27) return not_(or_(and_(z, y), andn_(z, x)));
    if constexpr (op == 0x28) return and_(z, xor_(x, y));
    if constexpr (op == 0x29) return or_(and_(z, xor_(y, x)), andn_(z, not_(or_(y, x))));
    if constexpr (op == 0x2a) return andn_(and_(y, x), z);
    if constexpr (op == 0x2b) return or_(andn_(and_(y, x), z), andn_(z, not_(or_(y, x))));
    if constexpr (op == 0x2c) return and_(or_(y, z), xor_(x, y));
    if constexpr (op == 0x2d) return xor_(x, or_(y, not_(z)));
    if constexpr (op == 0x2e) return or_(and_(y, not_(x)), andn_(y, z));
    if constexpr (op == 0x2f) return or_(andn_(y, z), not_(x));

    if constexpr (op == 0x30) return andn_(y, x);
    if constexpr (op == 0x31) return andn_(y, or_(x, not_(z)));
    if constexpr (op == 0x32) return andn_(y, or_(x, z));
    if constexpr (op == 0x33) return not_(y);
    if constexpr (op == 0x34) return andn_(and_(y, z), xor_(x, y));
    if constexpr (op == 0x35) return not_(or_(and_(x, y), andn_(x, z)));
    if constexpr (op == 0x36) return xor_(y, or_(x, z));
    if constexpr (op == 0x37) return not_(and_(y, or_(x, z)));
    if constexpr (op == 0x38) return and_(or_(x, z), xor_(x, y));
    if constexpr (op == 0x39) return xor_(y, or_(x, not_(z)));
    if constexpr (op == 0x3a) return or_(and_(x, not_(y)), andn_(x, z));
    if constexpr (op == 0x3b) return or_(andn_(x, z), not_(y));
    if constexpr (op == 0x3c) return xor_(y, x);
    if constexpr (op == 0x3d) return or_(xor_(x, y), not_(or_(x, z)));
    if constexpr (op == 0x3e) return or_(andn_(x, z), xor_(x, y));
    if constexpr (op == 0x3f) return not_(and_(y, x));

    if constexpr (op == 0x40) return and_(andn_(z, x), y);
    if constexpr (op == 0x41) return not_(or_(z, xor_(y, x)));
    if constexpr (op == 0x42) return and_(xor_(x, z), xor_(y, z));
    if constexpr (op == 0x43) return andn_(and_(x, z), xor_(x, not_(y)));
    if constexpr (op == 0x44) return andn_(z, y);
    if constexpr (op == 0x45) return andn_(z, or_(not_(x), y));
    if constexpr (op == 0x46) return andn_(and_(x, z), xor_(y, z));
    if constexpr (op == 0x47) return not_(or_(and_(y, z), andn_(y, x)));
    if constexpr (op == 0x48) return and_(y, xor_(x, z));
    if constexpr (op == 0x49) return or_(and_(y, xor_(x, z)), andn_(y, not_(or_(x, z))));
    if constexpr (op == 0x4a) return and_(or_(y, z), xor_(x, z));
    if constexpr (op == 0x4b) return xor_(x, or_(not_(y), z));
    if constexpr (op == 0x4c) return andn_(and_(x, z), y);
    if constexpr (op == 0x4d) return or_(andn_(and_(x, z), y), andn_(y, not_(or_(x, z))));
    if constexpr (op == 0x4e) return or_(and_(z, not_(x)), andn_(z, y));
    if constexpr (op == 0x4f) return or_(not_(x), andn_(z, y));

    if constexpr (op == 0x50) return andn_(z, x);
    if constexpr (op == 0x51) return andn_(z, or_(x, not_(y)));
    if constexpr (op == 0x52) return andn_(and_(y, z), xor_(x, z));
    if constexpr (op == 0x53) return not_(or_(and_(x, z), andn_(x, y)));
    if constexpr (op == 0x54) return andn_(z, or_(x, y));
    if constexpr (op == 0x55) return not_(z);
    if constexpr (op == 0x56) return xor_(z, or_(y, x));
    if constexpr (op == 0x57) return not_(and_(z, or_(y, x)));
    if constexpr (op == 0x58) return and_(or_(x, y), xor_(x, z));
    if constexpr (op == 0x59) return xor_(z, or_(x, not_(y)));
    if constexpr (op == 0x5a) return xor_(z, x);
    if constexpr (op == 0x5b) return or_(xor_(x, z), not_(or_(x, y)));
    if constexpr (op == 0x5c) return or_(and_(x, not_(z)), andn_(x, y));
    if constexpr (op == 0x5d) return or_(andn_(x, y), not_(z));
    if constexpr (op == 0x5e) return or_(andn_(z, y), xor_(x, z));
    if constexpr (op == 0x5f) return not_(and_(z, x));

    if constexpr (op == 0x60) return and_(x, xor_(y, z));
    if constexpr (op == 0x61) return or_(and_(x, xor_(y, z)), andn_(x, not_(or_(y, z))));
    if constexpr (op == 0x62) return and_(or_(x, z), xor_(y, z));
    if constexpr (op == 0x63) return xor_(y, or_(not_(x), z));
    if constexpr (op == 0x64) return and_(or_(x, y), xor_(y, z));
    if constexpr (op == 0x65) return xor_(z, or_(not_(x), y));
    if constexpr (op == 0x66) return xor_(z, y);
    if constexpr (op == 0x67) return or_(xor_(y, z), not_(or_(x, y)));
    if constexpr (op == 0x68) return or_(and_(x, xor_(y, z)), andn_(x, and_(y, z)));
    if constexpr (op == 0x69) return not_(xor_(x, xor_(y, z)));
    if constexpr (op == 0x6a) return xor_(z, and_(y, x));
    if constexpr (op == 0x6b) return or_(andn_(x, z), xor_(not_(x), xor_(y, z)));
    if constexpr (op == 0x6c) return xor_(y, and_(x, z));
    if constexpr (op == 0x6d) return or_(andn_(x, y), xor_(not_(x), xor_(y, z)));
    if constexpr (op == 0x6e) return or_(andn_(x, y), xor_(y, z));
    if constexpr (op == 0x6f) return or_(xor_(y, z), not_(x));

    if constexpr (op == 0x70) return andn_(and_(y, z), x);
    if constexpr (op == 0x71) return or_(not_(or_(y, z)), and_(x, xor_(y, z)));
    if constexpr (op == 0x72) return or_(and_(z, not_(y)), andn_(z, x));
    if constexpr (op == 0x73) return or_(andn_(z, x), not_(y));
    if constexpr (op == 0x74) return or_(and_(y, not_(z)), andn_(y, x));
    if constexpr (op == 0x75) return or_(andn_(y, x), not_(z));
    if constexpr (op == 0x76) return or_(andn_(y, x), xor_(y, z));
    if constexpr (op == 0x77) return not_(and_(y, z));
    if constexpr (op == 0x78) return xor_(x, and_(y, z));
    if constexpr (op == 0x79) return or_(andn_(y, x), xor_(not_(y), xor_(x, z)));
    if constexpr (op == 0x7a) return or_(andn_(y, x), xor_(x, z));
    if constexpr (op == 0x7b) return or_(xor_(x, z), not_(y));
    if constexpr (op == 0x7c) return or_(andn_(z, x), xor_(x, y));
    if constexpr (op == 0x7d) return or_(xor_(x, y), not_(z));
    if constexpr (op == 0x7e) return or_(xor_(x, y), xor_(x, z));
    if constexpr (op == 0x7f) return not_(and_(x, and_(y, z)));

    if constexpr (op == 0x80) return and_(x, and_(y, z));
    if constexpr (op == 0x81) return andn_(xor_(x, z), xor_(x, not_(y)));
    if constexpr (op == 0x82) return andn_(xor_(y, x), z);
    if constexpr (op == 0x83) return andn_(xor_(x, y), or_(not_(x), z));
    if constexpr (op == 0x84) return andn_(xor_(x, z), y);
    if constexpr (op == 0x85) return andn_(xor_(x, z), or_(y, not_(z)));
    if constexpr (op == 0x86) return and_(or_(y, z), xor_(z, xor_(x, y)));
    if constexpr (op == 0x87) return not_(xor_(x, and_(y, z)));
    if constexpr (op == 0x88) return and_(z, y);
    if constexpr (op == 0x89) return andn_(xor_(y, z), or_(not_(x), y));
    if constexpr (op == 0x8a) return andn_(andn_(y, x), z);
    if constexpr (op == 0x8b) return or_(and_(y, z), andn_(y, not_(x)));
    if constexpr (op == 0x8c) return andn_(andn_(z, x), y);
    if constexpr (op == 0x8d) return or_(and_(z, y), andn_(z, not_(x)));
    if constexpr (op == 0x8e) return or_(and_(y, z), andn_(x, xor_(y, z)));
    if constexpr (op == 0x8f) return or_(and_(y, z), not_(x));

    if constexpr (op == 0x90) return andn_(xor_(y, z), x);
    if constexpr (op == 0x91) return andn_(xor_(y, z), or_(x, not_(y)));
    if constexpr (op == 0x92) return and_(or_(x, z), xor_(z, xor_(x, y)));
    if constexpr (op == 0x93) return not_(xor_(y, and_(x, z)));
    if constexpr (op == 0x94) return and_(or_(x, y), xor_(y, xor_(x, z)));
    if constexpr (op == 0x95) return not_(xor_(z, and_(y, x)));
    if constexpr (op == 0x96) return xor_(x, xor_(y, z));
    if constexpr (op == 0x97) return or_(andn_(xor_(y, z), x), andn_(x, not_(and_(y, z))));
    if constexpr (op == 0x98) return andn_(xor_(y, z), or_(x, y));
    if constexpr (op == 0x99) return not_(xor_(z, y));
    if constexpr (op == 0x9a) return xor_(andn_(y, x), z);
    if constexpr (op == 0x9b) return or_(andn_(x, z), xor_(y, not_(z)));
    if constexpr (op == 0x9c) return xor_(andn_(z, x), y);
    if constexpr (op == 0x9d) return or_(andn_(x, y), xor_(y, not_(z)));
    if constexpr (op == 0x9e) return or_(and_(y, z), xor_(z, xor_(x, y)));
    if constexpr (op == 0x9f) return not_(and_(x, xor_(y, z)));

    if constexpr (op == 0xa0) return and_(z, x);
    if constexpr (op == 0xa1) return andn_(xor_(x, z), or_(x, not_(y)));
    if constexpr (op == 0xa2) return andn_(andn_(x, y), z);
    if constexpr (op == 0xa3) return or_(and_(x, z), andn_(x, not_(y)));
    if constexpr (op == 0xa4) return andn_(xor_(x, z), or_(x, y));
    if constexpr (op == 0xa5) return not_(xor_(z, x));
    if constexpr (op == 0xa6) return xor_(andn_(x, y), z);
    if constexpr (op == 0xa7) return or_(andn_(y, z), xor_(x, not_(z)));
    if constexpr (op == 0xa8) return and_(z, or_(x, y));
    if constexpr (op == 0xa9) return not_(xor_(z, or_(y, x)));
    if constexpr (op == 0xaa) return z;
    if constexpr (op == 0xab) return or_(z, not_(or_(y, x)));
    if constexpr (op == 0xac) return or_(and_(x, z), andn_(x, y));
    if constexpr (op == 0xad) return or_(and_(y, z), xor_(x, not_(z)));
    if constexpr (op == 0xae) return or_(z, andn_(x, y));
    if constexpr (op == 0xaf) return or_(z, not_(x));

    if constexpr (op == 0xb0) return andn_(andn_(z, y), x);
    if constexpr (op == 0xb1) return or_(and_(z, x), andn_(z, not_(y)));
    if constexpr (op == 0xb2) return or_(and_(y, and_(x, z)), andn_(y, or_(x, z)));
    if constexpr (op == 0xb3) return or_(and_(x, z), not_(y));
    if constexpr (op == 0xb4) return xor_(andn_(z, y), x);
    if constexpr (op == 0xb5) return or_(andn_(y, x), xor_(x, not_(z)));
    if constexpr (op == 0xb6) return or_(and_(x, z), xor_(x, xor_(y, z)));
    if constexpr (op == 0xb7) return not_(and_(y, xor_(x, z)));
    if constexpr (op == 0xb8) return or_(and_(y, z), andn_(y, x));
    if constexpr (op == 0xb9) return or_(and_(x, z), xor_(y, not_(z)));
    if constexpr (op == 0xba) return or_(andn_(y, x), z);
    if constexpr (op == 0xbb) return or_(z, not_(y));
    if constexpr (op == 0xbc) return or_(and_(x, z), xor_(x, y));
    if constexpr (op == 0xbd) return or_(xor_(x, y), xor_(x, not_(z)));
    if constexpr (op == 0xbe) return or_(z, xor_(y, x));
    if constexpr (op == 0xbf) return or_(z, not_(and_(y, x)));

    if constexpr (op == 0xc0) return and_(y, x);
    if constexpr (op == 0xc1) return andn_(xor_(x, y), or_(x, not_(z)));
    if constexpr (op == 0xc2) return andn_(xor_(y, x), or_(x, z));
    if constexpr (op == 0xc3) return not_(xor_(y, x));
    if constexpr (op == 0xc4) return andn_(andn_(x, z), y);
    if constexpr (op == 0xc5) return or_(and_(x, y), andn_(x, not_(z)));
    if constexpr (op == 0xc6) return xor_(andn_(x, z), y);
    if constexpr (op == 0xc7) return or_(andn_(z, y), xor_(x, not_(y)));
    if constexpr (op == 0xc8) return and_(y, or_(x, z));
    if constexpr (op == 0xc9) return not_(xor_(y, or_(x, z)));
    if constexpr (op == 0xca) return or_(and_(x, y), andn_(x, z));
    if constexpr (op == 0xcb) return or_(and_(y, z), xor_(x, not_(y)));
    if constexpr (op == 0xcc) return y;
    if constexpr (op == 0xcd) return or_(y, not_(or_(x, z)));
    if constexpr (op == 0xce) return or_(andn_(x, z), y);
    if constexpr (op == 0xcf) return or_(y, not_(x));

    if constexpr (op == 0xd0) return andn_(andn_(y, z), x);
    if constexpr (op == 0xd1) return or_(and_(y, x), andn_(y, not_(z)));
    if constexpr (op == 0xd2) return xor_(andn_(y, z), x);
    if constexpr (op == 0xd3) return or_(andn_(z, x), xor_(x, not_(y)));
    if constexpr (op == 0xd4) return or_(andn_(z, y), andn_(xor_(y, z), x));
    if constexpr (op == 0xd5) return or_(and_(x, y), not_(z));
    if constexpr (op == 0xd6) return or_(and_(x, y), xor_(y, xor_(x, z)));
    if constexpr (op == 0xd7) return not_(and_(z, xor_(y, x)));
    if constexpr (op == 0xd8) return or_(and_(z, y), andn_(z, x));
    if constexpr (op == 0xd9) return or_(and_(x, y), xor_(y, not_(z)));
    if constexpr (op == 0xda) return or_(and_(x, y), xor_(x, z));
    if constexpr (op == 0xdb) return or_(xor_(x, z), xor_(x, not_(y)));
    if constexpr (op == 0xdc) return or_(andn_(z, x), y);
    if constexpr (op == 0xdd) return or_(y, not_(z));
    if constexpr (op == 0xde) return or_(y, xor_(x, z));
    if constexpr (op == 0xdf) return or_(y, not_(and_(x, z)));

    if constexpr (op == 0xe0) return and_(x, or_(y, z));
    if constexpr (op == 0xe1) return not_(xor_(x, or_(y, z)));
    if constexpr (op == 0xe2) return or_(and_(y, x), andn_(y, z));
    if constexpr (op == 0xe3) return or_(and_(x, z), xor_(x, not_(y)));
    if constexpr (op == 0xe4) return or_(and_(z, x), andn_(z, y));
    if constexpr (op == 0xe5) return or_(and_(x, y), xor_(x, not_(z)));
    if constexpr (op == 0xe6) return or_(and_(x, y), xor_(y, z));
    if constexpr (op == 0xe7) return or_(xor_(y, z), xor_(x, not_(y)));
    if constexpr (op == 0xe8) return or_(and_(y, z), and_(x, xor_(y, z)));
    if constexpr (op == 0xe9) return or_(and_(x, y), xor_(y, xor_(x, not_(z))));
    if constexpr (op == 0xea) return or_(z, and_(y, x));
    if constexpr (op == 0xeb) return or_(z, not_(xor_(y, x)));
    if constexpr (op == 0xec) return or_(y, and_(x, z));
    if constexpr (op == 0xed) return or_(y, not_(xor_(x, z)));
    if constexpr (op == 0xee) return or_(z, y);
    if constexpr (op == 0xef) return or_(y, or_(not_(x), z));

    if constexpr (op == 0xf0) return x;
    if constexpr (op == 0xf1) return or_(x, not_(or_(y, z)));
    if constexpr (op == 0xf2) return or_(andn_(y, z), x);
    if constexpr (op == 0xf3) return or_(x, not_(y));
    if constexpr (op == 0xf4) return or_(andn_(z, y), x);
    if constexpr (op == 0xf5) return or_(x, not_(z));
    if constexpr (op == 0xf6) return or_(x, xor_(y, z));
    if constexpr (op == 0xf7) return or_(x, not_(and_(y, z)));
    if constexpr (op == 0xf8) return or_(x, and_(y, z));
    if constexpr (op == 0xf9) return or_(x, not_(xor_(y, z)));
    if constexpr (op == 0xfa) return or_(z, x);
    if constexpr (op == 0xfb) return or_(x, or_(not_(y), z));
    if constexpr (op == 0xfc) return or_(y, x);
    if constexpr (op == 0xfd) return or_(or_(x, y), not_(z));
    if constexpr (op == 0xfe) return or_(or_(x, y), z);
    if constexpr (op == 0xff) return not_(zero());
}

template <class V, u8 Op, raw_mask_type M, class Select, class Or,
    class Xor, class And, class Andnot, class Not, class Zero>
raze_nodiscard raze_always_inline V ternarylogic_emulated_(V x, V y, V z,
    std::integral_constant<u8, Op> op, M mask, Select select, Or or_,
    Xor xor_, And and_, Andnot andn_, Not not_, Zero zero) noexcept
{
    return __select(ternarylogic_emulated_(x, y, z, op, or_, xor_, and_, andn_, not_, zero), mask);
}


template <class V, u8 Op, raw_mask_type M, class Select, class Or,
    class Xor, class And, class Andnot, class Not, class Zero>
raze_always_inline V ternarylogic_emulated_(V x, V y, V z,
    std::integral_constant<u8, Op> op, M mask, Select select, V src, Or or_,
    Xor xor_, And and_, Andnot andn_, Not not_, Zero zero) noexcept
{
    return select(ternarylogic_emulated_(x, y, z, op, or_, xor_, and_, andn_, not_, zero), src, mask);
}

__RAZE_VX_NAMESPACE_END
