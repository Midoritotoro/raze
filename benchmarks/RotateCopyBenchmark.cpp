#include <raze/algorithm/order/RotateCopy.h>
#include <benchmarks/tools/BenchmarkHelper.h>

template <class T, std::size_t Size>
static void BM_StdRotateCopySmall(benchmark::State& state) {
    TestData<T, Size> test;
    std::array<T, Size> dst{};
    constexpr std::size_t MidPos = 1;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        benchmark::DoNotOptimize(dst);

        std::ranges::rotate_copy(test.data, test.data.begin() + MidPos, dst.begin());

        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_RazeRotateCopySmall(benchmark::State& state) {
    TestData<T, Size> test;
    std::array<T, Size> dst{};
    constexpr std::size_t MidPos = 1;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        benchmark::DoNotOptimize(dst);

        raze::algorithm::rotate_copy(test.data, test.data.begin() + MidPos, dst.begin());

        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_StdRotateCopyHalf(benchmark::State& state) {
    TestData<T, Size> test;
    std::array<T, Size> dst{};
    constexpr std::size_t MidPos = Size / 2;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        benchmark::DoNotOptimize(dst);

        std::ranges::rotate_copy(test.data, test.data.begin() + MidPos, dst.begin());

        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_RazeRotateCopyHalf(benchmark::State& state) {
    TestData<T, Size> test;
    std::array<T, Size> dst{};
    constexpr std::size_t MidPos = Size / 2;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        benchmark::DoNotOptimize(dst);

        raze::algorithm::rotate_copy(test.data, test.data.begin() + MidPos, dst.begin());

        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_StdRotateCopyLarge(benchmark::State& state) {
    TestData<T, Size> test;
    std::array<T, Size> dst{};
    constexpr std::size_t MidPos = Size > 1 ? Size - 1 : 0;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        benchmark::DoNotOptimize(dst);

        std::ranges::rotate_copy(test.data, test.data.begin() + MidPos, dst.begin());

        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_RazeRotateCopyLarge(benchmark::State& state) {
    TestData<T, Size> test;
    std::array<T, Size> dst{};
    constexpr std::size_t MidPos = Size > 1 ? Size - 1 : 0;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        benchmark::DoNotOptimize(dst);

        raze::algorithm::rotate_copy(test.data, test.data.begin() + MidPos, dst.begin());

        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

#define RAZE_BENCHMARK_ROTATE_COPY_GROUP(name1, name2) \
    BENCHMARK(name1<raze::i8, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);

void RegisterAll()
{
    RAZE_BENCHMARK_ROTATE_COPY_GROUP(BM_RazeRotateCopySmall, BM_StdRotateCopySmall);
    RAZE_BENCHMARK_ROTATE_COPY_GROUP(BM_RazeRotateCopyHalf, BM_StdRotateCopyHalf);
    RAZE_BENCHMARK_ROTATE_COPY_GROUP(BM_RazeRotateCopyLarge, BM_StdRotateCopyLarge);
}

RAZE_BENCHMARK_MAIN();