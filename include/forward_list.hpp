#include <memory_resource>
#include <cstddef>
#include <utility>
#include <new>
#include <iterator>

template <typename T>
class PmrSinglyList {
public:
    using node_alloc = std::pmr::polymorphic_allocator<typename std::remove_cv<T>::type>;
    struct Node {
        T value;
        Node* next = nullptr;
    };

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = std::ptrdiff_t;

        Iterator(Node* n = nullptr) : node_(n) {}
        reference operator*() const { return node_->value; }
        pointer operator->() const { return std::addressof(node_->value); }
        Iterator& operator++() { node_ = node_->next; return *this; }
        Iterator operator++(int) { Iterator t = *this; node_ = node_->next; return t; }
        bool operator==(const Iterator& o) const { return node_ == o.node_; }
        bool operator!=(const Iterator& o) const { return node_ != o.node_; }

    private:
        Node* node_;
    };

    explicit PmrSinglyList(std::pmr::polymorphic_allocator<std::byte> a = {})
        : alloc_(a.resource()) {}

    ~PmrSinglyList() { clear(); }

    PmrSinglyList(const PmrSinglyList&) = delete;
    PmrSinglyList& operator=(const PmrSinglyList&) = delete;

    void push_front(const T& v) {
        Node* n = allocate_node();
        new (&n->value) T(v);
        n->next = head_;
        head_ = n;
        ++size_;
    }

    void push_front(T&& v) {
        Node* n = allocate_node();
        new (&n->value) T(std::move(v));
        n->next = head_;
        head_ = n;
        ++size_;
    }

    void pop_front() {
        if (!head_) return;
        Node* n = head_;
        head_ = head_->next;
        n->value.~T();
        deallocate_node(n);
        --size_;
    }

    Iterator begin() { return Iterator(head_); }
    Iterator end() { return Iterator(nullptr); }

    bool empty() const { return head_ == nullptr; }
    std::size_t size() const { return size_; }

    void clear() {
        while (head_) pop_front();
    }

private:
    Node* allocate_node() {
        Node* raw = alloc_.allocate(1); // выделяем память для одного узла
        return raw;
    }

    void deallocate_node(Node* n) {
        alloc_.deallocate(n, 1);
    }

private:
    Node* head_ = nullptr;
    std::size_t size_ = 0;
    std::pmr::polymorphic_allocator<Node> alloc_;
};