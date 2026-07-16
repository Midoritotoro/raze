#include <raze/algorithm/find/Mismatch.h>
#include <benchmarks/tools/BenchmarkHelper.h>

template <class T, std::size_t Size>
struct MismatchTestData {
    std::array<T, Size> data1;
    std::array<T, Size> data2;

    MismatchTestData() {
        for (std::size_t i = 0; i < Size; ++i) {
            data1[i] = static_cast<T>(i + 1);
            data2[i] = static_cast<T>(i + 1);
        }

        data1[Size - 1] = 42;
    }
};

template <class T, std::size_t Size>
static void BM_StdMismatch(benchmark::State& state) {
    MismatchTestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data1);
        benchmark::DoNotOptimize(test.data2);

        auto result = std::ranges::mismatch(test.data1, test.data2);
        benchmark::DoNotOptimize(result);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_RazeMismatch(benchmark::State& state) {
    MismatchTestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data1);
        benchmark::DoNotOptimize(test.data2);

        auto result = raze::algorithm::mismatch(test.data1, test.data2);
        benchmark::DoNotOptimize(result);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_StdMismatchPred(benchmark::State& state) {
    MismatchTestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data1);
        benchmark::DoNotOptimize(test.data2);

        auto result = std::ranges::mismatch(test.data1, test.data2, [](auto a, auto b) { return (a * 2 + 1) == (b * 2 + 1); });
        benchmark::DoNotOptimize(result);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_RazeMismatchPred(benchmark::State& state) {
    MismatchTestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data1);
        benchmark::DoNotOptimize(test.data2);

        auto result = raze::algorithm::mismatch(test.data1, test.data2, [](auto a, auto b) { return (a * 2 + 1) == (b * 2 + 1); });
        benchmark::DoNotOptimize(result);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

#define RAZE_BENCHMARK_MISMATCH(name1, name2) \
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
    BENCHMARK(name2<raze::f64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);

void RegisterAll()
{
    RAZE_BENCHMARK_MISMATCH(BM_RazeMismatch, BM_StdMismatch);
    RAZE_BENCHMARK_MISMATCH(BM_RazeMismatchPred, BM_StdMismatchPred);
}

RAZE_BENCHMARK_MAIN();