#include <raze/algorithm/sort/NthElement.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_nth_element(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto src = rb::make_unsorted<T>(size);
    const auto nth_off = static_cast<std::ptrdiff_t>(size / 2);
    rb::bench_inplace_reset<T>(state, src, [&](auto& d) { return fn(d, d.begin() + nth_off); });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("NthElement",
        [](benchmark::State& s) { run_nth_element<T>(s, [](auto& d, auto nth) { return raze::algorithm::nth_element(d, nth); }); },
        [](benchmark::State& s) { run_nth_element<T>(s, [](auto& d, auto nth) { return std::ranges::nth_element(d, nth); }); },
        [](benchmark::State& s) { run_nth_element<T>(s, [](auto& d, auto nth) { return raze::algorithm::nth_element[raze::options::fscalar](d, nth); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

