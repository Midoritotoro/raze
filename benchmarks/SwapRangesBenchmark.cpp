#include <raze/algorithm/swap/Swap.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_swap_ranges(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    auto a = rb::make_iota<T>(size, 100, T(1));
    auto b = rb::make_iota<T>(size, 100, T(2));
    rb::bench_swap<T>(state, a, b, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("SwapRanges",
        [](benchmark::State& s) { run_swap_ranges<T>(s, [](auto& a, auto& b) { return raze::algorithm::swap_ranges(a, b); }); },
        [](benchmark::State& s) { run_swap_ranges<T>(s, [](auto& a, auto& b) { return std::ranges::swap_ranges(a, b); }); },
        [](benchmark::State& s) { run_swap_ranges<T>(s, [](auto& a, auto& b) { return raze::algorithm::swap_ranges[raze::options::fscalar](a, b); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();