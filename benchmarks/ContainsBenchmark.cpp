#include <raze/algorithm/find/Contains.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_contains(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_find_data<T>(rb::arg_size(state, 0), rb::arg_size(state, 1));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Contains",
        [](benchmark::State& s) { run_contains<T>(s, [](const auto& d) { return raze::algorithm::contains(d, rb::needle_value<T>()); }); },
        [](benchmark::State& s) { run_contains<T>(s, [](const auto& d) { return std::ranges::contains(d, rb::needle_value<T>()); }); },
        [](benchmark::State& s) { run_contains<T>(s, [](const auto& d) { return raze::algorithm::contains[raze::options::fscalar](d, rb::needle_value<T>()); }); },
        rb::end_pos_cases);
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

