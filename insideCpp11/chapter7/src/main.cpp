#include<iostream>
#include<vector>
#include<algorithm>

// nullptr
// 0 && NULL && (void*)0 && nullptr
//
// NULL is a macro defination like below(stddef.h):
/*
 * 
 * #undef NULL
 * #if defined(__cplusplus)
 * #define NULL 0  // literal const 0
 * #else
 * #define NULl ((void*)0)  // none-type pointer const
 * #endif
 */
// nullptr:  empty pointer type(nullptr_t) const , a rvalue pointer const
// nullptr_t: the type of nullptr
// nullptr_t rules:
// 1. all nullptr_t type var is equality and the same behavior
// 2. nullptr_t type can convert to any pointer type implicitly
// 3. nullptr_t type can NOT convert to none-pointer type even using reinterpret_cast
// 4. nullptr_t type arithmetic expression is NOT allowed
// 5. nullptr_t type relationship expression is allowed only with another nullptr_t type or pointer type //GCC UNALLOWED
//

void func(char* c)
{
    std::cout << "invoke func(char*)" << std::endl;
}

void func(int i)
{
    std::cout << "invoke func(int)" << std::endl;
}

// template deduce nullptr to a normal type NOT a pointer type
//
template<typename T> void g(T* t) {}
template<typename T> void h(T t) {}

void nullptrTest()
{
    func(0);   // invoke int
    // func(NULL);  // error: calling is  ambiguous (NULL --> __null inner transition in GCC)
    func((char*)0); // invoke char*
    func(nullptr); // invoke char*

    // nullptr_t rules test
    char *p = nullptr;  // 2 allowed

    // int n1 = nullptr; // 3 error: unallowed
    // int n2 = reinterpret_cast<int>(nullptr); // 3 error: unallowed

    std::nullptr_t nptr;
    if(nptr == nullptr)
    {
        std::cout << "nullptr_t nptr  == nullptr" << std::endl;
    }
    // else if( nptr < nullptr)
    // {
    // std::cout << "nullptr_t nptr  < nullptr" << std::endl;
    // }
    // else if(nptr > nullptr)
    // {
    // std::cout << "nullptr_t nptr  > nullptr" << std::endl;
    // }
    else
    {
        std::cout << "nullptr_t nptr  != nullptr" << std::endl;
    }

    // if( 0 == nullptr)  // 3. error: unallowed 
    // if(nullptr) // 3. error: unallowed
    // nullptr += 1; // 4. error: unallowed;

    std::cout << "sizeof(nullptr) = " << sizeof(nullptr) << std::endl;;
    std::cout << "typeif(nullptr) = " << typeid(nullptr).name() << std::endl;;
    // throw(nullptr); ok

    // nullptr template deduce
    // g(nullptr);  error: nullptr is NOT a point type at deduce
    g((float*)nullptr);

    h(0);
    h(nullptr);
    h((float*)nullptr);

    // int* px = (void*)0; // error: rvalue bind to a pointer is NOT allowed
    int* py = nullptr;

    // nullptr: rvalue 
    std::cout << &nptr << std::endl; // ok: nptr is a lvalu
    // std::cout << &nullptr << std::endl;  // error:  rvalue can't be taken address
    const std::nullptr_t && d_nptr = nullptr;
    std::cout << &d_nptr << std::endl; // ok: d_nptr is a rvalue ref
}

// = default and = deleted 
// default function:
// 1. constructon function
// 2. copy construction function
// 3. copy assignment function (operator =)
// 4. move construction function
// 5. move copy function
// 6. deconstruction function
// default operator:
// 1. operator ,
// 2. operator &
// 3. operator &&
// 4. operator *
// 5. operator ->
// 6. operator ->*
// 7. operator new
// 8. operator delete


class twoCstor
{
    public:
        twoCstor() = default;
        twoCstor(int i) :  _i(i) {}
    private:
        int _i{0};
};

// delete copy construction
// outside default
class noCopyable
{
    public:
        noCopyable() = default;
        noCopyable(const noCopyable &) = delete;
    // old style(add private scope)
    // private:
        // noCopyable(const noCopyable &); 

        // no default keyword but move to outside of classs defination
        noCopyable & operator = (const noCopyable &);
};

// default keyword used outside class defination
inline noCopyable & noCopyable::operator = (const noCopyable &) = default;

class convertType
{
    public:
        convertType(int i) {}
        convertType(char c)  = delete; // delete char version
};

class convertTypeV2
{
    public:
        convertTypeV2(int i) {}
        explicit convertTypeV2(char c)  = delete; // delete with explicit: [!!!NO RECOMMENDED]
};


// delete class memeber function (none with explicit)
void convertFunc(convertType ct) {}

