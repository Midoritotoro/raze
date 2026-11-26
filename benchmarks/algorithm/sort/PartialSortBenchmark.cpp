#include <raze/algorithm/sort/PartialSort.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_partial_sort(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto src = rb::make_unsorted<T>(size);
    const auto mid_off = static_cast<std::ptrdiff_t>(size / 4);
    rb::bench_inplace_reset<T>(state, src, [&](auto& d) { return fn(d, d.begin() + mid_off); });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("PartialSort",
        [](benchmark::State& s) { run_partial_sort<T>(s, [](auto& d, auto mid) { return raze::algorithm::partial_sort(d, mid); }); },
        [](benchmark::State& s) { run_partial_sort<T>(s, [](auto& d, auto mid) { return std::ranges::partial_sort(d, mid); }); },
        [](benchmark::State& s) { run_partial_sort<T>(s, [](auto& d, auto mid) { return raze::algorithm::partial_sort[raze::options::fscalar](d, mid); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

