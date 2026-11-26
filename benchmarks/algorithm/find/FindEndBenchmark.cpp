#include <raze/algorithm/find/FindEnd.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_find_end(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state, 0);
    const auto pos = rb::arg_size(state, 1);
    const auto len = rb::arg_size(state, 2);
    const auto [haystack, needle] = rb::make_subrange_data<T>(size, pos, len);
    rb::bench_binary_read<T>(state, haystack, needle, std::forward<Fn>(fn), 1);
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("FindEnd",
        [](benchmark::State& s) { run_find_end<T>(s, [](const auto& h, const auto& n) { return raze::algorithm::find_end(h, n); }); },
        [](benchmark::State& s) { run_find_end<T>(s, [](const auto& h, const auto& n) { return std::ranges::find_end(h, n); }); },
        [](benchmark::State& s) { run_find_end<T>(s, [](const auto& h, const auto& n) { return raze::algorithm::find_end[raze::options::fscalar](h, n); }); },
        rb::search_start_cases);

    rb::register_triplet<T>("FindEndPred",
        [](benchmark::State& s) { run_find_end<T>(s, [](const auto& h, const auto& n) { return raze::algorithm::find_end(h, n, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_find_end<T>(s, [](const auto& h, const auto& n) { return std::ranges::find_end(h, n, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_find_end<T>(s, [](const auto& h, const auto& n) { return raze::algorithm::find_end[raze::options::fscalar](h, n, rb::binary_eq_pred<T>()); }); },
        rb::search_start_cases);
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

