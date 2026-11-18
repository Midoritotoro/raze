#include <raze/algorithm/reverse/Reverse.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {
    template <class T>
    std::vector<T> make_reverse_data(std::size_t size) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = static_cast<T>(i % 100);
        return data;
    }
}

template <class T>
static void BM_StdReverse(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    auto data = make_reverse_data<T>(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        std::ranges::reverse(data);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeReverse(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    auto data = make_reverse_data<T>(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        raze::algorithm::reverse(data);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeReverseScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    auto data = make_reverse_data<T>(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        raze::algorithm::reverse[raze::options::fscalar](data);
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

    reg("BM_RazeReverse",       &BM_RazeReverse<T>);
    reg("BM_StdReverse",        &BM_StdReverse<T>);
    reg("BM_RazeReverseScalar", &BM_RazeReverseScalar<T>);
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