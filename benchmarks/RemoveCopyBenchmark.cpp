#include <raze/algorithm/remove/RemoveCopy.h>
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
    std::vector<T> make_remove_copy_data(std::size_t size, T needle) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = (i & 1) ? needle : static_cast<T>((i % 100) + 1); // 50% совпадений
        return data;
    }
}

template <class T>
static void BM_StdRemoveCopy(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto in = make_remove_copy_data<T>(size, needle);
    std::vector<T> out(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = std::ranges::remove_copy(in, out.begin(), needle);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeRemoveCopy(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto in = make_remove_copy_data<T>(size, needle);
    std::vector<T> out(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = raze::algorithm::remove_copy(in, out.begin(), needle);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeRemoveCopyScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto in = make_remove_copy_data<T>(size, needle);
    std::vector<T> out(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = raze::algorithm::remove_copy[raze::options::fscalar](in, out.begin(), needle);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_StdRemoveCopyPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto in = make_remove_copy_data<T>(size, needle);
    std::vector<T> out(size);
    auto pred = [](auto x) { return x > T(100); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = std::ranges::remove_copy_if(in, out.begin(), pred);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeRemoveCopyPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto in = make_remove_copy_data<T>(size, needle);
    std::vector<T> out(size);
    auto pred = [](auto x) { return x > T(100); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = raze::algorithm::remove_copy_if(in, out.begin(), pred);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeRemoveCopyPredScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto in = make_remove_copy_data<T>(size, needle);
    std::vector<T> out(size);
    auto pred = [](auto x) { return x > T(100); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = raze::algorithm::remove_copy_if[raze::options::fscalar](in, out.begin(), pred);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
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

    reg("BM_RazeRemoveCopy",           &BM_RazeRemoveCopy<T>);
    reg("BM_StdRemoveCopy",            &BM_StdRemoveCopy<T>);
    reg("BM_RazeRemoveCopyScalar",     &BM_RazeRemoveCopyScalar<T>);

    reg("BM_RazeRemoveCopyPred",       &BM_RazeRemoveCopyPred<T>);
    reg("BM_StdRemoveCopyPred",        &BM_StdRemoveCopyPred<T>);
    reg("BM_RazeRemoveCopyPredScalar", &BM_RazeRemoveCopyPredScalar<T>);
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