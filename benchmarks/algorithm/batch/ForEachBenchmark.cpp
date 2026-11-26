#include <raze/algorithm/batch/ForEach.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_for_each(benchmark::State& state, Fn&& fn) {
    auto data = rb::make_iota<T>(rb::arg_size(state));
    rb::bench_inplace<T>(state, data, std::forward<Fn>(fn), 2);
}

template <class T>
static void register_type() {
    auto op = [](auto& x) { x = (x * T(-1)) + T(100); };

    rb::register_triplet<T>("ForEach",
        [=](benchmark::State& s) { run_for_each<T>(s, [=](auto& d) { return raze::algorithm::for_each(d, op); }); },
        [=](benchmark::State& s) { run_for_each<T>(s, [=](auto& d) { return std::ranges::for_each(d, op); }); },
        [=](benchmark::State& s) { run_for_each<T>(s, [=](auto& d) { return raze::algorithm::for_each[raze::options::fscalar](d, op); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

