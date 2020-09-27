#pragma once

#include "control_block.h"

template<typename T>
struct shared_ptr;

template<typename T>
struct weak_ptr {
    template<typename Friend>
    friend
    struct shared_ptr;

    //

    constexpr weak_ptr() noexcept = default;

    weak_ptr(weak_ptr const& r) noexcept;

    weak_ptr(weak_ptr&& r) noexcept;

    weak_ptr& operator=(weak_ptr const& rhs) noexcept;

    weak_ptr& operator=(weak_ptr&& rhs) noexcept;

    //

    template<typename Y>
    weak_ptr(shared_ptr<Y>& r) noexcept;

    ~weak_ptr() noexcept;

    size_t use_count() const noexcept;

    bool expired() const noexcept;

    shared_ptr<T> lock() const noexcept;

    void swap(weak_ptr& rhs) noexcept;

private:
    control_block* cblock = nullptr;
    T* ptr = nullptr;
};

template<typename T>
weak_ptr<T>::weak_ptr(weak_ptr const& r) noexcept
        : cblock(r.cblock), ptr(r.ptr) {
    if (cblock != nullptr) {
        cblock->add_ref_weak();
    }
}

template<typename T>
weak_ptr<T>::weak_ptr(weak_ptr&& r) noexcept
        : cblock(r.cblock), ptr(r.ptr) {
    r.cblock = nullptr;
    r.ptr = nullptr;
}

template<typename T>
weak_ptr<T>& weak_ptr<T>::operator=(weak_ptr const& rhs) noexcept {
    weak_ptr(rhs).swap(*this);
    return *this;
}

template<typename T>
weak_ptr<T>& weak_ptr<T>::operator=(weak_ptr&& rhs) noexcept {
    if (cblock != rhs.cblock) {
        weak_ptr(std::move(rhs)).swap(*this);
    }
    return *this;
}

template<typename T>
template<typename Y>
weak_ptr<T>::weak_ptr(shared_ptr<Y>& r) noexcept
        : cblock(r.cblock), ptr(r.ptr) {
    if (cblock != nullptr) {
        cblock->add_ref_weak();
    }
}

template<typename T>
weak_ptr<T>::~weak_ptr() noexcept {
    if (cblock == nullptr) {
        return;
    }
    if (cblock->dec_ref_weak()) {
        delete cblock;
    }
}

template<typename T>
size_t weak_ptr<T>::use_count() const noexcept {
    return cblock == nullptr ? 0 : cblock->ref_count_shared();
}

template<typename T>
bool weak_ptr<T>::expired() const noexcept {
    return use_count() == 0;
}

template<typename T>
shared_ptr<T> weak_ptr<T>::lock() const noexcept {
    return expired() ? shared_ptr<T>() : shared_ptr<T>(*this);
}

template<typename T>
void weak_ptr<T>::swap(weak_ptr& rhs) noexcept {
    using std::swap;
    swap(cblock, rhs.cblock);
    swap(ptr, rhs.ptr);
}

template<typename T>
struct shared_ptr {
    template<typename Friend>
    friend
    struct weak_ptr;

    template<typename Friend>
    friend
    struct shared_ptr;

    constexpr shared_ptr() noexcept = default;

    constexpr explicit shared_ptr(std::nullptr_t) noexcept;

    template<typename Y, typename D = std::default_delete<Y>>
    explicit shared_ptr(Y* ptr, D deleter = std::default_delete<Y>());

    template<typename Y>
    shared_ptr(shared_ptr<Y> const& rhs) noexcept;

    //

    shared_ptr(shared_ptr const& rhs) noexcept;

    shared_ptr(shared_ptr&& rhs) noexcept;

    shared_ptr& operator=(shared_ptr const& rhs) noexcept;

    shared_ptr& operator=(shared_ptr&& rhs) noexcept;

    //

    template<class Y>
    shared_ptr(shared_ptr<Y> const& r, T* ptr) noexcept;

    template<class Y>
    shared_ptr(weak_ptr<Y> const& r);

    ~shared_ptr();

    T* get() const noexcept;

    T& operator*() const noexcept;

    T* operator->() const noexcept;

    size_t use_count() const noexcept;

    explicit operator bool() const noexcept;

    void reset() noexcept;

    template<class Y, class Deleter = std::default_delete<Y>>
    void reset(Y* ptr, Deleter&& d = std::default_delete<Y>());

    void swap(shared_ptr&) noexcept;

    template<typename U, typename... Args>
    friend shared_ptr<U> make_shared(Args&& ... args);

private:
    control_block* cblock = nullptr;
    T* ptr = nullptr;
};

