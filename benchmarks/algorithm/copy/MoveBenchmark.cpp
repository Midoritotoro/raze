#include <raze/algorithm/copy/Move.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_move(benchmark::State& state, Fn&& fn) {
    auto src = rb::make_iota<T>(rb::arg_size(state));
    std::vector<T> dst(src.size());
    const auto size = src.size();

    for (auto _ : state) {
        benchmark::DoNotOptimize(src);
        auto res = fn(src, dst);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Move",
        [](benchmark::State& s) { run_move<T>(s, [](auto& in, auto& out) { return raze::algorithm::move(in, out.begin()); }); },
        [](benchmark::State& s) { run_move<T>(s, [](auto& in, auto& out) { return std::ranges::move(in, out.begin()); }); },
        [](benchmark::State& s) { run_move<T>(s, [](auto& in, auto& out) { return raze::algorithm::move[raze::options::fscalar](in, out.begin()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

