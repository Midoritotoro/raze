#include <raze/algorithm/copy/Copy.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_copy(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_iota<T>(rb::arg_size(state));
    rb::bench_copy<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Copy",
        [](benchmark::State& s) { run_copy<T>(s, [](const auto& in, auto& out) { return raze::algorithm::copy(in, out.begin()); }); },
        [](benchmark::State& s) { run_copy<T>(s, [](const auto& in, auto& out) { return std::ranges::copy(in, out.begin()); }); },
        [](benchmark::State& s) { run_copy<T>(s, [](const auto& in, auto& out) { return raze::algorithm::copy[raze::options::fscalar](in, out.begin()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();