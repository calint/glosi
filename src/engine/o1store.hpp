#pragma once
//
// implements a O(1) store of objects
//
// * type: object type. 'type' must contain public field 'type **alloc_ptr'
// * instance_count: number of preallocated objects
// * store_id: id used when printing to identify the o1store
// * return_nullptr_when_no_free_instance_available: if false throws exception
//   when no free slot available
// * thread_safe: true to synchronize 'allocate_instance' and 'free_instance'
// * instance_size_B: custom size of object instance used to fit largest object
//   in an object hierarchy. must be multiple of 'cache_line_size_B'
// * cache_line_size_B: when 'instance_size_B' specified, object store allocated
//   cache line aligned
//
// reviewed: 2024-07-08

#include "../application/configuration.hpp"
#include "exception.hpp"
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace glos {

template <typename type, size_t instance_count, uint32_t store_id = 0,
          bool return_nullptr_when_no_free_instance_available = false,
          bool thread_safe = false, size_t instance_size_B = 0,
          size_t cache_line_size_B = 0>
class o1store final {
    type* all_ = nullptr;
    type** free_bgn_ = nullptr;
    type** free_ptr_ = nullptr;
    type** free_end_ = nullptr;
    type** alloc_bgn_ = nullptr;
    type** alloc_ptr_ = nullptr;
    type** del_bgn_ = nullptr;
    type** del_ptr_ = nullptr;
    type** del_end_ = nullptr;
    std::atomic_flag lock = ATOMIC_FLAG_INIT;

  public:
    inline o1store() {
        if (instance_size_B) {
            static_assert(instance_size_B % cache_line_size_B == 0);
            // allocate cache line aligned memory
            size_t constexpr mem_size = instance_count * instance_size_B;
            void* const mem = std::aligned_alloc(cache_line_size_B, mem_size);
            assert(mem);
            assert((uintptr_t(mem) % cache_line_size_B) == 0);
            std::memset(mem, 0, mem_size);
            all_ = static_cast<type*>(mem);
        } else {
            all_ =
                static_cast<type*>(std::calloc(instance_count, sizeof(type)));
            assert(all_);
        }

        free_ptr_ = free_bgn_ =
            static_cast<type**>(std::calloc(instance_count, sizeof(type*)));
        assert(free_ptr_);

        alloc_ptr_ = alloc_bgn_ =
            static_cast<type**>(std::calloc(instance_count, sizeof(type*)));
        assert(alloc_ptr_);

        del_ptr_ = del_bgn_ =
            static_cast<type**>(std::calloc(instance_count, sizeof(type*)));
        assert(del_ptr_);

        free_end_ = free_bgn_ + instance_count;
        del_end_ = del_bgn_ + instance_count;

        // write pointers to instances in the 'free' list
        type* all_it = all_;
        for (type** free_it = free_bgn_; free_it < free_end_; ++free_it) {
            *free_it = all_it;
            if (instance_size_B) {
                all_it = reinterpret_cast<type*>(
                    reinterpret_cast<char*>(all_it) + instance_size_B);
            } else {
                ++all_it;
            }
        }
    }

    inline ~o1store() {
        std::free(all_);
        std::free(alloc_bgn_);
        std::free(free_bgn_);
        std::free(del_bgn_);
    }

    // allocates an instance
    // @return nullptr or throws if instance could not be allocated
    inline auto allocate_instance() -> type* {
        if (thread_safe) {
            acquire_lock();
        }
        if (free_ptr_ >= free_end_) {
            if (thread_safe) {
                release_lock();
            }
            if (return_nullptr_when_no_free_instance_available) {
                return nullptr;
            } else {
                throw exception{std::format(
                    "store {}: out of free instances. consider increasing "
                    "the size of the store.",
                    store_id)};
            }
        }
        type* inst = *free_ptr_;
        ++free_ptr_;
        *alloc_ptr_ = inst;
        inst->alloc_ptr = alloc_ptr_;
        ++alloc_ptr_;
        if (thread_safe) {
            release_lock();
        }
        return inst;
    }

    // adds instance to list of instances to be freed with 'apply_free()'
    inline auto free_instance(type* const inst) -> void {
        if (thread_safe) {
            acquire_lock();
        }

        if (o1store_check_free_limits) {
            if (del_ptr_ >= del_end_) {
                throw exception{
                    std::format("store {}: free overrun", store_id)};
            }
        }

        if (o1store_check_double_free) {
            for (type** it = del_bgn_; it < del_ptr_; ++it) {
                if (*it == inst) {
                    throw exception{
                        std::format("store {}: double free", store_id)};
                }
            }
        }

        *del_ptr_ = inst;
        ++del_ptr_;

        if (thread_safe) {
            release_lock();
        }
    }

    // deallocates the instances that have been freed
    inline auto apply_free(auto&& callback) -> void {
        for (type** it = del_bgn_; it < del_ptr_; ++it) {
            type* inst_deleted = *it;
            callback(inst_deleted);
            alloc_ptr_--;
            type* inst_to_move = *alloc_ptr_;
            inst_to_move->alloc_ptr = inst_deleted->alloc_ptr;
            *(inst_deleted->alloc_ptr) = inst_to_move;
            free_ptr_--;
            *free_ptr_ = inst_deleted;
            inst_deleted->~type();
        }
        del_ptr_ = del_bgn_;
    }

    // @return list of allocated instances
    inline auto allocated_list() const -> type** { return alloc_bgn_; }

    // @return length of list of allocated instances
    inline auto allocated_list_len() const -> uint32_t {
        return uint32_t(alloc_ptr_ - alloc_bgn_);
    }

    // @return one past the end of allocated instances list
    inline auto allocated_list_end() const -> type** { return alloc_ptr_; }

    // @return the list with all preallocated instances
    inline auto all_list() const -> type* { return all_; }

    // @return the length of 'all' list
    inline auto constexpr all_list_len() const -> size_t {
        return instance_count;
    }

    // @return instance at index 'ix' from 'all' list
    inline auto instance(size_t const ix) const -> type* {
        if (!instance_size_B) {
            return &all_[ix];
        }
        // note: if instance size is specified do pointer shenanigans
        return reinterpret_cast<type*>(reinterpret_cast<char*>(all_) +
                                       instance_size_B * ix);
    }

    // @return the size of allocated heap memory in bytes
    inline auto constexpr allocated_data_size_B() const -> size_t {
        return instance_size_B ? (instance_count * instance_size_B +
                                  3 * instance_count * sizeof(type*))
                               : (instance_count * sizeof(type) +
                                  3 * instance_count * sizeof(type*));
    }

    inline auto acquire_lock() -> void {
        while (lock.test_and_set(std::memory_order_acquire)) {
        }
    }

    inline auto release_lock() -> void {
        lock.clear(std::memory_order_release);
    }
};

} // namespace glos
