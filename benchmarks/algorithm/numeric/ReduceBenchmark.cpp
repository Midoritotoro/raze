#include <raze/algorithm/numeric/Reduce.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_reduce(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_bounded_sum_data<T>(rb::arg_size(state));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Reduce",
        [](benchmark::State& s) { run_reduce<T>(s, [](const auto& d) { return raze::algorithm::reduce(d, T(0)); }); },
        [](benchmark::State& s) { run_reduce<T>(s, [](const auto& d) { return std::reduce(d.begin(), d.end(), T(0)); }); },
        [](benchmark::State& s) { run_reduce<T>(s, [](const auto& d) { return raze::algorithm::reduce[raze::options::fscalar](d, T(0)); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

