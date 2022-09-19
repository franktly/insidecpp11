#include <iostream>
#include <list>
#include <vector>
#include <deque>
#include <map>
#include <type_traits>
#include <string.h>

class Base
{
    public: 
        Base(int a = 3, double = 3.0){ std::cout << "base construction with first argument: " << a << std::endl;  }
        void fn(double i){ std::cout << "base common fn with double argument: " << i << std::endl; }
};

class Derive : public Base
{
    public:
        using Base::Base; // if use derive construction, derive class's default construction function will not be generated automatically AND NOT including a none-argument version
        Derive(){std:: cout << "derive none-argument construction" << std::endl;} // not generated if using derive construction 
        using Base::fn; // reuse base fn(double)
        void fn(int i){ std::cout << "derive common fn with int argument: " << i << std::endl; }
};

void deriveConstructFuncTest()
{
    Derive d; // none-argument construction function Derive()
    d.fn(3.0);  // derive fn(int) // base fn(double) is NOT derived (if only using Base::fn)
    d.fn(3);    // derive fn(int)

    Derive d2(6); // derive construction from Base
    Derive d3(9, 9.0); // derive construction from Base
}

// delegate construction must appear alone and can not be used with argument initial list together
// target construction function is executed early than delegate construction function
class Info
{
    public:
        Info(): Info(1){ std::cout << "delegate construction with none-argument" << std::endl;} // delegate construction function
        Info(int i): Info(i, 'a'){ std::cout << "delegate and target construction with int argument" << std::endl;} // delegate construction and also target construction function 
        Info(char e): Info(1, e){ std::cout << "delegate construction with char argument" << std::endl;} // delegate construction function
        // Info(int i, char e, double k): Info(i, e), _k(k){} // delegate construction must appear alone

    private:
        Info(int i, char e): _type(i), _name(e){ std::cout << "target construction with int and char argument" << std::endl;} // target construction function
        int _type;
        char _name;
        double _k{3.0f};
};


// generate target construct function using delegate construction template function
class TDConstructed
{
    private:
        template<class T> TDConstructed(T first, T last) : l(first, last) 
    {
        std::cout << "template target construction" << std::endl;
    }
        std::list<int> l;

    public:
        TDConstructed(std::vector<short> &v) : TDConstructed(v.begin(), v.end())
    {
        std::cout << "delegate construction with vector" << std::endl;
    }

        TDConstructed(std::deque<int> &d) : TDConstructed(d.begin(), d.end())
    {
        std::cout << "delegate construction with deque" << std::endl;
    }
};

// catch exception by delegate construction function that exception is generated by target construction function

class DCExcept
{
    public:
        DCExcept(double d)
        try : DCExcept(1, d)
        {
            std::cout << "delegate construction running body" << std::endl; // NOT EXECUTED for exception
            // write other codes
        }
        catch(...)
        {
            std::cout << "caught exception by delegate construction"<< std::endl;
        }
    private:
        DCExcept(int i, double d) : _type(i), _data(d)
    {
        std::cout << "target construction going to throw exception" << std::endl;
        throw 0;
    }
        int _type;
        double _data;
};

void delegateConstrucFuncTest()
{
    Info();
    Info(3);
    Info('A');

    std::vector<short> vec;
    TDConstructed td1 =  TDConstructed(vec);
    std::deque<int> deque;
    TDConstructed td2 =  TDConstructed(deque);

    DCExcept(1.2);
}

class HasPtrMem
{
    public:
        HasPtrMem() : d(new int(0))
        {
            std::cout << "Construct: " << ++_csn << std::endl;
        }
        HasPtrMem(const HasPtrMem &h) : d(new int(*h.d))
        {
            std::cout << "Copy Construct: " << ++_cpsn << std::endl;
        }
        HasPtrMem(HasPtrMem &&h) : d(h.d)
        {
            h.d = nullptr; // assign temp pointer mem to nullptr
            std::cout << "Move Construct: " << ++_msn << std::endl;
        }
        ~HasPtrMem()
        {
            std::cout << "Deconstruct: " << ++_dsn << std::endl;
        }

