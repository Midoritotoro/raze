#include <raze/algorithm/find/Equal.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_equal(benchmark::State& state, Fn&& fn) {
    const auto a = rb::make_iota<T>(rb::arg_size(state));
    const auto b = a;
    rb::bench_binary_read<T>(state, a, b, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Equal",
        [](benchmark::State& s) { run_equal<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::equal(a, b); }); },
        [](benchmark::State& s) { run_equal<T>(s, [](const auto& a, const auto& b) { return std::ranges::equal(a, b); }); },
        [](benchmark::State& s) { run_equal<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::equal[raze::options::fscalar](a, b); }); });

    rb::register_triplet<T>("EqualPred",
        [](benchmark::State& s) { run_equal<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::equal(a, b, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_equal<T>(s, [](const auto& a, const auto& b) { return std::ranges::equal(a, b, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_equal<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::equal[raze::options::fscalar](a, b, rb::binary_eq_pred<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();