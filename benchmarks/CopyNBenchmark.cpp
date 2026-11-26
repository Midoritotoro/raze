#include <raze/algorithm/copy/CopyN.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_copy_n(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_iota<T>(rb::arg_size(state));
    const auto n = static_cast<std::ptrdiff_t>(src.size());
    rb::bench_copy<T>(state, src, [&](const auto& in, auto& out) { return fn(in, n, out); });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("CopyN",
        [](benchmark::State& s) { run_copy_n<T>(s, [](const auto& in, std::ptrdiff_t n, auto& out) { return raze::algorithm::copy_n(in.begin(), n, out.begin()); }); },
        [](benchmark::State& s) { run_copy_n<T>(s, [](const auto& in, std::ptrdiff_t n, auto& out) { return std::ranges::copy_n(in.begin(), n, out.begin()); }); },
        [](benchmark::State& s) { run_copy_n<T>(s, [](const auto& in, std::ptrdiff_t n, auto& out) { return raze::algorithm::copy_n[raze::options::fscalar](in.begin(), n, out.begin()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

