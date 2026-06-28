#include <raze/algorithm/replace/ReplaceCopy.h>
#include <benchmarks/tools/BenchmarkHelper.h>

template <class T, std::size_t Size>
static void BM_StdReplaceCopy(benchmark::State& state) {
    TestData<T, Size> source;
    std::array<T, Size> destination{};

    constexpr T old_val = T(0);
    constexpr T new_val = T(42);

    for (auto _ : state) {
        benchmark::DoNotOptimize(source.data);
        benchmark::DoNotOptimize(destination);

        std::ranges::replace_copy(source.data, destination.begin(), old_val, new_val);
        benchmark::DoNotOptimize(destination);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_RazeReplaceCopy(benchmark::State& state) {
    TestData<T, Size> source;
    std::array<T, Size> destination{};

    constexpr T old_val = T(0);
    constexpr T new_val = T(42);

    for (auto _ : state) {
        benchmark::DoNotOptimize(source.data);
        benchmark::DoNotOptimize(destination);

        raze::algorithm::replace_copy(source.data, destination.begin(), old_val, new_val);
        benchmark::DoNotOptimize(destination);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_StdReplaceCopyIf(benchmark::State& state) {
    TestData<T, Size> source;
    std::array<T, Size> destination{};

    for (auto _ : state) {
        benchmark::DoNotOptimize(source.data);
        benchmark::DoNotOptimize(destination);

        std::ranges::replace_copy_if(source.data, destination.begin(), [](auto x) { return x > T(0); }, T(99));
        benchmark::DoNotOptimize(destination);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_RazeReplaceCopyIf(benchmark::State& state) {
    TestData<T, Size> source;
    std::array<T, Size> destination{};

    for (auto _ : state) {
        benchmark::DoNotOptimize(source.data);
        benchmark::DoNotOptimize(destination);

        raze::algorithm::replace_copy_if(source.data, destination.begin(), [](auto x) { return x > T(0); }, T(99));
        benchmark::DoNotOptimize(destination);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_StdReplaceCopyIfComplex(benchmark::State& state) {
    TestData<T, Size> source;
    std::array<T, Size> destination{};

    for (auto _ : state) {
        benchmark::DoNotOptimize(source.data);
        benchmark::DoNotOptimize(destination);

        std::ranges::replace_copy_if(source.data, destination.begin(), [](auto x) { return ((x * 3 + 7) - 5) == 0; }, T(0));
        benchmark::DoNotOptimize(destination);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_RazeReplaceCopyIfComplex(benchmark::State& state) {
    TestData<T, Size> source;
    std::array<T, Size> destination{};

    for (auto _ : state) {
        benchmark::DoNotOptimize(source.data);
        benchmark::DoNotOptimize(destination);

        raze::algorithm::replace_copy_if(source.data, destination.begin(), [](auto x) { return ((x * 3 + 7) - 5) == 0; }, T(0));
        benchmark::DoNotOptimize(destination);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

#define RAZE_BENCHMARK_REPLACE_COPY(name1, name2) \
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
    RAZE_BENCHMARK_REPLACE_COPY(BM_RazeReplaceCopy, BM_StdReplaceCopy);
    RAZE_BENCHMARK_REPLACE_COPY(BM_RazeReplaceCopyIf, BM_StdReplaceCopyIf);
    RAZE_BENCHMARK_REPLACE_COPY(BM_RazeReplaceCopyIfComplex, BM_StdReplaceCopyIfComplex);
}

RAZE_BENCHMARK_MAIN();