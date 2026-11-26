#include <raze/algorithm/fold/FoldLeftWithIter.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_fold_left_with_iter(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_bounded_sum_data<T>(rb::arg_size(state));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("FoldLeftWithIter",
        [](benchmark::State& s) { run_fold_left_with_iter<T>(s, [](const auto& d) { return raze::algorithm::fold_left_with_iter(d, T(0), std::plus<>{}); }); },
        [](benchmark::State& s) { run_fold_left_with_iter<T>(s, [](const auto& d) { return std::ranges::fold_left_with_iter(d, T(0), std::plus<>{}); }); },
        [](benchmark::State& s) { run_fold_left_with_iter<T>(s, [](const auto& d) { return raze::algorithm::fold_left_with_iter[raze::options::fscalar](d, T(0), std::plus<>{}); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

