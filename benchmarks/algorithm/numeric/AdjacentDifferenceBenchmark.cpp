#include <raze/algorithm/numeric/AdjacentDifference.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_adjacent_difference(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_iota<T>(rb::arg_size(state), 50);
    rb::bench_copy<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("AdjacentDifference",
        [](benchmark::State& s) { run_adjacent_difference<T>(s, [](const auto& in, auto& out) { return raze::algorithm::adjacent_difference(in, out.begin()); }); },
        [](benchmark::State& s) { run_adjacent_difference<T>(s, [](const auto& in, auto& out) { return std::adjacent_difference(in.begin(), in.end(), out.begin()); }); },
        [](benchmark::State& s) { run_adjacent_difference<T>(s, [](const auto& in, auto& out) { return raze::algorithm::adjacent_difference[raze::options::fscalar](in, out.begin()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

