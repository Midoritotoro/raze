#include <raze/algorithm/copy/MoveBackward.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_move_backward(benchmark::State& state, Fn&& fn) {
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
    rb::register_triplet<T>("MoveBackward",
        [](benchmark::State& s) { run_move_backward<T>(s, [](auto& in, auto& out) { return raze::algorithm::move_backward(in, out.end()); }); },
        [](benchmark::State& s) { run_move_backward<T>(s, [](auto& in, auto& out) { return std::ranges::move_backward(in, out.end()); }); },
        [](benchmark::State& s) { run_move_backward<T>(s, [](auto& in, auto& out) { return raze::algorithm::move_backward[raze::options::fscalar](in, out.end()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

