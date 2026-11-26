#include <raze/algorithm/replace/Replace.h>
#include <raze/algorithm/replace/ReplaceIf.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_replace(benchmark::State& state, Fn&& fn) {
    T old_v = rb::needle_value<T>();
    T new_v = rb::alt_value<T>();
    auto data = rb::make_half_match<T>(rb::arg_size(state), old_v);

    rb::bench_inplace<T>(state, data, [&](auto& d) {
        fn(d, old_v, new_v);
        std::swap(old_v, new_v);
    });
}

template <class T, class Fn>
static void run_replace_if(benchmark::State& state, Fn&& fn) {
    auto data = rb::make_alternating<T>(rb::arg_size(state), T(1), T(-1));
    rb::bench_inplace<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Replace",
        [](benchmark::State& s) { run_replace<T>(s, [](auto& d, T o, T n) { raze::algorithm::replace(d, o, n); }); },
        [](benchmark::State& s) { run_replace<T>(s, [](auto& d, T o, T n) { std::ranges::replace(d, o, n); }); },
        [](benchmark::State& s) { run_replace<T>(s, [](auto& d, T o, T n) { raze::algorithm::replace[raze::options::fscalar](d, o, n); }); });

    rb::register_triplet<T>("ReplacePred",
        [](benchmark::State& s) { run_replace_if<T>(s, [](auto& d) { raze::algorithm::replace_if(d, [](auto x) { return x > T(0); }, T(2)); }); },
        [](benchmark::State& s) { run_replace_if<T>(s, [](auto& d) { std::ranges::replace_if(d, [](auto x) { return x > T(0); }, T(2)); }); },
        [](benchmark::State& s) { run_replace_if<T>(s, [](auto& d) { raze::algorithm::replace_if[raze::options::fscalar](d, [](auto x) { return x > T(0); }, T(2)); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();