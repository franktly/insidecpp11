#include <bits/c++config.h>
#include <iostream>
#include <cstdarg>
#include <cassert>
#include <tuple>
#include <atomic>
#include <thread>
#include <pthread.h>

// constexpr
// runtime const: const, compile-time const: constexpr
//
//constexpr function rules:
//1. function body must have only single one return statement
//2. function must have a return value(Not void)
//3. function must have its declare and defination before use
//4. return body expression must also be a const-expression(Not none-const-expression or global data)

const int getConst() {return 1;}
constexpr int getConstV2() {return 1;}
enum bitSet
{
    V0 = 1 << 0,
    V1 = 1 << 1,
    V2 = 1 << 2,
    VMAX = 1 << 3
};

// overload operator | to ensure return value less than VMAX
// const bitSet operator | (bitSet x, bitSet y)
constexpr bitSet operator | (bitSet x, bitSet y)
{
    return static_cast<bitSet>(((int)x | y) & (VMAX - 1));
}

// template <int i = V0 | V1>  // (const) error: template param default value should be clear at compile-time 
template <int i = V0 | V1>  // (constexpr) ok
void linkConst(){}

// constexpr function
//
// constexpr int f1() {const int i = 1; return i;}    // error: 1
constexpr int f2(int x) {static_assert(0 == 0, "assert failed!!!"); return x;}  // none-generated expression ignored(using, typedef, xxx_assert and so on)
// constexpr void f3() {}   // error: 2

constexpr int f4();
int fa = f4();
const int fb = f4();
// constexpr int fc = f4();  // error: 3 
constexpr int f4() {return 1;}
constexpr int fd = f4();

constexpr int f5();
// int f5();   // error: conflict with constexpr type of f5 
// const int f66(){return 1; }
// constexpr int f6() {return  f66();} // error: 4

// int fg = 3;
// constexpr int f7() {return fg;} // error: 4

struct Date
{
    constexpr Date(int y, int m, int d) : year(y), month(m), day(d){}  // constexpr construction
    constexpr int getYear() const {return year;}    // constexpr member function, const must also be appended
    constexpr int getMonth() const {return month;}
    constexpr int getDay() const {return day;}
private:
    int year;
    int month;
    int day;
};

// constexpr compile-time run calculation
constexpr int fibonacci(int n)
{
    return (n == 1) ? 1 : ((n == 2)? 1 : (fibonacci( n - 1) +  fibonacci( n - 2)));
}
// another compile-time calculation using template meta-programming
template<long num>
struct fibo
{
    static const long val = fibo<num - 1>::val + fibo<num - 2>::val;
};

template<> struct fibo<2> {static const long val = 1;};
template<> struct fibo<1> {static const long val = 1;};
template<> struct fibo<0> {static const long val = 0;};

void constexprTest()
{
    // constexpr varaible and compile-time 
    // int arr[getConst()] = {0};      //  error: array size should be clear at compile-time
    int arr[getConstV2()] = {0};      //  ok
    // enum { e1 = getConst(), e2 };   // error: member value should be clear at compile-time as well
    enum { e1 = getConstV2(), e2 };   // ok
    int type  = 0;
    switch(type)
    {
        // case getConst():  // error: expression value should be clear at compile-time too
        case getConstV2():  // ok
            break;
        default:
            break;
    }

    // constexpr struct myType {int i; };  // struct can't be marked constexpr
    struct myType { int i; };  
    constexpr myType mt = {0};

    constexpr Date birthDay {1989, 1, 2};
    constexpr int by = birthDay.getYear();
    constexpr int bm = birthDay.getMonth();
    constexpr int bd = birthDay.getDay();
    std::cout << "birthday: "  <<  by << " year " << bm << " month " << bd  << " day"<< std::endl;

    // constexpr compile-time run calculation
    int fib[] = {fibonacci(11), fibonacci(12), fibonacci(13)};
    std::cout << "fibonacci(11,12,13) version1(constexpr): ";
    for(auto &f : fib)
    {
        std::cout << f << '\t';
    }
    std::cout << std::endl;

    // template meta-programming compile-time run calculation
    int fibv2[] = {fibo<11>::val, fibo<12>::val, fibo<13>::val};
    std::cout << "fibonacci(11,12,13) version2(meta programming): ";
    for(auto &f : fibv2)
    {
        std::cout << f << '\t';
    }
    std::cout << std::endl;
}

