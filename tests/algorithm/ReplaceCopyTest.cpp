#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>
#include <array>
#include <deque>
#include <list>
#include <iostream>
#include <raze/algorithm/replace/ReplaceCopy.h> 

template <typename Container, typename T>
void fill_sequential(Container& c, T start = T(1)) {
    size_t i = 0;
    for (auto& x : c) {
        x = static_cast<T>(start + T(i++));
    }
}


template <typename Container, typename T>
void test_replace_copy_container(Container& c, const T& old_value, const T& new_value) {
    Container c_simd = c;
    Container c_std = c;

    raze::algorithm::replace_copy(c.begin(), c.end(), c_simd.begin(), old_value, new_value);

    std::replace_copy(c.begin(), c.end(), c_std.begin(), old_value, new_value);

    raze_assert(std::equal(c_simd.begin(), c_simd.end(), c_std.begin()));
}

template <typename T>
void test_replace_copy_large() {
    const size_t total_bytes = 3000;
    const size_t N = std::max<size_t>(1, total_bytes / sizeof(T));

    {
        std::vector<T> v(N);
        fill_sequential<std::vector<T>, T>(v);
        test_replace_copy_container<std::vector<T>, T>(v, T(10), T(120));
    }

    {
        std::array<T, 1024> arr{};
        fill_sequential<std::array<T, 1024>, T>(arr);
        test_replace_copy_container<std::array<T, 1024>, T>(arr, T(20), T(110));
    }

    {
        std::deque<T> dq(N);
        fill_sequential<std::deque<T>, T>(dq);
        test_replace_copy_container<std::deque<T>, T>(dq, T(30), T(120));
    }

    {
        std::list<T> lst;
        for (size_t i = 0; i < N; ++i) lst.push_back(static_cast<T>(i + 1));
        test_replace_copy_container<std::list<T>, T>(lst, T(40), T(118));
    }
}

int main() {
    test_replace_copy_large<int8_t>();
    test_replace_copy_large<int16_t>();
    test_replace_copy_large<int32_t>();
    test_replace_copy_large<int64_t>();
    test_replace_copy_large<float>();
    test_replace_copy_large<double>();

    return 0;
}
