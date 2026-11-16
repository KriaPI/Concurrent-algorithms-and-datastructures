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
    std::mutex lock;
};

// TODO: implement fine-grained locking, look at your solution to IPP assignment 2

/// A set implementation using a linked list with fine grained locking.
class FineSet: public Set {
private:
    // A04: You can add or remove fields as needed. Just having the `head`
    // pointer should be sufficient for task 4
    FineSetNode* head;
    EventMonitor<FineSet, StdSet, SetOperator>* monitor;

    /// Get the node before the node with the value "element", a value larger than "element",
    /// or the end of the list. The pointer returned will always be non-null. 
    FineSetNode* search(int element) {
        auto previous {head};
        auto current {head};
        while (current != nullptr && current->value < element) {
            previous = current;
            current = current->next;
        }
        return previous;
    }
public:
    FineSet(EventMonitor<FineSet, StdSet, SetOperator>* monitor) :
        monitor(monitor)
    {
        // A04: Initiate the internal state
        head = new FineSetNode(INT_MIN, nullptr);        
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
        auto beforeInsertionPoint {search(elem)};
        if (beforeInsertionPoint->next == nullptr || beforeInsertionPoint->next->value != elem) {
            auto newNode {new FineSetNode(elem, beforeInsertionPoint->next)};
            beforeInsertionPoint->next = newNode;
            result = true;
        }

        this->monitor->add(SetEvent(SetOperator::Add, elem, result));
        return result;
    }

    bool rmv(int elem) override {
        bool result = false;
        // A04: Add code to remove the element from the set and update `result`.
        //      Also make sure, to insert the event inside the locked region of
        //      the linearization point.
        auto beforeRemovalPoint {search(elem)};
        auto removalPoint {beforeRemovalPoint->next};
        if (removalPoint != nullptr && removalPoint->value == elem) {
            auto afterRemovalPoint {removalPoint->next};
            beforeRemovalPoint->next = afterRemovalPoint;
            delete removalPoint;
            result = true;
        }

        this->monitor->add(SetEvent(SetOperator::Remove, elem, result));
        return result;
    }

    bool ctn(int elem) override {
        bool result = false;
        // A04: Add code to check if the element is inside the set and update `result`.
        //      Also make sure, to insert the event inside the locked region of
        //      the linearization point.
        auto expectedElement {search(elem)->next};
        if (expectedElement != nullptr && expectedElement->value == elem) {
            result = true;
        }

        this->monitor->add(SetEvent(SetOperator::Contains, elem, result));
        return result;
    }

    void print_state() override {
        // A04: Optionally, add code to print the state. This is useful for debugging,
        // but not part of the assignment
        std::cout << "FineSet {...}";
    }
};

