#include <raze/algorithm/order/Rotate.h>
#include <benchmarks/tools/BenchmarkHelper.h>

template <class T, std::size_t Size>
static void BM_StdRotateSmall(benchmark::State& state) {
    TestData<T, Size> test;
    constexpr std::size_t MidPos = 1;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        std::ranges::rotate(test.data, test.data.begin() + MidPos);
        benchmark::DoNotOptimize(test.data);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeRotateSmall(benchmark::State& state) {
    TestData<T, Size> test;
    constexpr std::size_t MidPos = 1;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        raze::algorithm::rotate(test.data, test.data.begin() + MidPos);
        benchmark::DoNotOptimize(test.data);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_StdRotateHalf(benchmark::State& state) {
    TestData<T, Size> test;
    constexpr std::size_t MidPos = Size / 2;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        std::ranges::rotate(test.data, test.data.begin() + MidPos);
        benchmark::DoNotOptimize(test.data);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeRotateHalf(benchmark::State& state) {
    TestData<T, Size> test;
    constexpr std::size_t MidPos = Size / 2;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        raze::algorithm::rotate(test.data, test.data.begin() + MidPos);
        benchmark::DoNotOptimize(test.data);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_StdRotateLarge(benchmark::State& state) {
    TestData<T, Size> test;
    constexpr std::size_t MidPos = Size > 1 ? Size - 1 : 0;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        std::ranges::rotate(test.data, test.data.begin() + MidPos);
        benchmark::DoNotOptimize(test.data);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeRotateLarge(benchmark::State& state) {
    TestData<T, Size> test;
    constexpr std::size_t MidPos = Size > 1 ? Size - 1 : 0;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        raze::algorithm::rotate(test.data, test.data.begin() + MidPos);
        benchmark::DoNotOptimize(test.data);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

#define RAZE_BENCHMARK_ROTATE_GROUP(name1, name2) \
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

void RegisterAll()
{
    RAZE_BENCHMARK_ROTATE_GROUP(BM_RazeRotateSmall, BM_StdRotateSmall);
    RAZE_BENCHMARK_ROTATE_GROUP(BM_RazeRotateHalf, BM_StdRotateHalf);
    RAZE_BENCHMARK_ROTATE_GROUP(BM_RazeRotateLarge, BM_StdRotateLarge);
}

RAZE_BENCHMARK_MAIN();