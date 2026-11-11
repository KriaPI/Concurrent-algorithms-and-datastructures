#pragma once

#include "monitoring.hpp"
#include "set.hpp"
#include "std_set.hpp"
#include <climits>

/// The node used for the linked list implementation of a set in the [`SimpleSet`]
/// class. This struct is used for task 2
struct SimpleSetNode {
    // A02: You can add or remove fields as needed.
    int value;
    SimpleSetNode* next;
};

/// A simple set implementation using a linked list. This class shouldn't have
// any synchronization yet.
class SimpleSet: public Set {
private:
    // A02: You can add or remove fields as needed. Just having the `head`
    // pointer should be sufficient for task 2
    SimpleSetNode* head;
    EventMonitor<SimpleSet, StdSet, SetOperator>* monitor;

    /// Get the node before the node with the value "element", a value larger than "element",
    /// or the end of the list. The pointer returned will always be non-null. 
    SimpleSetNode* search(int element) {
        auto previous {head};
        auto current {head};
        while (current != nullptr && current->value < element) {
            previous = current;
            current = current->next;
        }
        return previous;
    }

public:
    SimpleSet(EventMonitor<SimpleSet, StdSet, SetOperator>* monitor) :
        monitor(monitor)
    {
        // A02: Initiate the internal state
        head = new SimpleSetNode(INT_MIN, nullptr);
    }

    ~SimpleSet() override {
        // A02: Cleanup any memory that was allocated
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
        // A02: Add code to insert the element into the set and update `result`.
        auto beforeInsertionPoint {search(elem)};
        if (beforeInsertionPoint->next == nullptr || beforeInsertionPoint->next->value != elem) {
            auto newNode {new SimpleSetNode(elem, beforeInsertionPoint->next)};
            beforeInsertionPoint->next = newNode;
            result = true;
        }

        this->monitor->add(SetEvent(SetOperator::Add, elem, result));
        return result;
    }

    bool rmv(int elem) override {
        bool result = false;
        // A02: Add code to remove the element from the set and update `result`.
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
        // A02: Add code to check if the element is inside the set and update `result`.
        auto expectedElement {search(elem)->next};
        if (expectedElement != nullptr && expectedElement->value == elem) {
            result = true;
        }

        this->monitor->add(SetEvent(SetOperator::Contains, elem, result));
        return result;
    }

    void print_state() override {
        // A02: Optionally, add code to print the state. This is useful for debugging,
        // but not part of the assignment.
        std::cout << "SimpleSet {...}";
    }
};

