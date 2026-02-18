#pragma once

#if !defined(raze_big_endian)
#  if defined(__ORDER_BIG_ENDIAN__)
#    define raze_big_endian __ORDER_BIG_ENDIAN__
#  else
#    define raze_big_endian 4321
#  endif // defined(__ORDER_BIG_ENDIAN__)
#endif // !defined(raze_big_endian)


#if !defined(raze_little_endian)
#  if defined(__ORDER_LITTLE_ENDIAN__)
#    define raze_little_endian __ORDER_LITTLE_ENDIAN__
#  else
#    define raze_little_endian 1234
#  endif // defined(__ORDER_LITTLE_ENDIAN__)
#endif // !defined(raze_little_endian)


#if defined(__alpha__) || defined(_M_ALPHA)
#  define raze_processor_alpha

#elif       defined(__arm__)    || defined(__TARGET_ARCH_ARM)   || defined(_M_ARM) || \
            defined(_M_ARM64)   || defined(__aarch64__)         || defined(__ARM64__)

#  if defined(__aarch64__) || defined(__ARM64__) || defined(_M_ARM64)
#    define raze_processor_arm_64
#    define raze_processor_wordsize 8
#  else
#    define raze_processor_arm_32
#  endif // defined(__aarch64__) || defined(__ARM64__) || defined(_M_ARM64)


#  if defined(__ARM_ARCH) && __ARM_ARCH > 1
#    define raze_processor_arm __ARM_ARCH

#  elif defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM > 1
#    define raze_processor_arm __TARGET_ARCH_ARM

#  elif defined(_M_ARM) && _M_ARM > 1
#    define raze_processor_arm _M_ARM

#  elif     defined(__ARM64_ARCH_8__) || defined(__aarch64__) || defined(__ARMv8__) \
      ||    defined(__ARMv8_A__) || defined(_M_ARM64)

#    define raze_processor_arm 8

#  elif defined(__ARM_ARCH_7__)     || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) ||\
        defined(__ARM_ARCH_7M__)    || defined(__ARM_ARCH_7S__) || defined(_ARM_ARCH_7) || defined(__CORE_CORTEXA__)
#    define raze_processor_arm 7

#  elif defined(__ARM_ARCH_6__)     || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6T2__) || \
        defined(__ARM_ARCH_6Z__)    || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6M__)
#    define raze_processor_arm 6

#  elif defined(__ARM_ARCH_5TEJ__) || defined(__ARM_ARCH_5TE__)
#    define raze_processor_arm 5

#  else
#    define raze_processor_arm 0
#  endif

#  if PROCESSOR_ARM >= 8
#    define raze_processor_arm_v8
#  endif // PROCESSOR_ARM >= 8

#  if PROCESSOR_ARM >= 7
#    define raze_processor_arm_v7
#  endif // PROCESSOR_ARM >= 7

#  if PROCESSOR_ARM >= 6
#    define raze_processor_arm_v6
#  endif // PROCESSOR_ARM >= 6

#  if PROCESSOR_ARM >= 5
#    define raze_processor_arm_v5
#  else
#    error "ARM architecture too old"
#  endif // PROCESSOR_ARM >= 5

#  if defined(__ARMEL__) || defined(_M_ARM64)
#    define raze_byte_order raze_little_endian
#  elif defined(__ARMEB__)
#    define raze_byte_order raze_big_endian
#  else
#  endif // defined(__ARMEL__) || defined(_M_ARM64) || defined(__ARMEB__)

#elif defined(__hppa__)
#  define raze_processor_hppa
#  define raze_byte_order raze_big_endian

#elif defined(__i386) || defined(__i386__) || defined(_M_IX86)
#  define raze_processor_x86_32
#  define raze_byte_order raze_little_endian
#  define raze_processor_wordsize   4


#  if defined(_M_IX86)
#    define raze_processor_x86     (_M_IX86/100)

#  elif defined(__i686__) || defined(__athlon__) || defined(__SSE__) || defined(__pentiumpro__)
#    define raze_processor_x86     6

#  elif defined(__i586__) || defined(__k6__) || defined(__pentium__)
#    define raze_processor_x86     5

#  elif defined(__i486__) || defined(__80486__)
#    define raze_processor_x86     4

#  else
#    define raze_processor_x86     3
#  endif

#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
#  define raze_processor_x86       6
#  define raze_processor_x86_64
#  define raze_byte_order raze_little_endian
#  define raze_processor_wordsize   8

#elif defined(__ia64) || defined(__ia64__) || defined(_M_IA64)
#  define raze_processor_ia64
#  define raze_processor_wordsize   8

#elif defined(__loongarch__)
#  define raze_processor_loongarch
#  if __loongarch_grlen == 64
#    define raze_processor_loongarch_64
#  else
#    define raze_processor_loongarch_32
#  endif
#  define raze_byte_order raze_little_endian


#elif defined(__m68k__)
#  define raze_processor_m68k
#  define raze_byte_order raze_big_endian

#elif defined(__mips) || defined(__mips__) || defined(_M_MRX000)
#  define raze_processor_mips

#  if defined(_MIPS_ARCH_MIPS1) || (defined(__mips) && __mips - 0 >= 1)
#    define raze_processor_mips_I
#  endif // defined(_MIPS_ARCH_MIPS1) || (defined(__mips) && __mips - 0 >= 1)