        static int _csn;
        static int _dsn;
        static int _cpsn;
        static int _msn;
        int *d;
};

int HasPtrMem::_csn = 0;
int HasPtrMem::_dsn = 0;
int HasPtrMem::_cpsn = 0;
int HasPtrMem::_msn = 0;
HasPtrMem getTemp() 
{ 
    HasPtrMem h;
    std::cout << "Resource from " << __func__ << ": " << std::hex << h.d << std::endl;
    return h;
}

void ptrMemCopyConstructTest()
{
    HasPtrMem a = getTemp();
    std::cout << "Resource from " << __func__ << ": " << std::hex << a.d << std::endl;
}

// LeftValue: addressable, named
// RightValue:  unaddressable, unmamed; xvalue(right-value refer related: T&& type return, std::move return) + prvalue(none-refer function return temp value; literal const, type convert return value, lambda express)
//
// const left value refer
struct Copyable
{
    Copyable(){}
    Copyable(const Copyable &o){std::cout << "Copy Construction" << std::endl;}
    Copyable(Copyable &&o){std::cout << "Move Construction" << std::endl;}
};

Copyable returnRValue(){return Copyable();}
void AcceptValue(Copyable){}
void AcceptRef(const Copyable &){}
void AcceptRvalueRef(Copyable &&){}

void constLeftValueRefTest()
{
    std::cout << "Pass by value(indirectly): " << std::endl;
    Copyable tmp = returnRValue();
    std::cout << "[Copyable]: " << std::is_reference<Copyable>::value << std::endl;
    std::cout << "[Copyable&]: " << std::is_lvalue_reference<Copyable&>::value << std::endl;
    std::cout << "[const Copyable&]: " << std::is_lvalue_reference<const Copyable&>::value << std::endl;
    std::cout << "[Copyable&&]: " << std::is_rvalue_reference<Copyable &&>::value << std::endl;
    AcceptValue(tmp); 
    std::cout << "Pass by value(directly): " << std::endl;
    AcceptValue(returnRValue()); // 
    std::cout << "Pass by referennce(indirectly): " << std::endl;
    const Copyable& tmpRef = returnRValue(); // must be const left refer(none-const will cause error)
    AcceptRef(tmpRef); // 
    std::cout << "Pass by referennce(directly): " << std::endl;
    AcceptRef(returnRValue()); // 
    // std::cout << "Pass by right-value reference(indirectly): " << std::endl;
    // Copyable&& tmpRvalueRef = returnRValue();
    // Copyable&& tmpRvalueRef = std::move(returnRValue());
    // AcceptRvalueRef(tmpRvalueRef); // 
    // Copyable tmpRvalue = returnRValue();
    // AcceptRvalueRef(tmpRvalue); // 
    // auto&& moveRvalue = std::move(tmpRvalue);
    // auto moveRvalue = std::move(tmpRvalue);
    // AcceptRvalueRef(moveRvalue); // 
    std::cout << "Pass by right-value reference(directly): " << std::endl;
    AcceptRvalueRef(returnRValue()); // 
}

class HugeMem
{
    public:
        HugeMem(int size) : nsize(size > 0 ? size : 1)
        {
            ptr = new int[nsize];
        }
        ~HugeMem(){ if(nullptr != ptr){delete []ptr;}}
        HugeMem(const HugeMem &rhs): nsize(rhs.nsize) 
        {
            ptr = new int[nsize];
            std::cout << "HugeMem Copy Construction" << std::endl;
        }

        HugeMem(HugeMem &&rhs): nsize(rhs.nsize),ptr(rhs.ptr)
        {
            rhs.ptr = nullptr;
            std::cout << "HugeMem Move Construction" << std::endl;
        }

        int *ptr = nullptr;
        int nsize{0};
};


