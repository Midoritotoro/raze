#include <raze/algorithm/set/SetUnion.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_set_union(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto [a, b] = rb::make_two_sorted<T>(size);
    rb::bench_binary_copy_sized<T>(state, a, b, size * 2, std::forward<Fn>(fn), 3);
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("SetUnion",
        [](benchmark::State& s) { run_set_union<T>(s, [](const auto& a, const auto& b, auto& out) { return raze::algorithm::set_union(a, b, out.begin()); }); },
        [](benchmark::State& s) { run_set_union<T>(s, [](const auto& a, const auto& b, auto& out) { return std::ranges::set_union(a, b, out.begin()); }); },
        [](benchmark::State& s) { run_set_union<T>(s, [](const auto& a, const auto& b, auto& out) { return raze::algorithm::set_union[raze::options::fscalar](a, b, out.begin()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

