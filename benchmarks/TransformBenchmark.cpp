#include <raze/algorithm/transform/Transform.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {
    template <class T>
    std::vector<T> make_transform_data(std::size_t size, T offset = T(0)) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = static_cast<T>((i % 50) + offset);
        return data;
    }
}

template <class T>
static void BM_StdTransform(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto in = make_transform_data<T>(size, T(1));
    std::vector<T> out(size);
    auto op = [](auto x) { return x * T(3) + T(7); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = std::ranges::transform(in, out.begin(), op);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_RazeTransform(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto in = make_transform_data<T>(size, T(1));
    std::vector<T> out(size);
    auto op = [](auto x) { return x * T(3) + T(7); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = raze::algorithm::transform(in, out.begin(), op);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_RazeTransformScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto in = make_transform_data<T>(size, T(1));
    std::vector<T> out(size);
    auto op = [](auto x) { return x * T(3) + T(7); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = raze::algorithm::transform[raze::options::fscalar](in, out.begin(), op);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_StdTransformBinary(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto in1 = make_transform_data<T>(size, T(1));
    const auto in2 = make_transform_data<T>(size, T(2));
    std::vector<T> out(size);
    auto op = [](auto x, auto y) { return x + y; };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in1);
        benchmark::DoNotOptimize(in2);
        auto res = std::ranges::transform(in1, in2, out.begin(), op);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 3));
}

template <class T>
static void BM_RazeTransformBinary(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto in1 = make_transform_data<T>(size, T(1));
    const auto in2 = make_transform_data<T>(size, T(2));
    std::vector<T> out(size);
    auto op = [](auto x, auto y) { return x + y; };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in1);
        benchmark::DoNotOptimize(in2);
        auto res = raze::algorithm::transform(in1, in2, out.begin(), op);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 3));
}

template <class T>
static void BM_RazeTransformBinaryScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto in1 = make_transform_data<T>(size, T(1));
    const auto in2 = make_transform_data<T>(size, T(2));
    std::vector<T> out(size);
    auto op = [](auto x, auto y) { return x + y; };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in1);
        benchmark::DoNotOptimize(in2);
        auto res = raze::algorithm::transform[raze::options::fscalar](in1, in2, out.begin(), op);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 3));
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

    reg("BM_RazeTransform",             &BM_RazeTransform<T>);
    reg("BM_StdTransform",              &BM_StdTransform<T>);
    reg("BM_RazeTransformScalar",       &BM_RazeTransformScalar<T>);

    reg("BM_RazeTransformBinary",       &BM_RazeTransformBinary<T>);
    reg("BM_StdTransformBinary",        &BM_StdTransformBinary<T>);
    reg("BM_RazeTransformBinaryScalar", &BM_RazeTransformBinaryScalar<T>);
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