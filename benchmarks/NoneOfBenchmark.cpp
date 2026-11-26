#include <raze/algorithm/find/NoneOf.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_none_of(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_iota<T>(rb::arg_size(state));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("NoneOf",
        [](benchmark::State& s) { run_none_of<T>(s, [](const auto& d) { return raze::algorithm::none_of(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_none_of<T>(s, [](const auto& d) { return std::ranges::none_of(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_none_of<T>(s, [](const auto& d) { return raze::algorithm::none_of[raze::options::fscalar](d, rb::unary_pred<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

