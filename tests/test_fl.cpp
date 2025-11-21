#include <gtest/gtest.h>
#include "../include/forward_list.hpp"
#include "../include/fixed_block_mr.hpp"

// Тестовый сложный тип
struct TestStruct {
    int id;
    std::string name;
    double value;
    
    TestStruct(int i, std::string n, double v) : id(i), name(std::move(n)), value(v) {}
    
    bool operator==(const TestStruct& other) const {
        return id == other.id && name == other.name && value == other.value;
    }
};

class PmrForwardListTest : public ::testing::Test {
protected:
    void SetUp() override {
        mr = std::make_unique<FixedBlockMemoryResource>(4096);
    }
    
    void TearDown() override {
        mr.reset();
    }
    
    std::unique_ptr<FixedBlockMemoryResource> mr;
};

// Тесты для int
TEST_F(PmrForwardListTest, IntListPushFrontAndIteration) {
    PmrSinglyList<int> list{mr.get()};
    
    list.push_front(3);
    list.push_front(2);
    list.push_front(1);
    
    auto it = list.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_EQ(it, list.end());
}

TEST_F(PmrForwardListTest, IntListSizeAndEmpty) {
    PmrSinglyList<int> list{mr.get()};
    
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
    
    list.push_front(1);
    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.size(), 1);
    
    list.push_front(2);
    EXPECT_EQ(list.size(), 2);
}

TEST_F(PmrForwardListTest, IntListPopFront) {
    PmrSinglyList<int> list{mr.get()};
    
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);
    
    list.pop_front();
    EXPECT_EQ(*list.begin(), 2);
    EXPECT_EQ(list.size(), 2);
    
    list.pop_front();
    EXPECT_EQ(*list.begin(), 1);
    EXPECT_EQ(list.size(), 1);
    
    list.pop_front();
    EXPECT_TRUE(list.empty());
}

TEST_F(PmrForwardListTest, IntListRangeBasedFor) {
    PmrSinglyList<int> list{mr.get()};
    std::vector<int> expected = {3, 2, 1};
    
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);
    
    std::vector<int> result;
    for (const auto& item : list) {
        result.push_back(item);
    }
    
    EXPECT_EQ(result, expected);
}

// Тесты для сложного типа
TEST_F(PmrForwardListTest, StructListPushFrontAndIteration) {
    PmrSinglyList<TestStruct> list{mr.get()};
    
    list.push_front(TestStruct{1, "first", 1.1});
    list.push_front(TestStruct{2, "second", 2.2});
    
    auto it = list.begin();
    EXPECT_EQ(it->id, 2);
    EXPECT_EQ(it->name, "second");
    EXPECT_EQ(it->value, 2.2);
    
    ++it;
    EXPECT_EQ(it->id, 1);
    EXPECT_EQ(it->name, "first");
    EXPECT_EQ(it->value, 1.1);
}

TEST_F(PmrForwardListTest, StructListMoveSemantics) {
    PmrSinglyList<TestStruct> list{mr.get()};
    
    TestStruct s1{1, "test", 1.5};
    list.push_front(std::move(s1));
    
    EXPECT_EQ(list.begin()->id, 1);
    EXPECT_EQ(list.begin()->name, "test");
}

TEST_F(PmrForwardListTest, StructListClear) {
    PmrSinglyList<TestStruct> list{mr.get()};
    
    list.push_front(TestStruct{1, "a", 1.0});
    list.push_front(TestStruct{2, "b", 2.0});
    
    EXPECT_FALSE(list.empty());
    list.clear();
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
}

TEST_F(PmrForwardListTest, MemoryReuse) {
    PmrSinglyList<int> list{mr.get()};
    for (int i = 0; i < 10; ++i) {
        list.push_front(i);
    }
    const size_t original_size = list.size();
    while (!list.empty()) {
        list.pop_front();
    }
    for (int i = 0; i < 10; ++i) {
        list.push_front(i * 2);
    }
    EXPECT_EQ(list.size(), original_size);
    int expected = 18;
    for (auto it = list.begin(); it != list.end(); ++it) {
        EXPECT_EQ(*it, expected);
        expected -= 2;
    }
}
TEST_F(PmrForwardListTest, OutOfMemory) {
    FixedBlockMemoryResource small_mr(64);
    PmrSinglyList<int> list{&small_mr};
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);
    EXPECT_THROW({
        while (true) {
            list.push_front(42);
        }
    }, std::bad_alloc);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}