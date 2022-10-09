#include<cstdio>
#include<iostream>
#include<cassert>
#include<algorithm>
#include<memory>
#include<vector>
#include<array> 
#include<utility> 
#include<atomic> 
#include<thread>
#include<mutex>
#include<chrono>
#include<condition_variable>
#include<future> 
#include<cmath>
#include<functional>
#include<numeric>
#include<sstream>
#include<iterator>
#include<cfenv>
#include<iomanip>
#include<ctime>
#include<cinttypes>
#include<forward_list>
#include<initializer_list>
#include<random>
#include<map>
#include<ratio>
#include<regex>
#include<set>
#include<typeindex>
#include<unordered_map>
#include<type_traits>
#include<unordered_set>
#include<bit>
#include<list>
#include<exception>
#include<stdexcept>

// array lib: containers
// a container with a fixed size that is specified at compile time as a template argument
// like a built-in("C-style") array 
//
// Complexity: 
// Insertion: NOT
// Deletion: NOT
// Access: O(1)
//
// Can return a std::array BUT NOT a C-style arrays
// Can store a std::array in a container BUT NOT a C-style arrays
//
// bz std::array has a copy constructor and a copy assignment operator
//
// auto cross_product(const int (&a)[3], const int (&b)[3]) -> int[3]{} // error
auto cross_product(const std::array<int, 3> &a, const std::array<int, 3> &b) -> std::array<int, 3> { }  // ok
void arrayTest()
{
    std::array<int, 3> arr1;
    std::cout << "uninitialized std::array: ";  // random value
    for(auto &a: arr1)
    {
        std::cout << a << '\t';
    }
    std::cout << std::endl;

    arr1.fill(3);
    std::cout << "fill std::array: ";
    for(auto &a: arr1)
    {
        std::cout << a << '\t';
    }
    std::cout << std::endl;


    std::array<int, 3> arr2 {1,2};
    std::cout << "partitial initialized std::array: ";  // 0 default value
    for(auto &a: arr2)
    {
        std::cout << a << '\t';
    }
    std::cout << std::endl;

    // C-style array version
    //
    std::string c_style[3] = {"hello", "world", "array"};
    assert(c_style[2] == "array");
    // assert(std::size(c_style) == 3);  //C++17 feature
    assert(std::distance(std::begin(c_style), std::end(c_style)) == 3);

    // copying via operator = isn't supported
    std::string other[3];
    std::copy(std::begin(c_style), std::end(c_style), std::begin(other));

    // swapping is supported ... in linear time
    using std::swap;
    swap(c_style, other);

    // comparision isn't supported, have to use a standard algorithm
    // Worse, operator == does the 'wrong": address comparision
    assert(c_style != other); // address comparision
    // assert(std::equal(c_style, c_style + 3, other, other + 3)); // error: 

    // std::array version
    //
    std::array<std::string,3> arr = {"hello", "world", "array"};
    assert(arr[2] == "array");

    // begin(), end(), size() are all provided
    assert(arr.size() == 3);
    assert(std::distance(arr.begin(), arr.end()) == 3);

    // coying via operator = is supported ... in linear time
    std::array<std::string, 3> other2;
    other2 = arr;

    // swapping is supported ... in linear time
    using std::swap;
    swap(arr, other2);

    // the arrays have different address
    assert(&arr != &other2); 

    // operator == does the nature thing: value comparision 
    // compare lexicographically equal
    assert(arr == other2);  // value comparision

    // relational operators are alsoe supported
    assert(arr >= other2); 

    int c_aaa[3] = {0};
    std::vector<int[3]> va;  // ok
    // va.emplace_back(c_aaa);  // error

    std::array<int, 3> arr_aaa = {0};
    std::vector<std::array<int, 3>> va2;  // ok
    va2.emplace_back(arr_aaa);// ok
}

// atomic lib: concurrency
//
//compare_excheange_XXX(): 
// CAS(Compare And Swap): CPU intrinsic instructor
//
// expected = this.load(); 
// this.compare_exchange_weak(expected&, desired)  : 
// mostly used in loop expression
//
// if(this == expected) 
// { this = desired  return true;} MAY return false in some plateform (NOT X86)
// else 
// { expected = this; return false;}
//
// expected = this.load(); 
// this.compare_exchange_strong(expected, desired)
//  mostly used in none-loop expression
//
// if(this == expected) 
// { this = desired  return true;} MUST return true always
// else 
// { expected = this; return false;}
//

template<typename T>
struct node 
{
    T data;
    node* next;
    node(const T& data): data(data), next(nullptr) {}
};

template<typename T>
class MyStack
{
    std::atomic<node<T>*> head { nullptr};  // node head(shared between threads)
public:
    // 3 operation:
    void push(const T& data)
    {
        // first, create new_node
        node<T>* new_node = new node<T>(data);

        // second, put head to new_node->next
        if(nullptr == head)
        {
            head = new_node;
            return;
        }
        new_node->next = head.load(std::memory_order_relaxed); // Here Other thread may change head 

        // third, change head with new_node

        // now change head with new_node,
        // there has 2 case: 
        // 1) other threads have changed head (other threads must have inserted a node just now)
        // that means head(new) != new_node->next(old), than exec
        // new_node->next = head and return false;
        // 2) other threads have NOT changed head
        // that means head == new_node->next,than exec
        // head = new_node(Read-Modify-Write) and return true
        // if return true means current thread has inserted success
        // else return false means current thread has NOT finish inserting
        // and do while loop again, but new_node->next HAS already updated latest
        while(!head.compare_exchange_weak(new_node->next, new_node,
                                        std::memory_order_release,
                                        std::memory_order_relaxed))
            ; // th body of the loop is empty
    }
    node<T>* getHead() const
    {
        return head.load(std::memory_order_relaxed);
    }
};

void atomicTest()
{
    std::atomic_int ai;
    std::atomic<int> ai2;
    std::cout << "typeid(ai): "<< typeid(ai).name()  << ", "
        << "typeid(ai2): "<< typeid(ai2).name() << std::endl;

    std::atomic_int ai3 {-123};
    std::cout << "ai3:  "<< ai3 << std::endl;
    ai3 = 333;
    // ai3 = ai2;  // error: NO copy-assignment operator and move-assignment operator, CAN NOT finish in one single atomic operation
    std::cout << "ai3:  "<< ai3 << std::endl;

    ai3.store(336);
    std::cout << "ai3:  "<< ai3 << std::endl;

    auto ai4 = ai3.load();
    std::cout << "ai4:  "<< ai4 << std::endl;

    std::atomic<float> af;
    std::cout << std::boolalpha << af.is_lock_free() << std::endl;
    std::cout << std::boolalpha << ai2.is_lock_free() << std::endl;
    //
    // std::atomic<std::array<int,2>> asmall;
    // std::atomic<std::array<int,100>> alarge;
    // struct A {int a[100];};
    // struct B {int x, y;};
    // std::atomic<A> large;
    // std::atomic<B> small;
    // std::cout << std::boolalpha << af.is_lock_free() << '\t'
    // << small.is_lock_free() << '\t'
    // << large.is_lock_free() << std::endl;

    // compare_exchange_weak()
    // my custom statck push operation implementation with compare_exchange_weak()
    MyStack<int> s;
    s.push(1);
    s.push(2);
    s.push(3);

    auto head = s.getHead();
    std::cout << "MyStack: ";
    while(nullptr != head)
    {
        std::cout << head->data << '\t';
        head = head->next;
    }
    std::cout <<  std::endl;

    // init status 
    std::atomic_int current;
    int expected = 4;
    int desired = 5;
    current = 3;
    bool exchanged = false;

    std::cout << "cur = " << current << ", "
        << "expected = " << expected  << ", "
        << "desired = " << desired  << ", "
        << "exchanged = " << std::boolalpha << exchanged  << std::endl;

    // current != expected  =>  expected is modified with current and return false;
    exchanged = current.compare_exchange_strong(expected, desired);
    std::cout << "cur = " << current << ", "
        << "expected = " << expected  << ", "
        << "desired = " << desired  << ", "
        << "exchanged = " << std::boolalpha << exchanged  << std::endl;

    // current == expected  =>  current is modified with desired and return true;
    exchanged = current.compare_exchange_strong(expected, desired);
    std::cout << "cur = " << current << ", "
        << "expected = " << expected  << ", "
        << "desired = " << desired  << ", "
        << "exchanged = " << std::boolalpha << exchanged  << std::endl;
}

// mutex lib: concurrency
//
//

std::once_flag flag1, flag2;
void simpleDoOnce()
{
    std::call_once(flag1, [](){std::cout << "Simple: called once" << std::endl;});
}

void mayThrowFunc(bool do_throw)
{
    if(do_throw)
    {
        std::cout << "throw: call_once will retry" << std::endl;
        throw std::exception();  // this may appear more than once
    }

    std::cout << "Didn't throw, call_once will not attempt again" << std::endl;
    // guaranteed once
}

void mayDoOnce(bool do_throw)
{
    try
    {
        std::call_once(flag2, mayThrowFunc, do_throw);
    }
    catch(...)
    {
        std::cout << "caught exception..." << std::endl;
    }
}

struct Logger
{
    std::mutex _mtx;
    void logV1(const char* ms)
    {
        _mtx.lock();
        int i = 0;
        do{
            i++;
            std::cout << "[Log1]: " << ms << std::endl;
        }while(i <= 3);
        _mtx.unlock();
    }

    void logV2(const char* ms)
    {
        _mtx.lock();
        int i = 0;
        {
            i++;
            std::cout << "[Log2]: " << ms << std::endl;
        }while(i <=3);
        _mtx.unlock();
    }
};

template<typename M>
class MyUniqueLock
{
    public:
        constexpr MyUniqueLock() noexcept = default;
        constexpr MyUniqueLock(M* p) noexcept: _mtx(p) {};
        MyUniqueLock(MyUniqueLock&& rhs) noexcept
        {
            _mtx = std::exchange(rhs._mtx, nullptr);  // std::exchange: C++14, std::atomic::exchange:C++11
            _locked = std::exchange(rhs._locked, false);
        }

        MyUniqueLock& operator=(MyUniqueLock&& rhs)
        {
            if(_locked)
            {
                unlock();
            }
            _mtx = std::exchange(rhs._mtx, nullptr);
            _locked = std::exchange(rhs._locked, false);
            return *this;
        }
        ~MyUniqueLock()
        {
            if(_locked)
            {
                unlock();
            }
        }

        M* mutex() const noexcept {return _mtx;}
        bool owns_lock() const noexcept {return _locked;}

        void lock()   {_mtx->lock(); _locked = true;}
        void unlock() {_mtx->unlock(); _locked = false;}

    private:
        M* _mtx = nullptr;
        bool _locked = false;
};

// Always associate a mutex with its controlled data
template<class T>
class MyGuard
{
    class Handle
    {
        std::unique_lock<std::mutex> _lk;
        T* _ptr;

        public:
        Handle(std::unique_lock<std::mutex> lk, T *p): _lk(std::move(lk)), _ptr(p) {}
        auto operator->() const { return _ptr;}  
    };

    public:
    Handle lock()
    {
        // std::unique_lock lk(_mtx);  // C++14: error, need template parameter C++17: ok, non-need template parameter
        std::unique_lock<std::mutex> lk(_mtx);
        return Handle {std::move(lk), &_data};
    }

    private:
        std::mutex _mtx;
        T _data;
};

class StreamAvg
{
    public:
        void addValue(double x)
        {
            auto h = _sc.lock();
            h->_sum += x;
            h->_count += 1;
        }
        int getCount()
        {
            auto h = _sc.lock();
            return h->_count;
        }
        double getCurAvg()
        {
            auto h = _sc.lock();
            _last_avg = h->_sum / h->_count;
            return _last_avg;
        }
    private:
        struct Gards
        {
            double _sum = 0;
            int _count = 0;
        };
        MyGuard<Gards> _sc;
        double _last_avg = 0.0;
};

// recursive mutex
class X
{
    public:
        void func1()
        {
            std::lock_guard<std::recursive_mutex> lk(_m);
            _shared = "func1";
            std::cout << "in func1 shared variable is now " << _shared << std::endl;
        }
        void func2()
        {
            std::lock_guard<std::recursive_mutex> lk(_m);
            _shared = "func2";
            std::cout << "in func2 shared variable is now " << _shared << std::endl;
            func1();  // recursive lock becomes useful here
            std::cout << "back in func2 shared variable is " << _shared << std::endl;
        }
    private:
        std::recursive_mutex _m;
        std::string _shared;
};

// defer_lock_t : do not acquire ownership of the mutex
// try_to_lock_t: try to acquire ownership of the mutex without blocking
// adopt_lock_t: assume the calling thread already has owership of the mutex
//

struct bankAccout
{
    explicit bankAccout(int balance) : balance(balance) {}
    int balance {0};
    std::mutex m;
};

void transfer(bankAccout &from, bankAccout &to, int amout)
{
    if(&from == &to) return;  // avoid deadlock in case of self transfer

    //lock both mutexes without deadlock
    // std::lock(from.m, to.m);
    // std::lock_guard<std::mutex> lock1 {from.m, std::adopt_lock};
    // std::lock_guard<std::mutex> lock2 {to.m, std::adopt_lock};

    // equivalent approach: 
    std::unique_lock<std::mutex> lk1 { from.m, std::defer_lock};
    std::unique_lock<std::mutex> lk2 { to.m, std::defer_lock};
    std::lock(lk1, lk2);

    // equivalent approach 2:(C++17 scoped_lock)
    // use std::scoped_lock to acquire two locks without worrying about
    // other calls to deadlocking 
    // and it also provides a convenient RAII-style mechanism
    //
    // std::scoped_lock lock(from.m, to.m); 
    //

    from.balance -= amout;
    to.balance += amout;
}

