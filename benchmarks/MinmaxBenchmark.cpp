#include <raze/algorithm/extreme/ExtremeElements.h>
#include <raze/algorithm/extreme/MinElement.h>
#include <raze/algorithm/extreme/MaxElement.h>
#include <raze/algorithm/extreme/MinmaxElement.h>

#include <benchmarks/tools/BenchmarkHelper.h>
#include <algorithm>

template <class T, std::size_t Size>
static void BM_StdMin(benchmark::State& state) {
    TestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto it = std::ranges::min_element(test.data);
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeMin(benchmark::State& state) {
    TestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto min_it = raze::algorithm::min_element(test.data);
        benchmark::DoNotOptimize(min_it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_StdMax(benchmark::State& state) {
    TestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto it = std::ranges::max_element(test.data);
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeMax(benchmark::State& state) {
    TestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto max_it = raze::algorithm::max_element(test.data);
        benchmark::DoNotOptimize(max_it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_StdMinMax(benchmark::State& state) {
    TestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto [min_it, max_it] = std::ranges::minmax_element(test.data);
        benchmark::DoNotOptimize(min_it);
        benchmark::DoNotOptimize(max_it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeMinMax(benchmark::State& state) {
    TestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto [min_it, max_it] = raze::algorithm::minmax_element(test.data);
        benchmark::DoNotOptimize(min_it);
        benchmark::DoNotOptimize(max_it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

#define RAZE_BENCHMARK_EXTREME(name_std, name_raze) \
    BENCHMARK(name_std<raze::i8, 16>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::i8, 16>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::i16, 16>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::i16, 16>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::i32, 16>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::i32, 16>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::i64, 16>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::i64, 16>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::f32, 16>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::f32, 16>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::f64, 16>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::f64, 16>)->Repetitions(10)->ReportAggregatesOnly(true); \
    \
    BENCHMARK(name_std<raze::i8, 1024>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::i8, 1024>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::i16, 1024>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::i16, 1024>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::i32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::i32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::i64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::i64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::f32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::f32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::f64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::f64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true); \
    \
    BENCHMARK(name_std<raze::i8, 4096>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::i8, 4096>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::i16, 4096>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::i16, 4096>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::i32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::i32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::i64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::i64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::f32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::f32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_std<raze::f64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name_raze<raze::f64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);

void RegisterAll()
{
    RAZE_BENCHMARK_EXTREME(BM_StdMin, BM_RazeMin);
    RAZE_BENCHMARK_EXTREME(BM_StdMax, BM_RazeMax);
    RAZE_BENCHMARK_EXTREME(BM_StdMinMax, BM_RazeMinMax);
}

RAZE_BENCHMARK_MAIN();