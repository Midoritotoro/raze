#include <raze/algorithm/find/AnyOf.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_any_of(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_find_data<T>(rb::arg_size(state, 0), rb::arg_size(state, 1));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("AnyOf",
        [](benchmark::State& s) { run_any_of<T>(s, [](const auto& d) { return raze::algorithm::any_of(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_any_of<T>(s, [](const auto& d) { return std::ranges::any_of(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_any_of<T>(s, [](const auto& d) { return raze::algorithm::any_of[raze::options::fscalar](d, rb::unary_pred<T>()); }); },
        rb::end_pos_cases);
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

