#include <raze/algorithm/partition/IsPartitioned.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_is_partitioned(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_partitioned_data<T>(rb::arg_size(state));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("IsPartitioned",
        [](benchmark::State& s) { run_is_partitioned<T>(s, [](const auto& d) { return raze::algorithm::is_partitioned(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_is_partitioned<T>(s, [](const auto& d) { return std::ranges::is_partitioned(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_is_partitioned<T>(s, [](const auto& d) { return raze::algorithm::is_partitioned[raze::options::fscalar](d, rb::unary_pred<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