class Moveable
{
    public:
        Moveable(): i(new int(3)), h(1024) {std::cout << "Moveable Common Construction" << std::endl;}
        ~Moveable() {delete i;}
        Moveable(const Moveable &rhs): i(new int(*rhs.i)), h(rhs.h) 
        {
            std::cout << "Moveable Copy Construction" << std::endl;
        }
        Moveable(Moveable && rhs): i(rhs.i), h(std::move(rhs.h))
        {
            rhs.i = nullptr;
            std::cout << "Moveable Move Construction" << std::endl;
        }
        int* i;
        HugeMem h;
};

Moveable getTempMoveable()
{
    Moveable tmp = Moveable();
    std::cout << std::hex << "Huge Mem from " << __func__ << " @" << tmp.h.ptr << std::endl;
    return tmp;
}


void moveTest()
{
    Moveable a;
    std::cout << "*a.i: " << *a.i << std::endl; // 
    Moveable b(a);
    std::cout << "*b.i: " << *b.i << std::endl; // 
    Moveable c(std::move(a));
    std::cout << "*c.i: " << *c.i << std::endl; // 
    // std::cout << *a.i << std::endl; // after move a, a is UB 

    Moveable d(getTempMoveable());
    std::cout << std::hex << "Huge Mem from " << __func__ << " @" << d.h.ptr << std::endl;
    std::cout << "Moveable is_move_constructible: " << std::is_move_constructible<Moveable>::value  << std::endl;
    std::cout << "HugeMem is_move_constructible: " << std::is_move_constructible<HugeMem>::value  << std::endl;
    std::cout << "Copyable is_move_constructible: " << std::is_move_constructible<Copyable>::value  << std::endl;
}

// C++11 copy/move construction function including 3 types:
// 1. T Object(T &)
// 2. T Object(const T &)
// 3. T Object(T &&)

//high performance swap function using move
template <class T>
void swap(T& a, T& b)
{
    T tmp(move(a));
    a = move(b);
    b = move(tmp);
}

// move noexcept
struct MayThrow
{
    MayThrow(){}
    MayThrow(const MayThrow&) { std::cout << "MayThrow Copy Construction" << std::endl; }
    MayThrow(MayThrow&&) { std::cout << "MayThrow Move Construction" << std::endl; }
};

struct NoThrow
{
    NoThrow(){}
    NoThrow(const NoThrow&) { std::cout << "NoThrow Copy Construction" << std::endl; }
    NoThrow(NoThrow&&) noexcept { std::cout << "NoThrow Move Construction" << std::endl; }
};

void moveNoExceptTest()
{
    MayThrow m;
    NoThrow n;
    MayThrow mt = std::move_if_noexcept(m); // copy
    NoThrow nt = std::move_if_noexcept(n); // move
}

// perfect forwarding
// transfer and target function type accept ability 
// 2 rules: 
// 1). template function type deduce:  real argument (X) type: left-value X-> X&; right-value X -> X;
// 2). reference collapsing: if only including left-value ref -> left-value ref otherwise right-value ref -> right-value ref
// move and forward both just static_cast: move always do but forward only do if argument is right-value-ref
// RVO,NRVO(named right value optimization), satisfy 3 basic rules all:
// 1) local temp
// 2) local temp type is the same as return type
// 3) local temp value is just the to be returned value
// such as: std::move(local_temp) NOT 3) ; input formal parameter NOT 1)

void runCode(int &&m){std::cout << "rvalue reference" << std::endl;}
void runCode(int &m){std::cout << "lvalue reference" << std::endl;}
void runCode(const int &&m){std::cout << "const rvalue reference" << std::endl;}
void runCode(const int &m){std::cout << "const lvalue reference" << std::endl;}

template <typename T>
void perfectForwardingImpl(T&& t) { runCode(std::forward<T>(t));}