// variadic template
// C++98 variadic function
double sumOfFloat(int count, ...)
{
    va_list ap;
    double sum = 0;
    va_start(ap, count);  // get variadic list handle
    for(int i = 0 ; i < count; i++)
    {
        sum += va_arg(ap, double); // traverse each parameter
    }
    va_end(ap);
    return sum;
}

// variadic template declare formal
template<typename... Types> class A;  // Types: template parameter pack: type
template<int ... Values> class B;    // Values: template parameter pack: none-type
template<typename... Types> class C : private A<Types...> {}; // pack expansion: Types...( expands to base class, and can expand to other formal as well)

// variadic template and template parameter pack is type
// simple my tuple
//
template <typename... Types> class myTuple; // variadic template declare

template<typename Head, typename... Tail>   // template partitial specialization recursely
class myTuple<Head, Tail...> : private myTuple<Tail...>
{
    Head head;
};
template<> class myTuple<> {};    // boader or initialization condition

// variadic template and template parameter pack is none-type ( A kind of meta programming --- compile-time calculation)
//
template<long... Values> struct multiply; // none-type variadic template declare

template<long First, long... Last>
struct multiply<First, Last...>   
{
    static const long val = First * multiply<Last...>::val; // none-type template partitial specialization  recursely(expands to expresson)
};

template<> struct multiply<> {static const long val = 1;}; // boader or initialization condition

// variadic template function parameter to function parameter pack
template<typename... Types> void func(Types... args); // both type and paramter are variadic

void myPrint(const char* s)
{
    while(*s)
    {
        if(*s == '%' && *++s != '%')
        {
            throw std::runtime_error("invalid format string: missing parameter");
        }
        std::cout << *s++;
    }
}

template<typename T, typename... Args>
void myPrint(const char* s, T value, Args... args)
{
    while(*s)
    {
        if(*s == '%' && *++s != '%' )
        {
            std::cout << value;  // print value to string: replace %x with value and output to stdio
            return  myPrint(++s, args...);   // recursely print
        }
        std::cout << *s++;  // print orign string
    }
    throw std::runtime_error("extra arguments provided to myPrint");
}

// 
// variadic template pack expansion: 7 locations
// 1. expression:  multiply none-type  for example
// 2. base class:  myTuple type for example
// 3. initializer-list
// 4. class member initializer-list
// 5. template paramter list
// 6. general attribution list
// 7. lambda function catch list

template<typename... Types> class N;
template<typename... Types> class M : private M<Types> ... {}; // multi-base  with one template parameter  each
template<typename... Types> class M2 : private M<Types...> {}; // single base with multi template parameter

// multi-base with one template parameter 
template<typename... Types> void dummyWrapper(Types... t){}
template<typename T> 
T pr(T t)
{
    std::cout << t;
    return t;
}

template<typename... Types>
void vtPrint(Types... args)
{
    dummyWrapper(pr(args)...);  // parameter pack unpack to pr(1), pr(", "), ... pr(", abc\n")
    std::cout << std::endl;
}

// sizeof... : calculate parameter pack count
//
template<typename... Types> void print(Types... args)
{
    assert(false);  // None-6-count parameter will trig this assert
}

void print(int a1, int a2, int a3, int a4, int a5, int a6)
{
    std::cout << a1 << ", " << a2 << ", " << a3 << ", "
              << a4 << ", " << a5 << ", " << a6 << std::endl;
}

