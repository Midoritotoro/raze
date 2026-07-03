#include <raze/algorithm/find/SearchN.h>
#include <benchmarks/tools/BenchmarkHelper.h>

template <class T, std::size_t Size, std::size_t Position, std::size_t Count>
static void BM_StdSearchN(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle = std::numeric_limits<T>::max();
    
    for (std::size_t i = 0; i < Count && Position + i < Size; ++i) {
        test.data[Position + i] = needle;
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto result = std::ranges::search_n(test.data, Count, needle);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size, std::size_t Position, std::size_t Count>
static void BM_RazeSearchN(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle = std::numeric_limits<T>::max();
    
    for (std::size_t i = 0; i < Count && Position + i < Size; ++i) {
        test.data[Position + i] = needle;
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto result = raze::algorithm::search_n(test.data, Count, needle);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size, std::size_t Position, std::size_t Count>
static void BM_StdSearchNPred(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle = std::numeric_limits<T>::max();
    
    for (std::size_t i = 0; i < Count && Position + i < Size; ++i) {
        test.data[Position + i] = needle;
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto result = std::ranges::search_n(test.data, Count, needle, 
            [](T elem, T val) { return elem == val; });
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size, std::size_t Position, std::size_t Count>
static void BM_RazeSearchNPred(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle = std::numeric_limits<T>::max();
    
    for (std::size_t i = 0; i < Count && Position + i < Size; ++i) {
        test.data[Position + i] = needle;
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto result = raze::algorithm::search_n(test.data, Count, needle,
            [](T elem, T val) { return elem == val; });
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

#define RAZE_BENCHMARK_SEARCH_N(name1, name2, Size, Pos, Count) \
    BENCHMARK(name1<raze::i8, Size, Pos, Count>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, Size, Pos, Count>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, Size, Pos, Count>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, Size, Pos, Count>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, Size, Pos, Count>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, Size, Pos, Count>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, Size, Pos, Count>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, Size, Pos, Count>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, Size, Pos, Count>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, Size, Pos, Count>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, Size, Pos, Count>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, Size, Pos, Count>)->Repetitions(10)->ReportAggregatesOnly(true)

#define RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(name1, name2, Pos, Count) \
    RAZE_BENCHMARK_SEARCH_N(name1, name2, 16, Pos, Count);\
    RAZE_BENCHMARK_SEARCH_N(name1, name2, 1024, Pos, Count);\
    RAZE_BENCHMARK_SEARCH_N(name1, name2, 4096, Pos, Count)

#define RAZE_BENCHMARK_SEARCH_N_LARGE_SIZES(name1, name2, Pos, Count) \
    RAZE_BENCHMARK_SEARCH_N(name1, name2, 1024, Pos, Count);\
    RAZE_BENCHMARK_SEARCH_N(name1, name2, 4096, Pos, Count)

void RegisterAll()
{
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 15, 1);
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 1023, 1);
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 4095, 1);
    
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 14, 2);
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 1022, 2);
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 4094, 2);
    
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 12, 4);
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 1020, 4);
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 4092, 4);
    
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 8, 8);
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 1016, 8);
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 4088, 8);
    
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 0, 16);
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 1008, 16);
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchN, BM_StdSearchN, 4080, 16);
    
    RAZE_BENCHMARK_SEARCH_N_LARGE_SIZES(BM_RazeSearchN, BM_StdSearchN, 0, 32);
    RAZE_BENCHMARK_SEARCH_N_LARGE_SIZES(BM_RazeSearchN, BM_StdSearchN, 992, 32);
    RAZE_BENCHMARK_SEARCH_N_LARGE_SIZES(BM_RazeSearchN, BM_StdSearchN, 4064, 32);
    
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchNPred, BM_StdSearchNPred, 15, 1);
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchNPred, BM_StdSearchNPred, 1020, 4);
    RAZE_BENCHMARK_SEARCH_N_ALL_SIZES(BM_RazeSearchNPred, BM_StdSearchNPred, 4088, 8);
}

RAZE_BENCHMARK_MAIN();