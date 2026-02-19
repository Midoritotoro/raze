#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/utility/Assert.h>

#if defined(min)
#  undef min
#endif // defined(min)

#if defined(max)
#  undef max
#endif // defined(max)

#include <benchmark/benchmark.h>

#include <iostream>
#include <map>

#include <sstream>
#include <array>

#include <iomanip>

#if !defined(RAZE_BENCHMARK_REPITITIONS)
#  define RAZE_BENCHMARK_REPITITIONS 1000000
#endif // RAZE_BENCHMARK_REPITITIONS

#if !defined(RAZE_BENCHMARK_ITERATIONS)
#  define RAZE_BENCHMARK_ITERATIONS 1
#endif // RAZE_BENCHMARK_ITERATIONS

#if defined(RAZE_BENCHMARK_IN_MILLISECONDS)
#  define RAZE_BENCHMARK_UNIT_OF_MEASUREMENT benchmark::kMillisecond
#else 
#  define RAZE_BENCHMARK_UNIT_OF_MEASUREMENT benchmark::kNanosecond
#endif // RAZE_BENCHMARK_UNIT_OF_MEASUREMENT

enum SizeForBenchmark : raze::uint32 {
    Tiny        = 16,       
    VerySmall   = 32,   
    Small       = 64,    
    MediumSmall = 128,
    Medium      = 256,
    MediumLarge = 512,
    Large       = 1024,
    VeryLarge   = 2048,
    Huge        = 4096,
    ExtraHuge   = 8192,
    MegaHuge    = 16384,
    GigaHuge    = 32768,
    TeraHuge    = 65536  
};

#define RAZE_FIXED_INTEGER_ARRAY(name) \
    template <typename _Type_, size_t N>\
    struct name {\
        _Type_ data[N + 1];\
    \
        constexpr name() {\
            for (size_t i = 0; i < N; ++i) {\
                data[i] = i;\
            }\
        }\
    }

#define RAZE_FIXED_CHAR_ARRAY(name, prefix) \
    template <typename _Type_, size_t N>\
    struct name {\
        _Type_ data[N + 1];\
    \
        constexpr name() {\
            for (size_t i = 0; i < N; ++i) {\
                data[i] = prefix'A' + (i % 26);\
            }\
            data[N] = prefix'\0';\
        }\
    };

#define RAZE_FIXED_REVERSED_CHAR_ARRAY(name, prefix) \
    template <typename _Type_, size_t N>\
    struct name {\
        _Type_ data[N + 1];\
    \
        constexpr name() {\
            for (size_t i = 0; i < N; ++i) {\
                data[i] = prefix'Z' - (i % 26);\
            }\
            data[N] = prefix'\0';\
        }\
    };

#define RAZE_ADD_SPECIALIZATION_TO_FIXED_CHAR_ARRAY(name, type, prefix) \
    template <size_t N>\
    struct name<type, N> {\
        type data[N + 1];\
    \
        constexpr name() {\
            for (size_t i = 0; i < N; ++i) {\
                data[i] = prefix'A' + (i % 26);\
            }\
            data[N] = prefix'\0';\
        }\
    };

#define RAZE_ADD_SPECIALIZATION_TO_FIXED_INTEGER_ARRAY(name, type) \
    template <size_t N>\
    struct name<type, N> {\
        type data[N];\
    \
        constexpr name() {\
            for (size_t i = 0; i < N; ++i) {\
                data[i] = i;\
            }\
        }\
    }

#define RAZE_ADD_SPECIALIZATION_TO_FIXED_REVERSED_CHAR_ARRAY(name, type, prefix) \
    template <size_t N>\
    struct name<type, N> {\
        type data[N + 1];\
    \
        constexpr name() {\
            for (size_t i = 0; i < N; ++i) {\
                data[i] = prefix'Z' - (i % 26);\
            }\
            data[N] = prefix'\0';\
        }\
    };


