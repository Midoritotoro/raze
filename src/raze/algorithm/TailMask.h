#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/Concepts.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN


/**
 * @ingroup raze_algo_dispatch
 * @brief Lazy generator wrapper representing a SIMD tail mask for partial final vector iterations.
 *
 * Stores the exact remaining byte count (`tail_bytes()`) and a callable generator `Gen` that
 * materializes the underlying `simd_mask` on demand.
*/
template <class Gen>
struct tail_mask {
	using mask_type = decltype(std::declval<Gen>()());
	tail_mask(sizetype tail_bytes, Gen&& gen) : _gen(std::move(gen)), _tail_bytes(tail_bytes) {}

	raze_always_inline operator mask_type() const {
		return _gen();
	}

	raze_always_inline mask_type operator()() const {
		return _gen();
	}

	raze_always_inline mask_type mask() const {
		return _gen();
	}

	raze_always_inline sizetype tail_bytes() const {
		return _tail_bytes;
	}

	raze_always_inline sizetype tail_elements() const {
		return _tail_bytes / sizeof(typename mask_type::value_type);
	}
private:
	Gen _gen;
	sizetype _tail_bytes;
};

/**
 * @ingroup raze_algo_dispatch
 * @brief Concept satisfied by lazy tail-mask descriptors passed to masked tail kernel overloads
 *        and mask inspection functions (e.g., `vx::all_of`, `vx::any_of`, `vx::find_first_set`).
*/
template <class T>
concept tail_mask_type = requires(T const& ignore, sizetype bytes) {
	typename T::mask_type;

    { ignore.tail_bytes() } -> std::convertible_to<sizetype>;
    { ignore.tail_elements() } -> std::convertible_to<sizetype>;
	{ ignore.mask() } -> std::same_as<typename T::mask_type>;
    { ignore() } -> std::same_as<typename T::mask_type>;
	{ static_cast<typename T::mask_type>(ignore) } -> std::same_as<typename T::mask_type>;
};

__RAZE_ALGORITHM_NAMESPACE_END
