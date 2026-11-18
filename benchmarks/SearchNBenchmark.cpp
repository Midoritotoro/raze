#include <raze/algorithm/find/SearchN.h>
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
    std::vector<T> make_search_n_data(std::size_t size, std::size_t pos, std::size_t count, T needle) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = static_cast<T>(i % 100);  

        for (std::size_t i = 0; i < count && pos + i < size; ++i)
            data[pos + i] = needle;

        return data;
    }
}

template <class T>
static void BM_StdSearchN(benchmark::State& state) {
    const auto size  = static_cast<std::size_t>(state.range(0));
    const auto pos   = static_cast<std::size_t>(state.range(1));
    const auto count = static_cast<std::ptrdiff_t>(state.range(2));

    constexpr T needle = std::numeric_limits<T>::max();
    auto data = make_search_n_data<T>(size, pos, static_cast<std::size_t>(count), needle);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto result = std::ranges::search_n(data, count, needle);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeSearchN(benchmark::State& state) {
    const auto size  = static_cast<std::size_t>(state.range(0));
    const auto pos   = static_cast<std::size_t>(state.range(1));
    const auto count = static_cast<std::ptrdiff_t>(state.range(2));

    constexpr T needle = std::numeric_limits<T>::max();
    auto data = make_search_n_data<T>(size, pos, static_cast<std::size_t>(count), needle);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto result = raze::algorithm::search_n(data, count, needle);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeSearchNScalar(benchmark::State& state) {
    const auto size  = static_cast<std::size_t>(state.range(0));
    const auto pos   = static_cast<std::size_t>(state.range(1));
    const auto count = static_cast<std::ptrdiff_t>(state.range(2));

    constexpr T needle = std::numeric_limits<T>::max();
    auto data = make_search_n_data<T>(size, pos, static_cast<std::size_t>(count), needle);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto result = raze::algorithm::search_n[raze::options::fscalar](data, count, needle);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void register_type_benchmarks(const char* type_name) {
    static constexpr std::array<std::array<int64_t, 3>, 11> main_cases{{
        { 16,    12,    4  },
        { 64,    60,    4  },
        { 256,   252,   4  },
        { 1024,  1020,  4  },
        { 4096,  4092,  4  },
        { 16384, 16380, 4  },
        { 64,    32,    32 },
        { 256,   224,   32 },
        { 1024,  992,   32 },
        { 4096,  4064,  32 },
        { 16384, 16352, 32 },
    }};

    auto reg = [&](const char* fn_name, void(*fn)(benchmark::State&), auto const& cases) {
        const std::string full = std::string(fn_name) + "<" + type_name + ">";
        auto* bm = ::benchmark::RegisterBenchmark(full.c_str(), fn);
        for (const auto& c : cases)
            bm->Args({ c[0], c[1], c[2] });
        bm->MinTime(0.5)->Repetitions(10)->ReportAggregatesOnly(true);
    };

    reg("BM_RazeSearchN",       &BM_RazeSearchN<T>,       main_cases);
    reg("BM_StdSearchN",        &BM_StdSearchN<T>,        main_cases);
    reg("BM_RazeSearchNScalar", &BM_RazeSearchNScalar<T>, main_cases);
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