RAZE_FIXED_CHAR_ARRAY(FixedArray, );
#if __cpp_lib_char8_t
RAZE_ADD_SPECIALIZATION_TO_FIXED_CHAR_ARRAY(FixedArray, char8_t, u8);
#endif
RAZE_ADD_SPECIALIZATION_TO_FIXED_CHAR_ARRAY(FixedArray, char16_t, u);
RAZE_ADD_SPECIALIZATION_TO_FIXED_CHAR_ARRAY(FixedArray, char32_t, U);
RAZE_ADD_SPECIALIZATION_TO_FIXED_CHAR_ARRAY(FixedArray, wchar_t, L);



RAZE_FIXED_REVERSED_CHAR_ARRAY(FixedReversedArray, );
#if __cpp_lib_char8_t
RAZE_ADD_SPECIALIZATION_TO_FIXED_REVERSED_CHAR_ARRAY(FixedReversedArray, char8_t, u8);
#endif
RAZE_ADD_SPECIALIZATION_TO_FIXED_REVERSED_CHAR_ARRAY(FixedReversedArray, char16_t, u);
RAZE_ADD_SPECIALIZATION_TO_FIXED_REVERSED_CHAR_ARRAY(FixedReversedArray, char32_t, U);
RAZE_ADD_SPECIALIZATION_TO_FIXED_REVERSED_CHAR_ARRAY(FixedReversedArray, wchar_t, L);


RAZE_FIXED_INTEGER_ARRAY(FixedIntegerArray);
RAZE_ADD_SPECIALIZATION_TO_FIXED_INTEGER_ARRAY(FixedIntegerArray, raze::uint8);
RAZE_ADD_SPECIALIZATION_TO_FIXED_INTEGER_ARRAY(FixedIntegerArray, raze::uint16);
RAZE_ADD_SPECIALIZATION_TO_FIXED_INTEGER_ARRAY(FixedIntegerArray, raze::uint32);
RAZE_ADD_SPECIALIZATION_TO_FIXED_INTEGER_ARRAY(FixedIntegerArray, raze::uint64);


#if !defined(RAZE_ADD_BENCHMARK_WITH_CUSTOM_REPITITIONS)
#  define RAZE_ADD_BENCHMARK_WITH_CUSTOM_REPITITIONS(benchFirst, benchSecond, repititions)                   \
     BENCHMARK(benchFirst)->Unit(RAZE_BENCHMARK_UNIT_OF_MEASUREMENT)    \
        ->Iterations(RAZE_BENCHMARK_ITERATIONS)                         \
        ->Repetitions(repititions)                       \
        ->ReportAggregatesOnly(false)                                    \
        ->DisplayAggregatesOnly(true);                                  \
    BENCHMARK(benchSecond)->Unit(RAZE_BENCHMARK_UNIT_OF_MEASUREMENT)    \
        ->Iterations(RAZE_BENCHMARK_ITERATIONS)                         \
        ->Repetitions(repititions)                       \
        ->ReportAggregatesOnly(false)                                    \
        ->DisplayAggregatesOnly(true);                                   
#endif // RAZE_ADD_BENCHMARK_WITH_CUSTOM_REPITITIONS



#if !defined(RAZE_ADD_BENCHMARK)
#  define RAZE_ADD_BENCHMARK(benchFirst, benchSecond)                   \
     BENCHMARK(benchFirst)->Unit(RAZE_BENCHMARK_UNIT_OF_MEASUREMENT);   \
     BENCHMARK(benchSecond)->Unit(RAZE_BENCHMARK_UNIT_OF_MEASUREMENT);
#endif // RAZE_ADD_BENCHMARK

