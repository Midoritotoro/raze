#include <raze/algorithm/remove/RemoveCopy.h>
#include <benchmarks/tools/BenchmarkHelper.h>
#include <numeric>
#include <algorithm>
#include <chrono>

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
static void BM_StdRemoveCopyIf(benchmark::State& state) {
    TestData<T, Size> src;
    FillTestData<T, Size, RemovePercent>(src.data);
    std::vector<T> dest(Size);

    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();

        benchmark::DoNotOptimize(src.data);
        benchmark::DoNotOptimize(dest);

        auto result = std::ranges::remove_copy(src.data, dest.begin(), T(0));
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(dest);

        benchmark::ClobberMemory();

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

        state.SetIterationTime(elapsed_seconds.count());
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
    state.SetLabel("remove_copy_" + std::to_string(RemovePercent) + "%");
}

template <class T, std::size_t Size, std::size_t RemovePercent>
static void BM_RazeRemoveCopyIf(benchmark::State& state) {
    TestData<T, Size> src;
    FillTestData<T, Size, RemovePercent>(src.data);

    std::vector<T> dest(Size);

    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();

        benchmark::DoNotOptimize(src.data);
        benchmark::DoNotOptimize(dest);

        auto result = raze::algorithm::remove_copy(src.data, dest.begin(), T(0));
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(dest);

        benchmark::ClobberMemory();

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

        state.SetIterationTime(elapsed_seconds.count());
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
    state.SetLabel("remove_copy_" + std::to_string(RemovePercent) + "%");
}

#define RAZE_BENCHMARK_REMOVE_COPY_PERCENT(name1, name2, pct) \
    BENCHMARK(name1<raze::i8, 16, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i8, 16, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i16, 16, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i16, 16, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i32, 16, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i32, 16, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i64, 16, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i64, 16, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f32, 16, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f32, 16, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f64, 16, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f64, 16, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    \
    BENCHMARK(name1<raze::i8, 1024, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i8, 1024, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i16, 1024, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i16, 1024, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i32, 1024, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i32, 1024, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i64, 1024, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i64, 1024, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f32, 1024, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f32, 1024, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f64, 1024, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f64, 1024, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    \
    BENCHMARK(name1<raze::i8, 4096, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i8, 4096, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i16, 4096, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i16, 4096, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i32, 4096, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i32, 4096, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i64, 4096, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i64, 4096, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f32, 4096, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f32, 4096, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f64, 4096, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f64, 4096, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    \
    BENCHMARK(name1<raze::i8, 16384, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i8, 16384, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i16, 16384, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i16, 16384, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i32, 16384, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i32, 16384, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i64, 16384, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i64, 16384, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f32, 16384, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f32, 16384, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f64, 16384, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f64, 16384, pct>)->UseManualTime()->Repetitions(3)->ReportAggregatesOnly(true)

void RegisterAll()
{
    RAZE_BENCHMARK_REMOVE_COPY_PERCENT(BM_RazeRemoveCopyIf, BM_StdRemoveCopyIf, 0);
    RAZE_BENCHMARK_REMOVE_COPY_PERCENT(BM_RazeRemoveCopyIf, BM_StdRemoveCopyIf, 25);
    RAZE_BENCHMARK_REMOVE_COPY_PERCENT(BM_RazeRemoveCopyIf, BM_StdRemoveCopyIf, 50);
    RAZE_BENCHMARK_REMOVE_COPY_PERCENT(BM_RazeRemoveCopyIf, BM_StdRemoveCopyIf, 75);
    RAZE_BENCHMARK_REMOVE_COPY_PERCENT(BM_RazeRemoveCopyIf, BM_StdRemoveCopyIf, 100);
}

RAZE_BENCHMARK_MAIN();