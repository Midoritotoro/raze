#include <raze/algorithm/fill/Generate.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_generate(benchmark::State& state, Fn&& fn) {
    auto data = rb::make_iota<T>(rb::arg_size(state));
    std::uint32_t counter = 0;

    rb::bench_inplace<T>(state, data, [&](auto& d) {
        return fn(d, [&counter]() noexcept -> T {
            return static_cast<T>(++counter & 0x7Fu);
        });
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Generate",
        [](benchmark::State& s) { run_generate<T>(s, [](auto& d, auto gen) { return raze::algorithm::generate(d, gen); }); },
        [](benchmark::State& s) { run_generate<T>(s, [](auto& d, auto gen) { return std::ranges::generate(d, gen); }); },
        [](benchmark::State& s) { run_generate<T>(s, [](auto& d, auto gen) { return raze::algorithm::generate[raze::options::fscalar](d, gen); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

