#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <time.h>

// in console: g++ -std=c++11 -pthread -o thread lab.cpp;./thread
void showExecutionTime(std::string, uint64_t, uint64_t ,uint64_t, bool);
void testConfiguration(uint64_t, uint64_t, bool);
void checkNumArray(uint64_t);
std::vector<uint64_t> numArray;
std::mutex _m;

// MUTEX
void incrementMutex(uint64_t &index, uint64_t NumsPerThread, uint64_t numTasks, bool sleep = false) {
    int currentIndex;
    for (uint64_t i = 0; i < NumsPerThread; i++)
    {
        if (index >= numTasks) break;
        _m.lock();
        currentIndex = index++;
        _m.unlock();
        numArray.at(currentIndex)++;
        
        if (sleep) std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
}
void threadTaskMutex(uint64_t NumsPerThread, uint64_t NumThreads, uint64_t numTasks, bool sleep) {
    uint64_t index = 0;
    std::vector<std::thread> threadArray(NumThreads);
    for (uint8_t j = 0; j < NumThreads; j++)
    {
        threadArray[j] = std::thread(incrementMutex, std::ref(index), NumsPerThread, numTasks, sleep);
    }
    for (auto &t : threadArray) if (t.joinable()) t.join();
}
// MUTEX END


// ATOMIC
static std::atomic<int> indexAtomic{ 0 };
void incrementAtomic(uint64_t NumsPerThread, uint64_t numTasks, bool sleep = false) {
    for (uint64_t i = 0; i < NumsPerThread; i++)
    {
        int currentIndex = indexAtomic.fetch_add(1);

        if (currentIndex >= numTasks) break;
        numArray.at(currentIndex)++;
        if (sleep) std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
}
void threadTaskAtomic(uint64_t NumsPerThread, uint64_t NumThreads, uint64_t numTasks, bool sleep = false) {
    std::vector<std::thread> threadArray(NumThreads);
    for (uint64_t j = 0; j < NumThreads; j++)
    {
        threadArray[j] = std::thread(incrementAtomic, NumsPerThread, numTasks, sleep);
    }
    for (auto &t : threadArray) if (t.joinable()) t.join();
}
// ATOMIC END


// DEFAULT
void nonThreadTask(int numTasks, bool sleep) {
    int index = 0;
    while (index < numTasks)
    {
        numArray.at(index++)++;
        if (sleep) std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
}
// DEFAULT END

int main()
{
    std::cout << "Your cpu has " << std::thread::hardware_concurrency() << " physical cores" << std::endl;
    uint64_t numTasks = 1024*100;
    // instantiating
    numArray = std::vector<uint64_t>(numTasks, 0);
    std::cout << "numArray size is " << numArray.size() << '\n';
    std::cout << "numTasks is " << numTasks << '\n';
    
    // testing with different number of threads
    testConfiguration(numTasks, 2, false);
    testConfiguration(numTasks, 4, false);
    testConfiguration(numTasks, 8, false);
    testConfiguration(numTasks, 16, false);
    testConfiguration(numTasks, 32, false);

    // checking if numarray has the same number
    checkNumArray(numTasks);

    return 0;
}

void testConfiguration(uint64_t numTasks, uint64_t NumThreads, bool showNonThread = false) {
    bool sleep;
    uint64_t NumsPerThread = numTasks / NumThreads + 1;
    std::cout << "\n-----------Threads: " << NumThreads << " -----------\n\n";
    std::cout << "............without sleep.........\n";
    sleep = false;
    // show in console time of execution of function
    if (showNonThread) showExecutionTime("default", numTasks, NumsPerThread, NumThreads, sleep);
    showExecutionTime("mutex",   numTasks, NumsPerThread, NumThreads, sleep);
    showExecutionTime("atomic",  numTasks, NumsPerThread, NumThreads, sleep);

    std::cout << "...............with sleep.........\n";
    sleep = true;
    if (showNonThread) showExecutionTime("default", numTasks, NumsPerThread, NumThreads, sleep);
    showExecutionTime("mutex",   numTasks, NumsPerThread, NumThreads, sleep);
    showExecutionTime("atomic",  numTasks, NumsPerThread, NumThreads, sleep);
}

void showExecutionTime(std::string type, uint64_t numTasks, uint64_t NumsPerThread = 0, uint64_t NumThreads = 0, bool sleep = false) {
    auto t1 = std::chrono::high_resolution_clock::now();

    // every thread function has it's own join at the end, no need to do it here
    if (!type.compare("default"))
    {
        nonThreadTask(numTasks, sleep);
        std::cout << "Non   ";
    } else if (!type.compare("mutex")) {
        threadTaskMutex(NumsPerThread, NumThreads, numTasks, sleep);
        std::cout << "Mutex ";
    } else if (!type.compare("atomic")) {
        indexAtomic.fetch_xor(indexAtomic);
        threadTaskAtomic(NumsPerThread, NumThreads, numTasks, sleep);
        std::cout << "Atomic";
    }

    // show time from start of execution function to end
    auto t2 = std::chrono::high_resolution_clock::now();
    uint64_t total = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    std::cout << " Thread: time is " << total << " microseconds\n";
    return;
}

void checkNumArray(uint64_t numTasks) {
    bool checkedNumArray = true;
    for (size_t i = 1; i < numTasks; i++)
    {
        if (numArray[i] != numArray[i-1]) {
            checkedNumArray = false;
            break;
        }
    }
    std::cout<< "checking numArray...\n";
    std::cout<< "checked: ";
    if (checkedNumArray)
        std::cout<< "right\n";
    else
        std::cout<< "wrong\n";
}

// Your cpu has 8 physical cores
// numTasks is 1048576

// -----------Threads: 4 -----------

// ............without sleep.........
// Non    Thread: time is 5290 microseconds
// Mutex  Thread: time is 215005 microseconds
// Atomic Thread: time is 56581 microseconds
// ...............with sleep.........
// Non    Thread: time is 64519071 microseconds
// Mutex  Thread: time is 16729044 microseconds
// Atomic Thread: time is 16862122 microseconds

// -----------Threads: 8 -----------

// ............without sleep.........
// Mutex  Thread: time is 456875 microseconds
// Atomic Thread: time is 47612 microseconds
// ...............with sleep.........
// Mutex  Thread: time is 8183799 microseconds
// Atomic Thread: time is 8133899 microseconds

// -----------Threads: 16 -----------

// ............without sleep.........
// Mutex  Thread: time is 438240 microseconds
// Atomic Thread: time is 45426 microseconds
// ...............with sleep.........
// Mutex  Thread: time is 3906870 microseconds
// Atomic Thread: time is 3817900 microseconds

// -----------Threads: 32 -----------

// ............without sleep.........
// Mutex  Thread: time is 396506 microseconds
// Atomic Thread: time is 44706 microseconds
// ...............with sleep.........
// Mutex  Thread: time is 1660540 microseconds
// Atomic Thread: time is 1880826 microseconds
// checking numArray...
// checked: right