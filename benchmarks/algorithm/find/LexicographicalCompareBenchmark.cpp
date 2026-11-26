#include <raze/algorithm/find/LexicographicalCompare.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_lex_cmp(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto a = rb::make_iota<T>(size);
    auto b = a;
    if (!b.empty()) b.back() = rb::needle_value<T>();
    rb::bench_binary_read<T>(state, a, b, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("LexicographicalCompare",
        [](benchmark::State& s) { run_lex_cmp<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::lexicographical_compare(a, b); }); },
        [](benchmark::State& s) { run_lex_cmp<T>(s, [](const auto& a, const auto& b) { return std::ranges::lexicographical_compare(a, b); }); },
        [](benchmark::State& s) { run_lex_cmp<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::lexicographical_compare[raze::options::fscalar](a, b); }); });

    rb::register_triplet<T>("LexicographicalCompareComp",
        [](benchmark::State& s) { run_lex_cmp<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::lexicographical_compare(a, b, rb::custom_comp()); }); },
        [](benchmark::State& s) { run_lex_cmp<T>(s, [](const auto& a, const auto& b) { return std::ranges::lexicographical_compare(a, b, rb::custom_comp()); }); },
        [](benchmark::State& s) { run_lex_cmp<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::lexicographical_compare[raze::options::fscalar](a, b, rb::custom_comp()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