void mutexTest()
{
    // call once
    std::thread st1(simpleDoOnce);
    std::thread st2(simpleDoOnce);
    std::thread st3(simpleDoOnce);
    std::thread st4(simpleDoOnce);
    st1.join();
    st2.join();
    st3.join();
    st4.join();
/*
 * 
 *     std::thread t1(mayDoOnce, true);
 *     std::thread t2(mayDoOnce, true);
 *     std::thread t3(mayDoOnce, true);
 *     std::thread t4(mayDoOnce, true);
 *     std::thread t5(mayDoOnce, false);
 *     std::thread t6(mayDoOnce, true);
 *     std::thread t7(mayDoOnce, true);
 *     t1.join();
 *     t2.join();
 *     t3.join();
 *     t4.join();
 *     t5.join();
 *     t6.join();
 *     t7.join();
 * 
 */

/*
 * 
 *     Logger log;
 *     std::thread t11([&](){log.logV1("thread 1 print");});
 *     std::thread t22([&](){log.logV2("thread 2 print");});
 *     t11.join();
 *     t22.join();
 * 
 */
    StreamAvg sa;
    std::thread t13([&](){
            std::cout << "thread 1 add 1.2: " << std::endl;
            sa.addValue(1.2);
            std::cout << "thread 1 add 2.3: " << std::endl;
            sa.addValue(2.3);
            std::cout << "thread 1 add 3.4: " << std::endl;
            sa.addValue(3.4);
            });

    using namespace std::chrono_literals;
    std::thread t23([&](){
            std::this_thread::sleep_for(0ms);  //C++11: error, C++14: ok, chrono literals
            std::cout << "thread 2 getCount: " << sa.getCount() << std::endl;
            std::cout << "thread 2 getCurAvg: " << sa.getCurAvg() << std::endl;
            });
    t13.join();
    t23.join();

    // special-purpose mutex
    std::timed_mutex m;
    std::atomic<bool> ready(false); 

    std::thread thread_b([&](){
            std::lock_guard<std::timed_mutex> lk(m);
            puts("Thread B got the lock and sleep 10ms");
            ready = true;
            std::this_thread::sleep_for(100ms);
            });

    while(!ready)
    {
        puts("Main Thread is waitting for thread B to launch");
        std::this_thread::sleep_for(10ms);
    }

    while(!m.try_lock_for(10ms))
    {
        puts("Main Thread spent 10ms trying to get the lock and failed");
    }

    puts("Main Thread finally got the lock");
    m.unlock();
    thread_b.join();

    // recursive_mutex
    X x;
    std::thread tt1(&X::func1, &x);
    std::thread tt2(&X::func2, &x);
    tt1.join();
    tt2.join();

    // std::lock and lock strategy
    bankAccout myAccout{100};
    bankAccout yourAccout{180};

    std::thread tb1 {transfer, std::ref(myAccout), std::ref(yourAccout), 10};
    std::thread tb2 {transfer, std::ref(yourAccout), std::ref(myAccout), 20};

    tb1.join();
    tb2.join();

    std::cout << "my balance: " << myAccout.balance << '\t'  << 
                 "your balance: " << yourAccout.balance << std::endl;
}

// condition variable 
//

std::condition_variable cv;
std::mutex cv_m; // This mutex is used for three purposes:
                 // 1) to synchronize access to i
                 // 2) to synchronize access to std::cerr
                 // 3) for the condition variable cv
int i = 0;

void waits()
{
    std::unique_lock<std::mutex> lk(cv_m);
    std::cerr << "Waiting..."<< std::endl;
    cv.wait(lk,[] { return i == 1; });
    std::cerr << "Finished Waiting i = 1..."<< std::endl;
}

void signals()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        std::lock_guard<std::mutex> lk(cv_m);
        std::cerr << "Notifying..." << std::endl;
    }
    cv.notify_all();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        std::lock_guard<std::mutex> lk(cv_m);
        i = 1;
        std::cerr << "Notifying again..." << std::endl;
    }
    cv.notify_all();
}

std::mutex cv_m2;
std::condition_variable cv2;
std::string data;
bool ready = false;
bool processed = false;

void workerThread()
{
    // Wait until main() sends data
    std::unique_lock<std::mutex> lk(cv_m2);
    cv2.wait(lk, []{return ready;});

    // Own the lock after the wait
    std::cout << "Worker thread is processing data... " << std::endl;
    data += "after processing";

    // Send data back to main()
    processed = true;
    std::cout << "Worker thread finish processing data... " << std::endl;

    // Manualy unlocking is done before notifyiing, to avoid waking up
    // the waiting thread only to block again
    lk.unlock();
    cv2.notify_one();
}

// notify_all_at_thread_exit()
//

std::mutex cv_m3;
std::condition_variable cv3;
bool ready3 = false;
std::string result; // some arbitrary type

void threadFunc()
{
    thread_local std::string thread_local_data = "33";
    std::unique_lock<std::mutex> lk(cv_m3);

    // assign a value to result using thread_local data
    result = thread_local_data;
    ready3 = true;

    std::notify_all_at_thread_exit(cv3, std::move(lk));
    // 1. destroy thread_locals
    // 2. unlock mutex
    // 3. notify cv
}

void conditionVariableTest()
{
    std::thread t1(waits), t2(waits), t3(waits), t4(signals);
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    std::thread worker(workerThread);

    // Send data to the worker thread
    {
        std::lock_guard<std::mutex> lk(cv_m2);
        ready = true;
        std::cout << "Main Thread signals data ready for processing..." << std::endl;
    }
    cv2.notify_one();

    // Waiting for the worker
    {
        std::unique_lock<std::mutex> lk(cv_m2);
        cv2.wait(lk, []{return processed;});
    }
    std::cout << "Back to Main Thread, data = " << data << std::endl;

    worker.join();

    std::thread t(threadFunc);
    t.detach();

    using namespace std::chrono_literals;  // C++14
    std::this_thread::sleep_for(300ms);

    // Waiting for the detached thread( by using std::notify_all_at_thread_exit())
    std::unique_lock<std::mutex> lk(cv_m3);
    cv3.wait(lk, []{return ready3;});

    // result is ready and thread_local destructors have finished, no UB
    std::cout << "result " << result << std::endl;
}

// thread
//