void runCode1(int &&m){std::cout << "rvalue reference" << std::endl;}
void runCode2(int &m){std::cout << "lvalue reference" << std::endl;} 
template <typename T, typename U>
void perfectForwardingImpl2(T&& t, U& Func) 
{ 
    std::cout << t << "\tforwarded..." << std::endl;
    Func(std::forward<T>(t));
}

void perfectForwardingTest()
{
    int a;
    int b;
    const int c = 1;
    const int d = 3;

    perfectForwardingImpl(a); // lvalue ref
    perfectForwardingImpl(std::move(b)); // rvalue ref
    perfectForwardingImpl(c); // const lvalue ref
    perfectForwardingImpl(std::move(d)); // const rvalue ref

    perfectForwardingImpl2(3, runCode1); // rvalue ref
    perfectForwardingImpl2(std::move(a), runCode1); // rvalue ref
    perfectForwardingImpl2(a, runCode2);// lvalue ref
    perfectForwardingImpl2(const_cast<int&>(c), runCode2);// lvalue ref
}

void rightValueRefTest()
{
    // ptrMemCopyConstructTest();
    // constLeftValueRefTest();
    // moveTest();
    // moveNoExceptTest();
    perfectForwardingTest();
}

// explicitOperator
struct Rational1
{
    Rational1(int n = 0, int d = 1) : num(n), den(d)
    {
        std::cout << "[" << num << "/" << den << "]" <<std::endl;
    }
    int num{0};
    int den{0};
};

struct Rational2
{
    explicit Rational2(int n = 0, int d = 1) : num(n), den(d)
    {
        std::cout << "[" << num << "/" << den << "]" <<std::endl;
    }
    int num{0};
    int den{0};
};
void display1(Rational1 r1)
{
    std::cout << "Rational1 Num:" << r1.num << "Rational1 Den:" << r1.den << std::endl;
}

void display2(Rational2 r2)
{
    std::cout << "Rational2 Num:" << r2.num << "Rational2 Den:" << r2.den << std::endl;
}

void basicExplicitTest()
{
    Rational1 r1_1 = 11;
    Rational1 r1_2(12);

    // Rational2 r2_1 = 21;  // implicit transition failed and  compile error using explicit
    Rational2 r2_2(22);

    display1(1);
    display1(Rational1(1));
    // display2(1);         // implicit transition failed and  compile error using explicit
    display2(Rational2(1));
}


template <typename T>
class Ptr
{
    public:
        Ptr(T* p) :_p(p){}
        operator bool() const 
        {
            if(0 != _p)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    private:
        T* _p;
};

void semanticsLessImplicitTransTest()
{
    int a;
    Ptr<int> pi (&a); // pi is 1
    if(pi){std::cout << "valid pointer: "  << pi << std::endl;}
    else {std::cout << "invalid pointer: " << pi << std::endl;}
    double d;
    Ptr<double>  pd(&d);  // pd is 1
    std::cout << pi + pd << std::endl;  // result is 1 + 1 = 2 but meaningless
}

class ConvertTo {};
class Convertable
{
    public:
        explicit operator ConvertTo() const {return ConvertTo();}
};
void Func(ConvertTo to){}

// copy construct and none-explicit type transition is forbidded
void explicitConstructTest()
{
    Convertable c;
    ConvertTo ct(c);
    // ConvertTo ct2 = c;  // copy construct forbidded and compile error
    ConvertTo ct3 = static_cast<ConvertTo>(c);
    // Func(c); // copy construct forbidded and compile error
}


void explicitOperatorTest()
{
    basicExplicitTest();
    semanticsLessImplicitTransTest();
    explicitConstructTest();
}

// initializer list
void stardardLibInitializeListTest()
{
    int a[] = {1,2,3}; // c++98 and c++11 both ok
    int b[] {1,2,3}; // c++98 error but c++11 ok
    std::vector<int> c{1,2,3}; // c++98 error but c++11 ok
    std::map<int, float> m = {{1,1.0},{2,2.0},{3,3.0}}; // c++98 error but c++11 ok

    // four kinds of initialize
    int d1 =  3 + 6;
    int d2 = (3 + 6);
    int d3 = {3 + 6};
    int d4  (3 + 6);
    int d5  {3 + 6};

    int* pi = new int(1);
    double* pd = new double{3.3};
    delete pi;
    delete pd;
}

enum Gender {boy, girl};
class Person
{
    public:
        Person(std::initializer_list<std::pair<std::string, Gender>> il)
        {
            for(auto i = il.begin(); i != il.end(); ++i)
            {
                _data.push_back(*i);
            }
        }
        void Print()
        {
            for(auto &d : _data)
            {
                std::cout << d.first << ": " <<d.second << std::endl;
            }
        }

