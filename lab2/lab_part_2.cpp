#include <iostream>
#include <chrono>
#include <vector>
#include <atomic>
#include <time.h>
#include <condition_variable>

// g++ -std=c++17 -pthread -o thread FILENAME.cpp;./thread
// Queue, Producer, Consumer, SafeThreadQueue
#include "lab_part_2_2.h"

std::vector<uint64_t> consumedNumbers;
std::vector<uint64_t> threadTimes;
bool ready = false;
std::condition_variable _cv;
std::mutex _m;
bool g = false;

void testConfiguration(uint64_t TaskNum, uint16_t ConsumerNum, uint16_t ProducerNum, IQueue<uint8_t> *q) {
    uint64_t total = 0;
    std::atomic<int> endedProducers{ 0 };
    std::cout << "---------------------------\n";
    std::cout << "ConsumerNum = "<< ConsumerNum << ", ProducerNum = "<< ProducerNum << "\n";
    std::cout << "Total threads = " << ProducerNum + ConsumerNum << '\n';
    std::cout << "TaskNum for every producer = " << TaskNum << ", total must be = "<< TaskNum*ProducerNum << "\n";
    std::vector<std::thread> threadArray(ProducerNum+ConsumerNum);
    consumedNumbers = std::vector<uint64_t>(TaskNum, 0);
    threadTimes = std::vector<uint64_t>(ProducerNum + ConsumerNum, 0);
    
    for (uint8_t i = 0; i < ProducerNum + ConsumerNum; i++)
    {
        if (i < ProducerNum) {
            // Producerss
            threadArray[i] = std::thread([&q, &endedProducers] (uint64_t TaskNum, uint8_t threadIndex) {
                auto t1 = std::chrono::high_resolution_clock::now();

                for (size_t j = 0; j < TaskNum; j++)
                {
                    q->push(1);
                }

                endedProducers++;

                auto t2 = std::chrono::high_resolution_clock::now();
                uint64_t total = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
                threadTimes[threadIndex] = total;
            }, TaskNum, i);
        } else {
            // Consumers
            threadArray[i] = std::thread([&q, &endedProducers, &ProducerNum] (uint8_t index, uint8_t threadIndex) {
                auto t1 = std::chrono::high_resolution_clock::now();

                uint8_t value = 0;
                while (endedProducers != ProducerNum)
                {
                    if (q->pop(value)) {
                        consumedNumbers[index]+=value;
                    }
                }

                auto t2 = std::chrono::high_resolution_clock::now();
                uint64_t total = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
                threadTimes[threadIndex] = total;
            }, i - ProducerNum, i);
        }
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
    uint64_t numTasks = 1024*1;
    std::cout << "Your cpu has " << numThreads << " physical cores" << std::endl;
    
    testConfiguration(numTasks, 1, 1, new SafeThreadSizeQueueMutex<uint8_t>(1));
    testConfiguration(numTasks, 2, 1, new SafeThreadSizeQueueMutex<uint8_t>(1));
    testConfiguration(numTasks, 2, 2, new SafeThreadSizeQueueMutex<uint8_t>(4));
    testConfiguration(numTasks, 4, 4, new SafeThreadSizeQueueMutex<uint8_t>(16));

    testConfiguration(numTasks, 1, 1, new SafeThreadQueue<uint8_t>());
    testConfiguration(numTasks, 2, 1, new SafeThreadQueue<uint8_t>());
    testConfiguration(numTasks, 2, 2, new SafeThreadQueue<uint8_t>());
    testConfiguration(numTasks, 4, 4, new SafeThreadQueue<uint8_t>());

    SafeThreadQueue<uint8_t>* q2 = new SafeThreadQueue<uint8_t>();
    return 0;
}

// EXAM
// Your cpu has 8 physical cores
// ---------------------------
// ConsumerNum = 1, ProducerNum = 1
// Queue size = 1
// Total threads = 2
// TaskNum for every producer = 104448, total must be = 104448
// Producer 1 took 121002673 microseconds
// Consumer 1 took 121004950 microseconds, consumed 104448
// Total consumed: 104448
// ---------------------------
// ConsumerNum = 2, ProducerNum = 2
// Queue size = 4
// Total threads = 4
// TaskNum for every producer = 104448, total must be = 208896
// Producer 1 took 52501935 microseconds
// Producer 2 took 50087120 microseconds
// Consumer 1 took 44491 microseconds, consumed 184
// Consumer 2 took 52503983 microseconds, consumed 208712
// Total consumed: 208896
// ---------------------------
// ConsumerNum = 4, ProducerNum = 4
// Queue size = 16
// Total threads = 8
// TaskNum for every producer = 104448, total must be = 417792
// Producer 1 took 12523571 microseconds
// Producer 2 took 12728785 microseconds
// Producer 3 took 12603032 microseconds
// Producer 4 took 12876198 microseconds
// Consumer 1 took 12878262 microseconds, consumed 388486
// Consumer 2 took 71561 microseconds, consumed 1314
// Consumer 3 took 86143 microseconds, consumed 2034
// Consumer 4 took 950567 microseconds, consumed 25958
// Total consumed: 417792