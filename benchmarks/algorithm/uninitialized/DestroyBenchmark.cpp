#include <raze/algorithm/uninitialized/Destroy.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_destroy(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    rb::bench_raw_storage<T>(state, size, [&](T* raw, raze::sizetype n) {
        auto res = fn(raw, raw + n);
        benchmark::DoNotOptimize(res);
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Destroy",
        [](benchmark::State& s) { run_destroy<T>(s, [](T* f, T* l) { return raze::algorithm::destroy(f, l); }); },
        [](benchmark::State& s) { run_destroy<T>(s, [](T* f, T* l) { return std::ranges::destroy(f, l); }); },
        [](benchmark::State& s) { run_destroy<T>(s, [](T* f, T* l) { return raze::algorithm::destroy[raze::options::fscalar](f, l); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

