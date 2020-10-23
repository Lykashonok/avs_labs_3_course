#include <iostream>
#include <chrono>
#include <vector>
#include <atomic>
#include <time.h>
#include <condition_variable>

// g++ -std=c++11 -pthread -o thread lab.cpp;./thread
// Queue, Producer, Consumer, SafeThreadQueue
#include "lab_part_2_1.h"


std::vector<uint64_t> consumedNumbers;
std::vector<uint64_t> threadTimes;
std::mutex _m;
std::condition_variable cv;
bool ready = false;

void testConfiguration(uint64_t TaskNum, uint16_t ConsumerNum, uint16_t ProducerNum) {
    uint64_t total = 0;
    std::cout << "---------------------------\n";
    std::cout << "ConsumerNum = "<< ConsumerNum << ", ProducerNum = "<< ProducerNum << "\n";
    std::cout << "Total threads = " << ProducerNum + ConsumerNum << '\n';
    std::cout << "TaskNum for every producer = " << TaskNum << ", total must be = "<< TaskNum*ProducerNum << "\n";
    SafeThreadQueue<uint8_t>* q = new SafeThreadQueue<uint8_t>();
    std::vector<std::thread> threadArray(ProducerNum+ConsumerNum);
    consumedNumbers = std::vector<uint64_t>(TaskNum, 0);
    threadTimes = std::vector<uint64_t>(ProducerNum + ConsumerNum, 0);
    

    
    for (uint8_t i = 0; i < ProducerNum + ConsumerNum; i++)
    {
        if (i < ProducerNum) {
            // Producerss
            threadArray[i] = std::thread([&q] (uint64_t TaskNum, uint8_t threadIndex) {
                auto t1 = std::chrono::high_resolution_clock::now();

                for (size_t j = 0; j < TaskNum; j++)
                {
                    q->push(1);
                }

                auto t2 = std::chrono::high_resolution_clock::now();
                uint64_t total = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
                threadTimes[threadIndex] = total;
            }, TaskNum, i);
        } else {
            // Consumers
            threadArray[i] = std::thread([&q] (uint8_t index, uint8_t threadIndex) {
                auto t1 = std::chrono::high_resolution_clock::now();

                uint8_t value = 0;
                while (true)
                {
                    if (q->pop(value)) {
                        consumedNumbers[index]+=value;
                    } else break;
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
    std::cout << "Your cpu has " << numThreads << " physical cores" << std::endl;
    int taskNum = 4*1024*1024;
    testConfiguration(taskNum, 1, 1);
    testConfiguration(taskNum, 2, 2);
    testConfiguration(taskNum, 4, 4);

    return 0;
}

// Your cpu has 8 physical cores
// ---------------------------
// ConsumerNum = 1, ProducerNum = 1
// Total threads = 2
// TaskNum for every producer = 104448, total must be = 104448
// Producer 1 took 12386990 microseconds
// Consumer 1 took 11216706 microseconds, consumed 81682
// Total consumed: 81682
// ---------------------------
// ConsumerNum = 2, ProducerNum = 2
// Total threads = 4
// TaskNum for every producer = 104448, total must be = 208896
// Producer 1 took 214743 microseconds
// Producer 2 took 199930 microseconds
// Consumer 1 took 214860 microseconds, consumed 202988
// Consumer 2 took 11383 microseconds, consumed 5908
// Total consumed: 208896
// ---------------------------
// ConsumerNum = 4, ProducerNum = 4
// Total threads = 8
// TaskNum for every producer = 104448, total must be = 417792
// Producer 1 took 175787098 microseconds
// Producer 2 took 169899624 microseconds
// Producer 3 took 41117417 microseconds
// Producer 4 took 171878962 microseconds
// Consumer 1 took 14658 microseconds, consumed 4157
// Consumer 2 took 169899632 microseconds, consumed 251316
// Consumer 3 took 175789230 microseconds, consumed 161960
// Consumer 4 took 1514 microseconds, consumed 359
// Total consumed: 417792