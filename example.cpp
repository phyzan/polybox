#include <iostream>
#include <polybox/polybox.hpp>

class MyClass {
public:
    MyClass() = default;
    MyClass(const MyClass&) = default;
    MyClass(MyClass&&) noexcept = default;
    MyClass& operator=(const MyClass&) = default;
    MyClass& operator=(MyClass&&) noexcept = default;
    ~MyClass() = default;
    std::unique_ptr<MyClass> clone() const {
        return std::make_unique<MyClass>(*this);
    }
    void foo() { std::cout << "Called foo method" << std::endl; }
};

int main() {
    pbox::Box<MyClass> box = pbox::make_box<MyClass>();
    box->foo(); // Calls MyClass::foo()
    
    pbox::owner<MyClass> owner = pbox::make_box<MyClass>();    
    pbox::owner<MyClass> owner_copy(owner); // Deep copy using clone()
    owner_copy->foo(); // Calls MyClass::foo()

    owner_copy = std::move(owner); // Move semantics, now `owner` does not own the object anymore
    owner_copy->foo();

    // owner_copy = box; // Does not compile, because `pbox::Box` does not allow copy semantics
    owner_copy = std::move(box); // Move semantics, now `box` does not own its object anymore
    owner_copy->foo();

    owner_copy = owner_copy->clone(); // Reassigning a new deep copy of the underlying object
    owner_copy->foo();

    // constructing from another `owner`
    pbox::owner<MyClass> new_owner(std::move(owner_copy)); // Move semantics, now `owner_copy` does not own the object anymore
    new_owner->foo();

    // Make sure all objects besides `new_owner` do not have any ownership:
    if ((bool)owner || (bool)owner_copy || (bool)box) {
        // Should not happen
        std::cout << "Some objects still own the underlying object! (FAILED)" << std::endl;
    } else {
        std::cout << "All other objects have released ownership. (PASSED)" << std::endl;
    }

    if ((bool)new_owner) {
        std::cout << "new_owner still owns the underlying object as it should. (PASSED)" << std::endl;
    } else {
        // Should not happen
        std::cout << "new_owner does not own the underlying object. (FAILED)" << std::endl;
    }
    
    return 0;
}

// g++ -std=c++20 -Iinclude example.cpp -o example && ./example