#if !defined(RAZE_ADD_BENCHMARK_ARGS)
#  define RAZE_ADD_BENCHMARK_ARGS(benchFirst, benchSecond, ...)         \
     BENCHMARK(benchFirst)->Unit(RAZE_BENCHMARK_UNIT_OF_MEASUREMENT)    \
        ->Iterations(RAZE_BENCHMARK_ITERATIONS)                         \
        ->Repetitions(RAZE_BENCHMARK_REPITITIONS)                       \
        ->ReportAggregatesOnly(false)                                    \
        ->DisplayAggregatesOnly(true)                                   \
        ->Args({__VA_ARGS__});                                          \
    BENCHMARK(benchSecond)->Unit(RAZE_BENCHMARK_UNIT_OF_MEASUREMENT)    \
        ->Iterations(RAZE_BENCHMARK_ITERATIONS)                         \
        ->Repetitions(RAZE_BENCHMARK_REPITITIONS)                       \
        ->ReportAggregatesOnly(false)                                    \
        ->DisplayAggregatesOnly(true)                                   \
        ->Args({__VA_ARGS__});                                            
#endif // RAZE_ADD_BENCHMARK_ARGS


#if !defined(RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE)
#  define RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(nameFirst, nameSecond, Type, funcN)                                                                                                          \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Tiny>::funcN));                    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VerySmall>::funcN),RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VerySmall>::funcN));           \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Small>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Small>::funcN));                  \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumSmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumSmall>::funcN));      \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Medium>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Medium>::funcN));                \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumLarge>::funcN),RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumLarge>::funcN));       \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Large>::funcN),RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Large>::funcN));                   \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VeryLarge>::funcN),RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VeryLarge>::funcN));           \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Huge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Huge>::funcN));                    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::ExtraHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::ExtraHuge>::funcN));          \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MegaHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MegaHuge>::funcN));            \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge>::funcN));            \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge>::funcN));
#endif // RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE

#if !defined(RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE_ARGS)
#  define RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE_ARGS(nameFirst, nameSecond, Type, funcN, ...)                                                                                                                \
    RAZE_ADD_BENCHMARK_ARGS(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Tiny>::funcN), __VA_ARGS__);                  \
    RAZE_ADD_BENCHMARK_ARGS(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VerySmall>::funcN),RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VerySmall>::funcN), __VA_ARGS__);         \
    RAZE_ADD_BENCHMARK_ARGS(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Small>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Small>::funcN), __VA_ARGS__);                \
    RAZE_ADD_BENCHMARK_ARGS(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumSmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumSmall>::funcN), __VA_ARGS__);    \
    RAZE_ADD_BENCHMARK_ARGS(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Medium>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Medium>::funcN), __VA_ARGS__);              \
    RAZE_ADD_BENCHMARK_ARGS(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumLarge>::funcN),RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumLarge>::funcN), __VA_ARGS__);     \
    RAZE_ADD_BENCHMARK_ARGS(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Large>::funcN),RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Large>::funcN), __VA_ARGS__);                 \
    RAZE_ADD_BENCHMARK_ARGS(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VeryLarge>::funcN),RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VeryLarge>::funcN), __VA_ARGS__);         \
    RAZE_ADD_BENCHMARK_ARGS(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Huge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Huge>::funcN), __VA_ARGS__);                  \
    RAZE_ADD_BENCHMARK_ARGS(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::ExtraHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::ExtraHuge>::funcN), __VA_ARGS__);        \
    RAZE_ADD_BENCHMARK_ARGS(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MegaHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MegaHuge>::funcN), __VA_ARGS__);          \
    RAZE_ADD_BENCHMARK_ARGS(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge>::funcN), __VA_ARGS__);          \
    RAZE_ADD_BENCHMARK_ARGS(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge>::funcN), __VA_ARGS__);
#endif // RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE_ARGS

