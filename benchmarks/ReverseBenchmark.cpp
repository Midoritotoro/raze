#include <raze/algorithm/order/Reverse.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_reverse(benchmark::State& state, Fn&& fn) {
    auto data = rb::make_iota<T>(rb::arg_size(state));
    rb::bench_inplace<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Reverse",
        [](benchmark::State& s) { run_reverse<T>(s, [](auto& d) { return raze::algorithm::reverse(d); }); },
        [](benchmark::State& s) { run_reverse<T>(s, [](auto& d) { return std::ranges::reverse(d); }); },
        [](benchmark::State& s) { run_reverse<T>(s, [](auto& d) { return raze::algorithm::reverse[raze::options::fscalar](d); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();