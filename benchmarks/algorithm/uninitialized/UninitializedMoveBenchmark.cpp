#include <raze/algorithm/uninitialized/UninitializedMove.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_uninit_move(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    auto src = rb::make_iota<T>(size);
    rb::bench_raw_storage<T>(state, size, [&](T* raw, raze::sizetype n) {
        benchmark::DoNotOptimize(src);
        auto res = fn(src, raw, raw + n);
        benchmark::DoNotOptimize(res);
    }, 2);
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("UninitializedMove",
        [](benchmark::State& s) { run_uninit_move<T>(s, [](auto& in, T* df, T* dl) { return raze::algorithm::uninitialized_move(in.begin(), in.end(), df, dl); }); },
        [](benchmark::State& s) { run_uninit_move<T>(s, [](auto& in, T* df, T* dl) { return std::ranges::uninitialized_move(in.begin(), in.end(), df, dl); }); },
        [](benchmark::State& s) { run_uninit_move<T>(s, [](auto& in, T* df, T* dl) { return raze::algorithm::uninitialized_move[raze::options::fscalar](in.begin(), in.end(), df, dl); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

