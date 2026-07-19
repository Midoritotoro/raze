#include <raze/algorithm/remove/Remove.h>
#include <benchmarks/tools/BenchmarkHelper.h>
#include <numeric>
#include <algorithm>

template <class T, std::size_t Size, std::size_t RemovePercent>
static void FillTestData(std::vector<T>& data) {
    static_assert(RemovePercent <= 100, "RemovePercent must be in [0, 100]");

    if constexpr (RemovePercent == 0) {
        for (std::size_t i = 0; i < Size; ++i) {
            data[i] = static_cast<T>(i * 2 + 1);
        }
    }
    else if constexpr (RemovePercent == 100) {
        for (std::size_t i = 0; i < Size; ++i) {
            data[i] = static_cast<T>(0);
        }
    }
    else {
        constexpr std::size_t stride = 100 / RemovePercent;
        for (std::size_t i = 0; i < Size; ++i) {
            data[i] = (i % stride == 0) ? static_cast<T>(0) : static_cast<T>((i % stride) * 2 + 1);
        }
    }
}

template <class T, std::size_t Size, std::size_t RemovePercent>
static void BM_StdRemoveIf(benchmark::State& state) {
    TestData<T, Size> test;
    FillTestData<T, Size, RemovePercent>(test.data);

    for (auto _ : state) {
        FillTestData<T, Size, RemovePercent>(test.data);

        benchmark::DoNotOptimize(test.data);

        auto result = std::ranges::remove(test.data, T(0));
        benchmark::DoNotOptimize(result);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
    state.SetLabel("remove_" + std::to_string(RemovePercent) + "%");
}

template <class T, std::size_t Size, std::size_t RemovePercent>
static void BM_RazeRemoveIf(benchmark::State& state) {
    TestData<T, Size> test;
    FillTestData<T, Size, RemovePercent>(test.data);

    for (auto _ : state) {
        FillTestData<T, Size, RemovePercent>(test.data);

        benchmark::DoNotOptimize(test.data);

        auto result = raze::algorithm::remove(test.data, T(0));
        benchmark::DoNotOptimize(result);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
    state.SetLabel("remove_" + std::to_string(RemovePercent) + "%");
}

#define RAZE_BENCHMARK_REMOVE_PERCENT(name1, name2, pct) \
    BENCHMARK(name1<raze::i8, 16, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i8, 16, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i16, 16, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i16, 16, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i32, 16, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i32, 16, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i64, 16, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i64, 16, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f32, 16, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f32, 16, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f64, 16, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f64, 16, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    \
    BENCHMARK(name1<raze::i8, 1024, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i8, 1024, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i16, 1024, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i16, 1024, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i32, 1024, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i32, 1024, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i64, 1024, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i64, 1024, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f32, 1024, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f32, 1024, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f64, 1024, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f64, 1024, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    \
    BENCHMARK(name1<raze::i8, 4096, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i8, 4096, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i16, 4096, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i16, 4096, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i32, 4096, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i32, 4096, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i64, 4096, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i64, 4096, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f32, 4096, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f32, 4096, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f64, 4096, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f64, 4096, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    \
    BENCHMARK(name1<raze::i8, 16384, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i8, 16384, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i16, 16384, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i16, 16384, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i32, 16384, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i32, 16384, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i64, 16384, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i64, 16384, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f32, 16384, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f32, 16384, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f64, 16384, pct>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f64, 16384, pct>)->Repetitions(10)->ReportAggregatesOnly(true)

void RegisterAll()
{
    //RAZE_BENCHMARK_REMOVE_PERCENT(BM_RazeRemoveIf, BM_StdRemoveIf, 0);
    //RAZE_BENCHMARK_REMOVE_PERCENT(BM_RazeRemoveIf, BM_StdRemoveIf, 25);
    //RAZE_BENCHMARK_REMOVE_PERCENT(BM_RazeRemoveIf, BM_StdRemoveIf, 50);
    //RAZE_BENCHMARK_REMOVE_PERCENT(BM_RazeRemoveIf, BM_StdRemoveIf, 75);
    RAZE_BENCHMARK_REMOVE_PERCENT(BM_RazeRemoveIf, BM_StdRemoveIf, 100);
}

RAZE_BENCHMARK_MAIN();