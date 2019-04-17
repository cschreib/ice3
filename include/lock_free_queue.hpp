#ifndef LOCK_FREE_QUEUE_HPP
#define LOCK_FREE_QUEUE_HPP

#include <atomic>

// Note : implementation is from
// http://www.drdobbs.com/parallel/writing-lock-free-code-a-corrected-queue/210604448

template<typename T>
class lock_free_queue
{
    struct node
    {
        node() : next(nullptr) {}
        node(const T& t) : data(t), next(nullptr) {}

        T     data;
        node* next;
    };

    node* first;
    std::atomic<node*> dummy, last;

public :

    lock_free_queue() {
        // Always keep a dummy separator between head and tail
        first = last = dummy = new node();
    }

    ~lock_free_queue() {
        // Clear the whole queue
        while (first != nullptr)
        {
            node* temp = first;
            first = temp->next;
            delete temp;
        }
    }

    // Disable copy
    lock_free_queue(const lock_free_queue& q) = delete;
    lock_free_queue& operator = (const lock_free_queue& q) = delete;

    // To be called by the 'producer' thread
    void push(const T& t) {
        // Add the new item to the queue
        (*last).next = new node(t);
        last = (*last).next;

        // Clear consumed items
        while (first != dummy)
        {
            node* temp = first;
            first = temp->next;
            delete temp;
        }
    }

    // To be called by the 'consumer' thread
    bool pop(T& t) {
        // Return false if queue is empty
        if (dummy != last)
        {
            // Consume the value
            t = (*dummy).next->data;
            dummy = (*dummy).next;
            return true;
        }
        else
            return false;
    }

    // To be called by the 'consumer' thread
    bool empty() const {
        return dummy == last;
    }

    // This method should not be used in concurrent situations
    void clear() {
        while (first != dummy)
        {
            node* temp = first;
            first = temp->next;
            delete temp;
        }

        first = dummy.load();

        while (first->next != nullptr)
        {
            node* temp = first;
            first = temp->next;
            delete temp;
        }

        last = dummy = first;
    }
};

#endif
