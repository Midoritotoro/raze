#include <raze/algorithm/find/Count.h>
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
    std::vector<T> make_count_data(std::size_t size, T needle) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = (i & 1) ? needle : static_cast<T>(i % 100);
        return data;
    }
}

template <class T>
static void BM_StdCount(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto data = make_count_data<T>(size, needle);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = std::ranges::count(data, needle);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeCount(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto data = make_count_data<T>(size, needle);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::count(data, needle);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeCountScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto data = make_count_data<T>(size, needle);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::count[raze::options::fscalar](data, needle);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_StdCountPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto data = make_count_data<T>(size, needle);
    auto pred = [](auto x) { return x > T(50); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = std::ranges::count_if(data, pred);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeCountPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto data = make_count_data<T>(size, needle);
    auto pred = [](auto x) { return x > T(50); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::count_if(data, pred);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeCountPredScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto data = make_count_data<T>(size, needle);
    auto pred = [](auto x) { return x > T(50); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::count_if[raze::options::fscalar](data, pred);
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

    reg("BM_RazeCount",           &BM_RazeCount<T>);
    reg("BM_StdCount",            &BM_StdCount<T>);
    reg("BM_RazeCountScalar",     &BM_RazeCountScalar<T>);

    reg("BM_RazeCountPred",       &BM_RazeCountPred<T>);
    reg("BM_StdCountPred",        &BM_StdCountPred<T>);
    reg("BM_RazeCountPredScalar", &BM_RazeCountPredScalar<T>);
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