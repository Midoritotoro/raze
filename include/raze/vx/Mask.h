#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/MaskBroadcast.h>
#include <src/raze/vx/hw/x86/mask/operations/LoadMask.h>
#include <src/raze/vx/hw/x86/mask/operations/StoreMask.h>
#include <src/raze/vx/hw/configurable/bitwise/And.h>
#include <src/raze/vx/hw/configurable/bitwise/Or.h>
#include <src/raze/vx/hw/configurable/bitwise/Xor.h>
#include <src/raze/vx/hw/configurable/bitwise/Not.h>
#include <src/raze/traits/IteratorCheck.h>
#include <src/raze/vx/reference/SimdMaskReference.h>
#include <src/raze/utility/UninitializedTag.h>
#include <src/raze/vx/Storage.h>


__RAZE_VX_NAMESPACE_BEGIN

/**
 * @class simd_mask
 * @brief A fixed-size SIMD mask with ISA/ABI-aware backend dispatch.
 *
 * `simd_mask<T, Abi>` is the mask counterpart of `raze::vx::simd<T, Abi>`.
 * It has the same lane count as `simd<T, Abi>` and is used as the operand
 * type for masked overloads of vector primitives and algorithms, e.g.:
 *
 * ```cpp
 * auto m = (a < b);                 // simd_mask
 * auto r = raze::vx::add[m](a, b);  // masked add
 * ```
 *
 * A mask stores one boolean per lane. The in-register representation is
 * ABI-dependent (byte masks on SSE/AVX2, `k`-registers on AVX-512), but the
 * public interface is uniform and lane-indexed.
 *
 * ## Key Characteristics
 * - Same lane count as the corresponding `simd<T, Abi>` (`size()`).
 * - **Default-constructs to all-false**, unlike `simd` which leaves lanes
 *   uninitialized. Use `simd_mask(uninitialized)` to skip initialization.
 * - Boolean operations (`&`, `|`, `^`, `!`) and comparisons (`==`, `!=`)
 *   operate element-wise and return `simd_mask`, not `bool`.
 *
 * ## Template Parameters
 * @tparam T    The scalar element type of the paired `simd`.
 * @tparam Abi  ABI descriptor; must match the paired `simd<T, Abi>`.
 *
 * ## Type aliases
 * - `value_type`, `element_type` — `T`.
 * - `abi_type`                   — the `Abi` descriptor.
 * - `storage_type`               — low-level mask storage for the active ABI.
 * - `reference_type`             — proxy reference used by mutable `operator[]`.
*/
template <class T, class Abi>
class simd_mask {
	static_assert(traits::is_vector_type_supported_v<T>);
public:
	using storage_type = mask_storage<T, Abi>;
	using element_type = T;
	using reference_type = simd_mask_reference<simd_mask>;
	using value_type = T;
	using abi_type = Abi;

