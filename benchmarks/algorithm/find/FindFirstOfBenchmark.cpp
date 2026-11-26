#include <raze/algorithm/find/FindFirstOf.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_find_first_of(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state, 0);
    const auto pos = rb::arg_size(state, 1);
    const auto data = rb::make_find_data<T>(size, pos);
    const std::vector<T> set{ static_cast<T>(120), static_cast<T>(121), static_cast<T>(122), rb::needle_value<T>() };
    rb::bench_binary_read<T>(state, data, set, std::forward<Fn>(fn), 1);
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("FindFirstOf",
        [](benchmark::State& s) { run_find_first_of<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::find_first_of(a, b); }); },
        [](benchmark::State& s) { run_find_first_of<T>(s, [](const auto& a, const auto& b) { return std::ranges::find_first_of(a, b); }); },
        [](benchmark::State& s) { run_find_first_of<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::find_first_of[raze::options::fscalar](a, b); }); },
        rb::end_pos_cases);

    rb::register_triplet<T>("FindFirstOfPred",
        [](benchmark::State& s) { run_find_first_of<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::find_first_of(a, b, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_find_first_of<T>(s, [](const auto& a, const auto& b) { return std::ranges::find_first_of(a, b, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_find_first_of<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::find_first_of[raze::options::fscalar](a, b, rb::binary_eq_pred<T>()); }); },
        rb::end_pos_cases);
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

