#include <raze/algorithm/find/AllOf.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_all_of(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_iota<T>(rb::arg_size(state));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("AllOf",
        [](benchmark::State& s) { run_all_of<T>(s, [](const auto& d) { return raze::algorithm::all_of(d, rb::unary_pred_not<T>()); }); },
        [](benchmark::State& s) { run_all_of<T>(s, [](const auto& d) { return std::ranges::all_of(d, rb::unary_pred_not<T>()); }); },
        [](benchmark::State& s) { run_all_of<T>(s, [](const auto& d) { return raze::algorithm::all_of[raze::options::fscalar](d, rb::unary_pred_not<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

