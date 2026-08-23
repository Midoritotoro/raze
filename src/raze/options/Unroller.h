#pragma once

#include <src/raze/options/Traits.h>
#include <src/raze/options/SameAs.h>
#include <src/raze/vx/hw/x86/memory/ConditionalMemAccessNative.h>
#include <src/raze/options/As.h>
#include <src/raze/vx/hw/configurable/mask/FirstN.h>
#include <src/raze/algorithm/TailMask.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class Traits>
struct unroller_t {
	template <class Tag>
	struct impl {
		constexpr impl() noexcept = default;
		constexpr ~impl() noexcept = default;

		static raze_always_inline auto make_mask_generator(i32 n) noexcept {
			return [tail = n] () raze_always_inline_lambda { return raze::vx::first_n(tail, as(typename Tag::mask_type{})); };
		}

		template <class F>
		constexpr raze_always_inline auto operator()(sizetype aligned_size, sizetype tail_size, F f) const noexcept
			requires(!std::same_as<Tag, vx::scalar_tag>) 
		{
			constexpr auto has_early_exit = std::same_as<decltype(f(Tag{}, aligned_size)), bool>;
			constexpr auto unrolling = get_unrolling<Traits>();
			const auto guard = vx::make_guard<Tag>();

#if defined(raze_cpp_msvc_only)
			// Ignore unrolling
			if constexpr (has_early_exit) {
				if (!f(Tag{}, aligned_size)) {
					if constexpr (requires { f.result(); }) return f.result();
					else return;
				}
			}
			else {
				f(Tag{}, aligned_size);
			}
#else
			if constexpr (unrolling > 1) {
				 auto unrolled_size = aligned_size - (aligned_size % (sizeof(Tag) * unrolling));

				 if (unrolled_size != 0) {
					 if constexpr (has_early_exit) {
						 if (!f(vx::simd<typename Tag::value_type, vx::resize_abi_t<vx::abi_t<Tag>, Tag::size()
							 * unrolling>>{}, unrolled_size))
						 {
							 if constexpr (requires { f.result(); }) return f.result();
							 else return;
						 }
					 }
					 else {
						 f(vx::simd<typename Tag::value_type, vx::resize_abi_t<vx::abi_t<Tag>, Tag::size()
							 * unrolling>>{}, unrolled_size);
					 }
				 }

				aligned_size -= unrolled_size;

				if (aligned_size >= sizeof(Tag)) {
					if constexpr (has_early_exit) {
						if (!f(Tag{}, aligned_size)) {
							if constexpr (requires { f.result(); }) return f.result();
							else return;
						}
					}
					else {
						f(Tag{}, aligned_size);
					}
				}
			}
			else {
				if constexpr (has_early_exit) {
					if (!f(Tag{}, aligned_size)) {
						if constexpr (requires { f.result(); }) return f.result();
						else return;
					}
				}
				else {
					f(Tag{}, aligned_size);
				}
			}
#endif // defined(raze_cpp_msvc)

			constexpr auto shift = std::countr_zero(sizeof(typename Tag::value_type));
			constexpr auto can_process_tail = requires { f(Tag{}, algorithm::tail_mask(tail_size,
				make_mask_generator(tail_size >> shift))); };

			if constexpr (vx::native_conditional_memory_access<vx::abi_t<Tag>::isa, typename Tag::value_type> && can_process_tail) {
				f(Tag{}, algorithm::tail_mask(tail_size, make_mask_generator(tail_size >> shift)));
			}
			else {
				f();
			}

			if constexpr (requires { f.result(); }) return f.result();
		}

		template <class F>
		constexpr raze_always_inline auto operator()(F f) const noexcept 
			requires(std::same_as<Tag, vx::scalar_tag>)
		{
			f();
			if constexpr (requires { f.result(); }) return f.result();
		}
	};
};

template <class Traits, class V>
static inline constexpr auto unroller_t = typename unroller_t<Traits>::template impl<V>{};

__RAZE_OPTIONS_NAMESPACE_END
