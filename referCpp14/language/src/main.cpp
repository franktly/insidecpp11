#include <iostream>
#include <iomanip>

template<class T>
constexpr T pi = T(3.1415926535897932385L); // variable template

template<class T>
T circleArea(T r) // function template
{
    return pi<T> * r * r; // pi<T> is a variable template instantiation
}

struct limits
{
    template<typename T>
    static const T min;  //  declaration of a static data member template
};

template<typename T>
const T limits::min = {}; // declaration of a static data member template


// variable template 
//

void variableTemplateTest()
{
    float r1 = 3.0;
    auto  c1 = circleArea(r1);
    std::cout << std::boolalpha <<  std::is_same<float, decltype(c1)>::value << std::endl;
    std::cout << "circleArea(float) = " << std::setprecision(24)<< c1 << '\n';

    double r2 = 3.0;
    auto  c2 = circleArea(r2);
    std::cout << std::boolalpha <<  std::is_same<double, decltype(c2)>::value << std::endl;
    std::cout << "circleArea(double) = " << std::setprecision(24) << c2 << '\n';
}

// generic lambdas
//

void genericLambdaTest()
{
    // generic lambda, operator() is a template with two parameters
    auto glambda = [](auto a, auto&& b) { return a > b;};
    bool b = glambda(3, 3.14);

    // generic lambda, operator() is a template with one parameters
    auto vglambda = [](auto printer)  // printer is a lambda 
    {
        return [=](auto&&... ts) // generic lambda, ts is a parameter pack
        {
            printer(std::forward<decltype(ts)>(ts)...);
            // nullary lambda (takes no parameters):
            return [=]{printer(ts...);};
        };
    };
    auto p = vglambda([](auto v1, auto v2, auto v3){ std::cout << v1 << " " << v2 <<" "<< v3;}); // lambda  nest 
    auto q = p(1, 'a', 3.14);
    q();

    int x = 4;
    auto y = [&r = x, x = x + 1]() -> int
    {
        r += 2; //  r : refer, x: copy
        std::cout << "\nx = " << x << "; r = " << r << std::endl;
        return x * x;
    }();
    std::cout << "x = " << x << "; y = " << y << std::endl;

    int z = 2;
    auto initlambda = [z = 3]() {std::cout << "z init captured: " << z << std::endl;};
    auto initlambda2 = [v = z + 3]() {std::cout << "v init captured: " << v << std::endl;};
    initlambda();
    initlambda2();
}

constexpr int factorial(int n)
{
    // return n <= 1 ? 1 : (n * factorial(n - 1)); // C++11
    //C++14
    if(n <= 1)
    {
        return 1;
    }
    else
    {
        return n * factorial(n - 1);
    }
}

void relaxedRestrictionOnConstexprFunctionTest()
{
     std::cout << "13! = " << factorial(13) << std::endl;
}

void binaryLiteralsTest()
{
    std::cout << 123 << '\n'   // dec
              << 0123 << '\n'  // oct
              << 0x123 << '\n' // hex
              << 0b100 << '\n' // binary C++14
              << 12345678901ull << '\n'
              << 12'345'678'901ull << '\n' // C++14
              << 123'4567'8901ull << '\n' // C++14
              << 12345678901u << '\n';
}

int xx = 1;
auto f1() { return xx;}
const auto& f2() { return xx;}
decltype(auto) f3() { return xx;};
decltype(auto) f4() { return (xx);};

void returnTypeDeductionForFunctionTest()
{
    std::cout << std::boolalpha << std::is_same<int, decltype(f1())>::value << std::endl;
    std::cout << std::boolalpha << std::is_same<const int&, decltype(f2())>::value << std::endl;
    std::cout << std::boolalpha << std::is_same<int, decltype(f3())>::value << std::endl;
    std::cout << std::boolalpha << std::is_same<int&, decltype(f4())>::value << std::endl;
}

void aggregateClassesWithNonStaticMemInitTest()
{
    struct S
    {
        int x;
        int y = 33;
    };
    // C++14
    S s = {3};
    std::cout << "aggregate init: s.x = " << s.x << ";s.y = " << s.y << std::endl;
}

int main(int argc, char** argv)
{
    // variableTemplateTest();
    // genericLambdaTest();
    // relaxedRestrictionOnConstexprFunctionTest();
    // binaryLiteralsTest();
    // returnTypeDeductionForFunctionTest();
    aggregateClassesWithNonStaticMemInitTest();
    return 0;
}
