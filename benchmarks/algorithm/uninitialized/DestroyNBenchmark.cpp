#include <raze/algorithm/uninitialized/DestroyN.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_destroy_n(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    rb::bench_raw_storage<T>(state, size, [&](T* raw, raze::sizetype n) {
        auto res = fn(raw, static_cast<std::ptrdiff_t>(n));
        benchmark::DoNotOptimize(res);
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("DestroyN",
        [](benchmark::State& s) { run_destroy_n<T>(s, [](T* f, std::ptrdiff_t n) { return raze::algorithm::destroy_n(f, n); }); },
        [](benchmark::State& s) { run_destroy_n<T>(s, [](T* f, std::ptrdiff_t n) { return std::ranges::destroy_n(f, n); }); },
        [](benchmark::State& s) { run_destroy_n<T>(s, [](T* f, std::ptrdiff_t n) { return raze::algorithm::destroy_n[raze::options::fscalar](f, n); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