// delete class memeber function (with explicit)
void convertFuncV2(convertTypeV2 ct) {}

// delete common function 
void func1(int i) {}
void func1(char c) = delete;

// delete new operator
class noHeapAlloc
{
    public:
        void* operator new(std::size_t) = delete;
};

// delete deconstruction with place new: alloc memeory in the specified memory position
// when a object which is constructed by placement new, then the compiler will not invoke its deconstruction function 
//

char p[30] = {0};  // placement new specified memory position
class placeNewAllocWithoutDeconstruction
{
    public:
        ~placeNewAllocWithoutDeconstruction() = delete;
};

void defaultAndDeletedTest()
{
    std::cout << "two Cstor ispod? " << std::is_pod<twoCstor>::value << std::endl;
    noCopyable a;
    // noCopyable b(a); // error
    // noCopyable c = a; // error

    convertFunc(3);
    // convertFunc('a'); // error: char version is deleted
    convertType ci(3);
    // convertType cc('a'); // error: char version is deleted

    // delete with explicit
    convertFuncV2(3);
    convertFuncV2('a'); // ok: char -> int first 
    convertTypeV2 ci2(3);
    // convertTypeV2 cc2('a'); // error: char version is deleted

    // common function with delete
    func1(3);
    // func1('a'); // error:  char version is deleted

    noHeapAlloc nha;
    // noHeapAlloc* p = new noHeapAlloc(); // error: new operator is deleted

    // placeNewAllocWithoutDeconstruction pnwd; // error: unable to construct normally for deconstruciton is deleted
    new (p) placeNewAllocWithoutDeconstruction(); // placement new construct new object in p memory
}

// lambda function
// [capture](parameters) mutable -> return-type { statement}
// 1. default: const , lambda function is a functor with const operator() by default 
// 2. parameters and return-type are optional 
// 3. capture: by value (const) and by reference(can be mutable)
// 3.1: [var]  captured var by value
// 3.2: [=]    captured all parent-scope vars by value
// 3.3: [&var] captured value by ref
// 3.4: [&]    captured all parent-scope vars by ref
// 3.5: [this] captured current this pointer by value
// lambda is a syntactic sugar of  functor
// lambda like a local function(nested function) and more simpler than outside static inline function definition
// lambda expression type is called closure-class-typed NOT a function pointer but none-captured lamdba expression can convert to a function pointer but NOT vice versa
// every lambda expression will generate a closure-class-typed temperate object (rvalue)
// lambda expression limitation: NOT capture scope that out of parent's scope range

class Functor
{
    public:
        int operator() (int x, int y) { return x + y;}
};

class Tax
{
    public:
        Tax(float r, int b) : _rate(r), _base(b){}
        float operator() (float money) {return (money - _base) * _rate; }
    private:
        float _rate;
        int _base;
};

int externFunc(int a){ return a;};

std::vector<int> vecs;
std::vector<int> largeVecs;
const int bound = 10;
inline void largeNum(int i)
{
    if( i > bound)
    {
        largeVecs.push_back(i);
    }
}

class LargeNumFunctor
{
    public:
        LargeNumFunctor(int u): _bound(u){}
        void operator() (int i) const
        {
            if( i > _bound)
            {
                largeVecs.push_back(i);
            }
        }
    private:
        int _bound{0};
};

