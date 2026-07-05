#include <raze/algorithm/order/IsSorted.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>

template <class T, std::size_t Size>
static void BM_StdIsSorted(benchmark::State& state) {
    TestData<T, Size> test;
    std::sort(test.data.begin(), test.data.end());

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto result = std::ranges::is_sorted(test.data);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeIsSorted(benchmark::State& state) {
    TestData<T, Size> test;
    std::sort(test.data.begin(), test.data.end());

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto result = raze::algorithm::is_sorted(test.data);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size, std::size_t BreakPos>
static void BM_StdIsSortedUnsorted(benchmark::State& state) {
    TestData<T, Size> test;
    std::sort(test.data.begin(), test.data.end());
    if constexpr (BreakPos + 1 < Size) {
        std::swap(test.data[BreakPos], test.data[BreakPos + 1]);
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto result = std::ranges::is_sorted(test.data);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size, std::size_t BreakPos>
static void BM_RazeIsSortedUnsorted(benchmark::State& state) {
    TestData<T, Size> test;
    std::sort(test.data.begin(), test.data.end());
    if constexpr (BreakPos + 1 < Size) {
        std::swap(test.data[BreakPos], test.data[BreakPos + 1]);
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto result = raze::algorithm::is_sorted(test.data);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

#define RAZE_BENCHMARK_IS_SORTED(name1, name2) \
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

#define RAZE_BENCHMARK_IS_SORTED_UNSORTED(name1, name2, BreakPos) \
    BENCHMARK(name1<raze::i8, 1024, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 1024, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 1024, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 1024, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 1024, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 1024, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 1024, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 1024, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 1024, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 1024, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 1024, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 1024, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 4096, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 4096, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 4096, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 4096, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 4096, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 4096, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 4096, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 4096, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 4096, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 4096, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 4096, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 4096, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 16384, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 16384, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 16384, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 16384, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 16384, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 16384, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 16384, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 16384, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 16384, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 16384, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 16384, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 16384, BreakPos>)->Repetitions(10)->ReportAggregatesOnly(true)

void RegisterAll() {
    RAZE_BENCHMARK_IS_SORTED(BM_RazeIsSorted, BM_StdIsSorted);
    RAZE_BENCHMARK_IS_SORTED_UNSORTED(BM_RazeIsSortedUnsorted, BM_StdIsSortedUnsorted, 0);
    RAZE_BENCHMARK_IS_SORTED_UNSORTED(BM_RazeIsSortedUnsorted, BM_StdIsSortedUnsorted, 511);
    RAZE_BENCHMARK_IS_SORTED_UNSORTED(BM_RazeIsSortedUnsorted, BM_StdIsSortedUnsorted, 1022);
}

RAZE_BENCHMARK_MAIN();