#if !defined(RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE)
#define RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(nameFirst, nameSecond, Type, funcN) \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Tiny, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Tiny, SizeForBenchmark::Tiny>::funcN)); \
    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VerySmall, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VerySmall, SizeForBenchmark::Tiny>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VerySmall, SizeForBenchmark::VerySmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VerySmall, SizeForBenchmark::VerySmall>::funcN)); \
    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Small, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Small, SizeForBenchmark::Tiny>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Small, SizeForBenchmark::VerySmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Small, SizeForBenchmark::VerySmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Small, SizeForBenchmark::Small>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Small, SizeForBenchmark::Small>::funcN)); \
    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumSmall, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumSmall, SizeForBenchmark::Tiny>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumSmall, SizeForBenchmark::VerySmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumSmall, SizeForBenchmark::VerySmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumSmall, SizeForBenchmark::Small>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumSmall, SizeForBenchmark::Small>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumSmall, SizeForBenchmark::MediumSmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumSmall, SizeForBenchmark::MediumSmall>::funcN)); \
    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Medium, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Medium, SizeForBenchmark::Tiny>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Medium, SizeForBenchmark::VerySmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Medium, SizeForBenchmark::VerySmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Medium, SizeForBenchmark::Small>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Medium, SizeForBenchmark::Small>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Medium, SizeForBenchmark::MediumSmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Medium, SizeForBenchmark::MediumSmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Medium, SizeForBenchmark::Medium>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Medium, SizeForBenchmark::Medium>::funcN)); \
    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumLarge, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumLarge, SizeForBenchmark::Tiny>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumLarge, SizeForBenchmark::VerySmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumLarge, SizeForBenchmark::VerySmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumLarge, SizeForBenchmark::Small>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumLarge, SizeForBenchmark::Small>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumLarge, SizeForBenchmark::MediumSmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumLarge, SizeForBenchmark::MediumSmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumLarge, SizeForBenchmark::Medium>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumLarge, SizeForBenchmark::Medium>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MediumLarge, SizeForBenchmark::MediumLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MediumLarge, SizeForBenchmark::MediumLarge>::funcN)); \
    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Large, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Large, SizeForBenchmark::Tiny>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Large, SizeForBenchmark::VerySmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Large, SizeForBenchmark::VerySmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Large, SizeForBenchmark::Small>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Large, SizeForBenchmark::Small>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Large, SizeForBenchmark::MediumSmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Large, SizeForBenchmark::MediumSmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Large, SizeForBenchmark::Medium>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Large, SizeForBenchmark::Medium>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Large, SizeForBenchmark::MediumLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Large, SizeForBenchmark::MediumLarge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Large, SizeForBenchmark::Large>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Large, SizeForBenchmark::Large>::funcN)); \
    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::Tiny>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::VerySmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::VerySmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::Small>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::Small>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::MediumSmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::MediumSmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::Medium>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::Medium>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::MediumLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::MediumLarge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::Large>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::Large>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::VeryLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::VeryLarge, SizeForBenchmark::VeryLarge>::funcN)); \
    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Huge, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Huge, SizeForBenchmark::Tiny>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Huge, SizeForBenchmark::VerySmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Huge, SizeForBenchmark::VerySmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Huge, SizeForBenchmark::Small>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Huge, SizeForBenchmark::Small>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Huge, SizeForBenchmark::MediumSmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Huge, SizeForBenchmark::MediumSmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Huge, SizeForBenchmark::Medium>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Huge, SizeForBenchmark::Medium>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Huge, SizeForBenchmark::MediumLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Huge, SizeForBenchmark::MediumLarge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Huge, SizeForBenchmark::Large>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Huge, SizeForBenchmark::Large>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Huge, SizeForBenchmark::VeryLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Huge, SizeForBenchmark::VeryLarge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::Huge, SizeForBenchmark::Huge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::Huge, SizeForBenchmark::Huge>::funcN)); \
    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::Tiny>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::VerySmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::VerySmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::Small>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::Small>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::MediumSmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::MediumSmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::Medium>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::Medium>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::MediumLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::MediumLarge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::Large>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::Large>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::VeryLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::VeryLarge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::Huge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::Huge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::ExtraHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::ExtraHuge, SizeForBenchmark::ExtraHuge>::funcN)); \
    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::Tiny>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::VerySmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::VerySmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::Small>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::Small>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::MediumSmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::MediumSmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::Medium>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::Medium>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::MediumLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::MediumLarge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::Large>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::Large>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::VeryLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::VeryLarge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::Huge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::Huge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::ExtraHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::ExtraHuge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::MegaHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::MegaHuge, SizeForBenchmark::MegaHuge>::funcN)); \
    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::Tiny>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::VerySmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::VerySmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::Small>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::Small>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::MediumSmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::MediumSmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::Medium>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::Medium>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::MediumLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::MediumLarge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::Large>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::Large>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::VeryLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::VeryLarge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::Huge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::Huge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::ExtraHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::ExtraHuge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::MegaHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::MegaHuge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::GigaHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::GigaHuge, SizeForBenchmark::GigaHuge>::funcN)); \
    \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::Tiny>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::Tiny>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::VerySmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::VerySmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::Small>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::Small>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::MediumSmall>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::MediumSmall>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::Medium>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::Medium>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::MediumLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::MediumLarge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::Large>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::Large>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::VeryLarge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::VeryLarge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::Huge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::Huge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::ExtraHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::ExtraHuge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::MegaHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::MegaHuge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::GigaHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::GigaHuge>::funcN)); \
    RAZE_ADD_BENCHMARK(RAZE_ECHO(nameFirst<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::TeraHuge>::funcN), RAZE_ECHO(nameSecond<Type, SizeForBenchmark::TeraHuge, SizeForBenchmark::GigaHuge>::funcN));
