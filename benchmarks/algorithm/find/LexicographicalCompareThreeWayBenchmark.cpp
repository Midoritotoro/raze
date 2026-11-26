#include <raze/algorithm/find/LexicographicalCompareThreeWay.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_lex_cmp_3way(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto a = rb::make_iota<T>(size);
    auto b = a;
    if (!b.empty()) b.back() = rb::needle_value<T>();
    rb::bench_binary_read<T>(state, a, b, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("LexicographicalCompareThreeWay",
        [](benchmark::State& s) { run_lex_cmp_3way<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::lexicographical_compare_three_way(a, b); }); },
        [](benchmark::State& s) { run_lex_cmp_3way<T>(s, [](const auto& a, const auto& b) { return std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end()); }); },
        [](benchmark::State& s) { run_lex_cmp_3way<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::lexicographical_compare_three_way[raze::options::fscalar](a, b); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