template<typename... Types> int vaArgs(Types... args)
{
    int size = sizeof...(Types);  // calculate variadic parameter pack count
    switch(size)
    {
        case 0:
            print(3,3,3,3,3,3);
            break;
        case 1:
            print(3,3,args...,3,3,3);
            break;
        case 2:
            print(3,3,args...,3,3);
            break;
        case 3:
            print(args...,3,3,3);
            break;
        case 4:
            print(3,args...,3);
            break;
        case 5:
            print(3,args...);
            break;
        case 6:
            print(args...);
            break;
        default:
            print(0,0,0,0,0,0);
            break;
    }

    return size;
}

// more than one tempate parameter pack
//
template<typename T1, typename T2> struct S{};

// variadic template class named C1 as the template type 1 declare(with parameter pack C1Args)
// variadic template class named C2 as the template type 2 declare(with parameter pack C2Args)
template<
template<typename...> class C1, typename... C1Args,
template<typename...> class C2, typename... C2Args >
// template<typename... C1Args> class C1,   // error
// template<typename... C2Args> class C2 >
struct S< C1< C1Args...>, C2< C2Args...>> {};

// variadic template paramter with perfect forward
//

struct S1
{
    S1() {}
    S1(const S1& s) {  std::cout << " copy construction " << __func__ << std::endl;}
    S1(S1&& s) {  std::cout << " move construction " << __func__ << std::endl;}
};

struct S2
{
    S2() {}
    S2(const S2& s) {  std::cout << " copy construction " << __func__ << std::endl;}
    S2(S2&& s) {  std::cout << " move construction " << __func__ << std::endl;}
};

struct S3
{
    S3() {}
    S3(const S3& s) {  std::cout << " copy construction " << __func__ << std::endl;}
    S3(S3&& s) {  std::cout << " move construction " << __func__ << std::endl;}
};


template<typename... Types> struct multiType;  // declare
template<typename T, typename... Types>
struct multiType<T, Types...> : public multiType<Types...>   // template partitial specialization:  expand to base class
{
    T t1;
    multiType<T, Types...>(T a, Types... b) : t1(a) , multiType<Types...>(b...)
    {
        std::cout << "types count: " << sizeof...(Types) << std::endl;
    }
};
template<> struct multiType<>   // boarder or initialization condition
{
    multiType<>() 
    {
        std::cout << "empty template parameter construction" << std::endl;
    }
};

// perfect forward for variadic template
// variadic template class named varidcType as the template type declare
template<template<typename... > class varidicType, typename... Args>
varidicType<Args...> build(Args&&... args)
{
    return varidicType<Args...>(std::forward<Args>(args)...);
}

void variadicTemplateTest()
{
    std::cout << "sum of float is " << sumOfFloat(3, 1.2f, 2.3f, 3.4f) << std::endl;;
    std::cout << "multiply result is " << multiply<2,3,4>::val << std::endl;
    std::cout << "multiply result is " << multiply<22,33,44>::val << std::endl;
    myPrint("hello %s\n", std::string("world"));
    vtPrint(1, ", ", 1.2, ", abc\n");  // pr sequece order may diffrent in gcc or clang
    vaArgs();
    vaArgs(1);
    vaArgs(1,2);
    vaArgs(1,2,3);
    vaArgs(1,2,3,4);
    vaArgs(1,2,3,4,5);
    vaArgs(1,2,3,4,5,6);
    vaArgs(1,2,3,4,5,6,7);

    S<int, float> p;
    S<std::tuple<int, char>, std::tuple<float>> s;

   S1 s1;
   S2 s2;
   S3 s3;
   build<multiType>(s1,s2,s3);
}

// atomic type and atomic operation
//
//  atomic type can not copy construction and move construction and operator =

// coarse-grained: mutex
//
static long long total = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
void* func1(void*)
{
    for(long long i = 0;  i < 1000000LL; i++)
    {
        pthread_mutex_lock(&m);
        total += i;
        pthread_mutex_unlock(&m);
    }
}
// fine-grained verison: atomic
//
std::atomic_llong total2 {0};
void func2(int)
{
    for(long long i = 0; i < 1000000LL; i++)
    {
        total2 += i;
    }
}

