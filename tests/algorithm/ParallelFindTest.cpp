#include <raze/concurrency/algorithm/find/ParallelFind.h>

void parallelFindTest() {
    std::vector<int> v{ 1, 2, 3, 4, 5 };

    {
        auto it = raze::algorithm::find(raze::concurrency::execution::sequenced,
            v.begin(), v.end(), 3);
        raze_assert(it != v.end());
        raze_assert(*it == 3);
    }
    {
        auto it = raze::algorithm::find(raze::concurrency::execution::sequenced,
            v.begin(), v.end(), 99);
        raze_assert(it == v.end());
    }

    {
        auto it = raze::algorithm::find(raze::concurrency::execution::parallel,
            v.begin(), v.end(), 1);
        raze_assert(it == v.begin());
        raze_assert(*it == 1);
    }
    //{
    //    auto it = raze::algorithm::find(raze::concurrency::execution::parallel,
    //        v.begin(), v.end(), 5);
    //    raze_assert(it != v.end());
    //    raze_assert(*it == 5);
    //}

    //{
    //    auto it = raze::algorithm::find(raze::concurrency::execution::parallel_unsequenced,
    //        v.begin(), v.end(), 2);
    //    raze_assert(it != v.end());
    //    raze_assert(*it == 2);
    //}
    //{
    //    auto it = raze::algorithm::find(raze::concurrency::execution::parallel_unsequenced,
    //        v.begin(), v.end(), 42);
    //    raze_assert(it == v.end());
    //}

    //{
    //    auto it = raze::algorithm::find(raze::concurrency::execution::unsequenced,
    //        v.begin(), v.end(), 4);
    //    raze_assert(it != v.end());
    //    raze_assert(*it == 4);
    //}
    //{
    //    auto it = raze::algorithm::find(raze::concurrency::execution::unsequenced,
    //        v.begin(), v.end(), -1);
    //    raze_assert(it == v.end());
    //}

    //{
    //    std::vector<int> empty;
    //    auto it = raze::algorithm::find(raze::concurrency::execution::sequenced,
    //        empty.begin(), empty.end(), 1);
    //    raze_assert(it == empty.end());
    //}
}

int main() {
	parallelFindTest();
	return 0;
}