    private:
        std::vector<std::pair<std::string, Gender>> _data;
};
void FuncInitializeList(std::initializer_list<int> il)
{
    for(auto i = il.begin(); i != il.end(); ++i)
    {
        std::cout << "func_initializer_list: " << *i << std::endl;
    }
}

class MyData
{
    public:
        MyData& operator [](std::initializer_list<int> il)
        {
            for(auto i = il.begin(); i != il.end(); ++i)
            {
                idx.push_back(*i);
            }
            return *this;
        }

        MyData& operator = (int v)
        {
            if(idx.empty() != true)
            {
                for(auto i = idx.begin(); i != idx.end(); ++i)
                {
                    data.resize((*i > data.size()) ? *i : data.size());
                    data[*i - 1] = v;
                }
                idx.clear();
            }

            return *this;
        }
        void Print()
        {
            for(auto &d : data)
            {
                std::cout << d << " ";
            }
            std::cout << std::endl;
        }

    private:
        std::vector<int> idx; // auxiliary array
        std::vector<int> data;
};

std::vector<int> getList1(){return {1,2,3};}
std::deque<int>  getList2(){return {1,2,3};}
const std::list<int>  getList3(){return {1,2,3};}

void customDefineTypeInitializeListTest()
{
    Person persons = {{"lilei",boy}, {"hanmeimei",girl}, {"frank",boy}};
    persons.Print();

    FuncInitializeList({1,2,3});
    FuncInitializeList({});

    MyData d;
    d[{2,3,5}] = 7;
    d.Print();
    d[{1,4,5,8}] = 3;
    d.Print();

    for(auto & l : getList1())
    {
        std::cout << l << " ";
    }
    std::cout << std::endl;

    for(auto & l : getList2())
    {
        std::cout << l << " ";
    }
    std::cout << std::endl;

    for(auto & l : getList3())
    {
        std::cout << l << " ";
    }
    std::cout << std::endl;
}

//initializer_list can prevent type from narrowing
// narrowing: converted type data can not resume to orgin type data
void initializerListPreventNarrowingTest()
{
    const int x  = 1024;
    const int y  = 125;

    char a = x; // narrowing ,ok
    // char aa = {x}; // narrowing, error

    char* b = new char(1024); // narrowing ,ok

    unsigned char c = -1; // narrowing, ok
    // unsigned char cc {-1}; // narrowing, error

    float f {7};  // ok

    int g = 7.0f;  // narrowing, ok
    // int gg {7.0f};  // narrowing, error
}

void initializerlistTest()
{
    stardardLibInitializeListTest();
    customDefineTypeInitializeListTest();
    initializerListPreventNarrowingTest();
}

// POD (Plain Old Data)
// Trivial and Standard Layout both meets is POD
//
// 1. Trivial
// 1) trial default constructor and destructor
// 2) trial copy constructor and move constructor
// 3) trial assignment operator and move operator
// 4) no including virtual function and virtual base
struct Trivial1 {};
struct Trivial2 {public: int a; private: int b;};
struct Trivial3 {Trivial1 a; Trivial2 b;};
struct Trivial4 {Trivial2 a[33];};
struct Trivial5 {int x; static int y;};
struct NoTrivial1 {NoTrivial1(): z(42){} int z;}; // 1)
struct NoTrivial2 {NoTrivial2(); int w;}; // 1)
NoTrivial2::NoTrivial2() = default;
struct NoTrivial3 {virtual void f();}; // 4)

void trivialTest()
{
    std::cout << "Trivial1: " << std::is_trivial<Trivial1>::value << std::endl;
    std::cout << "Trivial2: " << std::is_trivial<Trivial2>::value << std::endl;
    std::cout << "Trivial3: " << std::is_trivial<Trivial3>::value << std::endl;
    std::cout << "Trivial4: " << std::is_trivial<Trivial4>::value << std::endl;
    std::cout << "Trivial5: " << std::is_trivial<Trivial5>::value << std::endl;
    std::cout << "NoTrivial1: " << std::is_trivial<NoTrivial1>::value << std::endl;
    std::cout << "NoTrivial2: " << std::is_trivial<NoTrivial2>::value << std::endl;
    std::cout << "NoTrivial3: " << std::is_trivial<NoTrivial3>::value << std::endl;
}

// 2. Standard Layout
// 1) all none-static member have the same access authority(private, protected, public)
// 2) class or struct inherit(none-static member both exists in base and derive class; none-static member exists in multi-base MEANS none standard layout): 
//    2.1) derive class have none-static member AND have only one parent class whoes member is all static;
//    2.2) derive class does NOT have none-static member AND parent class have none-static memeber
// 3) derive class 's first none-static member type is different from its parent class
// 4) no including virtual function and virtual base
// 5) all none-static members and its parent classes are standard layout( recursion definition)

struct SLayout1 {};
struct SLayout2 {private: int x; int y;};
struct SLayout3 : SLayout1 {int x; int y; void fn();};
struct SLayout4 : SLayout1 {int x; SLayout1 y;};
struct SLayout5 : SLayout1, SLayout3 {};
struct SLayout6 {static int y;};
struct SLayout7 : SLayout6 {int x; };
struct NonSLayout1 : SLayout1 {SLayout1 x; int i; }; // 3) 
struct NonSLayout2 : SLayout2 {int i; }; // 2.1)
struct NonSLayout3 : NonSLayout2 {}; // 5)
struct NonSLayout4 {private: int x; public: int y;}; // 1)
struct NonSLayout5 {virtual void fn();}; // 1)

void standardLayoutTest()
{
    std::cout << "SLayout1 : " << std::is_standard_layout<SLayout1>::value << std::endl;
    std::cout << "SLayout2 : " << std::is_standard_layout<SLayout2>::value << std::endl;
    std::cout << "SLayout3 : " << std::is_standard_layout<SLayout3>::value << std::endl;
    std::cout << "SLayout4 : " << std::is_standard_layout<SLayout4>::value << std::endl;
    std::cout << "SLayout5 : " << std::is_standard_layout<SLayout5>::value << std::endl;
    std::cout << "SLayout6 : " << std::is_standard_layout<SLayout6>::value << std::endl;
    std::cout << "SLayout7 : " << std::is_standard_layout<SLayout7>::value << std::endl;
    std::cout << "NonSLayout1 : " << std::is_standard_layout<NonSLayout1>::value << std::endl;
    std::cout << "NonSLayout2 : " << std::is_standard_layout<NonSLayout2>::value << std::endl;
    std::cout << "NonSLayout3 : " << std::is_standard_layout<NonSLayout3>::value << std::endl;
    std::cout << "NonSLayout4 : " << std::is_standard_layout<NonSLayout4>::value << std::endl;
    std::cout << "NonSLayout5 : " << std::is_standard_layout<NonSLayout5>::value << std::endl;
}

union U{};
union U1 {U1(){}};
enum E1{};
typedef double* DA;
typedef void (*PF)(int, double);
// !!! std::is_pod is abandoned since C++20
void isPodTest()
{
    std::cout << std::is_pod<U>::value << std::endl;
    std::cout << std::is_pod<U1>::value << std::endl;
    std::cout << std::is_pod<E1>::value << std::endl;
    std::cout << std::is_pod<int>::value << std::endl;
    std::cout << std::is_pod<DA>::value << std::endl;
    std::cout << std::is_pod<PF>::value << std::endl;
}

// benefit of POD:
// 1. bytes assignment: memset and memcpy safe
// 2. compatible with C memeory layout and C++/C inter operation
// 3. static initialization safe (static initialization improve safe and performance)
void podTest()
{
    trivialTest();
    standardLayoutTest();
    isPodTest();
}

struct Student
{
    Student(bool g, int a) : gender(g), age(a) {}
    bool gender;
    int age;
};

// all none-reference type can be a data member in a union type
// static function member is allowed but static data member is forbidded in a union type
union T
{
    Student s; // C++98 error but C++11 ok (Student class have none-trivial construction so is NOT POD)
    int id;
    char name[12];
    static int getA() {return 33;}; // static function is allowed
};

// T t;  // default construction of T is deleted implicitly bz of Student is NOT POD 

union T2
{
    int x;
    double d;
    char b[sizeof(double)];
};
T2 t2 = {0};  // all bytes of T2 is initialized by 0 NOT only x

union T3
{
    std::string s; // s has none-trivial construction function
    int n;
    // custom construction and destruction function
public:
    T3() { new (&s) std::string;}
    // ~T3(){ s.~string();}

};

// T3 t3;  // default construction of T is deleted implicitly bz of string is NOT POD 
// ADD custom construction and destruction function

// union-like class (unnamed unrestricted union used in class declare)
class Singer
{
    public:
        enum Type{ STUDENT, NATIVE, FOREIGNER};
        Singer(bool g, int a) :s(g,a) {t = STUDENT;}  // use s only
        Singer(int i): id(i) {t = NATIVE;}  // use id only
        Singer(const char* str, int n)  // use name only
        {
            int size = (n > 12)? 12 : n;
            memcpy(name, str, size);
            name[n] = '\0';
            t = FOREIGNER;
        }
        ~Singer() {}
        void display()
        {
            switch(t)
            {
                case STUDENT:
                    std::cout << "[STUDENT] gender: " << s.gender << ", age: " << s.age << std::endl;
                    break;
                case NATIVE:
                    std::cout << "[NATIVE] id: " << id << std::endl;
                    break;
                case FOREIGNER:
                    std::cout << "[FOREIGNER] name: " << name << std::endl;
                    break;
                default:
                    std::cout << "[INVALID TYPE] " << name << std::endl;
                    break;
            }
        }

