#include <raze/algorithm/fold/FoldRightLast.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_fold_right_last(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_bounded_sum_data<T>(rb::arg_size(state));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("FoldRightLast",
        [](benchmark::State& s) { run_fold_right_last<T>(s, [](const auto& d) { return raze::algorithm::fold_right_last(d, std::plus<>{}); }); },
        [](benchmark::State& s) { run_fold_right_last<T>(s, [](const auto& d) { return std::ranges::fold_right_last(d, std::plus<>{}); }); },
        [](benchmark::State& s) { run_fold_right_last<T>(s, [](const auto& d) { return raze::algorithm::fold_right_last[raze::options::fscalar](d, std::plus<>{}); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