// atomic_flag (lock-free): test_and_set(), clear()
// spin lock implemenation:
//
std::atomic_flag lock = ATOMIC_FLAG_INIT;
void lock_free_func(int n)
{
    while(lock.test_and_set(std::memory_order_acquire)) // try to aquire lock
    {std::cout << "waiting from thread " << n << std::endl;}

    std::cout << "thread " << n << " starts working" << std::endl;
}
void lock_free_func2(int n)
{
    std::cout << "thread " << n << " is going to start" << std::endl;
    lock.clear();
    std::cout << "thread " << n << " starts working" << std::endl;
}

// sequential consistent and memory model
//
// sequential consistent:
// 1. compiler generated atomic machine instruction and coder sequence is the same
// 2. CPUs execute atomic assemble instruction order sequence is the same
// memory model : 7 kinds
// 1. memory_order_relaxed: none guarantee for exec order
// 2. memory_order_acquire: in the current thread, all subsequent read operations can be executed until current atomic operation has finished
// 3. memory_order_release: in the current thread, current atomic operations can be executed untial all previouse write operations have finished
// 4. memory_order_acq_rel: both including memory_order_acquire and memory_order_release
// 5. memory_order_consume: in the current thread, all subsequent related-current-atomic operation can be executed until current atomic operation has finished
// 6. memory_order_seq_cst: all guarantee for exec order
//
std::atomic<int> a {0};
std::atomic<int> b {0};
int setValue(int)
{
    int t = 1;
    a =  t;
    b = 2;
}

// CPU may shuffle and out of order by using memory_order_relaxed
int setValueV2(int)
{
    int t = 1;
    a.store(t, std::memory_order_relaxed);
    b.store(2, std::memory_order_relaxed);
}

// CPU may shuffle but satify memory_order_release(release-acquire memory sequence)
int setValueV3(int)
{
    int t = 1;
    a.store(t, std::memory_order_relaxed);
    b.store(2, std::memory_order_release);
}


int observer(int)
{
    std::cout << "observe (" << a << "," << b << ")" << std::endl;  // maybe variable output
}

int observerV2(int)
{
    std::cout << "V2 observe (" << a << "," << b << ")" << std::endl;  // maybe variable output
}

int waitBprintA(int)
{
    while(b != 2)
        ; // spin-lock waitting
    std::cout << "print A: " << a << std::endl;  // looking forward a will be 1
}

int waitBprintAV2(int)
{
    while(b != 2)
        ; // spin-lock waitting
    std::cout << "V2 print A: " << a << std::endl;  // looking forward a will be 1
}


// CPU may shuffle but satify memory_order_acquire(release-acquire memory sequence)
// 
int waitBprintAV3(int)
{
    while(b.load(std::memory_order_acquire) != 2)
        ; 
    std::cout << "V3 print A: " << a.load(std::memory_order_relaxed) << std::endl;  // guarantee: 1
}

// producer-consumer model 
// memory_order_release and memory_order_consume(relase-consume memory sequence)
//

std::atomic<std::string*> ptr;
std::atomic<int> data;

void producer()
{
    std::string* p = new std::string("frank");
    data.store(33, std::memory_order_relaxed);
    ptr.store(p, std::memory_order_release);
}

void consumer()
{
    std::string* p2 = nullptr;
    while(!(p2 = ptr.load(std::memory_order_consume)))
        ;
    assert(*p2 == "frank");  // always assert success
    assert(data.load(std::memory_order_relaxed) == 33);  // may assert failed
}

