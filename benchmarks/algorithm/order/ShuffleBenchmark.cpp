#include <raze/algorithm/order/Shuffle.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_shuffle(benchmark::State& state, Fn&& fn) {
    auto data = rb::make_iota<T>(rb::arg_size(state));
    std::mt19937 rng(42);
    rb::bench_inplace<T>(state, data, [&](auto& d) { return fn(d, rng); });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Shuffle",
        [](benchmark::State& s) { run_shuffle<T>(s, [](auto& d, auto& g) { return raze::algorithm::shuffle(d, g); }); },
        [](benchmark::State& s) { run_shuffle<T>(s, [](auto& d, auto& g) { return std::ranges::shuffle(d, g); }); },
        [](benchmark::State& s) { run_shuffle<T>(s, [](auto& d, auto& g) { return raze::algorithm::shuffle[raze::options::fscalar](d, g); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

