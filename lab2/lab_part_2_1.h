#include <iostream>
#include <mutex>
#include <thread>

template <class T>
class IQueue {
public:
    virtual void push_unsafe(T value) = 0;
    virtual T pop_unsafe() = 0;
};

template<class T>
class Queue : public IQueue<T>{
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
class SafeThreadQueue : Queue<T> {
    std::mutex _m;
public:
    void push(T value) {
        _m.lock();
        this->push_unsafe(value);
        _m.unlock();
    }
    bool pop(T& value) {
        _m.lock();
        if (this->head!=nullptr) 
        {
            value = this->pop_unsafe();
            _m.unlock();
            return true;
        } else {
            _m.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            _m.lock();

            if (this->head!=nullptr) 
            {
                value = this->pop_unsafe();
                _m.unlock();
                return true;
            } else {
                _m.unlock();
                return false;
            }
        }
    }
};