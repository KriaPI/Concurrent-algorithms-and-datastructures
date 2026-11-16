#pragma once

#include "monitoring.hpp"
#include "set.hpp"
#include "std_set.hpp"

#include <climits>
#include <mutex>

/// The node used for the linked list implementation of a set in the [`FineSet`]
/// class. This struct is used for task 4.
struct FineSetNode {
    // A04: You can add or remove fields as needed.
    int value;
    FineSetNode* next;
    std::mutex mutex;
};


/// A set implementation using a linked list with fine grained locking.
class FineSet: public Set {
private:
    // A04: You can add or remove fields as needed. Just having the `head`
    // pointer should be sufficient for task 4
    FineSetNode* head;
    FineSetNode* tail;
    EventMonitor<FineSet, StdSet, SetOperator>* monitor;

    /// Find the node whose value is larger than element and the node before it.
    /// @return A pair containing the nodes (previous, current), where current is the 
    /// found node and previous is the node preceding it. 
    std::pair<FineSetNode*, FineSetNode*> search(int element) {
        head->mutex.lock();
        auto previous {head};
        auto current {previous->next};
        current->mutex.lock();

        while (current->value < element) {
            previous->mutex.unlock();
            previous = current;
            current = current->next;
            current->mutex.lock();
        }
        return std::pair(previous, current);
    }

public:
    FineSet(EventMonitor<FineSet, StdSet, SetOperator>* monitor) :
        monitor(monitor)
    {
        // A04: Initiate the internal state
        tail = new FineSetNode(INT_MAX, nullptr);        
        head = new FineSetNode(INT_MIN, tail);        
    }

    ~FineSet() override {
        // A04: Cleanup any memory that was allocated
        auto previous {head};
        auto current {head};
        while (current != nullptr) {
            previous = current;
            current = current->next;
            delete previous;
        }
    }

    bool add(int elem) override {
        bool result = false;
        // A04: Add code to insert the element into the set and update `result`.
        //      Also make sure, to insert the event inside the locked region of
        //      the linearization point.
        auto [beforeFoundNode, foundNode] {search(elem)};

        if (foundNode->value != elem) {
            result = true;
            this->monitor->add(SetEvent(SetOperator::Add, elem, result));

            auto newNode {new FineSetNode(elem, foundNode)};
            beforeFoundNode->next = newNode;    
        }

        foundNode->mutex.unlock();
        beforeFoundNode->mutex.unlock();
        return result;
    }

    bool rmv(int elem) override {
        bool result = false;
        // A04: Add code to remove the element from the set and update `result`.
        //      Also make sure, to insert the event inside the locked region of
        //      the linearization point.
        auto [beforeFoundNode, foundNode] {search(elem)};

        if (foundNode->value == elem) {
            result = true;
            this->monitor->add(SetEvent(SetOperator::Remove, elem, result));

            auto afterFoundNode {foundNode->next};
            beforeFoundNode->next = afterFoundNode;
            
            // Note that we must unlock it before we delete it because it is otherwise undefined
            // behaviour.
            foundNode->mutex.unlock();
            delete foundNode;
        } else {
            foundNode->mutex.unlock();
        }

        beforeFoundNode->mutex.unlock();
        return result;
    }

    bool ctn(int elem) override {
        bool result = false;
        // A04: Add code to check if the element is inside the set and update `result`.
        //      Also make sure, to insert the event inside the locked region of
        //      the linearization point.

        auto [beforeFoundNode, foundNode] {search(elem)};

        if (foundNode->value == elem) {
            result = true;
            this->monitor->add(SetEvent(SetOperator::Contains, elem, result));
        }

        foundNode->mutex.unlock();
        beforeFoundNode->mutex.unlock();
        return result;
    }

    void print_state() override {
        // A04: Optionally, add code to print the state. This is useful for debugging,
        // but not part of the assignment
        std::cout << "FineSet {...}";
    }
};

