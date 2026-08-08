#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>
#include <random>
#include <algorithm>
#include <iostream>

template <class _Type_, raze::arch::ISA _ISA_, raze::u32 _Width_>
struct variable_length_mask_tests {

    template <raze::u32 _N_>
    void test_size() const {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _N_>>;
        using Mask = raze::vx::simd_mask<_Type_, raze::vx::runtime_abi<_ISA_, _N_>>;

        constexpr size_t N = _N_;

        if (!raze::arch::ProcessorFeatures::isSupported<Simd::__isa>())
            return;

        auto make_pattern = [](auto f) {
            Mask m(false);
            for (size_t i = 0; i < N; ++i)
                m[i] = f(i);
            return m;
            };

        {
            Mask t(true);
            Mask f(false);

            raze_assert(raze::vx::any_of(t));
            raze_assert(raze::vx::all_of(t));
            raze_assert(raze::vx::none_of(f));
        }

        {
            Mask t(true);
            Mask f(false);

            raze_assert(raze::vx::all_of(t));
            raze_assert(!raze::vx::none_of(t));

            raze_assert(raze::vx::none_of(f));
            raze_assert(!raze::vx::any_of(f));

            Mask m(false);
            m[0] = true;

            raze_assert(raze::vx::any_of(m));
            N == 1 ? raze_assert(raze::vx::all_of(m)) : raze_assert(!raze::vx::all_of(m));
        }

        {
            Mask m(false);

            for (size_t n = 0; n < N; ++n)
                for (size_t k = n + 1; k < N; ++k)
                    raze_assert(!raze::vx::is_contiguous(m, n, k));
        }

        {
            Mask m(true);

            for (size_t n = 0; n < N; ++n)
                for (size_t k = n; k < N; ++k)
                    raze_assert(raze::vx::is_contiguous(m, n, k));
        }

        {
            std::mt19937 rng(123456);

            for (int iter = 0; iter < 100; ++iter) {
                Mask m(false);

                for (size_t i = 0; i < N; ++i)
                    m[i] = (rng() & 1);

                for (size_t n = 0; n < N; ++n) {
                    for (size_t k = n; k < N; ++k) {

                        bool expected = true;
                        for (size_t i = n; i < k; ++i)
                            if (!m[i]) expected = false;

                        raze_assert(raze::vx::is_contiguous(m, n, k) == expected);
                    }
                }
            }
        }


        {
            Mask all_true(true);
            Mask all_false(false);
            Mask first_only(false);
            if constexpr (N > 0) first_only[0] = true;

            raze_assert(!raze::vx::any_of[all_false](all_true));
            raze_assert(!raze::vx::any_of[all_false](first_only));
            raze_assert(raze::vx::any_of[all_true](all_true));
            raze_assert(!raze::vx::any_of[all_true](all_false));
            if constexpr (N > 0) {
                raze_assert(raze::vx::any_of[all_true](first_only));
                raze_assert(raze::vx::any_of[first_only](all_true));
                raze_assert(raze::vx::any_of[first_only](first_only));
                raze_assert(!raze::vx::any_of[first_only](all_false));
            }

            raze_assert(raze::vx::all_of[all_false](all_true));
            raze_assert(raze::vx::all_of[all_false](first_only));
            raze_assert(raze::vx::all_of[all_true](all_true));
            raze_assert(!raze::vx::all_of[all_true](all_false));
            if constexpr (N > 0) {
                raze_assert(raze::vx::all_of[first_only](all_true));
                raze_assert(raze::vx::all_of[first_only](first_only));
                raze_assert(!raze::vx::all_of[first_only](all_false));
            }

            raze_assert(raze::vx::none_of[all_false](all_true));
            raze_assert(raze::vx::none_of[all_false](first_only));
            raze_assert(!raze::vx::none_of[all_true](all_true));
            raze_assert(raze::vx::none_of[all_true](all_false));
            if constexpr (N > 0) {
                raze_assert(!raze::vx::none_of[all_true](first_only));
                raze_assert(!raze::vx::none_of[first_only](all_true));
                raze_assert(!raze::vx::none_of[first_only](first_only));
                raze_assert(raze::vx::none_of[first_only](all_false));
            }
        }

        {
            std::mt19937 rng(789012);

            for (int iter = 0; iter < 100; ++iter) {
                Mask main(false), extra(false);
                for (size_t i = 0; i < N; ++i) {
                    main[i] = (rng() & 1);
                    extra[i] = (rng() & 1);
                }

                bool expected_any = false;
                bool expected_all = true;
                bool expected_none = true;
                bool has_true_in_extra = false;
                bool has_false_in_extra = false;

                for (size_t i = 0; i < N; ++i) {
                    if (extra[i]) {
                        if (main[i]) {
                            expected_any = true;
                            has_true_in_extra = true;
                        }
                        else {
                            expected_all = false;
                            has_false_in_extra = true;
                        }
                    }
                }
                expected_none = !expected_any;
                bool expected_some = has_true_in_extra && has_false_in_extra;

                raze_assert(raze::vx::any_of[extra](main) == expected_any);
                raze_assert(raze::vx::all_of[extra](main) == expected_all);
                raze_assert(raze::vx::none_of[extra](main) == expected_none);
            }
        }

        {
            if constexpr (N > 0) {
                Mask main(false), extra(false);
                main[0] = true;
                extra[0] = true;
                raze_assert(raze::vx::any_of[extra](main));
                raze_assert(raze::vx::all_of[extra](main));
                raze_assert(!raze::vx::none_of[extra](main));
            }

            if constexpr (N > 1) {
                Mask main(false), extra(false);
                main[0] = true;
                extra[0] = true;
                extra[1] = true;
                raze_assert(raze::vx::any_of[extra](main));
                raze_assert(!raze::vx::all_of[extra](main));
                raze_assert(!raze::vx::none_of[extra](main));
            }

            if constexpr (N > 1) {
                Mask main(false), extra(false);
                extra[0] = true;
                extra[1] = true;
                raze_assert(!raze::vx::any_of[extra](main));
                raze_assert(!raze::vx::all_of[extra](main));
                raze_assert(raze::vx::none_of[extra](main));
            }
        }
    }

    void operator()() const {
        test_size<(_Width_ / (sizeof(_Type_) * 8))>();
        test_size<(_Width_ / (sizeof(_Type_) * 8)) + 1>();
        test_size<1>();
    }
};

int main() {
    test_all<variable_length_mask_tests>();
}