int static_d = 0;
void lambdaFunctionTest()
{
    []{}; // simpleset lambda expression;
    int a = 3, b = 33;
    [=] { return a + b;};  // return-type is omitted and deduced by compiler
    auto f1 = [&](int c) { b = a + c;}; // captured by refer and can be mutable
    auto f2 = [=, &b](int c) -> int { return b += a + c;}; // complete lambda expression

    int boy = 4, girl = 3;
    auto child = [&boy, girl]() -> int {return boy + girl;};
    child();

    // common functor
    Functor functor;
    functor(boy, girl);

    // function with state version
    float tax_rate1 = 0.40;
    float tax_rate2 = 0.30;
    int base1 = 3000;
    int base2 = 20000;
    Tax high(tax_rate1, base1);
    Tax middle(tax_rate2, base2);
    std::cout << "tax over 30k: " << high(38000) << std::endl;
    std::cout << "tax over 20k: " << middle(28000) << std::endl;

    // lambda expression version
    auto highFunctor   = [=](float money) -> float {return (money - base1) * tax_rate1;};
    auto middleFunctor = [=](float money) -> float {return (money - base2) * tax_rate2;};
    std::cout << "tax over 30k: " << highFunctor(38000) << std::endl;
    std::cout << "tax over 20k: " << middleFunctor(28000) << std::endl;

    int x = 0;
    const int times = 5;
    try
    {
        for(int i = 0; i < times; i++)
            x += externFunc(i);
    }
    catch(...)
    {
        x = 0;
    }
    std::cout << "common lambda sum: " << x << std::endl;

    const int y = [=] 
    {
        int  val = 0;
        try
        {
            for(int i = 0; i < times; i++)
                val += externFunc(i);
        }
        catch(...)
        {
            val = 0;
        }
        return val;
    }();// lambda function definition and run;
    std::cout << "lambda sum version: " << y << std::endl;

    // lambda difference between captured by value and by reference
    // by value:         const and decided when lambda function is defined;
    // by reference: NOT const and decided when lambda function is invoked;

    int j = 33;
    auto by_val_lambda = [=] { return j+ 1;}; // lambda definition
    auto by_ref_lambda = [&] { return j+ 1;}; // lambda definition
    std::cout << "by_val_lambda: " << by_val_lambda() << std::endl; // lambda invoked
    std::cout << "by_ref_lambda: " << by_ref_lambda() << std::endl; // lambda invoked

    j++;
    std::cout << "by_val_lambda after ++: " << by_val_lambda() << std::endl; // lambda invoked
    std::cout << "by_ref_lambda after ++: " << by_ref_lambda() << std::endl; // lambda invoked

    // lamdba function and function pointer
    int z = 0;
    auto lambda1 = [](int x , int y) -> int {return x + y;};
    auto lambda2 = [z](int x , int y) -> int {return x + y + z;};
    typedef int (*func1)(int x, int y);
    typedef int (*func2)(int x);

    func1 p;
    p = lambda1; // ok: none-captured lambda can convert to function pointer

    // p = lambda2; // error: captured lambda CAN NOT convert to function pointer

    func2 p3;
    // p3 = lambda1;  // error: function paramter should be the same

    decltype(lambda1) lambda3 = lambda1;
    // decltype(lambda1) lambda4 = p; // error: function pointer CAN NOT convert to lambda

    // lambda const and mutable
    int m = 0;
    // auto const_val_lambda = [=]() { m = 3;};  // error:  default is const like const opertor() [const functor]
    auto mutable_val_lambda = [=]() mutable { m = 3;}; // ok, using mutable

    auto const_ref_lambda = [&] { m = 33;}; // ok, reference

    auto mutable_ref_lambda = [&]() mutable { m = 333;}; // ok

    auto const_param_lambda = [&](int v) { v = 3333;};

    // decided when lambda declare
    mutable_val_lambda();
    std::cout << "mutable by value lambda: " << m << std::endl;  // 0

    // decided when lambda invoked
    const_ref_lambda();
    std::cout << "const by reference lambda: " << m << std::endl;// 33

    // decided when lambda invoked
    mutable_ref_lambda();
    std::cout << "mutable by reference lambda: " << m << std::endl; // 333

    // decided when lambda invoked
    const_param_lambda(m);
    std::cout << "const param lambda: " << m << std::endl; // 3333

    // foreach and lambda

    vecs.clear();
    vecs.push_back(1);
    vecs.push_back(10);
    vecs.push_back(100);
    vecs.push_back(120);
    // verson 1: old traditional for loop
    largeVecs.clear();
    for(auto &e : vecs)
    {
        if(e > bound)
        {
            largeVecs.push_back(e);
        }
    }

    std::cout << "tradition loop: ";
    for(auto &e : largeVecs)
    {
        std::cout <<  e << '\t';
    }
    std::cout << std::endl;


    // verson 2: for loop using function pointer
    largeVecs.clear();
    for_each(vecs.begin(), vecs.end(), largeNum);

    std::cout << "function pointer loop: ";
    for(auto &e : largeVecs)
    {
        std::cout <<  e << '\t';
    }
    std::cout << std::endl;


    // verson 3: for loop using functor
    largeVecs.clear();
    for_each(vecs.begin(), vecs.end(), LargeNumFunctor(bound));

    std::cout << "functor loop: ";
    for(auto &e : largeVecs)
    {
        std::cout <<  e << '\t';
    }
    std::cout << std::endl;

    // verson 4: for loop using lambda expression
    largeVecs.clear();
    for_each(vecs.begin(), vecs.end(), [=](int i)
            {
                if( i > bound)
                {
                    largeVecs.push_back(i);
                }
            });

    std::cout << "lambda loop: ";
    for(auto &e : largeVecs)
    {
        std::cout <<  e << '\t';
    }
    std::cout << std::endl;

    // CAN NOT capture out of parent's scope range

    // auto illegal_lambda = [static_d]{}; // error: captured out of parent's scope range
    LargeNumFunctor lnf(static_d); // ok, functor can capture any scope range
}

int main(int argc, char** argv)
{
    // nullptrTest();
    // defaultAndDeletedTest();
    lambdaFunctionTest();

    return 0;
}
