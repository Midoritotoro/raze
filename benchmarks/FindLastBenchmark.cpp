#include <raze/algorithm/find/FindLast.h>
#include <raze/algorithm/find/FindLastIf.h>
#include <raze/algorithm/find/FindLastIfNot.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_find_last(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_find_data<T>(rb::arg_size(state, 0), rb::arg_size(state, 1));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("FindLast",
        [](benchmark::State& s) { run_find_last<T>(s, [](const auto& d) { return raze::algorithm::find_last(d, rb::needle_value<T>()); }); },
        [](benchmark::State& s) { run_find_last<T>(s, [](const auto& d) { return std::ranges::find_last(d, rb::needle_value<T>()); }); },
        [](benchmark::State& s) { run_find_last<T>(s, [](const auto& d) { return raze::algorithm::find_last[raze::options::fscalar](d, rb::needle_value<T>()); }); },
        rb::start_pos_cases);

    rb::register_triplet<T>("FindLastPred",
        [](benchmark::State& s) { run_find_last<T>(s, [](const auto& d) { return raze::algorithm::find_last_if(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_find_last<T>(s, [](const auto& d) { return std::ranges::find_last_if(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_find_last<T>(s, [](const auto& d) { return raze::algorithm::find_last_if[raze::options::fscalar](d, rb::unary_pred<T>()); }); },
        rb::start_pos_cases);

    rb::register_triplet<T>("FindLastIfNot",
        [](benchmark::State& s) { run_find_last<T>(s, [](const auto& d) { return raze::algorithm::find_last_if_not(d, rb::unary_pred_not<T>()); }); },
        [](benchmark::State& s) { run_find_last<T>(s, [](const auto& d) { return std::ranges::find_last_if_not(d, rb::unary_pred_not<T>()); }); },
        [](benchmark::State& s) { run_find_last<T>(s, [](const auto& d) { return raze::algorithm::find_last_if_not[raze::options::fscalar](d, rb::unary_pred_not<T>()); }); },
        rb::start_pos_cases);
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();