#  if defined(_MIPS_ARCH_MIPS2) || (defined(__mips) && __mips - 0 >= 2)
#    define raze_processor_mips_II
#  endif // defined(_MIPS_ARCH_MIPS2) || (defined(__mips) && __mips - 0 >= 2)

#  if defined(_MIPS_ARCH_MIPS3) || (defined(__mips) && __mips - 0 >= 3)
#    define raze_processor_mips_III
#  endif // defined(_MIPS_ARCH_MIPS3) || (defined(__mips) && __mips - 0 >= 3)

#  if defined(_MIPS_ARCH_MIPS4) || (defined(__mips) && __mips - 0 >= 4)
#    define raze_processor_mips_IV
#  endif // defined(_MIPS_ARCH_MIPS4) || (defined(__mips) && __mips - 0 >= 4)

#  if defined(_MIPS_ARCH_MIPS5) || (defined(__mips) && __mips - 0 >= 5)
#    define raze_processor_mips_V
#  endif // defined(_MIPS_ARCH_MIPS5) || (defined(__mips) && __mips - 0 >= 5)

#  if defined(_MIPS_ARCH_MIPS32) || defined(__mips32) || (defined(__mips) && __mips - 0 >= 32)
#    define raze_processor_mips_32
#  endif // defined(_MIPS_ARCH_MIPS32) || defined(__mips32) || (defined(__mips) && __mips - 0 >= 32)

#  if defined(_MIPS_ARCH_MIPS64) || defined(__mips64)
#    define raze_processor_mips_64
#    define raze_processor_wordsize 8
#  endif // defined(_MIPS_ARCH_MIPS64) || defined(__mips64)

#  if defined(__MIPSEL__)
#    define raze_byte_order raze_little_endian

#  elif defined(__MIPSEB__)
#    define raze_byte_order raze_big_endian

#  else
// BYTE_ORDER �� ���������, ������������� �������������� ����������� ������������������ ����
#  endif // defined(__MIPSEL__) || defined(__MIPSEB__)


#elif       defined(__ppc__)    || defined(__ppc)       || defined(__powerpc__) || defined(_ARCH_COM) \
        ||  defined(_ARCH_PWR)  || defined(_ARCH_PPC)   || defined(_M_MPPC)     || defined(_M_PPC)
#  define PROCESSOR_POWER

#  if defined(__ppc64__) || defined(__powerpc64__) || defined(__64BIT__)
#    define raze_processor_power_64
#    define raze_processor_wordsize 8
#  else
#    define raze_processor_power_32
#  endif // defined(__ppc64__) || defined(__powerpc64__) || defined(__64BIT__)


#elif defined(__riscv)
#  define raze_processor_riscv

#  if __riscv_xlen == 64
#    define raze_processor_riscv_64
#  else
#    define raze_processor_riscv_32
#  endif

#  define raze_byte_order raze_little_endian


#elif defined(__s390__)
#  define raze_processor_s390
#  if defined(__s390x__)
#    define raze_processor_s390_x
#  endif
#  define raze_byte_order raze_big_endian

#elif defined(__sparc__)
#  define raze_processor_sparc
#  if defined(__sparc_v9__) || defined(__sparcv9)
#    define raze_processor_sparc_v9
#  endif
#  if defined(__sparc64__)
#    define raze_processor_sparc_64
#  endif
#  define raze_byte_order raze_big_endian


// -- Web Assembly --
#elif defined(__EMSCRIPTEN__)
#  define raze_processor_wasm
#  define raze_byte_order raze_little_endian
#  define raze_processor_wordsize 8
#ifdef raze_compiler_supports_sse2
#  define raze_processor_x86 6   // �������� ��������� SIMD
# define raze_processor_x86_64 // wasm64
#  define raze_processor_wasm_64
#endif

#endif


#if !defined(raze_byte_order)
#  if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == raze_big_endian || __BYTE_ORDER__ == raze_little_endian)
// �������� ������������ __BYTE_ORDER__  ��� ����, ��������� ����������� *_ENDIAN ������������� ���������� ������������� �� ���������
#    define raze_byte_order __BYTE_ORDER__
#  elif defined(__BIG_ENDIAN__) || defined(_big_endian__) || defined(_BIG_ENDIAN)
#    define raze_byte_order raze_big_endian
#  elif defined(__LITTLE_ENDIAN__) || defined(_little_endian__) || defined(_LITTLE_ENDIAN)
#    define raze_byte_order raze_little_endian
#  else
#    error "Unable to determine byte order!"
#  endif
#endif // !defined(raze_byte_order)


#if defined(__SIZEOF_POINTER__)
#  define raze_pointer_size           __SIZEOF_POINTER__
#elif defined(__LP64__) || defined(_LP64)
#  define raze_pointer_size           8
#elif defined(PROCESSOR_WORDSIZE)
#  define raze_pointer_size           raze_processor_wordsize
#else
#  define raze_pointer_size           4
#endif // defined(__SIZEOF_POINTER__) || defined(__LP64__) || defined(_LP64) || defined(PROCESSOR_WORDSIZE)

#if !defined(raze_processor_wordsize)
#  define raze_processor_wordsize        raze_pointer_size
#endif // !defined(raze_processor_wordsize)