#endif


#if !defined(RAZE_BENCHMARK_MAIN)
#define RAZE_BENCHMARK_MAIN()                                                   \
    int main(int argc, char** argv) {                                           \
        benchmark::MaybeReenterWithoutASLR(argc, argv);                         \
        char arg0_default[] = "benchmark";                                      \
        char* args_default = reinterpret_cast<char*>(arg0_default);             \
        if (!argv) {                                                            \
            argc = 1;                                                           \
            argv = &args_default;                                               \
        }                                                                       \
        ::benchmark::Initialize(&argc, argv);                                   \
        if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;     \
        BenchmarksCompareReporter reporter;                                     \
        ::benchmark::RunSpecifiedBenchmarks(&reporter);                         \
        ::benchmark::Shutdown();                                                \
        return 0;                                                               \
    }                                                                           \
    int main(int, char**)
#endif // RAZE_BENCHMARK_MAIN


static struct Loc {
    Loc() {
        std::setlocale(LC_ALL, "en_US.UTF-8");
    }
} localeSet;


enum LogColor {
    COLOR_DEFAULT,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE
};

#ifdef raze_os_win
typedef WORD PlatformColorCode;
#else
typedef const char* PlatformColorCode;
#endif

PlatformColorCode GetPlatformColorCode(LogColor color) {
#ifdef raze_os_win
    switch (color) {
    case COLOR_RED:
        return FOREGROUND_RED;
    case COLOR_GREEN:
        return FOREGROUND_GREEN;
    case COLOR_YELLOW:
        return FOREGROUND_RED | FOREGROUND_GREEN;
    case COLOR_BLUE:
        return FOREGROUND_BLUE;
    case COLOR_MAGENTA:
        return FOREGROUND_BLUE | FOREGROUND_RED;
    case COLOR_CYAN:
        return FOREGROUND_BLUE | FOREGROUND_GREEN;
    case COLOR_WHITE:  // fall through to default
    default:
        return 0;
    }
#else
    switch (color) {
    case COLOR_RED:
        return "1";
    case COLOR_GREEN:
        return "2";
    case COLOR_YELLOW:
        return "3";
    case COLOR_BLUE:
        return "4";
    case COLOR_MAGENTA:
        return "5";
    case COLOR_CYAN:
        return "6";
    case COLOR_WHITE:
        return "7";
    default:
        return nullptr;
    };
#endif
}

