#pragma once

#include <raze/compatibility/Compatibility.h>
#include <raze/options/Options.h>
#include <src/raze/algorithm/AdvanceBytes.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/vx/Concepts.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct swap_3_ranges_t : Traits, dispatchable<swap_3_ranges_t<Traits>> {
    struct kernel {
        using vector_value_type = i32;

        static consteval auto targets() noexcept {
            return algorithm::targets<arch::ISA::AVX512F, arch::ISA::AVX, arch::ISA::SSE2>{};
        }

        static consteval bool vectorizable() noexcept {
            return true;
        }

        void* _first1;
        void* _last1;
        void* _first2;
        void* _first3;

        raze_always_inline explicit kernel(void* first1, void* last1, void* first2, void* first3) noexcept
            : _first1(first1),
              _last1(last1),
              _first2(first2),
              _first3(first3)
        {}

        raze_always_inline void operator()() noexcept {
            auto* ch_first1 = static_cast<char*>(_first1);
            auto* ch_first2 = static_cast<char*>(_first2);
            auto* ch_first3 = static_cast<char*>(_first3);
            const auto* ch_last1 = static_cast<const char*>(_last1);

            raze_disable_unrolling
            for (; ch_first1 != ch_last1; ++ch_first1, ++ch_first2, ++ch_first3) {
                const char v1 = *ch_first1;
                const char v2 = *ch_first2;
                const char v3 = *ch_first3;

                *ch_first1 = v2;
                *ch_first2 = v3;
                *ch_first3 = v1;
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, sizetype aligned_size) noexcept {
            const auto aligned_end = bytes_pointer_offset(_first1, aligned_size);

            raze_disable_unrolling do {
                const auto v1 = vx::load<Tag>(_first1);
                const auto v2 = vx::load<Tag>(_first2);
                const auto v3 = vx::load<Tag>(_first3);

                vx::store(_first1, v2);
                vx::store(_first2, v3);
                vx::store(_first3, v1);

                advance_bytes(_first1, _first3, sizeof(Tag));
                advance_bytes(_first2, sizeof(Tag));
            } while (_first1 != aligned_end);
        }

        raze_always_inline void result() const noexcept {}

        raze_nodiscard raze_always_inline sizetype size() const noexcept {
            return static_cast<sizetype>(static_cast<const char*>(_last1) - static_cast<const char*>(_first1));
        }
    };

    raze_always_inline void operator()(void* first1, void* last1, void* first2, void* first3) const noexcept {
        this->dispatch(first1, last1, first2, first3);
    }
};

constexpr inline auto __swap_3_ranges = options::function_with_traits<swap_3_ranges_t>;

__RAZE_ALGORITHM_NAMESPACE_END