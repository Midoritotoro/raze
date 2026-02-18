#pragma once 

#include <type_traits>
#include <raze/RazeNamespace.h>


__RAZE_CONCURRENCY_NAMESPACE_BEGIN

namespace execution {
    struct SequencedPolicy {
        using                   standard_execution_policy = int;
        static constexpr bool   parallelize = false;
        static constexpr bool   sequenced   = true;
    };

    struct ParallelPolicy {
        using                   standard_execution_policy = int;
        static constexpr bool   parallelize = true;
        static constexpr bool   sequenced   = true;
    };

    struct ParallelUnsequencedPolicy {
        using                   standard_execution_policy = int;
        static constexpr bool   parallelize = true;
        static constexpr bool   sequenced   = false;
    };

    struct UnsequencedPolicy {
        using                   standard_execution_policy = int;
        static constexpr bool   parallelize = false;
        static constexpr bool   sequenced   = false;
    };

    inline constexpr SequencedPolicy            sequenced{};
    inline constexpr UnsequencedPolicy          unsequenced{};

    inline constexpr ParallelPolicy             parallel{};
    inline constexpr ParallelUnsequencedPolicy  parallel_unsequenced{};
} // namespace execution

template <class _Type_>
constexpr inline bool is_execution_policy_v = false;

template <>
constexpr inline bool is_execution_policy_v<execution::SequencedPolicy> = true;

template <>
constexpr inline bool is_execution_policy_v<execution::UnsequencedPolicy> = true;

template <>
constexpr inline bool is_execution_policy_v<execution::ParallelPolicy> = true;

template <>
constexpr inline bool is_execution_policy_v<execution::ParallelUnsequencedPolicy> = true;

template <class _Type_>
using enable_if_execution_policy = std::remove_reference_t<_Type_>::standard_execution_policy;

__RAZE_CONCURRENCY_NAMESPACE_END
