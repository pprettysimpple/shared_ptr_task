//
// Created by kirill on 9/26/20.
//

#ifndef SHARED_PTR_TESTING_CONTROL_BLOCK_H
#define SHARED_PTR_TESTING_CONTROL_BLOCK_H

#include <cstddef>
#include <type_traits>
#include <utility>

struct control_block {
    control_block() noexcept;

    void add_ref_shared() noexcept;

    void add_ref_weak() noexcept;

    void dec_ref_shared() noexcept;

    void dec_ref_weak() noexcept;

    size_t ref_count_shared() const noexcept;

    size_t ref_count_weak() const noexcept;

    virtual void delete_object() noexcept = 0;

    virtual ~control_block() = default;

private:
    size_t ref_cnt_shared;
    size_t ref_cnt_weak;
};

void control_block::add_ref_shared() noexcept {
    ++ref_cnt_shared;
}

void control_block::add_ref_weak() noexcept {
    ++ref_cnt_weak;
}

void control_block::dec_ref_shared() noexcept {
    --ref_cnt_shared;
}

void control_block::dec_ref_weak() noexcept {
    --ref_cnt_weak;
}

size_t control_block::ref_count_shared() const noexcept {
    return ref_cnt_shared;
}

size_t control_block::ref_count_weak() const noexcept {
    return ref_cnt_weak;
}

control_block::control_block() noexcept: ref_cnt_shared(1), ref_cnt_weak(0) {}

template<typename T, typename D = std::default_delete<T>>
struct regular_control_block final : control_block {
    explicit regular_control_block(T* ptr, D deleter = std::default_delete<T>());

    void delete_object() noexcept override;

    ~regular_control_block() override = default;

private:
    T* ptr = nullptr;
    [[no_unique_address]] D deleter;
};

template<typename T, typename D>
regular_control_block<T, D>::regular_control_block(T* ptr, D deleter)
        : control_block(), ptr(ptr), deleter(std::move(deleter)) {}

template<typename T, typename D>
void regular_control_block<T, D>::delete_object() noexcept {
    deleter(ptr);
}

template<typename T>
struct inplace_control_block final : control_block {
    template<typename... Args>
    explicit inplace_control_block(Args&& ... args);

    T* get() noexcept;

    void delete_object() noexcept override;

    ~inplace_control_block() override = default;

private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type stg;
};

template<typename T>
template<typename... Args>
inplace_control_block<T>::inplace_control_block(Args&& ... args)
        : control_block() {
    new(&stg) T(std::forward<Args>(args)...);
}

template<typename T>
T* inplace_control_block<T>::get() noexcept {
    return reinterpret_cast<T*>(&stg);
}

template<typename T>
void inplace_control_block<T>::delete_object() noexcept {
    return reinterpret_cast<T*>(&stg)->~T();
}

#endif //SHARED_PTR_TESTING_CONTROL_BLOCK_H
