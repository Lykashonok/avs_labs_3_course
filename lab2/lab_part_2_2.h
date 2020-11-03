#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "lab_part_2_1.h"

template<class T>
class QueueSize {
protected:
    class Node {
    public:
        T value;
        Node* next_node;
        Node(T value) {
            this->value = value;
            next_node = nullptr;
        }
    };

    virtual void push_unsafe(T value) {
        if(isFull()) return;
        Node *new_node = new Node(value);
        Node *last = head;
        if (last == nullptr) {
            head = new_node;
        }
        else {
            while (last->next_node != nullptr) 
                last = last->next_node;
            last->next_node = new_node;
        }
        this->length+=1;
    }

    virtual T pop_unsafe() {
        if (head == nullptr) {
            return 0;
        } else {
            T value = head->value;
            head = head->next_node;
            this->length-=1;
            return value;
        }
    }
    Node* head;
    
    int size = 0;
    int length = 0;
public:
    QueueSize(int size) {
        head = nullptr;
        this->size = size;
    }

    bool isFull() {
        return this->length == this->size;
    }

    bool isEmpty() {
        return this->length == 0;
    }
};

template<class T>
class SafeThreadSizeQueueMutex : QueueSize<T>, public IQueue<T> {
    std::mutex _m;
    std::condition_variable _cv;
public:
    SafeThreadSizeQueueMutex(int size, bool debug = false) : QueueSize<T>(size) {}

    void push(T value) {
        std::unique_lock<std::mutex> _g(_m);
        
        if (this->isFull()) {

            _cv.wait(_g, [this] () {return !this->isFull();});

            this->push_unsafe(value);
        } else {
            this->push_unsafe(value);
        }
    }

    bool pop(T& value) {
        std::unique_lock<std::mutex> _g(_m);
        
        if (this->isEmpty()) 
        {
            _g.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            _g.lock();

            if (this->isEmpty()) 
            {
                _g.unlock();
                _cv.notify_one();
                _g.lock();
                return false;
            } else {
                
                value = this->pop_unsafe();
                _g.unlock();
                _cv.notify_one();
                _g.lock();
                return true;
            }
        } else {
            value = this->pop_unsafe();

            _g.unlock();
            _cv.notify_one();
            _g.lock();
            return true;   
        }
    }
};