#include <raze/algorithm/order/Sample.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_sample(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto src = rb::make_iota<T>(size);
    const auto k = static_cast<std::ptrdiff_t>(size / 2);
    std::vector<T> dst(static_cast<std::size_t>(k));
    std::mt19937 rng(42);

    for (auto _ : state) {
        benchmark::DoNotOptimize(src);
        auto res = fn(src, dst, k, rng);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(size * sizeof(T)));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Sample",
        [](benchmark::State& s) { run_sample<T>(s, [](const auto& in, auto& out, auto k, auto& g) { return raze::algorithm::sample(in, out.begin(), k, g); }); },
        [](benchmark::State& s) { run_sample<T>(s, [](const auto& in, auto& out, auto k, auto& g) { return std::ranges::sample(in, out.begin(), k, g); }); },
        [](benchmark::State& s) { run_sample<T>(s, [](const auto& in, auto& out, auto k, auto& g) { return raze::algorithm::sample[raze::options::fscalar](in, out.begin(), k, g); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

