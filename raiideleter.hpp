#ifndef RAIIDELETER_HPP
#define RAIIDELETER_HPP

#include <utility>

/**
 * @brief Call Deleter D on object T on scope exit
 *
 * RaiiDeleter wraps an object of type T and calls
 * Deleter D when the object is destructed
 */
template <class T, class D>
class RaiiDeleter {
private:
    T _object;
    D _deleter;

public:
    RaiiDeleter(T object, D deleter) :
        _object(object),
        _deleter(deleter) {
    }

    ~RaiiDeleter() {
        _deleter(_object);
    }

    RaiiDeleter& operator=(const T& other) {
        _object = other;
        return *this;
    }

    RaiiDeleter& operator=(T&& other) {
        _object = std::move(other);
        return *this;
    }

    T& get() {
        return _object;
    }
};

#endif // RAIIDELETER_HPP
