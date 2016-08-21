/*

The MIT License (MIT)

Copyright (c) 2014 https://github.com/labyrinthofdreams

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef PTRUTIL_HPP
#define PTRUTIL_HPP

#include <utility>
#include <memory>

namespace vfg {

/**
 * @brief The observer_ptr class wraps a raw pointer
 *
 * An observer_ptr implies no ownership and makes it
 * more difficult to accidentally do "delete ptr"
 * or pass it to a function that takes ownership.
 */
template <class T>
class observer_ptr {
    T* _ptr {nullptr};

public:
    observer_ptr() = default;
    observer_ptr(T* ptr) : _ptr(ptr) {}

    observer_ptr(const observer_ptr& other) = default;
    observer_ptr(observer_ptr&& other) = default;

    observer_ptr& operator=(const observer_ptr& other) = default;
    observer_ptr& operator=(observer_ptr&& other) = default;

    observer_ptr& operator=(T* new_ptr) { _ptr = new_ptr; return *this; }

    ~observer_ptr() = default;

    operator bool() const { return _ptr; }

    T& operator *() const { return *_ptr; }

    T* operator ->() const { return _ptr; }

    /**
     * @brief Get raw pointer to observed object
     * @return Pointer to observed object
     */
    T* get() const { return _ptr; }

    /**
     * @brief Set internal pointer value to nullptr and return old pointer
     * @return Pointer to observed object
     */
    T* release() { T* tmp = _ptr; _ptr = nullptr; return tmp; }

    /**
     * @brief Set internal pointer value to nullptr
     */
    void reset() { _ptr = nullptr; }
};

template <typename T1, typename T2>
inline
bool operator==(const observer_ptr<T1>& lhs, const observer_ptr<T2>& rhs) {
    return lhs.get() == rhs.get();
}

template <typename T1, typename T2>
inline
bool operator!=(const observer_ptr<T1>& lhs, const observer_ptr<T2>& rhs) {
    return !(lhs == rhs);
}

template <typename T1, typename T2>
inline
bool operator==(const observer_ptr<T1>& lhs, const T2* rhs) {
    return lhs.get() == rhs;
}

template <typename T1, typename T2>
inline
bool operator!=(const observer_ptr<T1>& lhs, const T2* rhs) {
    return !(lhs == rhs);
}

template <typename T1, typename T2>
inline
bool operator==(const T1* lhs, const observer_ptr<T2>& rhs) {
    return lhs == rhs.get();
}

template <typename T1, typename T2>
inline
bool operator!=(const T1* lhs, const observer_ptr<T2>& rhs) {
    return !(lhs == rhs);
}

template <typename T, typename... Args>
observer_ptr<T> make_observer(Args&&... args) {
    return observer_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace vfg

#endif