	raze_always_inline simd_mask() noexcept {
		_storage.__for_each_chunk([] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
			chunk = mask_zero_<abi_type::isa, typename Chunk::unwrapped_type>();
		});
	}

	simd_mask(const simd_mask&) = default;
	simd_mask(simd_mask&&) = default;
	~simd_mask() = default;

	raze_always_inline simd_mask& operator=(const simd_mask& other) noexcept = default;
	raze_always_inline simd_mask& operator=(simd_mask&&) noexcept = default;

	raze_always_inline simd_mask(uninitialized_tag) noexcept {}
	
	/**
	 * @brief Constructs a mask with every lane set to `v`.
	 *
	 * @param v  Value broadcast to all lanes.
	*/
	raze_no_stack_protector raze_always_inline explicit simd_mask(bool v) noexcept {
		_storage.__for_each_chunk([v] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
			chunk = mask_broadcast_<abi_type::isa, Chunk::size, typename Chunk::unwrapped_type, value_type>(v);
		});
	}

	/**
	 * @brief Constructs a mask by copying lane values from an iterator range.
	 *
	 * Equivalent to default-constructing and then calling `copy_from(it, policy)`.
	 * Reads exactly `size()` elements from `it`.
	 *
	 * @tparam FwdIt   Forward iterator; `std::iter_value_t<FwdIt>` must be
	 *                 convertible to `bool`.
	 * @tparam Policy  Access policy; defaults to `unaligned_policy`.
	 * @param it       Iterator to the first lane value.
	 * @param policy   Access policy forwarded to `copy_from`.
	 *
	 * @note `it` is taken by value; the range `[it, it + size())` must be valid
	 *       for the duration of the call.
	*/
	template <std::forward_iterator FwdIt, class Policy = unaligned_policy>
	raze_no_stack_protector raze_always_inline simd_mask(FwdIt it, Policy&& policy = {}) noexcept {
		copy_from(it, policy);
	}

	/**
	 * @brief Overwrites every lane of `*this` from an iterator range.
	 *
	 * Reads exactly `size()` elements starting at `it` and stores them lane-wise.
	 *
	 * @tparam FwdIt   Forward iterator; `std::iter_value_t<FwdIt>` must be
	 *                 convertible to `bool`.
	 * @tparam Policy  Access policy; defaults to `unaligned_policy`. Forwarded to
	 *                 `load_mask_` on the fast path; ignored on the fallback.
	 * @param it       Iterator to the first lane value.
	 * @param policy   Access policy.
	 *
	 * @note `it` is advanced `size()` times; the range `[it, it + size())` must be valid.  
	*/
	template <std::forward_iterator FwdIt, class Policy = unaligned_policy>
	raze_no_stack_protector raze_always_inline void copy_from(FwdIt it, Policy&& policy = {}) noexcept
		requires(std::convertible_to<std::iter_value_t<FwdIt>, bool>)
	{
		using Unwrapped = traits::unwrapped_iterator_type<FwdIt>;
		using Value = std::iter_value_t<Unwrapped>;

		if constexpr (sizeof(Value) == 1 && std::contiguous_iterator<Unwrapped> && sizeof(T) == 1) {
			auto current = reinterpret_cast<const bool*>(std::to_address(it));

			__for_each_chunk([&] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
				chunk = load_mask_<abi_type::isa, Chunk::size, typename Chunk::unwrapped_type, value_type>(current, policy);
				algorithm::advance_bytes(current, Chunk::size * sizeof(value_type));
			});
		}
		else {
			for (auto i = 0; i < size(); ++i)
				__insert(i, static_cast<bool>(*it++));
		}
	}

	/**
	 * @brief Writes every lane of `*this` to an output iterator range.
	 *
	 * Writes exactly `size()` boolean values starting at `out`.
	 *
	 * @tparam OutIt   Output iterator.
	 * @tparam Policy  Access policy; defaults to `unaligned_policy`. 
	 * @param out      Output iterator to the first destination slot.
	 * @param policy   Access policy.
	*/
	template <class OutIt, class Policy = unaligned_policy>
	raze_always_inline void copy_to(OutIt out, Policy&& policy = {}) noexcept {
		using Unwrapped = traits::unwrapped_iterator_type<OutIt>;
		using Value = std::iter_value_t<Unwrapped>;

		if constexpr (sizeof(Value) == 1 && std::contiguous_iterator<Unwrapped> && sizeof(T) == 1) {
			auto current = reinterpret_cast<bool*>(std::to_address(out));

			__for_each_chunk([&current, policy] <class Chunk> (const Chunk& chunk) raze_always_inline_lambda {
				store_mask_<abi_type::isa, Chunk::size, value_type>(current, chunk.data(), policy);
				algorithm::advance_bytes(current, Chunk::size * sizeof(value_type));
			});
		}
		else {
			for (auto i = 0; i < size(); ++i)
				*out++ = __extract(i);
		}
	}

	/**
	 * @brief Returns lane `i` as `bool` (runtime index, read-only).
	 *
	 * @param i  Lane index; must satisfy `0 <= i < size()`.
	 * @return `true` iff lane `i` is set.
	*/
	raze_always_inline bool operator[](i32 i) const noexcept {
		return __extract(i);
	}

	/**
	 * @brief Returns a mutable proxy referring to lane `i` (runtime index).
	 *
	 * The proxy is a `simd_mask_reference<simd_mask>`; it supports `m[i] = true;`
	 * and read-through, but is not a `bool&`.
	 *
	 * @param i  Lane index; must satisfy `0 <= i < size()`.
	 * @return A proxy referring to lane `i` of `*this`.
	 *
	 * @note No bounds checking; out-of-range `i` is undefined behavior.
	*/
	raze_always_inline auto operator[](i32 i) noexcept {
		return reference_type(*this, i);
	}

	/**
	 * @brief Returns the number of lanes (`= Abi::size`).
	 *
	 * Matches the lane count of the paired `simd<T, Abi>`.
	*/
	raze_nodiscard raze_always_inline static constexpr auto size() noexcept {
		return abi_type::size;
	}
	
	/**
	 * @brief Element-wise bitwise AND.
	 * @return Mask with lane `i` set if `x[i] && y[i]`.
	*/
	friend raze_always_inline simd_mask operator&(const simd_mask& x, const simd_mask& y) noexcept {
		return bit_and(x, y);
	}

	/**
	 * @brief Element-wise bitwise OR.
	 * @return Mask with lane `i` set if `x[i] || y[i]`.
	*/
	friend raze_always_inline simd_mask operator|(const simd_mask& x, const simd_mask& y) noexcept {
		return bit_or(x, y);
	}
	
	/**
	 * @brief Element-wise bitwise XOR.
	 * @return Mask with lane `i` set if `x[i] != y[i]`.
	*/
	friend raze_always_inline simd_mask operator^(const simd_mask& x, const simd_mask& y) noexcept {
		return bit_xor(x, y);
	}

	/**
	 * @brief Element-wise logical OR; boolean-friendly alias for `operator|`.
	 *
	 * For every lane `i`, the result lane is `x[i] || y[i]`. Because each lane
	 * of `simd_mask` is a boolean, the result is bit-identical to `x | y`. 
	 *
	 * @param x  Left mask operand.
	 * @param y  Right mask operand.
	 * @return `simd_mask` with lane `i` set if `x[i] || y[i]`.
	 *
	 * @code
	 * simd_mask either = (a < b) || (c < d);
	 * @endcode
	*/
	friend raze_always_inline simd_mask operator||(const simd_mask& x, const simd_mask& y) noexcept {
		return x | y;
	}

	/**
	 * @brief Element-wise logical AND; boolean-friendly alias for `operator&`.
	 *
	 * For every lane `i`, the result lane is `x[i] && y[i]`. Because each lane
	 * of `simd_mask` is a boolean, the result is
	 * bit-identical to `x & y`.
	 *
	 * @param x  Left mask operand.
	 * @param y  Right mask operand.
	 * @return `simd_mask` with lane `i` set if `x[i] && y[i]`.
	 *
	 * @code
	 * simd_mask both = (a < b) && (c < d);
	 * @endcode
	*/
	friend raze_always_inline simd_mask operator&&(const simd_mask& x, const simd_mask& y) noexcept {
		return x & y;
	}

	/**
	 * @brief In-place element-wise AND.
	 * Equivalent to `*this = *this & other`.
	 * @return Reference to `*this`.
	*/
	raze_always_inline simd_mask& operator&=(const simd_mask& other) noexcept {
		return *this = (*this & other);
	}

	/**
	 * @brief In-place element-wise OR.
	 * Equivalent to `*this = *this | other`.
	 * @return Reference to `*this`.
	*/
	raze_always_inline simd_mask& operator|=(const simd_mask& other) noexcept {
		return *this = (*this | other);
	}

	/**
	 * @brief In-place element-wise XOR.
	 * Equivalent to `*this = *this ^ other`.
	 * @return Reference to `*this`.
	*/
	raze_always_inline simd_mask& operator^=(const simd_mask& other) noexcept {
		return *this = (*this ^ other);
	}

	/**
	 * @brief Element-wise logical NOT.
	 * @return Mask with lane `i` set if `(*this)[i]` is `false`.
	 *
	 * @code
	 * simd_mask m = (a < b);
	 * simd_mask n = !m;   // lanes where a >= b
	 * @endcode
	*/
	raze_always_inline simd_mask operator!() const noexcept {
		return bit_not(*this);
	}

	raze_always_inline simd_mask operator~() const noexcept {
		return !(*this);
	}

	/**
	 * @brief Element-wise mask inequality.
	 *
	 * Returns a mask whose lane `i` is set if `x[i] != y[i]`, i.e. the lanes
	 * where the two masks disagree.
	 *
	 * @return Mask of lanes where `x` and `y` differ.
	*/
	friend raze_always_inline simd_mask operator!=(const simd_mask& x, const simd_mask& y) noexcept {
		return bit_xor(x, y);
	}

	/**
	 * @brief Element-wise mask equality.
	 *
	 * Returns a mask whose lane `i` is set if `x[i] == y[i]`, i.e. the lanes
	 * where the two masks agree.
	 *
	 * @return Mask of lanes where `x` and `y` are equal.
	*/
	friend raze_always_inline simd_mask operator==(const simd_mask& x, const simd_mask& y) noexcept {
		return !(x != y);
	}

	raze_nodiscard raze_always_inline const storage_type& __storage() const noexcept {
		return _storage;
	}

	raze_nodiscard raze_always_inline storage_type& __storage() noexcept {
		return _storage;
	}

	raze_nodiscard raze_always_inline static constexpr bool is_native() noexcept {
		return storage_type::is_native();
	}

	raze_nodiscard raze_always_inline static constexpr auto __chunks_count() noexcept {
		return storage_type::chunks_count();
	}

	template <sizetype I>
	raze_always_inline auto __get() const noexcept {
		return raze::vx::get<I>(_storage.storage());
	}

	template <class F, class ... Args>
	raze_always_inline void __for_each_chunk(F&& f, Args&& ... args) noexcept {
		_storage.__for_each_chunk(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline void __for_each_chunk_reverse(F&& f, Args&& ... args) noexcept {
		_storage.__for_each_chunk_reverse(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline void __for_each_chunk(F&& f, Args&& ... args) const noexcept {
		_storage.__for_each_chunk(std::forward<F>(f), std::forward<Args>(args)...);
	}
	
	template <class F, class ... Args>
	raze_always_inline void __for_each_chunk_reverse(F&& f, Args&& ... args) const noexcept {
		_storage.__for_each_chunk_reverse(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_any_of(F&& f, Args&& ... args) noexcept {
		return _storage.__for_each_chunk_any_of(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_any_of_reverse(F&& f, Args&& ... args) noexcept {
		return _storage.__for_each_chunk_any_of_reverse(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_any_of(F&& f, Args&& ... args) const noexcept {
		return _storage.__for_each_chunk_any_of(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_any_of_reverse(F&& f, Args&& ... args) const noexcept {
		return _storage.__for_each_chunk_any_of_reverse(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_all_of(F&& f, Args&& ... args) noexcept {
		return _storage.__for_each_chunk_all_of(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_all_of_reverse(F&& f, Args&& ... args) noexcept {
		return _storage.__for_each_chunk_all_of_reverse(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_all_of(F&& f, Args&& ... args) const noexcept {
		return _storage.__for_each_chunk_all_of(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_all_of_reverse(F&& f, Args&& ... args) const noexcept {
		return _storage.__for_each_chunk_all_of_reverse(std::forward<F>(f), std::forward<Args>(args)...);
	}

	raze_always_inline void __insert(i32 i, bool v) noexcept {
		_storage.insert(i, v);
    }

    raze_nodiscard raze_always_inline bool __extract(i32 i) const noexcept {
		return _storage.extract(i);
    }
private:
	storage_type _storage;
};

__RAZE_VX_NAMESPACE_END

/**
 * @page mask_semantics Mask semantics in `raze::vx`
 *
 * Most configurable primitives in `raze::vx` — `broadcast`, `all_bits`,
 * `zero`, `add`, `sub`, `mul`, `div`, `select`, `load`, `store`, and their
 * friends — accept mask modifiers through `operator[]`. This page describes
 * those modifiers once; individual primitive pages only list which forms
 * they support and link here.
 *
 * ### Call forms
 *
 * For a primitive `op` (a `options::functor`):
 *
 * ```cpp
 * op(...);              // (1) unmasked
 * op[m](...);           // (2) maskz
 * op[m, src](...);      // (3) merge
 * op[src, m](...);      // (3') merge, arguments in any order
 * ```
 *
 * - **Unmasked (1)** — every lane receives the primitive's natural result.
 * - **Maskz (2)** — lane `i` receives the result if `m` selects it, else the
 *   lane is zeroed. 
 * - **Merge (3)/(3')** — lane `i` receives the result if `m` selects it, else
 *   `src[i]`. `m` and `src` may be given in either order; both overloads
 *   normalize to the same `or_(m, src)` expression.
 * ### Low-level equivalence
 *
 * This is mostly useful inside generic code that builds conditions
 * dynamically; ordinary call sites should use the `[m]` / `[m, src]`
 * shorthands.
*/