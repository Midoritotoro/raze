#include <raze/algorithm/find/AdjacentFind.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace {
    template <class T>
    std::vector<T> make_adjacent_data(std::size_t size) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = static_cast<T>(i % 100);  

        if (size >= 2) {
            constexpr T dup = std::numeric_limits<T>::max();
            data[size - 2] = dup;
            data[size - 1] = dup;
        }
        return data;
    }
}

template <class T>
static void BM_StdAdjacentFind(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_adjacent_data<T>(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = std::ranges::adjacent_find(data);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeAdjacentFind(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_adjacent_data<T>(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::adjacent_find(data);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeAdjacentFindScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_adjacent_data<T>(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::adjacent_find[raze::options::fscalar](data);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_StdAdjacentFindPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_adjacent_data<T>(size);
    auto pred = [](auto a, auto b) { return (a + T(1)) == (b + T(1)); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = std::ranges::adjacent_find(data, pred);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeAdjacentFindPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_adjacent_data<T>(size);
    auto pred = [](auto a, auto b) { return (a + T(1)) == (b + T(1)); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::adjacent_find(data, pred);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeAdjacentFindPredScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_adjacent_data<T>(size);
    auto pred = [](auto a, auto b) { return (a + T(1)) == (b + T(1)); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::adjacent_find[raze::options::fscalar](data, pred);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void register_type_benchmarks(const char* type_name) {
    static constexpr std::array<int64_t, 6> sizes{ 16, 64, 256, 1024, 4096, 16384 };

    auto reg = [&](const char* fn_name, void(*fn)(benchmark::State&)) {
        const std::string full = std::string(fn_name) + "<" + type_name + ">";
        auto* bm = ::benchmark::RegisterBenchmark(full.c_str(), fn);
        for (int64_t s : sizes)
            bm->Args({ s });
        bm->MinTime(0.5)->Repetitions(10)->ReportAggregatesOnly(true);
    };

    reg("BM_RazeAdjacentFind",           &BM_RazeAdjacentFind<T>);
    reg("BM_StdAdjacentFind",            &BM_StdAdjacentFind<T>);
    reg("BM_RazeAdjacentFindScalar",     &BM_RazeAdjacentFindScalar<T>);

    reg("BM_RazeAdjacentFindPred",       &BM_RazeAdjacentFindPred<T>);
    reg("BM_StdAdjacentFindPred",        &BM_StdAdjacentFindPred<T>);
    reg("BM_RazeAdjacentFindPredScalar", &BM_RazeAdjacentFindPredScalar<T>);
}

void RegisterAll() {
    register_type_benchmarks<raze::i8>("raze::i8");
    register_type_benchmarks<raze::i16>("raze::i16");
    register_type_benchmarks<raze::i32>("raze::i32");
    register_type_benchmarks<raze::i64>("raze::i64");
    register_type_benchmarks<raze::f32>("raze::f32");
    register_type_benchmarks<raze::f64>("raze::f64");
}

RAZE_BENCHMARK_MAIN();