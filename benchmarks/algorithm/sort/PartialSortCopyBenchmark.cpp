#include <raze/algorithm/sort/PartialSortCopy.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_partial_sort_copy(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto src = rb::make_unsorted<T>(size);
    std::vector<T> dst(size / 4 > 0 ? size / 4 : 1);

    for (auto _ : state) {
        benchmark::DoNotOptimize(src);
        auto res = fn(src, dst);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(size * sizeof(T)));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("PartialSortCopy",
        [](benchmark::State& s) { run_partial_sort_copy<T>(s, [](const auto& in, auto& out) { return raze::algorithm::partial_sort_copy(in, out); }); },
        [](benchmark::State& s) { run_partial_sort_copy<T>(s, [](const auto& in, auto& out) { return std::ranges::partial_sort_copy(in, out); }); },
        [](benchmark::State& s) { run_partial_sort_copy<T>(s, [](const auto& in, auto& out) { return raze::algorithm::partial_sort_copy[raze::options::fscalar](in, out); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

