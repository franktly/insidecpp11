#include <iostream>
#include <memory>

// strong-typed enum
// benefit: 
// 1. strong scope: strong-typed enum's member name won't visable to its parent scope
// 2. implicit cast restriction: strong-typed enum and integer type won't cast implicitly
// 3. inner type customed defined: default is int, any other integer type except wchar_t (append Type: type_name)

void strongTypedEnumTest()
{
    // common-typed enum
    enum A {A1 = 1, A2 = 2};
    enum B {B1 = 1, B2 = 2, Bbig = 0xFFFFFFF0U};
    enum C {C1 = 1, C2 = 2, Cbig = 0xFFFFFFFFFLL};
    A a1 = A1;
    A a2 = A::A1;  //both ok and NO strong scope

    std::cout << A1 << ": " << sizeof(A1) << std::endl;
    std::cout << B1 << ": " << sizeof(B1) << std::endl;
    std::cout << Bbig << ": "<< sizeof(Bbig) << std::endl; 
    std::cout << C1 << ": " << sizeof(C1) << std::endl;
    std::cout << Cbig << ": "<< sizeof(Cbig) << std::endl; // diffrent type of compile's inner integer type is different

    // strong-typed enum
    enum class Type {General, Light, Heavy};
    enum class Category {General = 1, Pistol, Cannon};

    Type t = Type::Light;
    // Type t2 = Light; // error: strong scope
    // t = General; // error: strong scope

    // if(t == Category::General) // error: must use Type::General
    if(t == Type::General) // ok
    {
        std::cout << (int)t <<" is same with Type::General" << std::endl;
    } 
    // if(t > 0)  // error: cast restriction
    if((int)t > 0)
    {
        std::cout << (int)t <<" is larger than 0" << std::endl;
    }

    std::cout << "Type is pod? " << std::is_pod<Type>::value << std::endl;
    std::cout << "Category is pod? " << std::is_pod<Category>::value << std::endl;

    // specify strong-typed enum inner type
    enum class D : char { D1 = 1, D2 = 2};
    enum class E : unsigned int { E1 = 1, E2 = 2, Ebig = 0xFFFFFFF0U};

    std::cout << (int)D::D1 << ": " << sizeof(D::D1) << std::endl;
    std::cout << (unsigned int)E::E1 << ": " << sizeof(E::E1) << std::endl;
    std::cout << (unsigned int)E::Ebig << ": " << sizeof(E::Ebig) << std::endl;

    // anonymous strong-typed enum
    // enum class {General, Light, Heavy} weapon; // error: anonymous strong-typed enum needs a name
}

// smart pointer
// heap memory problem: 
// 1. wild pointer
// 2. double free pointer
// 3. memory leak
// auto_ptr is deprecated in C++11  for( auto_ptr copy construction returns a lvalue and delete [] is not allowd)
//
// GC(Garbage Collection) method:
// 1. reference counting GC
// 2. tracing GC
// 2.1 Mark-Sweep
// 2.2 Mark-Compact
// 2.3 Mark-Copy
// 2.4 Mark-Copy
//
// C++11 min-GC: 
// only new operator takes effect by min-GC, malloc or cumstom-defined new operator will not take effect
// pointer_safety {relaxed, preferred, strict}
// 1) relaxed:
// The validity of a pointer value does not depend on whether it is a safely-derived pointer value. 
// 2) preferred:
// The validity of a pointer value does not depend on whether it is a safely-derived pointer value. 
// A leak detector may be running so that the program can avoid spurious leak reports.
// 3) strict:
// A pointer value that is not a safely-derived pointer value is an invalid pointer value (unless the referenced complete object is of dynamic storage duration and has previously been declared reachable).
// A garbage collector may be running so that non-traceable object are automatically deleted.
//
//min-GC safely derived pointer operation including the following 4 types of operations:
//1. reference based dereference: &*p;
//2. explicitly pointer operaton: p + 1;
//3. explicitly pointer transition: static_cast<void*>(p);
//4. reinterpret_cast between pointer and integer type : reinterpret_cast<intptr_t>(p);

void check(std::weak_ptr<int> &wp)
{
    std::shared_ptr<int> sp = wp.lock(); //  weak_ptr -> shared_ptr
    if(nullptr == sp)
    {
        std::cout << "pointer is null and invalid" << std::endl;
    }
    else
    {
        std::cout << "pointer is still valid: " << *sp << std::endl;
    }
}

void smartPointerTest()
{
    std::unique_ptr<int> up1(new int(33));
    // std::unique_ptr<int> up2 = up1;  // error 
    std::cout << "up1 value: " << *up1 << std::endl;
    std::unique_ptr<int> up3 = std::move(up1);
    std::cout << "up3 value: " << *up3 << std::endl;
    // std::cout << "up1 value: " << *up1 << std::endl;  // runtime error
    up3.reset();  // free heap memory explictly
    up1.reset();  // no runtime error

    std::shared_ptr<int> sp1(new int(333));
    std::shared_ptr<int> sp2 = sp1;
    std::weak_ptr<int> wp = sp1;  // wp point to object refered by sp1 as well
    std::cout << "sp2 value: " << *sp2 << std::endl;
    std::cout << "sp1 value: " << *sp1 << std::endl;
    check(wp);  // valid

    sp1.reset();
    std::cout << "sp2 value: " << *sp2 << std::endl;
    check(wp);   // valid

    sp2.reset();
    check(wp);    // invalid

    // C++11 min-GC
    int* p = new int(3);
    p += 10;  // move pointer and may cause min-GC
    std::cout << "before min-GC: " << *p << std::endl;
    p -= 10;
    *p = 10;
    std::cout << "after min-GC: " << *p << std::endl;

    int *p2 = new int(3);
    std::declare_reachable(p);  // add reachable  declare [min-GC disable for p range begin]
    int *q = (int*)(reinterpret_cast<long long>(p2) ^ 2012); // q hides p
    std::cout << "before min-GC: " << *q << std::endl;
    // do other works
    q = std::undeclare_reachable<int>((int*)(reinterpret_cast<long long>(q) ^ 2012)); // come back to p : q == p  // add undeclare [min-GC disable for p range end]
    // q = (int*)(reinterpret_cast<long long>(q) ^ 2012); // come back to p : q == p
    *q = 10;
    std::cout << "after min-GC: " << *q << std::endl;

    std::cout << "pointer safety: ";
    switch (std::get_pointer_safety()) {
        case std::pointer_safety::relaxed:   std::cout << "relaxed";   break;
        case std::pointer_safety::preferred: std::cout << "preferred"; break;
        case std::pointer_safety::strict:    std::cout << "strict";    break;
    }
    std::cout << std::endl;
}

int main(int argc, char** argv)
{
    // strongTypedEnumTest();
    smartPointerTest();
    return 0;
}
