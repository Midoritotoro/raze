#include <raze/algorithm/find/Contains.h>

#include <vector>
#include <array>

using raze::algorithm::contains;

void containsTest() {
    {
        std::vector<int> v;
        raze_assert(!contains(v.begin(), v.end(), 42));
    }

    {
        std::vector<int> v = { 1, 2, 3, 4, 5 };
        raze_assert(contains(v.begin(), v.end(), 3));
        raze_assert(contains(v.begin(), v.end(), 1));
        raze_assert(contains(v.begin(), v.end(), 5));
    }

    {
        std::vector<int> v = { 10, 20, 30 };
        raze_assert(!contains(v.begin(), v.end(), 25));
        raze_assert(!contains(v.begin(), v.end(), 0));
    }

    {
        int arr[] = { 7, 8, 9 };
        raze_assert(contains(std::begin(arr), std::end(arr), 8));
        raze_assert(!contains(std::begin(arr), std::end(arr), 42));
    }

    {
        const std::array<char, 4> arr = { 'a', 'b', 'c', 'd' };
        raze_assert(contains(arr.begin(), arr.end(), 'c'));
        raze_assert(!contains(arr.begin(), arr.end(), 'z'));
    }

    {
        std::array<double, 3> arr = { 1.0, 2.5, 3.14 };
        raze_assert(contains(arr.begin(), arr.end(), 3.14));
        raze_assert(!contains(arr.begin(), arr.end(), 2.0));
    }

    {
        int arr[] = { 100, 200, 300 };
        raze_assert(contains(&arr[0], &arr[3], 200));
        raze_assert(!contains(&arr[0], &arr[3], 400));
    }

    {
        int arr[] = { 100, 200, 300, 100, 200, 300, 100, 200, 300, 100, 200, 300, 100, 200, 300, 100, 200, 300, 100, 200, 300, 400 };
        raze_assert(contains(&arr[0], &arr[22], 400));
        raze_assert(!contains(&arr[0], &arr[22], 500));
    }
}


int main() {
    containsTest();
    return 0;
}
