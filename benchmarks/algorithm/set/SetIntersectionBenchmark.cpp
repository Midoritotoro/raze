#include <raze/algorithm/set/SetIntersection.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_set_intersection(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto [a, b] = rb::make_two_sorted<T>(size);
    rb::bench_binary_copy_sized<T>(state, a, b, size, std::forward<Fn>(fn), 3);
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("SetIntersection",
        [](benchmark::State& s) { run_set_intersection<T>(s, [](const auto& a, const auto& b, auto& out) { return raze::algorithm::set_intersection(a, b, out.begin()); }); },
        [](benchmark::State& s) { run_set_intersection<T>(s, [](const auto& a, const auto& b, auto& out) { return std::ranges::set_intersection(a, b, out.begin()); }); },
        [](benchmark::State& s) { run_set_intersection<T>(s, [](const auto& a, const auto& b, auto& out) { return raze::algorithm::set_intersection[raze::options::fscalar](a, b, out.begin()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

