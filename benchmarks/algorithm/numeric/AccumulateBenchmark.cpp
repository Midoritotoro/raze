#include <raze/algorithm/numeric/Accumulate.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_accumulate(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_bounded_sum_data<T>(rb::arg_size(state));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Accumulate",
        [](benchmark::State& s) { run_accumulate<T>(s, [](const auto& d) { return raze::algorithm::accumulate(d, T(0)); }); },
        [](benchmark::State& s) { run_accumulate<T>(s, [](const auto& d) { return std::accumulate(d.begin(), d.end(), T(0)); }); },
        [](benchmark::State& s) { run_accumulate<T>(s, [](const auto& d) { return raze::algorithm::accumulate[raze::options::fscalar](d, T(0)); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

