#include <iostream>
#include "../include/forward_list.hpp"
#include "../include/fixed_block_mr.hpp"

struct Person {
    std::string name;
    int age;
    double salary;
    
    Person(std::string n, int a, double s) : name(std::move(n)), age(a), salary(s) {}
    
    friend std::ostream& operator<<(std::ostream& os, const Person& p) {
        return os << "Person {name='" << p.name << "', age=" << p.age << ", salary=" << p.salary << "}";
    }
};

void demo_int_list() {
    std::cout << "INT" << std::endl;
    FixedBlockMemoryResource mr(1024);
    PmrSinglyList<int> list{&mr};
    for (int i = 0; i < 5; ++i) {
        list.push_front(i * 10);
    }
    std::cout << "List contents: ";
    for (auto it = list.begin(); it != list.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    list.pop_front();
    std::cout << "After pop_front: ";
    for (const auto& item : list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
    std::cout << "List size: " << list.size() << std::endl;
}

void demo_struct_list() {
    std::cout << "\nSTRUCT" << std::endl;
    
    FixedBlockMemoryResource mr(2048);
    PmrSinglyList<Person> list{&mr};

    list.push_front(Person{"Alice", 25, 50000.0});
    list.push_front(Person{"Vasya", 30, 60000.0});
    list.push_front(Person{"Artme", 35, 70000.0});
    
    std::cout << "People list:" << std::endl;
    auto it = list.begin();
    while (it != list.end()) {
        std::cout << "  " << *it << std::endl;
        ++it;
    }
    
    std::cout << "Clearing list..." << std::endl;
    list.clear();
    std::cout << "List empty: " << std::boolalpha << list.empty() << std::endl;
}

void demo_memory_reuse() {
    std::cout << "\nMEMORY REUSE" << std::endl;
    
    FixedBlockMemoryResource mr(512);
    PmrSinglyList<int> list{&mr};
    
    std::cout << "Push" << std::endl;
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);
    
    std::cout << "Contents: ";
    for (const auto& item : list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
    std::cout << "Pop all elements and push new" << std::endl;
    while (!list.empty()) {
        list.pop_front();
    }
    list.push_front(100);
    list.push_front(200);
    
    std::cout << "New contents: ";
    for (const auto& item : list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    try {
        demo_int_list();
        demo_struct_list();
        demo_memory_reuse();
        
        std::cout << "\nOK" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}