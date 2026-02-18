#include <cassert>
#include <vector>
#include <string>
#include <list>
#include <deque>
#include <array>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <raze/algorithm/copy/Move.h>


template <typename It1, typename It2>
void assert_equal(It1 first1, It1 last1, It2 first2) {
    auto n = std::distance(first1, last1);
    for (decltype(n) i = 0; i < n; ++i) {
        raze_assert(*(first1 + i) == *(first2 + i));
    }
}

// ������������� ��� ��� �������� �����������
struct MoveCounted {
    int value;
    static inline size_t moves = 0;
    MoveCounted() : value(0) {}
    explicit MoveCounted(int v) : value(v) {}
    MoveCounted(const MoveCounted&) = delete;
    MoveCounted& operator=(const MoveCounted&) = delete;
    MoveCounted(MoveCounted&& other) noexcept : value(other.value) {
        ++moves;
        other.value = -999;
    }
    MoveCounted& operator=(MoveCounted&& other) noexcept {
        value = other.value;
        ++moves;
        other.value = -999;
        return *this;
    }
    bool operator==(const MoveCounted& rhs) const noexcept { return value == rhs.value; }
};

int main() {
    // ������ ��������
    {
        std::vector<int> src;
        std::vector<int> dst;
        auto out = raze::algorithm::move(src.begin(), src.end(), dst.begin());
        raze_assert(out == dst.begin());
    }

    // ���� �������
    {
        std::vector<int> src = { 42 };
        std::vector<int> dst(1, 0);
        auto out = raze::algorithm::move(src.begin(), src.end(), dst.begin());
        raze_assert(out == dst.end());
        raze_assert(dst[0] == 42);
    }

    // ��������� ���������
    {
        std::vector<int> src = { 1,2,3,4,5 };
        std::vector<int> dst(5, 0);
        auto out = raze::algorithm::move(src.begin(), src.end(), dst.begin());
        raze_assert(out == dst.end());
        assert_equal(dst.begin(), dst.end(), std::vector<int>{1, 2, 3, 4, 5}.begin());
    }

    // ������
    {
        std::string src = "hello";
        std::string dst(5, '_');
        auto out = raze::algorithm::move(src.begin(), src.end(), dst.begin());
        raze_assert(out == dst.end());
        raze_assert(dst == "hello");
    }
    // ����������: ����� ������
    {
        std::vector<int> v = { 1,2,3,4,5,6,7,8 };
        raze::algorithm::move(v.begin(), v.begin() + 4, v.begin() + 2);
        std::vector<int> expected = { 1,2,1,2,3,4,7,8 };
        raze_assert(v == expected);
    }

    // ����������: ����� �����
    {
        std::vector<int> v = { 10,20,30,40,50 };
        raze::algorithm::move(v.begin() + 2, v.end(), v.begin());
        std::vector<int> expected = { 30,40,50,40,50 };
        raze_assert(v == expected);
    }
    {
        const size_t N = 1'000'000'00;
        std::vector<int> src(N);
        for (size_t i = 0; i < N; ++i) src[i] = static_cast<int>(i);

        std::vector<int> dst(N, -1);

        auto start = std::chrono::high_resolution_clock::now();
        auto out = raze::algorithm::move(src.begin(), src.end(), dst.begin());
        auto stop = std::chrono::high_resolution_clock::now();

        raze_assert(out == dst.end());
        for (size_t i = 0; i < N; i += N / 10) raze_assert(dst[i] == static_cast<int>(i));

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
        std::cout << "Large move of " << N << " ints took " << ms << " ms\n";
    }
    {
        MoveCounted::moves = 0;
        std::vector<MoveCounted> src;
        for (int i = 0; i < 100; ++i) src.emplace_back(i);
        std::vector<MoveCounted> dst(src.size());

        auto out = raze::algorithm::move(src.begin(), src.end(), dst.begin());
        raze_assert(out == dst.end());
        for (int i = 0; i < 100; ++i) raze_assert(dst[i].value == i);
        raze_assert(MoveCounted::moves >= 100); // ����������� ������ ����
    }
    // deque
    {
        std::deque<int> src = { 1,2,3,4,5 };
        std::deque<int> dst(5);
        auto out = raze::algorithm::move(src.begin(), src.end(), dst.begin());
        raze_assert(out == dst.end());
        for (size_t i = 0; i < src.size(); ++i) raze_assert(dst[i] == i + 1);
    }

    // list (��������������� ���������)
    {
        std::list<int> src = { 7,8,9 };
        std::list<int> dst(3);
        auto out = raze::algorithm::move(src.begin(), src.end(), dst.begin());
        auto itS = src.begin();
        auto itD = dst.begin();
        for (; itS != src.end(); ++itS, ++itD) raze_assert(*itS == *itD);
    }

    return 0;
}
