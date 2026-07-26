#ifndef ZENITH_NULL_SAFETY_H
#define ZENITH_NULL_SAFETY_H

#include <stdexcept>
#include <functional>
#include <iostream>
#include <utility>

namespace zenith {

// Safe Optional Container preventing null pointer crashes in code-first game logic
template <typename T>
class Option {
private:
    T m_value;
    bool m_hasValue = false;

public:
    Option() : m_hasValue(false) {}
    Option(const T& value) : m_value(value), m_hasValue(true) {}
    Option(T&& value) : m_value(std::move(value)), m_hasValue(true) {}

    static Option<T> Some(const T& val) { return Option<T>(val); }
    static Option<T> None() { return Option<T>(); }

    bool isSome() const { return m_hasValue; }
    bool isNone() const { return !m_hasValue; }

    T unwrapOr(const T& fallback) const {
        return m_hasValue ? m_value : fallback;
    }

    T& unwrap() {
        if (!m_hasValue) {
            throw std::runtime_error("Attempted to unwrap a None Option value!");
        }
        return m_value;
    }

    const T& unwrap() const {
        if (!m_hasValue) {
            throw std::runtime_error("Attempted to unwrap a None Option value!");
        }
        return m_value;
    }

    template <typename Func>
    void ifSome(Func func) const {
        if (m_hasValue) {
            func(m_value);
        }
    }
};

// Null-Safe Object Reference
template <typename T>
class SafeRef {
private:
    T* m_ptr = nullptr;

public:
    SafeRef() = default;
    SafeRef(T* ptr) : m_ptr(ptr) {}

    bool isValid() const { return m_ptr != nullptr; }
    
    T* get() const { return m_ptr; }

    T& valueOrThrow() const {
        if (!m_ptr) {
            throw std::runtime_error("SafeRef dereferenced a null pointer!");
        }
        return *m_ptr;
    }

    template <typename Func>
    void with(Func func) const {
        if (m_ptr) {
            func(*m_ptr);
        }
    }
};

} // namespace zenith

#endif // ZENITH_NULL_SAFETY_H
