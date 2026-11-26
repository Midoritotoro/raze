#include <raze/algorithm/uninitialized/DestroyAt.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_destroy_at(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    rb::bench_raw_storage<T>(state, size, [&](T* raw, raze::sizetype n) {
        for (raze::sizetype i = 0; i < n; ++i)
            fn(raw + i);
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("DestroyAt",
        [](benchmark::State& s) { run_destroy_at<T>(s, [](T* p) { raze::algorithm::destroy_at(p); }); },
        [](benchmark::State& s) { run_destroy_at<T>(s, [](T* p) { std::ranges::destroy_at(p); }); },
        [](benchmark::State& s) { run_destroy_at<T>(s, [](T* p) { raze::algorithm::destroy_at[raze::options::fscalar](p); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

