#include <raze/concurrency/Thread.h>
#include <cassert>
#include <atomic>
#include <chrono>
#include <iostream>

void simple_task(std::atomic<bool>& flag) {
    flag.store(true);
}

struct Worker {
    void work(std::atomic<int>& counter, int value) {
        counter.fetch_add(value);
    }
};

int main() {
    {
        raze::concurrency::thread t;
        std::atomic<bool> flag{ false };
        t.start(simple_task, std::ref(flag));
        t.join();
        assert(flag.load() == true);
    }
    
    {
        raze::concurrency::thread t;
        Worker w;
        std::atomic<int> counter{ 0 };
        t.start(&Worker::work, &w, std::ref(counter), 5);
        t.join();
        assert(counter.load() == 5);
    }

    {
        raze::concurrency::thread t;
        std::atomic<bool> flag{ false };
        t.start(simple_task, std::ref(flag));
        assert(t.joinable());
        t.join();
        assert(!t.joinable());
    }

    {
        raze::concurrency::thread t;
        std::atomic<bool> flag{ false };
        t.start(simple_task, std::ref(flag));
        t.detach();

        raze::concurrency::this_thread::sleep_for(std::chrono::milliseconds(50));
        assert(flag.load() == true);
    }

    {
        std::atomic<bool> flag1{ false }, flag2{ false };
        raze::concurrency::thread t1, t2;

        t1.start(simple_task, std::ref(flag1));
        t2.start(simple_task, std::ref(flag2));

        t1.swap(t2);

        t1.join();
        t2.join();

        assert(flag1.load() == true);
        assert(flag2.load() == true);
    }

    {
        auto hc = raze::concurrency::thread::hardwareConcurrency();
        assert(hc > 0);
        std::cout << "hardwareConcurrency = " << hc << "\n";
    }

    {
        raze::concurrency::thread t;
        std::atomic<bool> ok{ false };
        t.start([&]() {
            if (t.isCurrentThread()) {
                ok.store(true);
            }
            });
        t.join();
        assert(ok.load() == true);
    }

    return 0;
}