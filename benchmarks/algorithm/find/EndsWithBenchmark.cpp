#include <raze/algorithm/find/EndsWith.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

namespace {

    template <class T>
    bool std_ends_with(const std::vector<T>& a, const std::vector<T>& b) {
#if defined(__cpp_lib_ranges_starts_ends_with)
        return std::ranges::ends_with(a, b);
#else
        if (a.size() < b.size()) return false;
        const auto off = static_cast<std::ptrdiff_t>(a.size() - b.size());
        return std::ranges::equal(a.begin() + off, a.end(), b.begin(), b.end());
#endif
    }

    template <class T, class Pred>
    bool std_ends_with(const std::vector<T>& a, const std::vector<T>& b, Pred pred) {
#if defined(__cpp_lib_ranges_starts_ends_with)
        return std::ranges::ends_with(a, b, pred);
#else
        if (a.size() < b.size()) return false;
        const auto off = static_cast<std::ptrdiff_t>(a.size() - b.size());
        return std::ranges::equal(a.begin() + off, a.end(), b.begin(), b.end(), pred);
#endif
    }

}

template <class T, class Fn>
static void run_ends_with(benchmark::State& state, Fn&& fn) {
    const auto a = rb::make_iota<T>(rb::arg_size(state));
    const auto b = a;
    rb::bench_binary_read<T>(state, a, b, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("EndsWith",
        [](benchmark::State& s) { run_ends_with<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::ends_with(a, b); }); },
        [](benchmark::State& s) { run_ends_with<T>(s, [](const auto& a, const auto& b) { return std_ends_with(a, b); }); },
        [](benchmark::State& s) { run_ends_with<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::ends_with[raze::options::fscalar](a, b); }); });

    rb::register_triplet<T>("EndsWithPred",
        [](benchmark::State& s) { run_ends_with<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::ends_with(a, b, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_ends_with<T>(s, [](const auto& a, const auto& b) { return std_ends_with(a, b, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_ends_with<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::ends_with[raze::options::fscalar](a, b, rb::binary_eq_pred<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();
