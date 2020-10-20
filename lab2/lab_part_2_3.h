#include <atomic>
#include <deque>

template <class T>
class IQueue {
public:
    virtual void push(T value) = 0;
    virtual bool pop(T& value) = 0;
};

template<class T>
class QueueSizeAtomic : public IQueue<T>{
protected:
    struct Node {
        T value;
        Node* next_node;
    };
    
    int size = 0;
    int length = 0;

    std::mutex _m;
    std::condition_variable _cv;
public:
    std::atomic<Node*> head;
    virtual void push(T value) {

    }

    virtual bool pop(T& value) {
    return 0;        
    }

    QueueSizeAtomic(int size) {
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

template <typename T>
class Queue
{
private:
    struct node
    {
        node* next;
        std::shared_ptr<T> data;
        node(const T& d, node* n = 0)
            :next(n), data(std::make_shared<T>(d)) {}
    };
    std::atomic<node*> head;
public:
    void push(const T& data)
    {
        std::cout << "pushing\n";
        node* new_node = new node(data, head.load());
        node* old_head=head.load();
        while (!head.compare_exchange_weak( old_head, new_node ));
    }
    bool pop(T& value)
    {
        std::cout << "popping\n";
        node* old_head=head.load();
        while (old_head && !head.compare_exchange_weak( old_head, old_head->next));
        value = old_head ? *(old_head->data) : 0;
        return true;
    }
};

template <typename T>
class Queue2
{
public:
    struct Node { T value; Node* next = nullptr; };
    std::atomic<Node*> head;
    Queue2() {
        head.store(nullptr);
    }

    // добавляю в начало, выбрасываю с конца
    void push(const T data) {
        Node* oldHead = head;
        Node* newNode = new Node {data, oldHead};

        // what follows is equivalent to: list_head = newNode, but in a thread-safe way:
        while (!head.compare_exchange_weak(oldHead,newNode)) newNode->next = oldHead;
    }
    
    bool pop(T& value)
    {
        std::atomic<Node*> currentNode = head.load();
        while(currentNode.load()->next){
            Node* oldHead = head;
            Node* newNode = new Node {value, oldHead};
            while (!head.compare_exchange_weak(oldHead,newNode)) newNode = oldHead;
        }
        value = currentNode.load()->value;
        delete currentNode;
        // {
        //     currentNode.load()->next = currentNode;
        // }
        
        return true;
    }
};