    private:
        Type t;
        union{   // unnamed unrestricted union
            Student s;
            int id;
            char name[13];
        };
};

void unrestrictedUnionTest()
{
    auto s1 = Singer(true, 13);
    s1.display();
    auto s2 = Singer(12345);
    s2.display();
    auto s3 = Singer("Frank", 5);
    s3.display();
}

// custom define literal

typedef unsigned char uint8;
struct RGBA
{
    uint8 _r;
    uint8 _g;
    uint8 _b;
    uint8 _a;

    RGBA(uint8 R, uint8 G, uint8 B, uint8 A = 0) : _r(R), _g(G), _b(B),_a(A){}
};

std::ostream & operator << (std::ostream & out, RGBA &color)
{
    return std::cout << "  r: " << (int)color._r
                     << ", g: " << (int)color._g
                     << ", b: " << (int)color._b
                     << ", a: " << (int)color._a << std::endl;
}

// right-value reference
void blend(RGBA &&color1, RGBA &&color2)
{
    std::cout << "blend: " << std::endl << color1 << color2 << std::endl;
}

RGBA operator "" _C(const char* color, size_t n)  // literal type is string
{
    const char* p = color;
    const char* end = color + n;
    const char* r, *g, *b, *a;
    r = g = b = a = nullptr;

    // "rXXX gXXX bXXX"_C format
    //  parse string end with "_C" 
    for(; p != end; ++p)
    {
        if(*p == 'r') r = p;
        else if(*p == 'g') g = p;
        else if(*p == 'b') b = p;
        else if(*p == 'a') a = p;
    }

    if((nullptr == r) || (nullptr == g) || (nullptr == b))
    {
        throw 0;
    }
    else if(nullptr == a)
    {
        return RGBA(atoi(r+1), atoi(g+1), atoi(b+1));
    }
    else
    {
        return RGBA(atoi(r+1), atoi(g+1), atoi(b+1), atoi(a+1));
    }
}

struct Watt 
{
    unsigned int v;
    void display()
    {
        std::cout << v << "W" << std::endl;
    }
};

Watt operator "" _W(unsigned long long w)  // literal type is integral 
{
    return {(unsigned int)w};
}

// literal type rules, format: ReturnType operator "" _XXX(paramter type YYY) [between "" _XXX HAS a Space]
// 1. "" is integer type,  YYY: unsigned long long OR const char* 
// 2. "" is float type,    YYY: long double OR const char* 
// 3. "" is string type,   YYY: const char* , size_t
// 4. "" is char type,     YYY: char

void customDefineLiteralTest()
{
    blend("r255 g233 b212"_C, "r123 g87 b44 a7"_C);

    Watt wat = 1024_W;
    wat.display();
}

// inline namespace
// allow parent namespace define or specialize sub namespace template and broken sub namespace's encapsulation

namespace Frank
{
#if __cplusplus == 201103L
    inline   // inline namespace
#endif
        namespace cpp
        {
#if __cplusplus == 201103L
            struct Work { Work() { std::cout << "Work in C++11" << std::endl; } };
#else
            struct Work { Work() { std::cout << "Work in C++98/03" << std::endl; } };
#endif
        }
}

using namespace Frank;
void inlineNamespaceTest()
{
    cpp::Work b;  // use cpp11 version mandatory
}

// template alias
// using and typedef and C++11 using keyword is more powerful

// template alias instantiation by using keyword
template<typename T>
using MapString = std::map<T, char*>;

void usingTypedefAliasTest()
{
    using uint = unsigned int;
    using sint = int;
    typedef unsigned int UINT;
    typedef int SINT;
    std::cout << " unit UNIT is the same: " << std::is_same<uint, UINT>::value << std::endl;
    std::cout << " snit SNIT is the same: " << std::is_same<sint, SINT>::value << std::endl;

    MapString<int> numberedStringMap;
    MapString<long> numberedStringMap2;
}

// SFINAE(Substitution Failure Is Not An Error)
//
struct Test { typedef int foo; };

template <typename T>
void func1(typename T::foo) {}  // first template definition

template <typename T>     
void func1(T) {}                // second template definition

template <int I> struct A{ void display() {std::cout << "tempate parameter is " << I << std::endl;}};
template <typename T>
A<sizeof((T)0)> f(T){};  // return a expression tempate class(struct)

void SFINAETest()
{
    func1<Test>(10); // first template match
    func1<int>(10);  // second template match  NO error bz of SFINAE(first template match failed)

    short int si = 1;
    auto a  = f(si);
    a.display();
    auto b  = f(1ll);
    b.display();
    auto c = f(3.0f);
    c.display();
    auto d  = f(2);
    d.display();
}

int main(int argc, char** argv)
{
    // deriveConstructFuncTest();
    // delegateConstrucFuncTest();
    // rightValueRefTest();
    // explicitOperatorTest();
    // initializerlistTest();
    // podTest();
    // unrestrictedUnionTest();
    // customDefineLiteralTest();
    // inlineNamespaceTest();
    // usingTypedefAliasTest();
    SFINAETest();

    return 0;
}