#include <iostream>
#include <mutex>
#include <thread>

template <class T>
class IQueue {
public:
    virtual void push(T value) = 0;
    virtual bool pop(T& value) = 0;
};
    // void push(T value) {
    // bool pop(T& value) {
    
    // void push(T value) {
    // bool pop(T& value) {
template<class T>
class Queue {
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
    }

    virtual T pop_unsafe() {
        if (head == nullptr) {
            return 0;
        } else {
            T value = head->value;

            head = head->next_node;
            return value;
        }
    }
    Node* head;
public:
    Queue() {
        head = nullptr;
    }
};

template<class T>
class SafeThreadQueue : Queue<T>, public IQueue<T> {
    std::mutex _m;
public:
    void push(T value) {
        std::lock_guard<std::mutex> _g(_m);
        this->push_unsafe(value);
    }
    bool pop(T& value) {
        std::lock_guard<std::mutex> _g(_m);
        if (this->head!=nullptr) 
        {
            value = this->pop_unsafe();
            return true;
        } else {
            _m.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            _m.lock();

            if (this->head!=nullptr) 
            {
                value = this->pop_unsafe();
                return true;
            } else {
        
                return false;
            }
        }
    }
};