std::string FormatString(const char* msg, va_list args) {
    // we might need a second shot at this, so pre-emptivly make a copy
    va_list args_cp;
    va_copy(args_cp, args);

    std::size_t size = 256;
    char local_buff[256];
    auto ret = vsnprintf(local_buff, size, msg, args_cp);

    va_end(args_cp);

    // currently there is no error handling for failure, so this is hack.
    raze_assert(ret >= 0);

    if (ret == 0) {  // handle empty expansion
        return {};
    }
    if (static_cast<size_t>(ret) < size) {
        return local_buff;
    }
    // we did not provide a long enough buffer on our first attempt.
    size = static_cast<size_t>(ret) + 1;  // + 1 for the null byte
    std::unique_ptr<char[]> buff(new char[size]);
    ret = vsnprintf(buff.get(), size, msg, args);
    raze_assert(ret > 0 && (static_cast<size_t>(ret)) < size);
    return buff.get();
}

std::string FormatString(const char* msg, ...) {
    va_list args;
    va_start(args, msg);
    auto tmp = FormatString(msg, args);
    va_end(args);
    return tmp;
}


void ColorPrintf(
    std::ostream& out,
    LogColor color,
    const char* fmt,
    ...) 
{
    va_list args;
    va_start(args, fmt);

#ifdef raze_os_win
    const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    // Gets the current text color.
    CONSOLE_SCREEN_BUFFER_INFO buffer_info;
    GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
    const WORD original_color_attrs = buffer_info.wAttributes;

    const WORD original_background_attrs =
        original_color_attrs & (BACKGROUND_RED | BACKGROUND_GREEN |
            BACKGROUND_BLUE | BACKGROUND_INTENSITY);

    SetConsoleTextAttribute(stdout_handle, GetPlatformColorCode(color) |
        FOREGROUND_INTENSITY |
        original_background_attrs);
    out << FormatString(fmt, args);

    out << std::flush;
    // Restores the text and background color.
    SetConsoleTextAttribute(stdout_handle, original_color_attrs);
#else
    const char* color_code = GetPlatformColorCode(color);
    if (color_code != nullptr) {
        out << FormatString("\033[0;3%sm", color_code);
    }
    out << FormatString(fmt, args) << "\033[m";
#endif
    va_end(args);
}


inline std::vector<benchmark::BenchmarkReporter::Run> _benchmarks;

class BenchmarksCompareReporter : public benchmark::ConsoleReporter
{
public:
    BenchmarksCompareReporter() {}

    bool ReportContext(const Context& context) override {
        return ConsoleReporter::ReportContext(context);
    }

    void ReportRuns(const std::vector<Run>& reports) override {
        std::ostream& out = GetOutputStream();
        ConsoleReporter::ReportRuns(reports);

        for (const auto& run : reports) {
            _benchmarks.push_back(run);

            if (_benchmarks.size() % 2 == 0) {
                const auto& first = _benchmarks[0];
                const auto& second = _benchmarks[1];

                const double t1 = first.GetAdjustedRealTime();
                const double t2 = second.GetAdjustedRealTime();

                const double speedup = t2 / t1;

                const auto name1 = first.benchmark_name().substr(0, first.benchmark_name().find('/'));
                const auto name2 = second.benchmark_name().substr(0, second.benchmark_name().find('/'));

                if (speedup > 1.0)
                {
                    ColorPrintf(out, COLOR_BLUE,
                        "%s (%.3f) is faster than %s (%.3f): speedup x%.3f\n",
                        name1.c_str(), t1,
                        name2.c_str(), t2,
                        speedup);
                }
                else if (speedup < 1.0)
                {
                    ColorPrintf(out, COLOR_RED,
                        "%s (%.3f) is slower than %s (%.3f): speedup x%.3f\n",
                        name1.c_str(), t1,
                        name2.c_str(), t2,
                        speedup);
                }
                else
                {
                    ColorPrintf(out, COLOR_WHITE, "Benchmarks are equal\n");
                }

                _benchmarks.clear();

            }
        }
    }
};