#include <raze/algorithm/extreme/Clamp.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_clamp(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_extreme_data<T>(rb::arg_size(state));
    const T lo = static_cast<T>(-50);
    const T hi = static_cast<T>(50);
    rb::bench_copy<T>(state, src, [&](const auto& in, auto& out) {
        const auto n = in.size();
        for (std::size_t i = 0; i < n; ++i)
            out[i] = fn(in[i], lo, hi);
        return out.end();
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Clamp",
        [](benchmark::State& s) { run_clamp<T>(s, [](const T& v, const T& lo, const T& hi) -> const T& { return raze::algorithm::clamp(v, lo, hi); }); },
        [](benchmark::State& s) { run_clamp<T>(s, [](const T& v, const T& lo, const T& hi) -> const T& { return std::ranges::clamp(v, lo, hi); }); },
        [](benchmark::State& s) { run_clamp<T>(s, [](const T& v, const T& lo, const T& hi) -> const T& { return raze::algorithm::clamp[raze::options::fscalar](v, lo, hi); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

