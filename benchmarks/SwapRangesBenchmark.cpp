#include <raze/algorithm/swap/Swap.h>
#include <benchmarks/tools/BenchmarkHelper.h>
#include <array>
#include <ranges>
#include <algorithm>

template <class T, std::size_t Size>
struct SwapBenchData {
    std::array<T, Size> data1;
    std::array<T, Size> data2;

    SwapBenchData() {
        for (std::size_t i = 0; i < Size; ++i) {
            data1[i] = static_cast<T>(i);
            data2[i] = static_cast<T>(i + 1);
        }
    }
};

template <class T, std::size_t Size>
static void BM_StdSwapRanges(benchmark::State& state) {
    SwapBenchData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data1);
        benchmark::DoNotOptimize(test.data2);

        std::ranges::swap_ranges(test.data1, test.data2);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_RazeSwapRanges(benchmark::State& state) {
    SwapBenchData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data1);
        benchmark::DoNotOptimize(test.data2);

        raze::algorithm::swap_ranges(test.data1, test.data2);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

#define RAZE_BENCHMARK_SWAP_RANGES(name1, name2) \
    BENCHMARK(name1<raze::i8, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 20>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 20>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 20>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 20>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 20>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 20>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 20>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 20>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 20>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 20>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 20>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 20>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 1030>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 1030>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 1030>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 1030>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 1030>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 1030>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 1030>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 1030>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 1030>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 1030>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 1030>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 1030>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 16384>)->Repetitions(10)->ReportAggregatesOnly(true)

void RegisterAll()
{
    RAZE_BENCHMARK_SWAP_RANGES(BM_RazeSwapRanges, BM_StdSwapRanges);
}

RAZE_BENCHMARK_MAIN();