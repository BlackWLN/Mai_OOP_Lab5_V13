#pragma once
#include <memory_resource>
#include <list>
#include <cstddef>
#include <new>
#include <iostream>
#include <algorithm>

class FixedBlockMemoryResource : public std::pmr::memory_resource {
public:
    explicit FixedBlockMemoryResource(std::size_t total_bytes) {
        total_size_ = align_up(total_bytes, alignof(std::max_align_t)); // округляем под выравнивание
        buffer_ = static_cast<std::byte*>(::operator new(total_size_));
        base_addr_ = reinterpret_cast<std::uintptr_t>(buffer_);
        free_segments_.push_back({0, total_size_});
    }

    ~FixedBlockMemoryResource() override {
        if (!allocated_.empty()) {
            std::cerr << "[PMR] Warning: leaked blocks = " << allocated_.size() << "\n";
        }
        ::operator delete(buffer_);
    }

    FixedBlockMemoryResource(const FixedBlockMemoryResource&) = delete; //запрещаем копирование
    FixedBlockMemoryResource& operator=(const FixedBlockMemoryResource&) = delete; //запрещаем перемещение

private:
    struct Segment { std::size_t offset; std::size_t size; };
    struct AllocRec { void* ptr; std::size_t size; std::size_t offset; };

    static std::size_t align_up(std::size_t n, std::size_t a) {
        return (n + a - 1) & ~(a - 1);
    }

    void* do_allocate(std::size_t bytes, std::size_t alignment) override {         //перебираем список свободных сегментов и ищем первый подходящий
        for (auto it = free_segments_.begin(); it != free_segments_.end(); ++it) {
            std::size_t seg_off = it->offset;
            std::size_t seg_size = it->size;

            std::uintptr_t seg_addr = base_addr_ + seg_off;
            std::uintptr_t aligned_addr = (seg_addr + (alignment - 1)) & ~(alignment - 1);
            std::size_t padding = aligned_addr - seg_addr;

            if (padding + bytes <= seg_size) {
                std::size_t used_total = padding + bytes;
                std::size_t result_offset = seg_off + padding;
                void* result_ptr = reinterpret_cast<void*>(base_addr_ + result_offset);
                if (used_total == seg_size) {
                    free_segments_.erase(it);
                } else {
                    it->offset += used_total;
                    it->size -= used_total;
                }

                allocated_.push_back({result_ptr, bytes, result_offset});
                return result_ptr;
            }
        }
        throw std::bad_alloc();
    }

    void do_deallocate(void* p, std::size_t bytes, std::size_t) override {
        auto it = std::find_if(allocated_.begin(), allocated_.end(), [p](const AllocRec& r){ return r.ptr == p; });
        if (it == allocated_.end()) {
            std::cerr << "[PMR] deallocate: unknown pointer\n";
            return;
        }
        Segment newseg{ it->offset, it->size }; // превращаем запись в свободный сегмент
        allocated_.erase(it);
        insert_and_coalesce(newseg);
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }

    void insert_and_coalesce(const Segment& seg) {
        auto it = free_segments_.begin();         // вставляем по месту
        for (; it != free_segments_.end(); ++it) {
            if (seg.offset < it->offset) break;
        }
        auto inserted = free_segments_.insert(it, seg);

        if (inserted != free_segments_.begin()) {
            auto prev = std::prev(inserted);
            if (prev->offset + prev->size == inserted->offset) {
                prev->size += inserted->size;
                inserted = free_segments_.erase(inserted);
            }
        }
        auto next = std::next(inserted);

        if (next != free_segments_.end()) {
            if (inserted->offset + inserted->size == next->offset) {
                inserted->size += next->size;
                free_segments_.erase(next);
            }
        }
    }

private:
    std::byte* buffer_ = nullptr;
    std::uintptr_t base_addr_ = 0;
    std::size_t total_size_ = 0;
    std::list<Segment> free_segments_;
    std::list<AllocRec> allocated_;
};