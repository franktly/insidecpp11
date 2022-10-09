#include<iostream>
#include<iomanip>
#include<memory>
#include<mutex>
#include<shared_mutex>
// #include<syncstream> C++20
#include<thread>
#include<chrono>
#include<string>
#include<vector>
#include<iterator>
#include<sstream>
#include<complex>

struct Vec3
{
    int x, y, z;
    Vec3(int x = 0, int y = 0, int z = 0) noexcept : x(x), y(y), z(z) {}

    friend std::ostream& operator<<(std::ostream& os, const Vec3& v)
    {
        return os << "{ x = " << v.x << ", y = " << v.y << ", z = " << v.z << " }";
    }
};


// make_unique
void makeUniqueTest()
{
    std::unique_ptr<Vec3> v1 = std::make_unique<Vec3>();
    std::unique_ptr<Vec3> v2 = std::make_unique<Vec3>(1,2,3);
    std::unique_ptr<Vec3[]> v3 = std::make_unique<Vec3[]>(5);

    std::cout << "make_unique<Vec3>():      " << *v1 << '\n'
        << "make_unique<Vec3>(0,1,2): " << *v2 << '\n'
        << "make_unique<Vec[]>(5):    ";

    for(int i = 0 ; i < 5; i++)
    {
        std::cout << std::setw(i ? 32 : 0) << v3[i] << '\n';
    }
}

// shared_timed_mutex
// synchronization primitive that can be used to protect shared data from being simultaneously accessed by multiple thread
class R
{
    public:
        mutable std::shared_timed_mutex _mut;
        /*
         * data
         */

    public:
        // handle multiple readers but only one writer
        R& operator=(const R& other)
        {
            // exclusive ownership to write to *this
            std::unique_lock<std::shared_timed_mutex> lhs(_mut, std::defer_lock);

            // shared ownership to read from other
            std::shared_lock<std::shared_timed_mutex> rhs(other._mut, std::defer_lock);

            std::lock(lhs, rhs);
            /*
             * assign data
             */
            return *this;
        }
};

std::shared_timed_mutex stm;
int n = 10;
void readShared(int id)
{
    // both the threads get access to the integer i
    std::shared_lock<std::shared_timed_mutex> sl(stm);
    const int i = n; // reads global n

    // std::osyncstream(std::cout) << "#" << id << " read n as " << i << "\n"; //C++20
    std::cout << "#" << id << " read n as " << i << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // std::osyncstream(std::cout) << "#" << id << " woke up" << "\n"; //C++20
    std::cout << "#" << id << " woke up" << "\n";
}

//shared_lock
//general-purpose shared mutex ownership wrapper allowing deferred locking,
//timed locking and transfer of lock ownership.
//locks the associated shared mutex in shared mode
//movable, but NOT copyable

std::string file = "Orignal content"; // simulates a file
std::mutex outputMutex;            // mutex that protects output operations
std::shared_timed_mutex fileMutex; // reader/writer mutex

void readImpl(int id)
{
    std::string content;
    {
        std::shared_lock<std::shared_timed_mutex> fileLock(fileMutex, std::defer_lock); // Do not lock it first
        fileLock.lock();
        content = file;
    }
    std::lock_guard<std::mutex> lock(outputMutex);
    std::cout << "Read by reader #" << id << ": " << content << std::endl;
}

void writeImpl(int id)
{
    {
        std::lock_guard<std::shared_timed_mutex> fileLock(fileMutex);
        file = "New content";
    }
    std::lock_guard<std::mutex> lock(outputMutex);
    std::cout << "Write by writer #" << id << std::endl;
}

void sharedMutexTest()
{
    R r;
    std::thread t1(readShared, 1);
    std::thread t2(readShared, 2);
    t1.join();
    t2.join();

    std::cout << "Three readers and Three writers competes each other..." << std::endl;
    std::thread reader1 {readImpl, 1};
    std::thread reader2 {readImpl, 2};
    std::thread reader3 {readImpl, 3};
    std::thread writer1 {writeImpl, 6};
    std::thread writer2 {writeImpl, 7};
    std::thread writer3 {writeImpl, 8};
    reader1.join();
    writer1.join();
    writer2.join();
    reader3.join();
    reader2.join();
    writer3.join();
}

// integer_sequence
//
// compile-time sequence of integers
//

template<typename T, T...ints>
void printSeqs(std::integer_sequence<T, ints...> s)
{
    std::cout << "sequence size is " << s.size() << ": ";
    ((std::cout << ints << ' '), ...);
    std::cout << std::endl;
}

// convert array to a tuple
template<typename Array, std::size_t... I>
auto a2tImpl(const Array& a, std::index_sequence<I...>)
{
    return std::make_tuple(a[I]...);
}

template<typename T, std::size_t N, typename Indices = std::make_index_sequence<N>>
auto a2t(const std::array<T,N>& a)
{
    return a2tImpl(a,Indices{});
}

template<std::size_t I = 0, typename... Tp>
typename std::enable_if<(I == sizeof...(Tp)), void>::type
printTuple(std::tuple<Tp...>& t)
{}

