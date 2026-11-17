#include <raze/algorithm/find/Find.h>
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
    std::vector<T> make_find_data(std::size_t size, std::size_t pos, T needle) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = static_cast<T>(i % 100); // < 127, чтобы needle не встречался раньше pos (в т.ч. для i8)

        if (pos < size)
            data[pos] = needle;

        return data;
    }
}

template <class T>
static void BM_StdFind(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos = static_cast<std::size_t>(state.range(1));

    constexpr T needle = std::numeric_limits<T>::max();
    auto data = make_find_data<T>(size, pos, needle);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto it = std::ranges::find(data, needle);
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeFind(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos = static_cast<std::size_t>(state.range(1));

    constexpr T needle = std::numeric_limits<T>::max();
    auto data = make_find_data<T>(size, pos, needle);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto it = raze::algorithm::find(data, needle);
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeFindScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos = static_cast<std::size_t>(state.range(1));

    constexpr T needle = std::numeric_limits<T>::max();
    auto data = make_find_data<T>(size, pos, needle);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto it = raze::algorithm::find[raze::options::fscalar](data, needle);
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_StdFindPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos = static_cast<std::size_t>(state.range(1));

    constexpr T needle = std::numeric_limits<T>::max();
    auto data = make_find_data<T>(size, pos, needle);
    auto pred = [](auto x) { return ((x * 3 + 7) * 2) == 2; };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto it = std::ranges::find_if(data, pred);
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeFindPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos = static_cast<std::size_t>(state.range(1));

    constexpr T needle = std::numeric_limits<T>::max();
    auto data = make_find_data<T>(size, pos, needle);
    auto pred = [](auto x) { return ((x * 3 + 7) * 2) == 2; };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto it = raze::algorithm::find_if(data, pred);
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeFindPredScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos = static_cast<std::size_t>(state.range(1));

    constexpr T needle = std::numeric_limits<T>::max();
    auto data = make_find_data<T>(size, pos, needle);
    auto pred = [](auto x) { return ((x * 3 + 7) * 2) == 2; };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto it = raze::algorithm::find_if[raze::options::fscalar](data, pred);
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void register_type_benchmarks(const char* type_name) {
    static constexpr std::array<std::array<int64_t, 2>, 6> cases{ {
        { 16,    15 },
        { 64,    63 },
        { 256,   255 },
        { 1024,  1023 },
        { 4096,  4095 },
        { 16384, 16383 },
    } };

    auto reg = [&](const char* fn_name, void(*fn)(benchmark::State&)) {
        const std::string full = std::string(fn_name) + "<" + type_name + ">";
        auto* bm = ::benchmark::RegisterBenchmark(full.c_str(), fn);
        for (const auto& c : cases)
            bm->Args({ c[0], c[1] });
        bm->MinTime(1)->Repetitions(5)->ReportAggregatesOnly(true);
        };

    reg("BM_RazeFind", &BM_RazeFind<T>);
    reg("BM_StdFind", &BM_StdFind<T>);
    reg("BM_RazeFindScalar", &BM_RazeFindScalar<T>);

    reg("BM_RazeFindPred", &BM_RazeFindPred<T>);
    reg("BM_StdFindPred", &BM_StdFindPred<T>);
    reg("BM_RazeFindPredScalar", &BM_RazeFindPredScalar<T>);
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