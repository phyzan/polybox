<div align="center">

# PolyBox

![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg?style=flat&logo=c%2B%2B)
![Header Only](https://img.shields.io/badge/Header-Only-green.svg?style=flat)
![License](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat)

**Wrapper over heap-allocated objects**

</div>

---

## Overview

PolyBox is a C++20 library that provides a wrapper over heap-allocated objects, automating copy and move semantics.

A very common pattern in C++ is creating a class that manages a pointer to a heap-allocated object:

```cpp
class MyClass {
public:
    MyClass() : ptr(new SomeType()) {}
    ~MyClass() { delete ptr; }
    MyClass(const MyClass& other) : ptr(new SomeType(*other.ptr)) {}
    MyClass(MyClass&& other) noexcept : ptr(other.ptr) { other.ptr = nullptr; }
    MyClass& operator=(const MyClass& other) {
        if (this != &other) {
            delete ptr;
            ptr = new SomeType(*other.ptr);
        }
        return *this;
    }
    MyClass& operator=(MyClass&& other) noexcept {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }
private:
    SomeType* ptr;
};
```

Such implementations are error-prone and verbose. PolyBox simplifies this pattern by providing a generic wrapper that handles the memory management and copy/move semantics automatically for some type `MyClass`, as long as the class provides a `MyClass::clone() const -> std::unique_ptr<SomeType>` where `SomeType*` is either convertible to `MyClass*` or `static_cast`-able to `MyClass*`.

Althout the first case is trivially valid, the latter is valid if e.g. `Derived` derives from `Base`, and
```cpp
std::unique_ptr<Base> Derived::clone() const {
    return std::make_unique<Derived>(*this);
}
```
Although `Base*` is not convertible to `Derived*`, this `clone()` implementation guarantees that `static_cast<Derived*>(base_ptr)` is valid, and does not cause undefined behavior.

As long as the `clone()` method is implemented correctly for a class `MyClass`, PolyBox provides
```cpp
pbox::owner<MyClass>
``` 
which has a trivial move semantics, and copy semantics that perform a deep copy of the underlying object by calling `MyClass::clone()`.

Additionally, `PolyBox` provides
```cpp
pbox::Box<MyClass>
```
which works like `std::unique_ptr<MyClass>`, taking full ownership of the underlying object, without allowing copy semantics but only move semantics for safe ownership transfer. It can be constructed/assigned from another `pbox::Box` or `std::unique_ptr`, conserving the uniqueness of the ownership of the underlying object.

Both `pbox::Box<MyClass>` and `pbox::owner<MyClass>` can be constructed using
```cpp
pbox::make_box<SomeType>
```
as long as `SomeType*` is convertible to `MyClass*`:
```cpp
pbox::Box<BaseClass> unique_box = pbox::make_box<DerivedClass>(args...);
pbox::owner<BaseClass> unique_owner = pbox::make_box<DerivedClass>(args...);
```
by forwarding the arguments to the constructor of `DerivedClass`.

Both `pbox::Box` and `pbox::owner` can be used with `operator->` just like `std::unique_ptr`:

```cpp
pbox::Box<MyClass> box = pbox::make_box<MyClass>(args...);
box->some_method(); // Calls MyClass::some_method()
```

## Quick Example

Compile `example.cpp` in the root directory:
```bash
g++ -std=c++20 -Iinclude example.cpp -o example
```
and run it:
```bash
./example
```

## CMake

Enable includes via
```cpp
#include <polybox/polybox.hpp>
```
by directing CMake to the `include` directory.

If PolyBox is vendored as a subdirectory of your project (e.g. `third_party/polybox`), point your target at its `include` directory:
```cmake
target_include_directories(your_target PRIVATE third_party/polybox/include)
```