template<typename T>
template<typename Y>
shared_ptr<T>::shared_ptr(shared_ptr<Y> const& rhs) noexcept
        : cblock(rhs.cblock), ptr(rhs.ptr) {
    if (cblock != nullptr) {
        cblock->add_ref_shared();
    }
}

template<typename T>
shared_ptr<T>::shared_ptr(shared_ptr const& rhs) noexcept
        : cblock(rhs.cblock), ptr(rhs.ptr) {
    if (cblock != nullptr) {
        cblock->add_ref_shared();
    }
}

template<typename T, typename... Args>
shared_ptr<T> make_shared(Args&& ... args) {
    auto* cblock = new inplace_control_block<T>(std::forward<Args>(args)...);
    shared_ptr<T> cur;
    cur.cblock = cblock;
    cur.ptr = cblock->get();
    return cur;
}

template<typename T>
template<typename Y, typename D>
shared_ptr<T>::shared_ptr(Y* p, D deleter)
try : cblock(new regular_control_block(p, deleter)), ptr(p) {}
catch (...) {
    deleter(p);
    throw;
}

template<typename T>
shared_ptr<T>::operator bool() const noexcept {
    return ptr != nullptr;
}

template<typename T>
template<class Y>
shared_ptr<T>::shared_ptr(shared_ptr<Y> const& r, T* ptr) noexcept
        : cblock(r.cblock), ptr(ptr) {
    if (cblock != nullptr) {
        cblock->add_ref_shared();
    }
}

template<typename T>
constexpr shared_ptr<T>::shared_ptr(std::nullptr_t) noexcept {}

template<typename T>
shared_ptr<T>::shared_ptr(shared_ptr&& rhs)  noexcept
        : cblock(rhs.cblock), ptr(rhs.ptr) {
    rhs.cblock = nullptr;
    rhs.ptr = nullptr;
}

template<typename T>
template<class Y>
shared_ptr<T>::shared_ptr(weak_ptr<Y> const& r)
        : cblock(r.cblock), ptr(r.ptr) {
    if (cblock != nullptr) {
        cblock->add_ref_shared();
    }
}

template<typename T>
shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr const& rhs) noexcept {
    shared_ptr(rhs).swap(*this);
    return *this;
}

template<typename T>
shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr&& rhs) noexcept {
    if (cblock != rhs.cblock) {
        shared_ptr(std::move(rhs)).swap(*this);
    }
    return *this;
}

template<typename T>
T* shared_ptr<T>::get() const noexcept {
    return ptr;
}

template<typename T>
T& shared_ptr<T>::operator*() const noexcept {
    return *get();
}

template<typename T>
T* shared_ptr<T>::operator->() const noexcept {
    return get();
}

template<typename T>
size_t shared_ptr<T>::use_count() const noexcept {
    return cblock == nullptr ? 0 : cblock->ref_count_shared();
}

template<typename T>
void shared_ptr<T>::swap(shared_ptr& rhs) noexcept {
    using std::swap;
    swap(ptr, rhs.ptr);
    swap(cblock, rhs.cblock);
}

template<typename T>
void shared_ptr<T>::reset() noexcept {
    shared_ptr().swap(*this);
}

template<typename T>
template<class Y, class D>
void shared_ptr<T>::reset(Y* p, D&& deleter) {
    shared_ptr<T>(p, std::forward<D>(deleter)).swap(*this);
}

template<typename T>
shared_ptr<T>::~shared_ptr() {
    if (cblock == nullptr) {
        return;
    }
    if (cblock->dec_ref_shared()) {
        delete cblock;
    }
}

template<typename V, typename U>
bool operator==(shared_ptr<V> const& lhs, shared_ptr<U> const& rhs) {
    return lhs.get() == rhs.get();
}

template<typename V, typename U>
bool operator!=(shared_ptr<V> const& lhs, shared_ptr<U> const& rhs) {
    return lhs.get() != rhs.get();
}

template<typename V>
bool operator==(shared_ptr<V> const& lhs, std::nullptr_t) {
    return lhs.get() == nullptr;
}

template<typename V>
bool operator==(std::nullptr_t, shared_ptr<V> const& lhs) {
    return lhs.get() == nullptr;
}

template<typename V>
bool operator!=(shared_ptr<V> const& lhs, std::nullptr_t) {
    return lhs.get() != nullptr;
}

template<typename V>
bool operator!=(std::nullptr_t, shared_ptr<V> const& lhs) {
    return lhs.get() != nullptr;
}
