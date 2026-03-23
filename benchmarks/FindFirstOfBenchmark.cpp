#include <raze/algorithm/find/FindFirstOf.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <cstring>
#include <algorithm>

template <size_t N, typename _Char_>
static void __fill_pattern(_Char_(&__pattern)[N], _Char_ __target) noexcept {
    for (size_t i = 0; i < N - 1; ++i)
        __pattern[i] = static_cast<_Char_>(i + 1);
    __pattern[N - 1] = __target;
}

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class StdFindFirstOfBenchmark {
public:
    template <size_t N>
    static void __run(benchmark::State& state, size_t __pos) noexcept {
        _Char_ __array[sizeForBenchmark];
        std::memset(__array, 0, sizeof(__array));

        _Char_ __pattern[N];
        __fill_pattern(__pattern, static_cast<_Char_>(42));

        __array[__pos] = 42;

        while (state.KeepRunning()) {
            auto* __result = std::find_first_of(
                __array, __array + sizeForBenchmark,
                __pattern, __pattern + N
            );
            benchmark::DoNotOptimize(__result);
            benchmark::ClobberMemory();
        }
    }

    static void FindAtBegin(benchmark::State& state)  noexcept { __run<3>(state, 0); }
    static void FindAtMiddle(benchmark::State& state) noexcept { __run<3>(state, sizeForBenchmark / 2); }
    static void FindAtEnd(benchmark::State& state)    noexcept { __run<3>(state, sizeForBenchmark - 1); }

    static void FindAtBegin_P8(benchmark::State& state)  noexcept { __run<8>(state, 0); }
    static void FindAtMiddle_P8(benchmark::State& state) noexcept { __run<8>(state, sizeForBenchmark / 2); }
    static void FindAtEnd_P8(benchmark::State& state)    noexcept { __run<8>(state, sizeForBenchmark - 1); }

    static void FindAtBegin_P16(benchmark::State& state)  noexcept { __run<16>(state, 0); }
    static void FindAtMiddle_P16(benchmark::State& state) noexcept { __run<16>(state, sizeForBenchmark / 2); }
    static void FindAtEnd_P16(benchmark::State& state)    noexcept { __run<16>(state, sizeForBenchmark - 1); }

    static void FindAtBegin_P32(benchmark::State& state)  noexcept { __run<32>(state, 0); }
    static void FindAtMiddle_P32(benchmark::State& state) noexcept { __run<32>(state, sizeForBenchmark / 2); }
    static void FindAtEnd_P32(benchmark::State& state)    noexcept { __run<32>(state, sizeForBenchmark - 1); }

    static void FindAtBegin_P64(benchmark::State& state)  noexcept { __run<64>(state, 0); }
    static void FindAtMiddle_P64(benchmark::State& state) noexcept { __run<64>(state, sizeForBenchmark / 2); }
    static void FindAtEnd_P64(benchmark::State& state)    noexcept { __run<64>(state, sizeForBenchmark - 1); }
};

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class RazeFindFirstOfBenchmark {
public:
    template <size_t N>
    static void __run(benchmark::State& state, size_t __pos) noexcept {
        _Char_ __array[sizeForBenchmark];
        std::memset(__array, 0, sizeof(__array));

        _Char_ __pattern[N];
        __fill_pattern(__pattern, static_cast<_Char_>(42));

        __array[__pos] = 42;

        while (state.KeepRunning()) {
            auto* __result = raze::algorithm::find_first_of(
                __array, __array + sizeForBenchmark,
                __pattern, __pattern + N
            );
            benchmark::DoNotOptimize(__result);
            benchmark::ClobberMemory();
        }
    }

    static void FindAtBegin(benchmark::State& state)  noexcept { __run<3>(state, 0); }
    static void FindAtMiddle(benchmark::State& state) noexcept { __run<3>(state, sizeForBenchmark / 2); }
    static void FindAtEnd(benchmark::State& state)    noexcept { __run<3>(state, sizeForBenchmark - 1); }

    static void FindAtBegin_P8(benchmark::State& state)  noexcept { __run<8>(state, 0); }
    static void FindAtMiddle_P8(benchmark::State& state) noexcept { __run<8>(state, sizeForBenchmark / 2); }
    static void FindAtEnd_P8(benchmark::State& state)    noexcept { __run<8>(state, sizeForBenchmark - 1); }

    static void FindAtBegin_P16(benchmark::State& state)  noexcept { __run<16>(state, 0); }
    static void FindAtMiddle_P16(benchmark::State& state) noexcept { __run<16>(state, sizeForBenchmark / 2); }
    static void FindAtEnd_P16(benchmark::State& state)    noexcept { __run<16>(state, sizeForBenchmark - 1); }

    static void FindAtBegin_P32(benchmark::State& state)  noexcept { __run<32>(state, 0); }
    static void FindAtMiddle_P32(benchmark::State& state) noexcept { __run<32>(state, sizeForBenchmark / 2); }
    static void FindAtEnd_P32(benchmark::State& state)    noexcept { __run<32>(state, sizeForBenchmark - 1); }

    static void FindAtBegin_P64(benchmark::State& state)  noexcept { __run<64>(state, 0); }
    static void FindAtMiddle_P64(benchmark::State& state) noexcept { __run<64>(state, sizeForBenchmark / 2); }
    static void FindAtEnd_P64(benchmark::State& state)    noexcept { __run<64>(state, sizeForBenchmark - 1); }
};

#define RAZE_REGISTER_ALL(_type_) \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtBegin); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtMiddle); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtEnd); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtBegin_P8); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtMiddle_P8); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtEnd_P8); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtBegin_P16); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtMiddle_P16); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtEnd_P16); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtBegin_P32); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtMiddle_P32); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtEnd_P32); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtBegin_P64); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtMiddle_P64); \
    RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindFirstOfBenchmark, StdFindFirstOfBenchmark, _type_, FindAtEnd_P64);

RAZE_REGISTER_ALL(raze::uint8)
RAZE_REGISTER_ALL(raze::uint16)
RAZE_REGISTER_ALL(raze::uint32)
RAZE_REGISTER_ALL(raze::uint64)
RAZE_REGISTER_ALL(float)
RAZE_REGISTER_ALL(double)

RAZE_BENCHMARK_MAIN();
