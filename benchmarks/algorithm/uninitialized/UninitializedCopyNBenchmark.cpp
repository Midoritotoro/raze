#include <raze/algorithm/uninitialized/UninitializedCopyN.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_uninit_copy_n(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto src = rb::make_iota<T>(size);
    rb::bench_raw_storage<T>(state, size, [&](T* raw, raze::sizetype n) {
        benchmark::DoNotOptimize(src);
        auto res = fn(src.begin(), static_cast<std::ptrdiff_t>(n), raw, raw + n);
        benchmark::DoNotOptimize(res);
    }, 2);
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("UninitializedCopyN",
        [](benchmark::State& s) { run_uninit_copy_n<T>(s, [](auto in, std::ptrdiff_t n, T* df, T* dl) { return raze::algorithm::uninitialized_copy_n(in, n, df, dl); }); },
        [](benchmark::State& s) { run_uninit_copy_n<T>(s, [](auto in, std::ptrdiff_t n, T* df, T* dl) { return std::ranges::uninitialized_copy_n(in, n, df, dl); }); },
        [](benchmark::State& s) { run_uninit_copy_n<T>(s, [](auto in, std::ptrdiff_t n, T* df, T* dl) { return raze::algorithm::uninitialized_copy_n[raze::options::fscalar](in, n, df, dl); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

