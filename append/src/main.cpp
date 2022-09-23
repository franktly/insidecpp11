#include<iostream>

void incompatibleWithCpp03()
{
    // 1. new keyword
    // u8, u, U (string type qualifier)
    // R, u8R, uR, UR, LR (raw literal const)
    //

    //2. cumstom define literal const
    // XXX_Y
    //

    //3. new keyword
    //alignof, alignas, char16_t, char32_t,
    //constexpr, decltype, noexcept, nullptr, static_assert, thread_local
    //

    //4. new type:
    //long long 
    // convert to unsigned long (C++03)
    //

    //5. auto : different semanteme
    // storage-type-qualifier(C++03)
    // expressison type deduce(C++11)
    //

    //6. array initialization 
    // type-narrowing is unallowed C++11)
    // int arr[] = {1.0}; // C++11 error, C++03 ok

    //7. implicit deleted
    struct A { const int _i; };
    struct A2 
    { 
        public: 
            A2(int i) : _i(i){}  // NEED add construction mandatory
            void print()
            {
                std::cout << "_i: " << _i <<std::endl;
            }
            const int _i{0};
    };

    // A a;  // error: const int i should be sdtatic initialize and A class default construction is deleted implicitly
    A2 a2(3);
    a2.print();

    //8. delete none-use keyword: export

    //9. new standard header files and header files compatible with C (XXX.h -> cXXX>

    //10. swap():
    //<algorithm> C++03
    //<utility> C++11

    //11. add new top namespace: posix

    //12. operator new: 
    //only throw std::bad_alloc exception (C++03)
    //allow other kinds of exception (C++11)

    //13. errno:
    //global (C++03)
    //TLS thread_local(C++11)

    //14. size() complexity
    //std::list allows linear complexity
    //all O(0) complexity

    //15. register: different semanteme
    // suggestion for compiler to storage var in register (C++03)
    //  like C++03 auto keyword (C++11)
}


void incompatibleWithISOC()
{
    //1. keyword

    //2. char const type
    //C: int
    //C++: char

    //3. string const type
    //C: char[]
    //C++: const char[]

    //4. whether allow var define duplicated
    //C: allowed
    //C++: unallowed

    //5. const var without extern
    //C: external linkage (allow being referred by same name var in other files)
    //C++: interal linkage

    //6. (void*) -> X*
    //C++: implicit unallowed
    //C: implicit allowed

    //7. const object 
    //C: no need initialized mandatory
    //C++: intialized mandatory

    //8. whether volatile object can be passed as implicit construction function and implicit assign function paramter
    //C: allowed
    //C++: unallowed

    struct X {int i;};
    volatile struct X x1 = {0};
    // struct X x2(x1); // error:C++11
    struct X x3;
    // struct X x4 = x1;// error: C++11
}

void improveCpp03()
{
    // 1. memory release function will not terminate bz of throwing exception

    // 2.  the result of typeid operator with parameter contains cv(const, volatile) qualifier is the same as none cv qualifier
}


int main(int argc, char** argv)
{
    incompatibleWithCpp03();
    incompatibleWithISOC();
    improveCpp03();
    return 0;
}