void atomicTest()
{
    pthread_t thread1, thread2;
    if(pthread_create(&thread1, NULL, func1, NULL))
    {
        throw;
    }
    if(pthread_create(&thread2, NULL, func1, NULL))
    {
        throw;
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    std::cout << "coarse-grained mutex pthread: " << total << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    std::thread t1(func2, 0);
    std::thread t2(func2, 0);

    t1.join();
    t2.join();
    std::cout << "fine-grained atomic: " << total2 << std::endl;

    // copy,move,operator = are all NOT allowed in atomic type
    std::atomic<float> f1 {1.2f};
    // std::atomic<float> f2 {f1}; // error
    float f3 = f1; // ok
    float f4 {f1};

    // custom simple spin lock implemenation by atomic_flag
    lock.test_and_set();
    std::thread t11(lock_free_func, 1);
    std::thread t12(lock_free_func2, 2);
    t11.join();
    t12.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    std::thread setT(setValue, 0);
    std::thread obsT(observer, 0);
    std::thread wpT(waitBprintA, 0);

    setT.join();
    obsT.join();
    wpT.join();
    std::cout << "Got (" << a << "," << b << ")" << std::endl;  // Got (1,2)

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::thread setT2(setValueV2, 0);
    std::thread obsT2(observerV2, 0);
    std::thread wpT2(waitBprintAV2, 0);

    setT2.join();
    obsT2.join();
    wpT2.join();
    std::cout << "V2 Got (" << a << "," << b << ")" << std::endl;  // Got random

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::thread setT3(setValueV3, 0);
    std::thread wpT3(waitBprintAV3, 0);

    setT3.join();
    wpT3.join();
    std::cout << "V3 Got (" << a << "," << b << ")" << std::endl;  // Got (1,2)

    std::thread proT(producer);
    std::thread conT(consumer);
    proT.join();
    conT.join();
}

// TLS: thread local storage
// relative to global storage or static storage (both shared betwwen threads), TLS NOT shared
//
// old global storage(shared)
//
int errorCode = 0;
void* maySetErr(void* input)
{
    if(*(int*)input == 1)
    {
        errorCode = 1;
    }
    else if(*(int*)input == 2)
    {
        errorCode = -1;
    }
    else
    {
        errorCode = 0;
    }
}

// TLS (none-shared)
void* maySetErrV2(void* input)
{
    int thread_local tlCode = 0;
    if(*(int*)input == 1)
    {
        tlCode = 1;
    }
    else if(*(int*)input == 2)
    {
        tlCode = -1;
    }
    else
    {
        tlCode = 0;
    }
}


void TLSTest()
{
    int input_a = 1;
    int input_b = 2;

    // old global shared version
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, &maySetErr, &input_a);
    pthread_create(&thread2, NULL, &maySetErr, &input_b);

    pthread_join(thread2, NULL);
    pthread_join(thread1, NULL);
    std::cout << "errorCode: " << errorCode << std::endl;

    // TLS none-shared version
    pthread_t thread11, thread22;
    pthread_create(&thread11, NULL, &maySetErrV2, &input_a);
    pthread_create(&thread22, NULL, &maySetErrV2, &input_b);

    pthread_join(thread22, NULL);
    pthread_join(thread11, NULL);
}

// quick_exit & at_quick_exit
//
// terminal(): unexpected behavior, invoke abort() by default (NOT invoke any deconstruction function)but can be changed by set_terminal()
// abort():  unnormal behavior, throw a SIGABRT, NOT invoke any deconstruction function
// exit(): normal behavior, invoke deconstruction automatically and invoke registered function by atexit()
// quick_exit: normal behavior, NOT invoke any deconstruction function and avoid deadlock problem when at exist but invoke registered function by at_quick_exit();
//

// #define TEST_OLD_EXIT
void openDevice() {std::cout << "device is opened" << std::endl;}
void resetDevice() {std::cout << "device is reset" << std::endl;}
void closeDevice() {std::cout << "device is closed" << std::endl;}

struct AA { ~AA() {std::cout << "AA is deconstructed" << std::endl;} };

void quickExitTest()
{
#ifdef TEST_OLD_EXIT
    atexit(closeDevice);
    atexit(resetDevice);
    openDevice();
    exit(0); // invoke sequence is opposite to registered sequence
#else
    AA a;
    at_quick_exit(closeDevice);
    quick_exit(0);
#endif
}

int main(int argc, char** argv)
{
    // constexprTest();
    // variadicTemplateTest();
    // atomicTest();
    // TLSTest();
    quickExitTest();

    return 0;
}