void f1()
{
    // simulate expensive operation
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void f2()
{
    // simulate expensive operation
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void independentThread()
{
    std::cout << "Starting concurrent thread" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Exiting concurrent thread" << std::endl;
}

void threadCaller()
{
    std::cout << "Starting thread caller" << std::endl;
    std::thread t(independentThread);
    t.detach();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(300ms);
    std::cout << "Exiting thread caller" << std::endl;
}

void threadTest()
{
    // join()
    std::cout << "starting first helper..." << std::endl;
    std::thread t1(f1);

    std::cout << "starting second helper..." << std::endl;
    std::thread t2(f2);

    std::cout << "waiting for helpers to finish..." << std::endl;
    t1.join();
    t2.join();

    std::cout << "done" << std::endl;

    // joinable()
    std::thread t;
    std::cout << "before starting, joinable: "<< std::boolalpha << t.joinable() << std::endl;
    t = std::thread(f1);
    std::cout << "after starting, joinable: "<< std::boolalpha << t.joinable() << std::endl;
    t.join();
    std::cout << "after joining, joinable: "<< std::boolalpha << t.joinable() << std::endl;

    // get_id(), std::swap(), thread.swap()
    std::thread st1(f1);
    std::thread st2(f2);

    std::cout << "thread 1 id: " << st1.get_id() << std::endl;
    std::cout << "thread 2 id: " << st2.get_id() << std::endl;

    std::swap(st1, st2);
    std::cout << "after swap(st1,st2): " << std::endl;
    std::cout << "thread 1 id: " << st1.get_id() << std::endl;
    std::cout << "thread 2 id: " << st2.get_id() << std::endl;

    st1.swap(st2);
    std::cout << "after st1.swap(st2): " << std::endl;
    std::cout << "thread 1 id: " << st1.get_id() << std::endl;
    std::cout << "thread 2 id: " << st2.get_id() << std::endl;

    st1.join();
    st2.join();

    // detach()
    threadCaller();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(3s);
}

// future lib: thread support
// A mechanism to access the result of asynchronous operatons
// A asynchronous operation can be created via std::async, std::packaged_task or std::promise
//
//

// instant time  display prefix 
std::string time()
{
    static auto start = std::chrono::steady_clock::now();
    std::chrono::duration<double>d = std::chrono::steady_clock::now() - start;
    return "[" + std::to_string(d.count()) + "s]";
}

// fibo compute function via recursively

int fib(int n)
{
    if(n < 3) { return 1;}
    else {return fib(n-1) + fib(n-2);}
}

int taskFunc(int x, int y)
{
    return std::pow(x,y);
}

void taskLambda()
{
    std::packaged_task<int(int, int)> task([](int a, int b){
            return std::pow(a,b); });
    std::future<int> result = task.get_future();

    // execute via operator()
    task(2,9);

    std::cout << "taskLambda: " << result.get() << std::endl;
}

void taskBind()
{
    std::packaged_task<int()> task(std::bind(taskFunc, 2, 10));
    std::future<int> result = task.get_future();

    // execute via operator()
    task();

    std::cout << "taskBind: " << result.get() << std::endl;
}

void taskThread()
{
    std::packaged_task<int(int, int)> task(taskFunc);
    std::future<int> result = task.get_future();

    // execute via operator()
    std::thread task_td(std::move(task), 2, 11);
    task_td.join();

    std::cout << "taskThread: " << result.get() << std::endl;
}

void taskWorker(std::future<void>& output)
{
    std::packaged_task<void(bool&)> task{[](bool& done) {done = true;}};
    auto result = task.get_future();
    bool done = false;
    task.make_ready_at_thread_exit(done); // execute task right away and done is true now
    std::cout << "Worker Thread: done = " << std::boolalpha << done << std::endl;

    auto status = result.wait_for(std::chrono::seconds(0));
    if(status == std::future_status::timeout)
    {
        std::cout << "Worker Thread: result is not ready yet" << std::endl; // worker thread result may not ready yet
    }
    output = std::move(result);  // when worker thread exit, result's future_status will be ready 
                                 // via calling make_ready_at_thread_exit(done);
}

// async 
std::mutex async_m;
struct AsyncFunc
{
    void f1(int i, const std::string& str)
    {
        std::lock_guard<std::mutex> lk(async_m);
        std::cout << str <<  " " << i << std::endl;
    }

    void f2(const std::string& str)
    {
        std::lock_guard<std::mutex> lk(async_m);
        std::cout << str << std::endl;
    }

    int operator()(int i)
    {
        std::lock_guard<std::mutex> lk(async_m);
        std::cout << i << std::endl;
        return i+10;
    }
};

// async parallel sum
const int MAX_LEN = 1000;
template<typename RandomIt>
int parallelSum(RandomIt beg, RandomIt end)
{
    auto len = end - beg;
    if( len < MAX_LEN)
    {
        return std::accumulate(beg, end, 0);
    }

    // mid, end
    RandomIt mid = beg + len/2;
    auto handle = std::async(std::launch::async, parallelSum<RandomIt>, mid, end);
    // beg, mid
    int sum = parallelSum(beg, mid);
    // beg, mid, end
    return sum + handle.get();
}


void accuImpl(std::vector<int>::iterator first,
              std::vector<int>::iterator last,
              std::promise<int> pro)
{
    int sum = std::accumulate(first, last, 0);
    pro.set_value(sum); // notify future with result
}

void doWorkImpl(std::promise<void> barrier)
{
    std::cout << "[Barrier]: do working ..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    barrier.set_value();  // notify future without result
    std::cout << "[Barrier]: finish working and notify ..." << std::endl;
}

void futureTest()
{
    // valid()
    // checks if the future refers a shared state
    // This is the case only for futures that were not default-constructed or
    // moved from(i.e. returned by std::promise::get_future(), std::packaged_task::get_future() 
    // or std::async()) until the first time get() or share() is called 
    // true if *this refers to a shared state, otherwise false

    std::promise<void> p;
    std::future<void> f = p.get_future();
    std::cout << std::boolalpha;
    std::cout << f.valid() << std::endl;
    p.set_value();
    std::cout << f.valid() << std::endl;
    f.get();
    std::cout << f.valid() << std::endl;

    // future_error
    // an exception object that is thrown on failure by the functions in the thread lib that 
    // deal with asynchronous execution and shared state(std::future, std::promise,etc)
    // inherited from exception <- logic_error <- future_error

    std::future<int> empty;
    try
    {
        int n = empty.get(); // The behavior is undefined, 
                             // but some implementations throw std::future_error
    }
    catch(const std::future_error& e)
    {
        std::cout << "Caught a future_error with code: " << e.code() << 
                     " , Message: " << e.what() << std::endl;
    }

    // get()
    // T get()
    // T& get()
    // void get()
    // waits until the future has a valid reuslt. It effectively calls wait() in order to wait for the result
    // the behavior is undefined if valid() is false before the call to this function
    // any shared state is released. valid() is false after a call to this function
    // The value v stored in the shared state as std::move();
    // std::launch: 
    // 1. std::launch::async: the task is executed on a different thread, potentially by creating and launchingit first
    // 2. std::launch::deferred: the task is executed on the calling thread the first time its result is requested(lazy evaluation), 

    using namespace std::chrono_literals;
    {
        std::cout << time() << "launching thread " << std::endl;
        std::future<int> f = std::async(std::launch::async, []{
                std::this_thread::sleep_for(1s);
                return 3; });

        std::cout << time() << "waiting for the future, f.valid() = " << f.valid() << std::endl;
        int n = f.get();
        std::cout << time() << "future.get() returned with " << n << ", f.valid() = " << f.valid() << std::endl;
    }

    {
        std::cout << time() << "lanching thread " << std::endl;
        std::future<int> f = std::async(std::launch::async,[]{
                std::this_thread::sleep_for(1s);
                return true ? throw std::runtime_error("33") : 33; });
        std::cout << time() << "waiting for the future, f.valid() = " << f.valid() << std::endl;
        try
        {
        int n = f.get();
        std::cout << time() << "future.get() returned with " << n << ", f.valid() = " << f.valid() << std::endl;
        }
        catch(const std::exception& e)
        {
            std::cout << time() << "caught exception,  message: " << e.what() << 
                ", f.valid() = " << f.valid() << std::endl;
        }
    }

    // wait()
    // blocks until the result becomes available. valid() == true after the call
    // the behavior is undefined if valid() == false before the call to this function
    //

    std::future<int> f1 = std::async(std::launch::async, []{
            return fib(40); });
    std::future<int> f2 = std::async(std::launch::async, []{
            return fib(43); });

    std::cout << "waiting... " << std::endl;
    const auto start = std::chrono::system_clock::now();
    f1.wait();
    f2.wait();
    const auto diff = std::chrono::system_clock::now() - start;
    std::cout << "wait results takes " << std::chrono::duration<double>(diff).count() << " seconds " << std::endl;
    std::cout << "f1 res: " << f1.get() << std::endl;
    std::cout << "f2 res: " << f2.get() << std::endl;

    // wait_for()
    // waits for the result to become available. Blocks until sepecified timeout_duration has elapsed or the result becomes available, whichever comes first
    // the return value identifies the state of the result
    // if the future is the result of a call to std::async that used lazy evaluation, this function returns immediately without waiting
    // this functio may block for longer that timeout_duration due to scheduling or resource contention delays
    // a steady clock is recommened to measure the duration 
    // the behavior is undefined if valid() is false before the call of this function
    // 1. future_status::deferred: the shared state contains a deferred function using lazy evaluation, so the result will be computed only when explicitly requested
    // 2. future_status::ready: the result is ready
    // 3. future_status::timeout: the timeout has expired

    using namespace std::chrono_literals;
    std::future<int> future = std::async(std::launch::async, []{
            std::this_thread::sleep_for(3s);
            return 3; });

    std::cout << "waiting..." << std::endl;

    std::future_status status;
    do
    {
        switch(status = future.wait_for(1s); status)
        {
            case std::future_status::deferred: std::cout << "deferred..." << std::endl; break;
            case std::future_status::timeout: std::cout << "timeout..." << std::endl; break;
            case std::future_status::ready: std::cout << "ready!..." << std::endl; break;
        }
    }while(status != std::future_status::ready);

    std::cout << "result is " << future.get() << std::endl;

    // packaged_task
    // wraps any Callable target(function, lambda expression, bind expressio, function object) so that it can be invoked asynchronously. Its return value or exception thrown is stored in a share stateA which can be accessed through std::future objects
    //
    //operator(): calls the stored task with args, the return value of the task or any exceptions thrown are stored in the shared state , the shared state is made ready and any threads waiting for this are unblocked
    //get_future(): returns a future which shares the same shared state as *this, can be called once for each packaged_task
    //make_ready_at_thread_exit(): calls the stored task with forworded args as arguments, the shared state is only made ready after the current thread exits and all objects of thread local storage duration are destoryed
    taskLambda();
    taskBind();
    taskThread();

    std::future<void> result;
    std::thread{taskWorker, std::ref(result)}.join();
    if(status == std::future_status::ready)
    {
        std::cout << "Main Thread: result is ready" << std::endl;
    }

    // async
    //
    // runs the function asynchronously(potentially in a separate thread which might be a part of a thread pool) and returns a std::future that will hold the result of that function call
    // returned std::future referring to the shared state created by this call to std::async
    //
    // if the std::future obtained from std::async is not moved from  or bound to a reference, the destructor of the std::future will block at the end of the full expression untill the asynchronous operation complates, essentially making code such as the following synchronous:
    //  std::async(std::launch::async, []{f();});  // temporary's dtor waits for f()
    //  std::async(std::launch::async, []{g();});  // does not start until f() completes
    std::vector<int> v(10000, 1);
    std::cout << "The sum is " << parallelSum(v.begin(), v.end()) << std::endl;

    AsyncFunc a;

    // call (&a)->f1(33, "Hello") with default policy
    // may print "hello 33" concurrently or defer execution
    auto a1 = std::async(&AsyncFunc::f1, &a, 33, "hello");

    // call (&a)->f2("world") with deferred policy
    // prints "world" when a2.get() or a2.wait() is called
    auto a2 = std::async(std::launch::deferred, &AsyncFunc::f2, a, "world");

    // call AsyncFunc()(333) with async policy
    // prints "333" concurrently
    auto a3 = std::async(std::launch::async, AsyncFunc(), 333);

    a2.wait();  // prints "world"
    std::cout << "a3: " << a3.get()<< std::endl; // prints "343"

    // promise
    // 1. base template
    // 2. non-void specialization, used to commnunicate objects between threads
    // 3. void specialization, used to communicate stateless events
    //
    // provides a facility to store a value or an exception that is later acquired
    // asynchronously via a std::future object created by the std::promise object
    // std::promise object is meant to be used only once
    // a promise may do 3 things with the shared state:
    // 1. make ready: stores the result or exception in the shared state. Marks the state
    // ready and unblocks any thread waiting on a future associated with the shared state
    // 2. release: gives up its reference to the shared state. if this was the last such reference ,
    // the shared state is destoryed.
    // 3. abandon: stores the  exception of type std::future_error with error code 
    // std::future_errc::broken_promise, makes the shared state ready and then release it 
    //
    // set_value(): atomically stores the value into the shared state and makes thte state ready

    // std::promise<void> used as signals between threads
    std::istringstream iss_numbers{"3 4 1 42 234 -23 93 2 -289 -789"};
    std::istringstream iss_letters{"a b c, def g hk, j hml"};

    std::vector<int>  numbers;
    std::vector<char> letters;
    std::promise<void> numbers_promise, letters_promise; // promise declare
    auto numbers_ready = numbers_promise.get_future();
    auto letters_ready = letters_promise.get_future();

    std::cout << "[Main]: starting..." << std::endl;
    std::thread reader([&]{

            std::cout << "[Reader]: starting number copying" << std::endl;
            // I/O operations
            std::copy(std::istream_iterator<int>{iss_numbers},
                      std::istream_iterator<int>{},
                      std::back_inserter(numbers));
            std::cout << "[Reader]: finishing number copying" << std::endl;

            std::cout << "[Reader]: starting notify number promise" << std::endl;
            // notify for numbers
            numbers_promise.set_value();
            std::cout << "[Reader]: finishing notify number promise" << std::endl;

            std::cout << "[Reader]: starting letter copying" << std::endl;
            std::copy_if(std::istreambuf_iterator<char>{iss_letters},
                         std::istreambuf_iterator<char>{},
                         std::back_inserter(letters),
                         ::isalpha);
            std::cout << "[Reader]: finishing letter copying" << std::endl;

            std::cout << "[Reader]: starting notify letter promise" << std::endl;
            // notify for letters
            letters_promise.set_value();
            std::cout << "[Reader]: finishing notify letter promise" << std::endl;
            });

    std::cout << "[Main]: waiting number promise" << std::endl;
    numbers_ready.wait();
    std::cout << "[Main]: finishing waiting number promise" << std::endl;

    std::sort(numbers.begin(), numbers.end());

    if(letters_ready.wait_for(1s) == std::future_status::timeout)
    {
        std::cout << "[Main]: letter ready timeout  " << std::endl;
        std::cout << "letters timeout: " << '\t';
        for(int num : numbers)  std::cout << num << " ";
        std::cout << std::endl;
        numbers.clear();
    }

    std::cout << "[Main]: waiting letter promise" << std::endl;
    letters_ready.wait();
    std::cout << "[Main]: finishing waiting letter promise" << std::endl;
    std::sort(letters.begin(), letters.end());

    std::cout << "[Main]: starting print vectors" << std::endl;
    std::cout << "numbers: : " << " ";
    for(int num: numbers) std::cout << num << " ";
    std::cout << std::endl;

    std::cout << "letters: : " << " ";
    for(char let: letters) std::cout << let << " ";
    std::cout << std::endl;

    std::cout << "[Main]: finishing print vectors" << std::endl;

    reader.join();

    std::cout << "[Main]: exiting..." << std::endl;

    // promise<int> can be sued as signals between threads
    //

    // demonstrate using promise<int> to transmit a result between thread
    std::vector<int> inputNumbers = {1,2,3,4,5,6};
    std::promise<int> promise1;
    std::future<int> future1 = promise1.get_future();
    std::thread workThread(accuImpl, inputNumbers.begin(), inputNumbers.end(),
                           std::move(promise1));

    // future.get() will wait until the future has a valid result and retrieves it 
    std::cout << "accumulate result: " << future1.get() << std::endl;
    workThread.join();  // wait for thread completion

    // demonstrate using promise<void> to signal state between thread
    std::promise<void> barrier;
    std::future<void> barrierFuture = barrier.get_future();
    std::cout << "[Main]: staring barrier ..." << std::endl;
    std::thread barrierThread(doWorkImpl, std::move(barrier));
    std::cout << "[Main]: waiting barrier..." << std::endl;
    barrierFuture.wait();
    std::cout << "[Main]: finish barrier..." << std::endl;
    barrierThread.join();

    // future
    //
    // future from a packaged_task
    //
    std::packaged_task<int()> task([]{
            std::cout << "[Future1]: working... " << std::endl;
            return 3;});  // wrap the function
    std::future<int> fut1 = task.get_future();
    std::thread thread1(std::move(task)); // launch task on a thread

    // future from an async
    std::future<int> fut2 = std::async(std::launch::async, []{ 
            std::cout << "[Future2]: working... " << std::endl;
            return 33;});

    // future from a promise
    std::promise<int> pro1;
    std::future<int> fut3 = pro1.get_future();
    std::thread([&pro1]{
            std::cout << "[Future3]: working... " << std::endl;
            pro1.set_value_at_thread_exit(333);}).detach();

    std::cout << "[Main]: Waiting..." << std::endl;
    fut1.wait();
    fut2.wait();
    fut3.wait();
    std::cout << "[Main]: Done, Result is "<< fut1.get() << ", "
                                   << fut2.get() << ", "
                                   << fut3.get() << std::endl; 
    thread1.join();

} // if a1 is not done at this point, destructor of a1 prints "Hello 33" here

// cfenv: floating-point environment lib
// fenv_t: the type representing the entire floating-point environment
// fexcept_t: the type representing all floating-point status flags collectively
//

#pragma STDC FENV_ACCESS ON
volatile double zero = 0.0;
volatile double one = 1.0;
void cfenvTest()
{
    // feclearexcept() fetestexcept()
    std::feclearexcept(FE_ALL_EXCEPT);
    std::cout << "1.0/0.0 = "  << 1.0/zero << std::endl;
    if(std::fetestexcept(FE_DIVBYZERO))
    {
        std::cout << "division by zero..." << std::endl;
    }
    else
    {
        std::cout << "non-division by zero..." << std::endl;
    }

    std::feclearexcept(FE_ALL_EXCEPT);
    std::cout << "1.0/10 = "  << 1.0/10 << std::endl;
    if(std::fetestexcept(FE_INEXACT))
    {
        std::cout << "inexact result..." << std::endl;
    }
    else
    {
        std::cout << "non-inexact result..." << std::endl;
    }

    std::feclearexcept(FE_ALL_EXCEPT);
    std::cout << "sqrt(-1) = "  << std::sqrt(-1) << std::endl;
    if(std::fetestexcept(FE_INVALID))
    {
        std::cout << "invalid result..." << std::endl;
    }
    else
    {
        std::cout << "non-invalid result..." << std::endl;
    }

    // fesetround()
    std::fesetround(FE_DOWNWARD);
    std::cout << "rounding down: \n" << std::setprecision(50)
              << "          pi  = " << std::acos(-1.f) << "\n"
              << "stof(\"1.1\") = " << std::stof("1.1") << "\n"
              << "rint(2.1)     = " << std::rint(2.1) << "\n";

    std::fesetround(FE_UPWARD);
    std::cout << "rounding up: \n" << std::setprecision(50)
              << "          pi  = " << std::acos(-1.f) << "\n"
              << "stof(\"1.1\") = " << std::stof("1.1") << "\n"
              << "rint(2.1)     = " << std::rint(2.1) << "\n";

    static constexpr std::pair<const char*, const double> samples[]
    {
        {" 12.0", 12.0}, {" 12.1", 12.1}, {"-12.1", -12.1}, {" 12.5", 12.5},
        {"-12.5", -12.5}, {" 12.9", 12.9}, {"-12.9", -12.9}, {" 13.0", 13.0},
    };
    std::cout << 
        "| sample |   FE_DOWNWARD |   FE_UPWARD   |  FE_TONEAREST | FE_TOWARDZERO |"
        << std::endl;
    for(const auto& [str, fp] : samples)  // C++17 
    {
        std::cout << "| " << std::setw(6) << str << " | ";
        for(const auto dir : {FE_DOWNWARD, FE_UPWARD, FE_TONEAREST, FE_TOWARDZERO})
        {
            std::fesetround(dir);
            std::cout << std::setprecision(10) << std::setw(12) << std::nearbyint(fp) << "  | ";
        }
        std::cout << std::endl;
    }
}

// chrono: date and time lib
//


template<typename T1, typename T2>
using mul = std::ratio_multiply<T1,T2>;

//  duration diff with std::common_type<>
//

template<typename T, typename S>
constexpr auto durationDiff(const T& t, const S& s) -> typename std::common_type<T,S>::type
{
    typedef typename std::common_type<T,S>::type commonType;
    return commonType(t) - commonType(s);
}

void chronoTest()
{
    using namespace std::chrono_literals;
    // duration
    using shakes = std::chrono::duration<int, mul<std::deca, std::nano>>;
    using jiffies = std::chrono::duration<int, std::centi>;

    std::cout << " 1 second is : " << std::endl;

    // integer scale conversion with no precision loss: no cast
    std::cout << std::chrono::milliseconds(1s).count() << " milliseconds" << std::endl;
    std::cout << std::chrono::microseconds(1s).count() << " microseconds" << std::endl;
    std::cout << std::chrono::nanoseconds(1s).count() << " nanoseconds" << std::endl;
    std::cout << shakes(1s).count() << " shakes" << std::endl;
    std::cout << jiffies(1s).count() << " jiffies" << std::endl;

    // integer scale conversion with precision loss:  duration_cast
    std::cout << std::chrono::duration_cast<std::chrono::minutes>(1s).count() << " minutes" << std::endl;

    // zero()
    static_assert(std::chrono::seconds::zero() == std::chrono::minutes::zero(), "");
    static_assert(std::chrono::minutes::zero() == std::chrono::hours::zero(), "");
    static_assert(std::chrono::duration<int, std::deca>::zero() == std::chrono::milliseconds::zero(), "");
    static_assert(std::chrono::duration<long, std::exa>::zero().count() == std::chrono::nanoseconds::zero().count(), "");

    // max() min()
    constexpr uint64_t chrono_hours_max = std::chrono::hours::max().count();
    constexpr uint64_t chrono_minutes_max = std::chrono::minutes::max().count();
    constexpr uint64_t chrono_seconds_max = std::chrono::seconds::max().count();
    constexpr uint64_t chrono_hours_min = std::chrono::hours::min().count();
    constexpr uint64_t chrono_minutes_min = std::chrono::minutes::min().count();
    constexpr uint64_t chrono_seconds_min = std::chrono::seconds::min().count();
    std::cout << "chrono::hours::min() = " << chrono_hours_min << ", sizeof(chrono::hours) = " << sizeof(std::chrono::hours) << " bytes" << std::endl;
    std::cout << "chrono::hours::max() = " << chrono_hours_max << ", sizeof(chrono::hours) = " << sizeof(std::chrono::hours) << " bytes" << std::endl;
    std::cout << "chrono::minutes::min() = " << chrono_minutes_min << ", sizeof(chrono::minutes) = " << sizeof(std::chrono::minutes) << " bytes" << std::endl;
    std::cout << "chrono::minutes::max() = " << chrono_minutes_max << ", sizeof(chrono::minutes) = " << sizeof(std::chrono::minutes) << " bytes" << std::endl;
    std::cout << "chrono::seconds::max() = " << chrono_seconds_max << ", sizeof(chrono::seconds) = " << sizeof(std::chrono::seconds) << " bytes" << std::endl;
    std::cout << "chrono::seconds::min() = " << chrono_seconds_min << ", sizeof(chrono::seconds) = " << sizeof(std::chrono::seconds) << " bytes" << std::endl;

    constexpr uint64_t age_of_universe_in_years{13'787'000'000};
    constexpr uint64_t seconds_per_year{365'25 * 24 * 36};
    constexpr uint64_t age_of_universe_in_seconds{age_of_universe_in_years * seconds_per_year};

    std::cout 
        << "The Age of the universe is about "
        << std::scientific << std::setprecision(3)
        << static_cast<double>(age_of_universe_in_years) << " years or "
        << static_cast<double>(age_of_universe_in_seconds) << " seconds\n And the Age "
        << (age_of_universe_in_seconds <= chrono_seconds_max ?  " CAN " : " CANNOT ")
        << "expressed in SECONDS"
        << std::endl;

    //common_type
    constexpr auto ms = 30ms;
    constexpr auto us = 1100us;
    constexpr auto diff = durationDiff(ms, us);
    std::cout << ms.count() << " ms - " << us.count() << " us = " << diff.count()
        << (std::is_same<decltype(diff), decltype(ms)>() ? " ms" : " us") << std::endl;

    //time-point
    const std::chrono::time_point<std::chrono::system_clock> now_sys = std::chrono::system_clock::now();
    const std::chrono::time_point<std::chrono::steady_clock> now_sdy = std::chrono::steady_clock::now();
    const std::chrono::time_point<std::chrono::high_resolution_clock> now_hrc = std::chrono::high_resolution_clock::now(); // system(GCC) clock or steady clock(MSVC)
    // const std::chrono::time_point<std::chrono::utc_clock> now_utc = std::chrono::utc_clock::now(); // C++20
    const std::time_t time_sys = std::chrono::system_clock::to_time_t(now_sys);
    std::cout << std::put_time(std::localtime(&time_sys), "%F %T") << std::endl;
    // const std::time_t time_sdy = std::chrono::system_clock::to_time_t(now_sdy);
    const std::time_t time_hrc = std::chrono::system_clock::to_time_t(now_hrc);
    std::cout << std::put_time(std::localtime(&time_hrc), "%F %T") << std::endl;
}

// cinttypes: 
//

std::string  euclidean_div(int dividend, int divisor)
{
    auto dv = std::div(dividend, divisor);
    assert(dividend == divisor * dv.quot + dv.rem);
    assert(dv.quot == dividend / divisor);
    assert(dv.rem  == dividend % divisor);

    std::ostringstream out;
    out << std::showpos << dividend << " = " << divisor << " * (" << dv.quot << ") " << std::showpos <<  dv.rem;
    return out.str();
}

std::string myItoa(int n, int radix /*[2...16]*/)
{
    std::string buf;
    std::div_t dv{}; 
    dv.quot = n;

    // division remainder
    do{
        dv = std::div(dv.quot, radix);
        buf += "0123456789abcdef"[std::abs(dv.rem)];
    } while(dv.quot);

    if(n < 0)
    {
        buf += '-';
    }

    return {buf.rbegin(), buf.rend()};
}

void cInttypesTest()
{
    std::string str1 ="hellowold";
    std::string str2 ="12345A13";
    std::intmax_t v1 =  std::strtoimax(str1.c_str(), nullptr,36);
    std::cout << str1 << " in base 36 is " << v1 << " based in 10"  << std::endl;

    std::intmax_t v2 =  std::strtoimax(str2.c_str(), nullptr,10);
    std::cout << str2 << " in base 10 is " << v2 << " based in 10"  << std::endl;

    std::intmax_t v3 =  std::strtoimax(str2.c_str(), nullptr,16);
    std::cout << str2 << " in base 16 is " << v3<< " based in 10"  << std::endl;

    char *nptr = nullptr;
    v1 = std::strtoimax(str1.c_str(), &nptr, 30);
    if(nptr != &str1[0] + str1.size())
    {
        std::cout << str1 << " in base 30 is invalid. The first invalid digit is " << *nptr << std::endl;
    }

    v2 = std::strtoimax(str2.c_str(), &nptr, 10);
    if(nptr != &str2[0] + str2.size())
    {
        std::cout << str2 << " in base 10 is invalid. The first invalid digit is " << *nptr << std::endl;
    }

    v3 = std::strtoimax(str2.c_str(), &nptr, 16);
    if(nptr != &str2[0] + str2.size())
    {
        std::cout << str2 << " in base 16 is invalid. The first invalid digit is " << *nptr << std::endl;
    }

    std::printf("%u\n", sizeof(std::int64_t));
    std::printf("%s\n", PRId64);
    std::printf("%+" PRId64 "\n", INT64_MIN);
    std::printf("%+" PRId64 "\n", INT64_MAX);

    std::int64_t  n = 7;
    std::printf("%+" PRId64 "\n", n);

    // abs
    std::cout << std::showpos
        << "abs(+3) = " << std::abs(3) << '\n'
        << "abs(-3) = " << std::abs(-3) << '\n';
    std::cout << "abs(int64MIN) = " << std::abs(INT64_MIN) << std::endl;
    std::cout << "abs(int64MAX) = " << std::abs(INT64_MAX) << std::endl;
    std::cout << "imaxabs(int64MIN) = " << std::imaxabs(INT64_MIN) << std::endl;
    std::cout << "imaxabs(int64MAX) = " << std::imaxabs(INT64_MAX) << std::endl;

    // std::div && std::div_t
    std::cout << euclidean_div(369,10) << '\n'
              << euclidean_div(369,-10) << '\n'
              << euclidean_div(-369,10) << '\n'
              << euclidean_div(-369,-10) << '\n';

    std::cout << myItoa(12345, 10) << '\n'
              << myItoa(-12345, 10) << '\n'
              << myItoa(42, 2) << '\n'
              << myItoa(655352, 16) << '\n';
}

// forward_list: containers lib
// container that supports fast insertion and removal of elements from anywhere, but fast random access is NOT supported.
// Singly-linked list
// more space efficient storage Compared to std::list
//

template<typename T>
std::ostream& operator<<(std::ostream& s, const std::forward_list<T>& v)
{
    s.put('[');
    char comma[3] = {'\0',' ','\0'};
    for(const auto & e: v)
    {
        s << comma << e;
        comma[0] = ',';
    }
    return s << ']';
}

void forward_listTest()
{
    // C++11 initializer-list syntax
    std::forward_list<std::string> w1 {"frank", "is", "in", "qingdao"};
    std::cout << "w1 : " << w1 << std::endl;
    // w2 == w1
    std::forward_list<std::string> w2(w1.begin(), w1.end()); 
    std::cout << "w2 : " << w2 << std::endl;

    // w3 == w1
    std::forward_list<std::string> w3(w1); 
    std::cout << "w3 : " << w3 << std::endl;

    // w4 is {"tly", "tly", "tly"}
    std::forward_list<std::string> w4(3, "tly"); 
    std::cout << "w4 : " << w4 << std::endl;

    auto forwardPrint = [](std::string const comment, std::forward_list<int> const& container){
        auto size = 0;
        for(auto const& ele : container)
        {
            size++;
        }
        std::cout << comment << "{ ";
        for(auto const& ele: container)
        {
            std::cout << ele << (--size? ", " : " ");
        }
        std::cout << "}" << std::endl;
    };

    std::forward_list<int> x{ 1, 2, 3}, y,z;
    const auto w = {4, 5, 6, 7 };
    std::cout << "Initially: \n";
    forwardPrint("x = ", x);
    forwardPrint("y = ", y);
    forwardPrint("z = ", z);

    std::cout << "Copy assignment from x to y: \n";
    y = x;
    forwardPrint("x = ", x);
    forwardPrint("y = ", y);

    std::cout << "Move assignment from x to z: \n";
    z = std::move(x);
    forwardPrint("x = ", x);
    forwardPrint("z = ", z);

    std::cout << " Assignment of initializer list w to z: \n";
    z = w;
    forwardPrint("w = ", w);
    forwardPrint("z = ", z);

    // assign: Replace the contents of the  container

    std::forward_list<char> chars;
    auto print =[&](){
        for(char c: chars)
            std::cout << c << ' ';
        std::cout << std::endl;
    };

    chars.assign(3, 'a');
    print();
    const std::string ext(6, 'b');
    chars.assign(ext.begin(), ext.end());
    print();

    chars.assign({'C','+','+','1','1'});
    print();

    std::forward_list<char> letters {'o','m','g','w','t','f'};

    if(!letters.empty())
    {
        std::cout << "first: " << letters.front() << std::endl;
        std::cout << "begin: " << *letters.begin() << std::endl;
        auto endIt = letters.end();
        // auto pv = std::prev(endIt,1);
        // std::cout << "end: " << ((letters.end() == pv)? "nullptr" : *pv) << std::endl;
        std::cout << "cbegin: " << *letters.cbegin() << std::endl;
        // std::cout << "cend: " << *std::prev(letters.cend(), 1) << std::endl;
    }

    std::cout << "max_size: " << letters.max_size() << std::endl;

    auto func = [&](const std::string comment){
    std::cout << comment << ": [";
    for(auto &e : letters)
    {
        std::cout << e << ",";
    }
    std::cout << "]" << std::endl;
    };


    func("initialize");

    auto beginIt = letters.begin();
    letters.insert_after(beginIt, 'f');
    func("insert_after single");

    auto anotherIt = letters.begin();
    ++anotherIt;
    anotherIt  =  letters.insert_after(anotherIt, 3, 'r');
    func("insert_after multi");

    std::vector<char> vs = {'q','d'};
    anotherIt = letters.insert_after(anotherIt, vs.begin(), vs.end());
    func("insert_after range");

    letters.insert_after(anotherIt,'z');
    func("insert_after initializer-list");

    letters.emplace_after(beginIt,'s');
    func("emplace_after single");

    auto befIt = letters.before_begin();
    letters.emplace_after(befIt,'S');
    func("emplace_after before_begin single");

    letters.erase_after(letters.before_begin());
    func("erase_after before_begin single");

    letters.push_front('k');
    func("push_front single");

    letters.emplace_front('K');
    func("emplace_front single");

    letters.pop_front();
    func("pop_front single");

    std::forward_list<char> letter2 {'f','r','a','n','k'};

    auto func2 = [&](const std::string comment){
    std::cout << comment << ": [";
    for(auto &e : letter2)
    {
        std::cout << e << ",";
    }
    std::cout << "]" << std::endl;
    };

    func2("initial");

    letters.swap(letter2);
    func("letters after swap");
    func2("letter2 after swap");

    letters.swap(letter2);
    func("letters after swap twice");
    func2("letter2 after swap twice");

    letters.sort();
    func("letters after sort");
    letter2.sort();
    func2("letter2 after sort");

    letter2.merge(letters);
    func("letters after merge");
    func2("letter2 after merge");

    letter2.unique();
    func2("letter2 after unique");

    // emplace_after
    struct Sum
    {
        std::string remark;
        int sum;
        Sum(std::string remark, int sum) : remark{std::move(remark)}, sum{sum} {}
        void print() const
        {
            std::cout << remark << " = " << sum << std::endl;
        }
    };

    std::forward_list<Sum> lSum;
    auto iter = lSum.before_begin();
    std::string str{"1"};
    for(int i{1}, sum{1}; i != 10; sum += i)
    {
        iter = lSum.emplace_after(iter, str, sum);
        ++i;
        str += " + " + std::to_string(i);
    }

    for(const Sum& s : lSum) s.print();
}

// initializer_list<T>:
// lightweight proxy object that provides access to an array of objects of type const T

template<typename T>
struct S
{
    std::vector<T> v;
    S(std::initializer_list<T> l) : v(l)
    {
        std::cout << "constructed with a " << l.size() << "-element list" << std::endl;
    }

    void append(std::initializer_list<T> l)
    {
        v.insert(v.end(), l.begin(), l.end());
    }

    std::pair<const T*, std::size_t> c_arr() const
    {
        return {&v[0], v.size()};
    }
};

template<typename T>
void templated_fn(T) {}

void initializer_listTest()
{
    S<int> s = {1,2,3,4,5};
    s.append({6,7});
    std::cout << "The vector size is " << s.c_arr().second << std::endl;

    for(auto n: s.v)
    {
        std::cout << n << " ";
    }
    std::cout << std::endl;

    for(int x : {1,2,3,4})
        std::cout << x << " ";
    std::cout << std::endl;

    auto al = {5,6,7};
    std::cout << "{5,6,7} size is " << al.size() << std::endl;

 templated_fn(std::initializer_list<int>{1,2,3});
 // templated_fn({1,2,3}); error: CANNOT deduce type
}

template <int Height = 5, int BarWidth = 1, int Padding = 1, int Offset = 0, class Seq>
void draw_vbars(Seq&& s, const bool DrawMinMax = true) {
    static_assert((Height > 0) && (BarWidth > 0) && (Padding >= 0) && (Offset >= 0));
    auto cout_n = [](auto&& v, int n = 1) { while (n-- > 0) std::cout << v; };
    const auto [min, max] = std::minmax_element(std::cbegin(s), std::cend(s));
    std::vector<std::div_t> qr;
    for (typedef decltype(*cbegin(s)) V; V e : s) 
        qr.push_back(std::div(std::lerp(V(0), Height*8, (e - *min)/(*max - *min)), 8)); // C++20 std::lerp
    for (auto h{Height}; h-- > 0; cout_n('\n')) {
        cout_n(' ', Offset);
        for (auto dv : qr) {
            const auto q{dv.quot}, r{dv.rem};
            unsigned char d[] { 0xe2, 0x96, 0x88, 0 }; // Full Block: '█'
            q < h ? d[0] = ' ', d[1] = 0 : q == h ? d[2] -= (7 - r) : 0;
            cout_n(d, BarWidth), cout_n(' ', Padding);
        }
        if (DrawMinMax && Height > 1)
            Height - 1 == h ? std::cout << "┬ " << *max:
                          h ? std::cout << "│ "
                            : std::cout << "┴ " << *min;
    }
}

void randomTest()
{
    std::mt19937 gen32;
    std::mt19937_64 gen64;
    for(auto n = 1; n != 10'000; gen32(),gen64(),++n);
    std::cout << "gen32() = " << gen32() << std::endl;
    std::cout << "gen64() = " << gen64() << std::endl;
    std::cout << "gen32 min = " << gen32.min() << std::endl;
    std::cout << "gen32 max = " << gen32.max() << std::endl;
    std::cout << "gen64 min = " << gen64.min() << std::endl;
    std::cout << "gen64 max = " << gen64.max() << std::endl;

    // random_device
    std::random_device rd;
    std::map<int, int> hist;
    std::uniform_int_distribution<int> dist(0,9);
    for(int n = 0; n < 20000; ++n)
    {
        ++hist[dist(rd)];
    }

    std::cout << "random_device min() = " << dist.min() << std::endl;
    std::cout << "random_device max() = " << dist.max() << std::endl;

    std::cout << "uniform_int_distribution: " << std::endl;
    for(auto p : hist)
    {
        std::cout << p.first << ": "
            << std::string(p.second/100, '*') << std::endl;
    }

    // uniform_int_distribution:
    // uniform_real_distribution:
    //
    // simulate throwing 6-sided dice
    std::random_device rd2; // will be used to obtain a seed for the random number engine
    std::mt19937 gen2(rd2()); // standard mersenne_twister_engine seeded with rd2()

    std::uniform_int_distribution<> dist2(1,6);
    std::uniform_real_distribution<> dist3(1.0,2.0);

    // Use dist to transform the random unsigned int generated by gen into a doule or int [start, stop]
    for(int n = 0; n < 10; ++n)
    {
        std::cout << dist2(gen2) << ' ';
    }
    std::cout << std::endl;

    for(int n = 0; n < 10; ++n)
    {
        std::cout << std::setprecision(3) << dist3(gen2) << ' ';
    }
    std::cout << std::endl;

    // bernoulli_distribution
    // produces random boolean values, according to the discrete probability function

    std::random_device rd3;
    std::mt19937 gen3(rd3());
    // give "true"  1/4 of the time
    // give "false" 3/4 of the time
    std::bernoulli_distribution d(0.25);
    std::map<bool, int> map1;
    for(int n{0}; n < 10000; ++n)
    {
        ++map1[d(gen3)];
    }

    std::cout << "bernoulli_distribution:" << std::endl;
    for(auto p : map1)
    {
        std::cout << std::boolalpha << std::setw(5) << p.first << ' ' << std::string(p.second/500, '*') << std::endl;
    }

    // binomial_distribution
    // produces random non-negative integer value
    std::random_device rd4;
    std::mt19937 gen4(rd4());
    // perform 4 trials, each succeeds 1 in 2 times
    std::binomial_distribution<> bd(4,0.5);
    std::map<int, int> map2;
    for(int n = 0; n < 10000; ++n)
    {
        ++map2[bd(gen4)];
    }

    std::cout << "binomial_distribution: " << std::endl;
    for(auto p: map2)
    {
        std::cout << p.first << ' '
            << std::string(p.second/100,'*')<< std::endl;
    }

    // negative_binomial_distribution
    // produces random non-negative integer value

    std::random_device rd5;
    std::mt19937 gen5(rd5());
    std::negative_binomial_distribution<> nbd(5, 0.75);
    std::map<int, int> map5;
    for(int i = 0; i< 10000; ++i)
    {
        ++map5[nbd(gen5)];
    }
    std::cout << "negative_binomial_distribution: " << std::endl;
    for(auto p: map5)
    {
        std::cout << std::setw(2) << p.first << ' ' << std::string(p.second/100, '*') << std::endl;
    }

    // geometric_distribution
    // produces random non-negative integer value

    std::random_device rd6;
    std::mt19937 gen6(rd6());
    std::geometric_distribution<> gd; // same as std::negative_binomial_distribution<> d(1,0.5)
    std::map<int, int> map6;
    for(int i = 0; i< 10000; ++i)
    {
        ++map6[gd(gen6)];
    }
    std::cout << "geometric_distribution: " << std::endl;
    for(auto p: map6)
    {
        std::cout << std::setw(2) << p.first << ' ' << std::string(p.second/100, '*') << std::endl;
    }

    // poisson_distribution
    // produces random non-negative integer value

    std::random_device rd7;
    std::mt19937 gen7(rd7());
    std::poisson_distribution<> pd; 
    std::map<int, int> map7;
    for(int i = 0; i< 10000; ++i)
    {
        ++map7[pd(gen7)];
    }
    std::cout << "poisson_distribution: " << std::endl;
    for(auto p: map7)
    {
        std::cout << std::hex << p.first << ' ' << std::string(p.second/100, '*') << std::endl;
    }

    // exponential_distribution
    // produces random non-negative floating-point value

    std::random_device rd8;
    std::mt19937 gen8(rd8());
    std::exponential_distribution<> ed(1); 
    std::map<int, int> map8;
    for(int i = 0; i< 10000; ++i)
    {
        ++map8[2 * ed(gen8)];
    }
    std::cout << "exponential_distribution: " << std::endl;
    for(auto p: map8)
    {
        std::cout << std::dec << std::fixed << std::setprecision(1) << p.first/2.0 << '-' << (p.first + 1)/2.0 << ' ' << std::string(p.second/200, '*') << std::endl;
    }

    // gamma_distribution
    // produces random non-negative floating-point value

    std::random_device rd9;
    std::mt19937 gen9(rd9());
    std::gamma_distribution<> gmd(1,2); 
    std::map<int, int> map9;
    for(int i = 0; i< 10000; ++i)
    {
        ++map9[2 * gmd(gen9)];
    }
    std::cout << "gamma_distribution: " << std::endl;
    for(auto p: map9)
    {
        std::cout << std::dec << std::fixed << std::setprecision(1) << p.first/2.0 << '-' << (p.first + 1)/2.0 << ' ' << std::string(p.second/100, '*') << std::endl;
    }

    // extreme_value_distribution
    std::random_device rd10;
    std::mt19937 gen10(rd10());
    std::extreme_value_distribution<> evd(-1.618f, 1.618f); 
    std::map<int, int> map10;
    const int norm = 10'000;
    const float cutoff = 0.000'3f;
    for(int i = 0; i< norm; ++i)
    {
        ++map10[std::round(evd(gen10))];
    }

    std::vector<float> bars;
    std::vector<int> indices;
    for(const auto& [n,p] : map10)
    {
        float x = p *(1.0f / norm);
        if(x > cutoff)
        {
            bars.push_back(x);
            indices.push_back(n);
        }
    }

    std::cout << "extreme_value_distribution: " << std::endl;
    draw_vbars<8,4>(bars);

    for(auto n: indices)
    {
        std::cout <<  std::dec << " " << std::setw(2) << n << " ";
    }
    std::cout << std::endl;

    // normal_distribution
    std::random_device rd11;
    std::mt19937 gen11(rd11());
    std::normal_distribution<> nd(5,2); 

    std::map<int, int> map11;
    for(int i = 0; i< 10000; ++i)
    {
        ++map11[std::round(nd(gen11))];
    }
    std::cout << "normal_distribution: " << std::endl;
    for(auto p: map11)
    {
        std::cout <<  std::dec << std::setw(2) << p.first << ' ' << std::string(p.second/200, '*') << std::endl;
    }

    // lognormal_distribution
    std::random_device rd12;
    std::mt19937 gen12(rd12());
    std::lognormal_distribution<> lnd(1.6,0.25); 

    std::map<int, int> map12;
    for(int i = 0; i< 10000; ++i)
    {
        ++map12[std::round(lnd(gen12))];
    }
    std::cout << "lognormal_distribution: " << std::endl;
    for(auto p: map12)
    {
        std::cout << std::dec << std::fixed << std::setprecision(1) << std::setw(3) << p.first << ' ' << std::string(p.second/200, '*') << std::endl;
    }

    // chi_squared_distribution
    std::random_device rd13;
    std::mt19937 gen13(rd13());
    auto x2 = [&gen13](const float dof) {
        std::chi_squared_distribution<float> csd(dof); 
        const int norm = 1'00'00;
        const float cutoff = 0.002f;
        std::map<int, int> map13;
        for(int i = 0; i< norm; ++i)
        {
            ++map13[std::round(csd(gen13))];
        }
        std::vector<float> bars;
        std::vector<int> indices;
        for(auto const& [n,p] : map13)
        {
            if(float x = p *(1.0/norm); cutoff < x)
            {
                bars.push_back(x);
                indices.push_back(n);
            }
        }
        std::cout <<  std::dec << "dof = " << dof  << std::endl;
        draw_vbars<4,3>(bars);
        for(int n : indices) 
        {
            std::cout << std::dec << "" << std::setw(3) << n <<  " ";
        }
        std::cout << "\n\n";
    };

    std::cout << "lognormal_distribution: " << std::endl;

    for(float dof : {1.f, 2.f,3.f, 4.f, 6.f, 9.f}) x2(dof);

    // cauchy_distribution
    std::random_device rd14;
    std::mt19937 gen14(rd14());
    auto cauchy = [&gen14](const float x0, const float r) {
        std::cauchy_distribution<float> cd(x0, r); 
        const int norm = 1'00'00;
        const float cutoff = 0.005f;
        std::map<int, int> map14;
        for(int i = 0; i< norm; ++i)
        {
            ++map14[std::round(cd(gen14))];
        }
        std::vector<float> bars;
        std::vector<int> indices;
        for(auto const& [n,p] : map14)
        {
            if(float x = p *(1.0/norm); cutoff < x)
            {
                bars.push_back(x);
                indices.push_back(n);
            }
        }
        std::cout << "x0 = " << x0 << ", r = " << r << std::endl;
        draw_vbars<4,3>(bars);
        for(int n : indices) 
        {
            std::cout << std::dec << "" << std::setw(3) << n <<  " ";
        }
        std::cout << "\n\n";
    };

    std::cout << "cauchy_distribution: " << std::endl;

    cauchy(-2.0f, 0.50f);
    cauchy(+0.0f, 1.25f);

    // fisher_f_distribution
    std::random_device rd15;
    std::mt19937 gen15(rd15());
    auto fisher = [&gen15](const float d1, const float d2) {
        std::fisher_f_distribution<float> fd(d1, d2); 
        const int norm = 1'00'00;
        const float cutoff = 0.002f;
        std::map<int, int> map15;
        for(int i = 0; i< norm; ++i)
        {
            ++map15[std::round(fd(gen15))];
        }
        std::vector<float> bars;
        std::vector<int> indices;
        for(auto const& [n,p] : map15)
        {
            if(float x = p *(1.0/norm); cutoff < x)
            {
                bars.push_back(x);
                indices.push_back(n);
            }
        }
        std::cout << "d1 = " << d1 << ", d2 = " << d2 << std::endl;
        draw_vbars<4,3>(bars);
        for(int n : indices) 
        {
            std::cout << std::dec << "" << std::setw(3) << n <<  " ";
        }
        std::cout << "\n\n";
    };

    std::cout << "fisher_f_distribution: " << std::endl;

    fisher(1.0f, 5.0f);
    fisher(15.0f, 10.0f);
    fisher(100.0f, 3.0f);

    // student_t_distribution
    std::random_device rd16;
    std::mt19937 gen16(rd16());
    std::student_t_distribution<> sd{10.0f}; 
    std::map<int, int> map16;
    for(int i = 0; i< norm; ++i)
    {
        ++map16[std::round(sd(gen16))];
    }

    bars.clear();
    indices.clear();
    for(auto const& [n,p] : map16)
    {
        if(float x = p *(1.0/norm); cutoff < x)
        {
            bars.push_back(x);
            indices.push_back(n);
        }
    }
    std::cout << "student_tdistribution: " << std::endl;
    draw_vbars<8,5>(bars);
    for(int n : indices) 
    {
        std::cout << std::dec << "" << std::setw(3) << n <<  " ";
    }
    std::cout << "\n";

    // discrete_distribution
    std::random_device rd17;
    std::mt19937 gen17(rd17());
    std::discrete_distribution<> dd({40, 10, 10, 40}); 
    std::map<int, int> map17;
    for(int i = 0; i< 10000; ++i)
    {
        ++map17[dd(gen17)];
    }
    std::cout << "gamma_distribution: " << std::endl;
    for(const auto &[num, count] : map17)
    {
        std::cout << std::dec << num << " generated " << std::setw(6) << count << " times" << std::endl;
    }

    // piecewise_constant_distribution
    std::random_device rd18;
    std::mt19937 gen18(rd18());
    std::vector<double> i{0, 1, 10, 15};
    std::vector<double> w{1, 0, 1};
    std::piecewise_constant_distribution<> pcd(i.begin(), i.end(), w.begin()); 
    std::map<int, int> map18;
    for(int i = 0; i< 10000; ++i)
    {
        ++map18[pcd(gen18)];
    }
    std::cout << "piecewise_constant_distribution: " << std::endl;
    for(auto p : map18)
    {
        std::cout << std::hex << std::uppercase << p.first << ' ' << std::string(p.second/100, '*') << '\n';
    }

    // piecewise_linear_distribution
    std::random_device rd19;
    std::mt19937 gen19(rd19());
    std::piecewise_linear_distribution<> pld(i.begin(), i.end(), w.begin()); 
    std::map<int, int> map19;
    for(int i = 0; i< 10000; ++i)
    {
        ++map19[pld(gen19)];
    }
    std::cout << "piecewise_linear_distribution: " << std::endl;
    for(auto p : map19)
    {
        std::cout << std::dec << std::setw(2) << std::setfill('0') << p.first << ' ' << std::string(p.second/100, '*') << '\n';
    }

    // seed_seq
    // consumes a sequence of integer-valued data and produces a requested number of unsigned integer values i based on the consumed data. The produced values are distributed over the entire 32-bit range even if the consumed values are close

    std::seed_seq seq{1,2,3,4,5};
    std::vector<std::uint32_t> seeds(10);
    seq.generate(seeds.begin(), seeds.end());
    std::cout << "seed_seq: " << std::endl;
    for(auto n : seeds)
    {
        std::cout << n << std::endl;
    }

    // generate_canonical
    // generates a random floating point number in range [0,1)
    std::random_device rdc;
    std::mt19937 genc(rdc());
    std::cout << "generate_canonical: " << std::endl;
    for(int n = 0; n < 10; ++n)
    {
        std::cout << std::generate_canonical<double, 10>(genc) << ' ';
    }
    std::cout << std::endl;
}

void ratioTest()
{
    using two_third  = std::ratio<2,3>;
    using one_sixth  = std::ratio<1,6>;

    using sum = std::ratio_add<two_third, one_sixth>;
    using sub = std::ratio_subtract<two_third, one_sixth>;
    using mul = std::ratio_multiply<two_third, one_sixth>;
    using div = std::ratio_divide<two_third, one_sixth>;
    std::cout << "2/3 + 1/6 = " << sum::num << '/' << sum::den << '\n';
    std::cout << "2/3 - 1/6 = " << sub::num << '/' << sub::den << '\n';
    std::cout << "2/3 * 1/6 = " << mul::num << '/' << mul::den << '\n';
    std::cout << "2/3 / 1/6 = " << div::num << '/' << div::den << '\n';
    constexpr bool eq = std::ratio_equal<std::ratio<2,3>, std::ratio<4,6>>::value;
    std::cout << "2/3 " << (eq? "==" : "!=" ) << " 4/6" << std::endl;
    constexpr bool neq = std::ratio_not_equal<std::ratio<2,3>, std::ratio<1,6>>::value;
    std::cout << "2/3 " << (neq? "!=" : "==" ) << " 1/6" << std::endl;
    constexpr bool gt = std::ratio_greater<std::ratio<2,3>, std::ratio<1,6>>::value;
    std::cout << "2/3 " << (gt? ">" : "!>" ) << " 1/6" << std::endl;
    constexpr bool gte = std::ratio_greater_equal<std::ratio<2,3>, std::ratio<4,6>>::value;
    std::cout << "2/3 " << (gte? ">=" : "<" ) << " 4/6" << std::endl;

    std::cout << "micro = " <<  std::micro::num << '/' << std::micro::den << std::endl;
    std::cout << "centi = " <<  std::centi::num << '/' << std::centi::den << std::endl;
    std::cout << "deci = " <<  std::deci::num << '/' << std::deci::den << std::endl;
    std::cout << "deca = " <<  std::deca::num << '/' << std::deca::den << std::endl;
}

// regex
void regexTest()
{
    // regex_match only considers full matches, and the same regex may give different matches between regex_match and regex_search
    std::regex re("Get|GetValue");
    std::cmatch m;
    auto res = std::regex_search("GetValue", m, re);
    std::cout << "search res " << std::boolalpha << res << ", match: " << m[0] << std::endl;
    res = std::regex_match("GetValue", m, re);
    std::cout << "match res " << std::boolalpha << res << ",  match: " << m[0] << std::endl;
    res = std::regex_search("GetValues", m, re);
    std::cout << "search res " << std::boolalpha << res << ", match: " << m[0] << std::endl;
    res = std::regex_match("GetValues", m, re);
    std::cout << "match res " << std::boolalpha << res << ", match: " << m[0] << std::endl;

    // regex_match

    // simple regular expression
    const std::string fnames[] = {"foo.txt", "bar.txt", "baz.bat", "frank"};
    const std::regex txt_regex("[a-z]+\\.txt");
    for(const auto &fname : fnames)
    {
        std::cout << fname << ": " << std::regex_match(fname, txt_regex) << std::endl;
    }

    // extraction of a sub-match
    const std::regex base_regex("([a-z]+)\\.txt");
    std::smatch base_match;
    for(const auto &fname: fnames)
    {
        if(std::regex_match(fname, base_match, base_regex))
        {
            // The first sub_match is the whole string;
            // the next sub_match is the first parenthesized expression
            if(base_match.size() == 2)
            {
                std::ssub_match whole_match = base_match[0];
                std::ssub_match base_sub_match = base_match[1];
                std::string whole = whole_match.str();
                std::string base = base_sub_match.str();
                std::cout << whole << " has a base of " << base << std::endl;
            }
        }
    }

    const std::regex pieces_regex("([a-z]+)\\.([a-z]+)");
    std::smatch pieces_match;
    for(const auto &fname: fnames)
    {
        if(std::regex_match(fname, pieces_match, pieces_regex))
        {
            std::cout << fname << ": " << std::endl;
            for(size_t i = 0; i < pieces_match.size(); ++i)
            {
                std::ssub_match sub_match = pieces_match[i];
                std::string piece = sub_match.str();
                std::cout << "submatch " << i << " :  " << piece  << std::endl;
            }
        }
    }

    // regex_search
    std::string lines[] = {"Roses are #ff0000", "violets are #0000ff", "frank hello"};
    std::regex color_regex("#([a-f0-9]{2})""([a-f0-9]{2})""([a-f0-9]{2})");

    // simple match
    for(const auto &line : lines)
    {
        std::cout << line << ": " << std::boolalpha << std::regex_search(line, color_regex) << std::endl;
    }
    std::cout << std::endl;

    // show contents of marked subexpressions 
    std::smatch color_match;
    for(const auto &line: lines)
    {
        if(std::regex_search(line, color_match, color_regex))
        {
            std::cout << "matches for " << line << "\n";
            std::cout << "prefix: " << color_match.prefix() << "\n" ;
            for(auto i = 0; i < color_match.size(); ++i)
            {
                std::cout << i << ": " << color_match[i] <<std::endl;
            }
            std::cout << "suffix: " << color_match.suffix() << "\n" ;
            std::cout << std::endl; 
        }
    }

    // repeated search
    std::string log(R"(
    Speed:  366
    Mass:   36
    Speed:  367
    Mass:   37
    Speed:  368
    Mass:   38)");

    std::regex r(R"(Speed:\t\d*)");
    std::smatch sm;
    while(regex_search(log, sm, r))
    {
        std::cout << sm.str() << std::endl;
        log = sm.suffix();
    }

    // C-style
    std::cmatch cm;
    std::string org_str = "this is frank";
    // const char* org_str = "this is frank";
    auto reg_str = std::regex("frank");
    if(std::regex_search(org_str.c_str(), cm, reg_str))
    {
        std::cout << "Found! " << cm[0] << " at position of " << cm.prefix().length() << " in " << org_str << std::endl;
    }

    // regex_replace
    std::string text = "Quick brown fox";
    std::regex rer("a|e|i|o|u");
    std::regex_replace(std::ostreambuf_iterator<char>(std::cout), text.begin(), text.end(), rer, "*");
    std::cout << std::endl;
    std::cout << std::regex_replace(text, rer, "[$&]") << std::endl;

    // regex_iterator
    std::regex words_reg("[^\\s]+");
    auto beg = std::sregex_iterator(text.begin(),text.end(), words_reg);
    auto end = std::sregex_iterator();

    std::cout <<"Found " << std::distance(beg, end) << " words as below: " << std::endl;
    for(auto i = beg; i!= end; ++i)
    {
        std::smatch sm = *i;
        std::string str = sm.str();
        std::cout << str << "\n";
    }

    //ready()
    //indicates if the match results are ready or not
    std::string target("pattern");
    std::smatch sm1;
    std::cout << "default constructed smatch is " 
        << (sm1.ready() ? " ready" : " not ready") << std::endl;
    std::regex re1("tte");
    std::regex_search(target, sm1, re1);
    std::cout << "after search, smatch is " 
        << (sm1.ready() ? " ready" : " not ready") << std::endl;
}

// tuple:  fixed-size collection of heterogeneous values
// generalization of std::pair

std::tuple<int,int> foo_tuple()
{
    const int type =0;
    switch (type)
    {
        case 0:
            return {1,-1};
        case 1:
            return std::tuple<int,int>{1,-1};
        case 2:
            return std::make_tuple<int,int>(1,-2);
        default:
            return std::make_tuple(1,-2);
    }
}

std::tuple<double, char, std::string> getStudent(int id)
{
    switch(id)
    {
        case  0:
            return std::make_tuple(3.8,'A',"Lisa");
        case  1:
            return std::make_tuple(2.9,'C',"Milhouse");
        case  2:
            return std::make_tuple(1.7,'D',"Ralph");
        default:
            throw std::invalid_argument("id");
    }
}

std::tuple<int, int> func()
{
    int x = 5;
    return std::make_tuple(x, 7);
}

// helper function to print a tuple of any size
template<class Tuple, std::size_t N>
struct TuplePrinter
{
    static void print(const Tuple& t)
    {
        TuplePrinter<Tuple, N-1>::print(t);
        std::cout << ", " << std::get<N-1>(t);
    }
};

// particial specialization 
template<class Tuple>
struct TuplePrinter<Tuple, 1>
{
    static void print(const Tuple& t)
    {
        std::cout << std::get<0>(t);
    }
};

template<typename... Args, std::enable_if_t<sizeof...(Args) == 0, int> = 0>
void print(const std::tuple<Args...>& t)
{
    std::cout << "()\n";
}

template<typename... Args, std::enable_if_t<sizeof...(Args) != 0, int> = 0>
void print(const std::tuple<Args...>& t)
{
    std::cout << "(";
    TuplePrinter<decltype(t), sizeof...(Args)>::print(t);
    std::cout << ")\n";
}

void tupleTest()
{
    auto s0 = getStudent(0);
    std::cout << "ID: 0, "
        << "GPA: " << std::get<0>(s0) << ","
        << "grade: " << std::get<1>(s0) << ","
        << "name: " << std::get<2>(s0) << "\n";

    double gpa1;
    char grade1;
    std::string name1;
    std::tie(gpa1, grade1, name1) = getStudent(1);
    std::cout << "ID: 1, "
        << "GPA: " << gpa1 << ","
        << "grade: " << grade1 << ","
        << "name: " << name1 << "\n";

    //C++17 structured binding:
    auto [gpa2, grade2, name2] = getStudent(2);
    std::cout << "ID: 2, "
        << "GPA: " << gpa2 << ","
        << "grade: " << grade2 << ","
        << "name: " << name2 << "\n";

    int n = 1;
    auto t = std::make_tuple(10, "test", 3.14, std::ref(n), n);
    n = 7;
    std::cout << "The value of t is " << "(" 
        << std::get<0>(t) << "," 
        << std::get<1>(t) << "," 
        << std::get<2>(t) << "," 
        << std::get<3>(t) << "," 
        << std::get<4>(t) << ")\n";

    int a, b;
    std::tie(a, b) = func();
    std::cout << "get tuple from func: " << a << " " << b << std::endl;

    // get()
    // index-based access
    auto t2 =std::make_tuple(1, "foo" ,3.14);
    std::cout << "(" << std::get<0>(t2) << "," 
        << std::get<1>(t2) << ","
        << std::get<2>(t2) << ")\n";

    // type-based access
    std::cout << "(" << std::get<int>(t2) << ","
        << std::get<const char*>(t2) << ","
        << std::get<double>(t2) << ")\n";

    // std::tie
    // creates a tuple of lvalue references to its arguments or instance of std::ignore
    struct S{
        int n;
        std::string s;
        float d;
        bool operator <(const S& rhs) const
        {
            return std::tie(n,s,d) < std::tie(rhs.n, rhs.s, rhs.d);
        }
    };

    std::set<S> setS;
    S val{3, "PI", 3.14};
    std::set<S>::iterator iter;
    bool inserted;

    // unpacks the return value of insert into iter and inserted
    std::tie(iter, inserted) = setS.insert(val);
    std::tie(std::ignore, inserted) = setS.insert(val); // ignore tie first argument

    if(inserted)
    {
        std::cout << "value inserted successfully" << std::endl;
    }
    else
    {
        std::cout << "value inserted failure" << std::endl;
    }

    // piecewise_construct
    // an empty class tag(maker) type used to  disambiguate between different functions that take two tuple arguments

    struct Foo
    {
        Foo(std::tuple<int, float>)
        {
            std::cout << "constructed a Foo from a tuple" << std::endl;
        }
        Foo(int, float)
        {
            std::cout << "constructed a Foo from a int and a float" << std::endl;
        }
    };

    std::tuple<int, float> pwc(1,3.14);
    std::pair<Foo, Foo> p1(pwc, pwc);  // from a tuple
    std::pair<Foo, Foo> p2(std::piecewise_construct, pwc, pwc); // unpack tuple to function arguments: from a int and a float

    // forward_as_tuple
    // constructs a tuple of references to the arguments in args suitable fro forwarding as an argument to a function
    std::map<int, std::string> m;
    m.emplace(std::piecewise_construct, std::forward_as_tuple(10), std::forward_as_tuple(20, 'a'));
    std::cout << "m[10] = " << m[10] << std::endl;

    // The following is an error: 
    // it produces a std::tuple<int&&, char&&> holding two dangling references
    //
    // auto tt = std::forward_as_tuple(30,'b');
    // m.emplace(std::piecewise_construct, std::forward_as_tuple(10), tt);
    // std::cout << "m[10] = " << m[10] << std::endl;

    // tuple_cat
    // constructs a tuple that is a concatenation of all tuples in args
    std::tuple<int, std::string, float> tp1(10, "Test", 3.14);
    print(tp1);
    n = 7;
    auto tp2  = std::tuple_cat(tp1, std::make_tuple("Foo", "bar"), tp1, std::tie(n));
    n = 43;
    print(tp2);

    // swap
    std::tuple<int, std::string, float> tp11{42, "ABC", 2.7}, tp22;
    tp22 = std::make_tuple(10, "123", 3.14);

    auto printP = [&](auto rem)
    {
        std::cout << rem
                  << "tp11 = {" << std::get<0>(tp11)
                  << ", "       << std::get<1>(tp11)
                  << ", "       << std::get<2>(tp11) << "}, "
                  << "tp22 = {" << std::get<0>(tp22)
                  << ", "       << std::get<1>(tp22)
                  << ", "       << std::get<2>(tp22) << "}\n";
    };

    printP("Before tp11.swap(tp22): "); 

    tp11.swap(tp22);
    printP("After tp11.swap(tp22): "); 

    std::swap(tp11, tp22);
    printP("After swap(tp11, tp22): "); 

    // type_index
    // a wrapper class around a std::type_info object
    // can be use as index in associative and unordered associative containers
    // constructed by a type_info object and the relationship with type_info object is maintained through a pointer

    struct A
    {
        virtual ~A() {}
    };
    struct B : A{};
    struct C : A{};

    std::unordered_map<std::type_index, std::string> type_names;
    type_names[std::type_index(typeid(int))] = "int";
    type_names[std::type_index(typeid(double))] = "double";
    type_names[std::type_index(typeid(bool))] = "bool";
    type_names[std::type_index(typeid(A))] = "A";
    type_names[std::type_index(typeid(B))] = "B";
    type_names[std::type_index(typeid(C))] = "C";

    int i;
    double d;
    bool bl;
    A aa;
    std::unique_ptr<A> bb(new B);
    std::unique_ptr<A> cc(new C);

    std::cout << "i is " << type_names[std::type_index(typeid(i))] << "\n";
    std::cout << "d is " << type_names[std::type_index(typeid(d))] << "\n";
    std::cout << "bl is " << type_names[std::type_index(typeid(bl))] << "\n";
    std::cout << "aa is " << type_names[std::type_index(typeid(aa))] << "\n";
    std::cout << "*bb is " << type_names[std::type_index(typeid(*bb))] << "\n";
    std::cout << "*cc is " << type_names[std::type_index(typeid(*cc))] << "\n";
}

void typeTraitsTest()
{
    typedef std::integral_constant<int, 3> three_t;
    typedef std::integral_constant<int, 6> six_t;
    // static_assert(std::is_same<three_t, six_t>::value, "three_t and six_t are not equal!");  // static_assert failed
    static_assert(three_t::value * 2 == six_t::value, "3 * 2 ! = 6");

    enum class ec {e1, e2};
    typedef std::integral_constant<ec, ec::e1> ec1_t;
    typedef std::integral_constant<ec, ec::e2> ec2_t;
    static_assert(ec1_t::value == ec1_t(), "ec1::value is not equal to ec1()"); 
    static_assert(ec2_t::value == ec2_t(), "ec2::value is not equal to ec2()"); 
    static_assert(ec1_t() == ec::e1, "ec1() is equal to ec::e1"); 

#define SHOW(...) std::cout << std::setw(23) << #__VA_ARGS__ << " == " << __VA_ARGS__ << "\n"
    class A{};
    struct B  { enum E {};};
    enum E: int {};
    enum class  Ec : int {};
    struct S{};
    typedef union {int a; float b;} U;
    class C{};
    class D{int func() const&;};
    auto func = []{};

    std::cout << std::boolalpha;
    SHOW(std::is_void<void>::value);
    SHOW(std::is_void<int>::value);
    SHOW(std::is_null_pointer<decltype(nullptr)>::value);
    SHOW(std::is_null_pointer<int*>::value);
    SHOW(std::is_pointer<decltype(nullptr)>::value);
    SHOW(std::is_pointer<int*>::value);
    SHOW(std::is_integral<A>::value);
    SHOW(std::is_integral<E>::value);
    SHOW(std::is_integral<float>::value);
    SHOW(std::is_integral<int>::value);
    SHOW(std::is_integral<const int>::value);
    SHOW(std::is_integral<volatile int>::value);
    SHOW(std::is_floating_point<float>::value);
    SHOW(std::is_floating_point<float&>::value);
    SHOW(std::is_floating_point<double>::value);
    SHOW(std::is_floating_point<double&>::value);
    SHOW(std::is_floating_point<int>::value);
    SHOW(std::is_array<A>::value);
    SHOW(std::is_array<A[]>::value);
    SHOW(std::is_array<A[3]>::value);
    SHOW(std::is_array<int[3]>::value);
    SHOW(std::is_array<std::array<int, 3>>::value);
    SHOW(std::is_enum<A>::value);
    SHOW(std::is_enum<B>::value);
    SHOW(std::is_enum<E>::value);
    SHOW(std::is_enum<Ec>::value);
    SHOW(std::is_enum<B::E>::value);
    SHOW(std::is_union<A>::value);
    SHOW(std::is_union<U>::value);
    SHOW(std::is_union<C>::value);
    SHOW(std::is_class<A>::value);
    SHOW(std::is_class<B>::value);
    SHOW(std::is_class<B&>::value);
    SHOW(std::is_class<const B>::value);
    SHOW(std::is_class<B*>::value);
    SHOW(std::is_class<Ec>::value);
    SHOW(std::is_class<C>::value);
    SHOW(std::is_function<D>::value);
    SHOW(std::is_function<int(int)>::value);
    SHOW(std::is_function<decltype(func)>::value);
    SHOW(std::is_fundamental<C>::value);
    SHOW(std::is_fundamental<int>::value);
    SHOW(std::is_fundamental<int&>::value);
    SHOW(std::is_fundamental<int*>::value);
    SHOW(std::is_fundamental<const int>::value);
    SHOW(std::is_arithmetic<C>::value);
    SHOW(std::is_arithmetic<int>::value);
    SHOW(std::is_arithmetic<const int>::value);
    SHOW(std::is_arithmetic<int&>::value);
    SHOW(std::is_arithmetic<int*>::value);
    SHOW(std::is_arithmetic<bool>::value);
    SHOW(std::is_arithmetic<float>::value);
    SHOW(std::is_arithmetic<char>::value);
    SHOW(std::is_arithmetic<long>::value);
    SHOW(std::is_arithmetic<E>::value);
}

void unordered_mapTest()
{
    std::unordered_map<std::string, std::string> u = {
        {"RED", "#FF0000"},
        {"GREEN", "#00FF00"},
        {"BLUE", "#0000FF"},
    };

    auto print = [](const auto& key, const auto& value)
    {
        std::cout << "Key:[" << key << "] Value:[" << value << "]\n";
    };
    std::cout << "Iterator unordered_map explicit with types: \n";
    for(const std::pair<std::string, std::string>& n : u)
    {
        print(n.first, n.second);
    }
    std::cout << "Iterator unordered_map using C++17 structured binding: \n";
    for(const auto& [key, value] : u)
    {
        print(key, value);
    }

    // add new entries 
    u["BLACK"] = "000000";
    u["WHITE"] = "FFFFFF";

    std::cout << "\nOutput values by key:\n"
        "RED is ["<< u["RED"] << "]\n"
        "BLACK is ["<< u["BLACK"] << "]\n";
    std::cout << "use operator[] with non-existent key to insert a new key-value pair: \n";
    print("new_key", u["new_key"]);


    std::cout << "Iterator unordered_map using auto: \n";
    for(const auto& n: u)
    {
        print(n.first, n.second);
    }
    std::cout << "Size is : " << u.size() << std::endl;
    std::cout << "Max Size is : " << u.max_size() << std::endl;
    for(const auto& [key, value] : u)
    {
        std::cout << "u.count[" << key << "] = " << u.count(key) << "\n";
    }

    u.emplace(std::make_pair(std::string("PURPLE"), std::string("800080")));

    u.emplace(std::make_pair("BROWN", "A52A2A"));

    // u.emplace(std::piecewise_construct, std::forward_as_tuple("GRAY"), std::forward_as_tuple(3, "80")); // error

    std::cout << "Iterator unordered_map after emplace: \n";
    for(const auto& n: u)
    {
        print(n.first, n.second);
    }
    std::cout << std::boolalpha << "u.empty(): " << u.empty() << "\n";

    u.insert({"SNOW", "FFFAFA"});
    u.insert(std::make_pair("CORAL", "FF7F50"));
    /*
     * 
     *     auto printResult = [](auto const &pair)
     *     {
     *         std::cout << (pair.second? "inserted: " : "assigned: ");
     *     };
     *     auto res = u.insert_or_assign("SNOW", "FFFAFA"); // C++17 
     *     printResult(res);
     *     print(res.first->first, res.first->second);
     */
    for(auto iter = u.begin(); iter != u.end();)
    {
        if(iter->first == "BLACK")
        {
            iter = u.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    std::cout << "Iterator unordered_map using iterator after erase 'BLACK': \n";
    for(auto iter = u.begin(); iter != u.end(); ++iter)
    {
        auto key = iter->first;
        auto value = iter->second;
        std::cout << "Key:[" << key << "] Value:[" << value << "]\n";
    }
    /*
     * 
     *     // extract is the only way to chang a key of a map element without realloction
     *     auto color = u.extract("CORAL");  // C++17
     *     color.key() = "SIENNA";
     *     color.value() = "A0522D";
     *     u.insert(std::move(color));
     * 
     */
    /*
     * 
     *     std::unordered_map<std::string, std::string> u2 = {
     *         {"WHEAT", "F5DEB3"},
     *         {"GOLD","FFD700"}
     *     };
     *     u.merge(u2); //C++17
     *     for(const auto& [key, value] : u)
     *     {
     *         print(key, value);
     *     }
     */

    std::cout << "find unordered_map with At: \n";
    std::cout << "At WHITE is " << u.at("WHITE") << std::endl;
    std::cout << "find unordered_map with Find: \n";
    auto search = u.find("GOLD");
    if(search != u.end())
    {
        std::cout << "FOUND: " << search->first << " color = " << search->second << "\n";
    }
    else
    {
        std::cout << "NOT FOUND\n";
    }
    /*
     * 
     *     auto ct = u.contains("GOLD"); // C++20
     *     if(ct != u.end())
     *     {
     *         std::cout << "CONTAINS: " << ct->first << " color = " << ct->second << "\n";
     *     }
     *     else
     *     {
     *         std::cout << "NOT CONTAINS\n";
     *     }
     * 
     */

    u.clear();
    std::cout << "Iterator unordered_map after clear: \n";
    for(const auto& [key, value] : u)
    {
        print(key, value);
    }
}

std::ostream& operator<<(std::ostream& os, const std::unordered_set<int>& s)
{
    for(os  << "[" << s.size() << "] { "; int i : s)
    {
        os << i << ' ' ;
    }
    return os << "}\n";
}

void unordered_setTest()
{
    std::unordered_set<int> nums = {1,2,3};
    std::cout << "Initially: " << nums;

    auto p = nums.insert(4);
    std::cout << "'4' was inserted : " << std::boolalpha << p.second << "\n";
    std::cout << "After Insert 4: " << nums;
    std::cout << "empty(): " << nums.empty() << std::endl;
    std::cout << "size(): " << nums.size() << std::endl;
    std::cout << "max_size(): " << nums.max_size() << std::endl;
    nums.emplace(5);
    std::cout << "After Emplace 5: " << nums;
}

void handleEptr(std::exception_ptr eptr)
{
    try
    {
        if(eptr)
        {
            std::rethrow_exception(eptr);
        }
    }
    catch(const std::exception& e)
    {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
}

void libFeaturesTest()
{
    std::vector<int> v {2,4,6,8,10};
    std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << "\n";
    if(std::all_of(v.cbegin(), v.cend(), [](int i){return i % 2 == 0;}))
    {
        std::cout << "All numbers are even\n";
    }

    if(std::none_of(v.cbegin(), v.cend(), [](int i){return i % 2 != 0;}))
    {
        std::cout << "None numbers are odd\n";
    }

    struct DivisibleBy
    {
        const int d;
        DivisibleBy(int n) : d(n) {}
        bool operator()(int n) const {return n % d == 0; }
    };
    if(std::any_of(v.cbegin(), v.cend(), DivisibleBy(3)))
    {
        std::cout << "At least one number is diviable by 3\n";
    }

    auto is_tri = [](int i) {return i % 3 == 0;};
    auto res = std::find_if(std::begin(v), std::end(v), is_tri);
    if(res != std::end(v))
    {
        std::cout << "find tri: " << * res << "\n";
    }
    else
    {
        std::cout << "can not find tri\n";
    }

    std::cout << "copy_if divisible by 3: \n";
    std::copy_if(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "),
            [](int x) { return x % 3 == 0;});
    std::cout << "\n";

    std::vector<int> v2 = {1,2,3,4,5,6,7,8,9,10};
    std::cout << "is_sorted: " << std::boolalpha << std::is_sorted(std::begin(v2), std::end(v2)) << "\n";
    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(v2.begin(), v2.end(), g);
    std::copy(v2.begin(), v2.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << "\n";
    std::cout << "is_sorted: " << std::boolalpha << std::is_sorted(std::begin(v2), std::end(v2)) << "\n";
    const int min_sorted_size = 4;
    for(int i = 0; i < min_sorted_size;)
    {
        std::shuffle(v2.begin(), v2.end(), g);
        auto sorted_end = std::is_sorted_until(v2.begin(), v2.end());
        i = std::distance(v2.begin(), sorted_end);
        assert(i >= 1);

        for(auto n : v2)
        {
            std::cout << n << ' ';
        }
        std::cout << " : " << i << " initial sorted elements\n"
            << std::string(i * 2 - 1, '^') << '\n';
    }

    std::array<int, 5> true_arr;
    std::array<int, 5> false_arr;
    std::partition_copy(std::begin(v2), std::end(v2), std::begin(true_arr), std::begin(false_arr), [] (int i) {return i > 5;});

    std::cout << "true_array: ";
    std::copy(true_arr.begin(), true_arr.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << "\n";
    std::cout << "false_array: ";
    std::copy(false_arr.begin(), false_arr.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << "\n";

    std::vector<int> v3 {3,1,4,1,5,9,2,6,5,3,5,8,9,7,9};
    std::cout << "initially, v3: \n";
    for(auto i : v3) std::cout << i << ' ';
    std::cout << std::endl;

    if(!std::is_heap(v3.begin(), v3.end()))
    {
        std::cout << "making heap...\n";
        std::make_heap(v3.begin(), v3.end());
    }

    auto pow_of_two = [](int i){ 
        if((i > 0) && (i & (i - 1)) == 0)
            return true;
        else
            return false;
    };

    std::cout << "after make_heap, v3: \n";
    for(auto t{1U}; auto i : v3)
    {
        std::cout << i << (pow_of_two(++t) ? " | " : " ");
        // std::cout << i << (std::has_single_bit(++t) ? " | " : " "); // C++20
    }
    std::cout << '\n';

    // minmax: returns references to the smaller and greater of a and b or initializer_list
    std::srand(std::time(0));
    std::pair<int, int> bounds = std::minmax(std::rand() % v3.size(),
            std::rand() % v3.size());
    std::cout << "v3[" << bounds.first << "," << bounds.second << "]: ";
    for(int i = bounds.first; i < bounds.second; ++i)
    {
        std::cout << v3[i] << ' ';
    }
    std::cout << std::endl;

    // is_permutation
    // testing or check the correctness of rearranging algorithm(sorting, shuffling, partitioning.etc)
    //  if x is an original range and y is a permuted range then std::is_permutation(x,y) == true means that y consist of "the same" elements, maybe staying at other position
    static constexpr auto a1 = {1,2,3,4,5};
    static constexpr auto a2 = {2,1,5,3,4};
    static constexpr auto a3 = {2,1,5,3,1};
    auto printV = [](auto const &v)
    {
        std::cout << "{ ";
        for(const auto& i : v)
        {
            std::cout << i << ' ';
        }
        std::cout << "} ";
    };

    printV(a2);
    std::cout << "is a permutation of ";
    printV(a1);
    std::cout << " : " << std::boolalpha << std::is_permutation(a1.begin(), a1.end(), a2.begin()) << '\n';

    printV(a3);
    std::cout << "is a permutation of ";
    printV(a1);
    std::cout << " : " << std::boolalpha << std::is_permutation(a1.begin(), a1.end(), a3.begin()) << '\n';

    // iota()
    // fills the range[first, last) with sequentially increasing values, starting with value and repetitively evaluating ++value.

    std::list<int> lt(10);
    std::iota(lt.begin(), lt.end(), 11);
    std::cout << "iota generated: ";
    printV(lt);
    std::cout << std::endl;

    // uninitialized_copy_n
    // copies count elements from a range beginning at first to an uninitialized memory area beginning at d_first
    std::vector<std::string> vv = {"This", "is", "an", "example"};
    std::string* p;
    std::size_t sz;
    std::tie(p, sz) = std::get_temporary_buffer<std::string>(vv.size());
    std::cout << "get_temporary_buffer return size: "<< sz << ", vv.size(): " << vv.size() << std::endl;
    sz = std::min(sz, vv.size());
    std::uninitialized_copy_n(vv.begin(), sz, p);
    std::cout << "uninitialized_copy_n result: ";
    for(std::string* i = p;  i != p + sz; ++i)
    {
        std::cout << *i << ' ';
        i->~basic_string<char>(); 
    }
    // same as std::destroy(p, p + sz); 

    std::cout << std::endl;
    std::return_temporary_buffer(p);

    // std::exception_ptr 
    // nullable pointer-like type that manages an exception object which has been thrown and captured with std::current_exception
    std::exception_ptr eptr;
    try
    {
        std::string().at(1); // generates an std::out_range exception
    }
    catch(...)
    {
        eptr = std::current_exception();// capture
    }
    handleEptr(eptr);

    std::vector<int> vec{3,1,4,5};
    std::cout << "orign vec: ";
    for(const auto & i: vec) std::cout << i << ' ';
    std::cout << std::endl;
    auto it = std::begin(vec);
    auto nx = std::next(it, 2);
    std::cout << "vec begin: " << *it << std::endl;
    std::cout << "vec next begin 2: " << *nx << std::endl;
    it = std::end(vec);
    auto pv = std::prev(it, 2);
    std::cout << "vec end: " << *it << std::endl;
    std::cout << "vec pre end 2: " << *pv << std::endl;
}

struct Foo
{
    Foo(int num) : _num(num){}
    void printAdd(int i) const {std::cout << _num + i << std::endl;}
    int _num{0};
};

void printNum(int i)
{
    std::cout << i << std::endl;
}

struct PrintNum
{
    void operator()(int i) const
    {
        std::cout << i << std::endl;
    }
};

void call(std::function<int()> f)
{
    std::cout << f() << std::endl;
}

int normalFunc()
{
    return 33;
}

int f(int, int) { return 1;}
int g(int, int) { return 2;}

void testFunc(std::function<int(int,int)> const& arg)
{
    std::cout << "test function: ";
    if(arg.target<std::plus<int>>())
    {
        std::cout << "it is plus function" << std::endl;
    }
    if(arg.target<std::minus<int>>())
    {
        std::cout << "it is minus function" << std::endl;
    }

    int (*const* ptr)(int, int) = arg.target<int(*)(int, int)>();
    if(ptr && *ptr == f)
    {
        std::cout << "it is f function" << std::endl;
    }
    if(ptr && *ptr == g)
    {
        std::cout << "it is g function" << std::endl;
    }
}

void functionTest()
{
    // store a free function
    std::function<void(int)> f1 = printNum;
    f1(3);

    // store a lambda
    std::function<void()> f2 = [](){printNum(33);};
    f2();

    // store the result of a call to std::bind
    std::function<void()> f3 = std::bind(printNum, 333);
    f3();

    // store a call to a memeber function
    std::function<void(const Foo&, int)> f4 = &Foo::printAdd;
    const Foo foo(3333);
    f4(foo, 1);
    f4(3333, 1); // implicit convert int to Foo

    //store a call to a data member accessor
    std::function<int(const Foo&)> f5 = &Foo::_num;
    std::cout << "_num: " << f5(foo) << std::endl;

    // store a call to a function object
    std::function<void(int)> f6 = PrintNum();
    f6(33333);

    auto factorial = [](int n)
    {
        // store a lambda object to emulate "recursive lambda" 
        std::function<int(int)> fac = [&](int n) { return (n < 2) ? 1 : n * fac(n-1);};
        
        // "auto fac = [&](int n) {...};" does not work in recursive calls
        return fac(n);
    };

    for(int i{5}; i != 8; ++i)
    {
        std::cout << i << "! = " << factorial(i) << "; ";
    }
    std::cout << std::endl;

    int n = 1;
    std::function<int()> ff = [&n]() {return n;};
    call(ff);

    n = 2;
    call(ff);

    ff = normalFunc;
    call(ff);

    testFunc(std::function<int(int,int)>(std::plus<int>()));
    testFunc(std::function<int(int,int)>(std::minus<int>()));
    testFunc(std::function<int(int,int)>(f));
    testFunc(std::function<int(int,int)>(g));
}

int main(int argc, char** argv)
{
    // arrayTest();
    // atomicTest();
    // mutexTest();
    // conditionVariableTest();
    // threadTest();
    // futureTest();
    // cfenvTest();
    // chronoTest();
    // cInttypesTest();
    // forward_listTest();
    // initializer_listTest();
    // randomTest();
    // ratioTest();
    // regexTest();
    // tupleTest();
    // typeTraitsTest();
    // unordered_mapTest();
    // unordered_setTest();
    // libFeaturesTest();
    functionTest();

    return 0;
}
