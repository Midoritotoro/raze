#include <raze/algorithm/remove/Remove.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

namespace {
    template <class T>
    std::vector<T> make_remove_data(std::size_t size, T needle) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = (i & 1) ? needle : static_cast<T>((i % 100) + 1); // 50% совпадений
        return data;
    }
}

template <class T>
static void BM_StdRemove(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto src = make_remove_data<T>(size, needle);
    auto work = src;

    for (auto _ : state) {
        state.PauseTiming();
        std::memcpy(work.data(), src.data(), size * sizeof(T));
        benchmark::DoNotOptimize(work);
        state.ResumeTiming();

        auto res = std::ranges::remove(work, needle);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeRemove(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto src = make_remove_data<T>(size, needle);
    auto work = src;

    for (auto _ : state) {
        state.PauseTiming();
        std::memcpy(work.data(), src.data(), size * sizeof(T));
        benchmark::DoNotOptimize(work);
        state.ResumeTiming();

        auto res = raze::algorithm::remove(work, needle);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeRemoveScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto src = make_remove_data<T>(size, needle);
    auto work = src;

    for (auto _ : state) {
        state.PauseTiming();
        std::memcpy(work.data(), src.data(), size * sizeof(T));
        benchmark::DoNotOptimize(work);
        state.ResumeTiming();

        auto res = raze::algorithm::remove[raze::options::fscalar](work, needle);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_StdRemovePred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto src = make_remove_data<T>(size, needle);
    auto work = src;
    auto pred = [](auto x) { return x > T(100); };

    for (auto _ : state) {
        state.PauseTiming();
        std::memcpy(work.data(), src.data(), size * sizeof(T));
        benchmark::DoNotOptimize(work);
        state.ResumeTiming();

        auto res = std::ranges::remove_if(work, pred);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeRemovePred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto src = make_remove_data<T>(size, needle);
    auto work = src;
    auto pred = [](auto x) { return x > T(100); };

    for (auto _ : state) {
        state.PauseTiming();
        std::memcpy(work.data(), src.data(), size * sizeof(T));
        benchmark::DoNotOptimize(work);
        state.ResumeTiming();

        auto res = raze::algorithm::remove_if(work, pred);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeRemovePredScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto src = make_remove_data<T>(size, needle);
    auto work = src;
    auto pred = [](auto x) { return x > T(100); };

    for (auto _ : state) {
        state.PauseTiming();
        std::memcpy(work.data(), src.data(), size * sizeof(T));
        benchmark::DoNotOptimize(work);
        state.ResumeTiming();

        auto res = raze::algorithm::remove_if[raze::options::fscalar](work, pred);
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

    reg("BM_RazeRemove",           &BM_RazeRemove<T>);
    reg("BM_StdRemove",            &BM_StdRemove<T>);
    reg("BM_RazeRemoveScalar",     &BM_RazeRemoveScalar<T>);

    reg("BM_RazeRemovePred",       &BM_RazeRemovePred<T>);
    reg("BM_StdRemovePred",        &BM_StdRemovePred<T>);
    reg("BM_RazeRemovePredScalar", &BM_RazeRemovePredScalar<T>);
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