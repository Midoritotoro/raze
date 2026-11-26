#include <raze/algorithm/numeric/InnerProduct.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_inner_product(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto a = rb::make_bounded_sum_data<T>(size);
    const std::vector<T> b(size, T(1));
    rb::bench_binary_read<T>(state, a, b, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("InnerProduct",
        [](benchmark::State& s) { run_inner_product<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::inner_product(a, b, T(0)); }); },
        [](benchmark::State& s) { run_inner_product<T>(s, [](const auto& a, const auto& b) { return std::inner_product(a.begin(), a.end(), b.begin(), T(0)); }); },
        [](benchmark::State& s) { run_inner_product<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::inner_product[raze::options::fscalar](a, b, T(0)); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

