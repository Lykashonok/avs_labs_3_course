#include <iostream>
#include <chrono>
#include <vector>
#include <atomic>
#include <time.h>
#include <condition_variable>
#include <thread>

// g++ -std=c++17 -pthread -o thread FILENAME.cpp;./thread
// Queue, Producer, Consumer, SafeThreadQueue
#include "lab_part_2_3.h"

std::vector<uint64_t> consumedNumbers;
std::vector<uint64_t> threadTimes;
bool ready = false;
std::condition_variable _cv;
std::mutex _m;
bool g = false;

void testConfiguration(uint64_t TaskNum, uint16_t ConsumerNum, uint16_t ProducerNum, uint16_t size) {
    uint64_t total = 0;
    std::cout << "---------------------------\n";
    std::cout << "ConsumerNum = "<< ConsumerNum << ", ProducerNum = "<< ProducerNum << "\n";
    std::cout << "Queue size = " << size << '\n';
    std::cout << "Total threads = " << ProducerNum + ConsumerNum << '\n';
    std::cout << "TaskNum for every producer = " << TaskNum << ", total must be = "<< TaskNum*ProducerNum << "\n";
    QueueSizeAtomic<uint8_t>* q = new QueueSizeAtomic<uint8_t>(size);


    Queue2<uint16_t> queue;
    uint16_t w = 0;
    queue.push(2);
    queue.push(4);
    queue.push(5);
    queue.pop(w);
    std::cout << w << '\n';
    queue.pop(w);
    std::cout << w << '\n';
    
    return;


    std::vector<std::thread> threadArray(ProducerNum+ConsumerNum);
    for (int i = 0; i < TaskNum; i++) consumedNumbers.push_back(0);
    for (int i = 0; i < ProducerNum + ConsumerNum; i++) threadTimes.push_back(0);

    //  Producers threads
    for (uint8_t i = 0; i < ProducerNum; i++)
    {
        threadArray[i] = std::thread([&q] (uint64_t TaskNum, uint8_t threadIndex) {
            auto t1 = std::chrono::high_resolution_clock::now();

            for (size_t j = 0; j < TaskNum; j++)
            {
                q->push(1);
                std::cout << "pushed 1\n";
            }

            auto t2 = std::chrono::high_resolution_clock::now();
            uint64_t total = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
            threadTimes[threadIndex] = total;
        }, TaskNum, i);
    }

    //  Consumers threads
    for (uint8_t i = ProducerNum; i < ProducerNum + ConsumerNum; i++)
    {
        threadArray[i] = std::thread([&q] (uint8_t index, uint8_t threadIndex) {
            auto t1 = std::chrono::high_resolution_clock::now();

            uint8_t value = 0;
            while (true)
            {
                if (q->pop(value))
                {
                    std::cout << "popped 1\n";
                    consumedNumbers[index]+=value;
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));

                    if(q->pop(value)) {
                        std::cout << "popped 2\n";
                        consumedNumbers[index]+=value;
                    } else {
                        break;
                    }
                }
            }

            auto t2 = std::chrono::high_resolution_clock::now();
            uint64_t total = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
            threadTimes[threadIndex] = total;
        }, i - ProducerNum, i);
    }

    for(auto &t : threadArray) if (t.joinable()) t.join();

    for (uint64_t i = 0; i < ProducerNum + ConsumerNum; i++)
    {
        if (i < ProducerNum)
        {
            std::cout << "Producer " << i + 1 << " took " << uint64_t(threadTimes[i]) << " microseconds\n";   
        } else {
            std::cout << "Consumer " << i - ProducerNum + 1 << " took " << uint64_t(threadTimes[i]) << " microseconds, consumed " << uint64_t(consumedNumbers[i-ProducerNum]) << '\n';
            total+=consumedNumbers[i- ProducerNum];
        }
    }

    for (size_t i = 0; i < ConsumerNum; i++) consumedNumbers[i] = 0;
    
    std::cout << "Total consumed: " << total << '\n';
}

int main()
{
    uint64_t numThreads = std::thread::hardware_concurrency();
    std::cout << "Your cpu has " << numThreads << " physical cores" << std::endl;
    testConfiguration(10, 1, 1, 1);
    // testConfiguration(1024*10, 2, 2, 4);
    // testConfiguration(1024*10, 4, 4, 16);
    return 0;
}