template<std::size_t I = 0, typename... Tp>
typename std::enable_if<(I < sizeof...(Tp)), void>::type
printTuple(std::tuple<Tp...>& t)
{
    std::cout << std::get<I>(t) << ' ';
    printTuple<I + 1, Tp...>(t);
}

void integerSequenceTest()
{
    printSeqs(std::integer_sequence<unsigned, 9, 2, 5, 1, 9, 1, 6>{});
    printSeqs(std::make_integer_sequence<int, 20>{});
    printSeqs(std::make_integer_sequence<char, 10>{});
    printSeqs(std::make_index_sequence<10>{});
    printSeqs(std::index_sequence_for<float, std::iostream, char, int> {});

    std::array<int, 4> array = {1,2,3,4};
    auto t = a2t(array);
    static_assert(std::is_same<decltype(t), std::tuple<int, int, int, int>>::value, "");
    std::cout << "tuple is ";
    // std::apply([](auto&&... args){((std::cout << args << ' '), ...);}, t); // C++17
    printTuple(t);
    std::cout << std::endl;
}

// std::exchange
// replaces the value of obj with new_value and returns the old value of obj

class Replace
{
    public:
        using flagType = int;
    public:
        flagType flag() const { return _flag;}
        flagType flag(flagType newF)
        {
            return std::exchange(_flag, newF);
        }
    private:
        flagType _flag = 0;
};

void func1() {std::cout << "exec func1" << std::endl;}

void exchangeTest()
{
    // std::exchange can be used when implementing move constructors and move assignment operators
    struct S
    {
        int n {-1};
        S(int i) : n(i) {}
        S(S&& other) noexcept : n{std::exchange(other.n, 0)}
        {}
        S(S& other) noexcept : n{std::exchange(other.n, 0)}
        {}

        S& operator=(S&& other) noexcept
        {
            if(this != &other)
            {
                n = std::exchange(other.n, 0); // move other.n to n while leaving 0 in other.n
            }
            return *this;
        }

        S& operator=(S& other) noexcept
        {
            if(this != &other)
            {
                n = std::exchange(other.n, 0); // move other.n to n while leaving 0 in other.n
            }
            return *this;
        }

        void print(std::string name)
        {
            std::cout << "[" << name <<"] n = " << n << std::endl;
        }

    };

    S s1(3);
    s1.print("s1");
    S s2 = s1;
    s2.print("s2");
    s1.print("s1");
    S s3(s2);
    s3.print("s3");
    s2.print("s2");

    // exchange class data member
    Replace r;
    std::cout << r.flag() << std::endl;
    std::cout << r.flag(12) << std::endl;
    std::cout << r.flag() << std::endl;

    // exchange container
    std::vector<int> v;
    std::exchange(v, {1,2,3,4,5,6});
    std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, ", "));
    std::cout << std::endl;

    // exchange function
    void (*func)();
    std::exchange(func, func1);
    func();

    std::cout << "fibonacci sequence: ";
    for(int a{0}, b{1}; a < 333; a = std::exchange(b, a + b))
    {
        std::cout << a << ", ";
    }
    std::cout << "..." << std::endl;
}

// std::quoted
// allows insertion and extraction of quoted strings, such as the ones found in CSV or XML
// default delim = ", escap = \

void defaultDelimiter()
{
    const std::string in = "std::quoted() quotes this string and embedded \"quotes\" too";
    std::stringstream ss;
    ss << std::quoted(in);
    std::string out;
    ss >> std::quoted(out);

    std::cout << "Default delimiter: \n"
              << "Read in    [" << in << "]\n"
              << "Stored as  [" << ss.str() << "]\n"
              << "Written out[" << out << "]\n";
}

void customDelimiter(bool useDefault = false)
{
    const char delim {'$'};
    const char escape {'%'};
    const std::string in = "std::quoted() quotes this string and embedded $quotes$ $too";
    std::stringstream ss;
    if(useDefault)
        ss << std::quoted(in);
    else
        ss << std::quoted(in, delim, escape);
    std::string out;
    if(useDefault)
        ss >> std::quoted(out);
    else
        ss >> std::quoted(out, delim, escape);

    std::cout << "Custom delimiter: \n"
              << "Read in    [" << in << "]\n"
              << "Stored as  [" << ss.str() << "]\n"
              << "Written out[" << out << "]\n";
}

void quotedTest()
{
    defaultDelimiter();
    customDelimiter();
    customDelimiter(true);
}

// complex
void complexLiteralTest()
{
    using namespace std::complex_literals;
    std::complex<double> c = 1.0 + 1i;
    std::cout << "c.real = " << c.real() << ", c.imag = " << c.imag() << std::endl;
    std::cout << "abs(" << c << ") = " << std::abs(c) << std::endl;

    std::complex<double> z = 1i * 1i;
    std::cout << "z.real = " << z.real() << ", z.imag = " << z.imag() << std::endl;
    std::cout << "abs(" << z << ") = " << std::abs(z) << std::endl;
}

int main(int argc, char** argv)
{
    // makeUniqueTest();
    // sharedMutexTest();
    // integerSequenceTest();
    // exchangeTest();
    // quotedTest();
    complexLiteralTest();

    return 0;
}
