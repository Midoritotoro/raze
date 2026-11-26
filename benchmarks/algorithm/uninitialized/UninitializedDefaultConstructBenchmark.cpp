#include <raze/algorithm/uninitialized/UninitializedDefaultConstruct.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_uninit_default_construct(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    rb::bench_raw_storage<T>(state, size, [&](T* raw, raze::sizetype n) {
        auto res = fn(raw, raw + n);
        benchmark::DoNotOptimize(res);
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("UninitializedDefaultConstruct",
        [](benchmark::State& s) { run_uninit_default_construct<T>(s, [](T* f, T* l) { return raze::algorithm::uninitialized_default_construct(f, l); }); },
        [](benchmark::State& s) { run_uninit_default_construct<T>(s, [](T* f, T* l) { return std::ranges::uninitialized_default_construct(f, l); }); },
        [](benchmark::State& s) { run_uninit_default_construct<T>(s, [](T* f, T* l) { return raze::algorithm::uninitialized_default_construct[raze::options::fscalar](f, l); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

