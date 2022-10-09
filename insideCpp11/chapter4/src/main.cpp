#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <typeinfo>

// double right angle bracket problem
template<int i> class X{};
template<class T> class Y{};

void doubleRightAngleBracketTest()
{
    Y<X<1> > x1;  // C++98 C++11 both ok(add space bt two right angle bracket
    Y<X<2>> x2;   // C++98 error C++11 ok
    const std::vector<int> vec = static_cast<std::vector<int>>(1); // C++98 error C++11 ok
}


// auto keyword
// static type lang vs dynamic type lang:  variable type check time( compile time vs run time)
// auto is a type-specifier keyword and deduce during compile time

template<typename T1, typename T2>
double sum(T1 &t1, T2 &t2)
{
    auto s = t1 + t2; // s type can be deduced when template instantiation
    return s;
}

double foo() {}
int& bar(){}
float* bar2(){}

void autoKeywordTest()
{
    // basic type deduce
    int a = 3;
    long b = 3;
    float c = 3.0;
    double d = 6.0;

    auto e = sum<int, long>(a, b);
    auto f = sum<float, double>(c, d);
    std::cout << "e type is : " << typeid(e).name() << " value is :" << e << std::endl;
    std::cout << "f type is : " << typeid(f).name() << " value is :" << f << std::endl;

    // point and reference deduce
    int x;
    int *y = &x;

    auto g = &x;  // int*
    auto &h = x; // int&
    auto i = y;  // int*
    auto *j = y; // j same with i int*
    auto k = foo(); // double
    // auto l = &foo(); // error: can not take address of a rvalue;
    // auto & m = foo(); // error: none-const lvalue reference can not bind to a rvlaue;
    const auto & n = foo(); // ok: const lvalue reference can bind to a rvlaue;
    auto o = bar(); // int
    auto & p = bar(); // int&

    // cv-qualifier(volatile and const)
    const auto q = foo(); // const double
    const auto &r = foo(); // const double&
    volatile auto *s = bar2(); // volatile float*

    // reference keeps cv others not
    auto t = q;     // double
    auto & u = q;  // const double &
    auto v = s;     // float*
    volatile auto &w = s; // volatile float* &

    // pointer and initializer_list auto deduce
    auto aa = 1; // int
    auto ab(1); // int

    auto ac{1};  // int
    auto ad = new auto(1); // int
    delete ad; ad = nullptr;

    // auto keyword can not deduce, 4 situations mainly:
    // 1. function formal paramter 
    // void func(auto x = 1){} // error
    // 2. struct or class none-static member data
    // struct myStr{ auto var = 10;} // error
    // 3. auto array
    char xx[3];
    auto yy = xx; // ok char[3]
    // auto zz[3] = xx; // error
    // 4. template parameter
    // std::vector<auto> v= {1};  // error
}

// decltype: 
// usage: decltype(expression)
// RTTI typeid() -> type_info
// disable RTTI: gcc: -fno-rttion; vc: /GR-;
//
class Black{};
class White{};

template<typename T1, typename T2>
void sum2(T1& t1, T2& t2, decltype(t1 + t2) &res)
{
    res = t1 + t2;
}

int hash(char*);
void basicDecltypeTest()
{
    White w;
    Black b;
    std::cout << typeid(w).name() << std::endl;
    std::cout << typeid(b).name() << std::endl;

    White w2;

    // compare type 
    bool w_b = (typeid(w).hash_code() == typeid(b).hash_code());
    bool w_2 = (typeid(w).hash_code() == typeid(w2).hash_code());

    std::cout << "w and b? " << (w_b? "true" : "false") << std::endl;
    std::cout << "w and w2? " << (w_2? "true" : "false") << std::endl;

    int i;
    decltype(i) j= 0;

    // display type name
    std::cout << "j type: " << typeid(j).name() << std::endl;

    float c; 
    double d;
    decltype(c+d) e;
    std::cout << "e type: " <<  typeid(e).name() << std::endl;


    // auto deduce type
    using t1 = decltype(sizeof(0));
    using t2 = decltype(sizeof((int*)0));
    using t3 = decltype(nullptr);
    std::cout << "t1 type: " <<  typeid(t1).name() << std::endl;
    std::cout << "t2 type: " <<  typeid(t2).name() << std::endl;
    std::cout << "t3 type: " <<  typeid(t3).name() << std::endl;


    // simplify declare
    std::vector<int> vecInt;
    // typedef decltype(vecInt.begin()) vecIntIter;  
    typedef decltype(vecInt)::iterator vecIntIter;  
    for(vecIntIter i = vecInt.begin(); i < vecInt.end(); ++i){}
    for(decltype(vecInt)::iterator i = vecInt.begin(); i < vecInt.end(); ++i) {}

    // anonymous type deduce
    enum {K1, K2, K3} anony_e; // anony enum
    union
    {
        decltype(anony_e) key;
        char* name;
    }anony_u; // anony union

    struct
    {
        int d;
        decltype(anony_u) id;
    }anony_s[100]; // anony struct array

    decltype(anony_s) as;
    as[0].id.key = decltype(anony_e)::K1;
    std::cout << "key: " << as[0].id.key << std::endl;

    // function template parameter deduce
    int aa = 3, bb= 2, dd;
    sum2(aa, bb,  dd);  // dd type should be specified previously
    std::cout << " 3 + 2 =" << dd << std::endl;

    // decltype accept expression 
    // std::map<char*, decltype(hash)> dic_key; // decltyp NOT accept function name as argument
    std::map<char*, decltype(hash(nullptr))> dic_key2;
    std::cout << typeid(decltype(dic_key2)::mapped_type).name() << std::endl;

    // result_of : deduce function return type
    typedef double (*func)();
    std::result_of<func()>::type f; // f is deduced type double and NOT really invoke func()
    std::cout << " typedef double (*func)() return type is  " << typeid(f).name() << std::endl;
}
//
// decltype deduce 4 rules: 
// decltype(e)
// 1. if e is a id-expression(token or identifier) without brackets or member-access-expression, decltype(e) is the type of entity named e , if e is a overload function then will cause compile error
// 2. otherwise, if type of e is T and e is rvalue(eXpiring Value), decltype(e) is T&&
// 3. otherwise, if type of e is T and e is lvalue, decltype(e) is T&
// 4. otherwise, if type of e is T , decltype(e) is T

void overloaded(int){};
void overloaded(char){}; // overload function
int && rValueRef(){};
const bool constFunc(int){};
void decltypeFourRulesTest()
{
    int i = 4;
    int arr[4] = {0};
    int *ptr = arr;
    struct S{ double d;} s;

    // rule 1 : basic data type, pointer, array, struct, class, 
    decltype(i) var1;
    decltype(arr) var2;
    decltype(ptr) var3;
    decltype(s.d) var4;
    // decltype(overloaded) var5;

    // rule2: right-value reference
    decltype(rValueRef()) var6 = 1;
    std::cout << "rValueRef() rv? " << std::is_rvalue_reference<decltype(rValueRef())>::value << std::endl;

    // rules3: left-value 
    decltype(true ? i : i) var7 = i;
    std::cout << "true ? i : i lv? " << std::is_lvalue_reference<decltype(true ? i : i)>::value << std::endl;
    decltype((i)) var8 = i;
    std::cout << "((i)) lv? " << std::is_lvalue_reference<decltype((i))>::value << std::endl;
    decltype(++i) var9 = i;     // ++i is left-value
    std::cout << "++i lv? " << std::is_lvalue_reference<decltype(++i)>::value << std::endl;
    decltype(arr[3]) var10 = i;
    std::cout << "arr[3] lv? " << std::is_lvalue_reference<decltype(arr[3])>::value << std::endl;
    decltype(*ptr) var11 = i;
    std::cout << "*ptr lv? " << std::is_lvalue_reference<decltype(*ptr)>::value << std::endl;
    decltype("lval") var12 = "lval"; // string literal const is left-value
    std::cout << "'lvar' lv? " << std::is_lvalue_reference<decltype("lval")>::value << std::endl;

    // rules4
    decltype(1) var13;          // none-string literal const is right-value(PureValue)
    std::cout << "1 lv? " << std::is_lvalue_reference<decltype(1)>::value << std::endl;
    std::cout << "1 rv? " << std::is_rvalue_reference<decltype(1)>::value << std::endl;
    std::cout << "1 ref? " << std::is_reference<decltype(1)>::value << std::endl;
    decltype(i++) var14;        // i++ is right-value
    std::cout << "i++ lv? " << std::is_lvalue_reference<decltype(i++)>::value << std::endl;
    std::cout << "i++ rv? " << std::is_rvalue_reference<decltype(i++)>::value << std::endl;
    std::cout << "i++ ref? " << std::is_reference<decltype(i++)>::value << std::endl;
    decltype(constFunc(1)) var15; 
    std::cout << "constFunc(1) lv? " << std::is_lvalue_reference<decltype(constFunc(1))>::value << std::endl;
    std::cout << "constFunc(1) rv? " << std::is_rvalue_reference<decltype(constFunc(1))>::value << std::endl;
    std::cout << "constFunc(1) ref? " << std::is_reference<decltype(constFunc(1))>::value << std::endl;
    decltype(true ? i : i + 1) var16; 
    std::cout << "true ? i : i + 1 lv? " << std::is_lvalue_reference<decltype(true ? i : i + 1)>::value << std::endl;
    std::cout << "true ? i : i + 1 rv? " << std::is_rvalue_reference<decltype(true ? i : i + 1)>::value << std::endl;
    std::cout << "true ? i : i + 1 ref? " << std::is_reference<decltype(true ? i : i + 1)>::value << std::endl;
}

// const volatile qualifier 
// decltype object takes cv-qualifier NOT object member
// cv-qualifier(const and volatile) and `&` redundancy will be ignored but `*` will not be ignored
void cvAndRedundancyDeclTypeTest()
{
    int i = 1;
    int &j = i;
    int *p = &i;
    const int k = 1;

    decltype(i) & var1 = i;
    decltype(j) & var2= i;  // redundant & , ignored
    std::cout << "var1 lv? " << std::is_lvalue_reference<decltype(var1)>::value << std::endl;
    std::cout << "var2 lv? " << std::is_lvalue_reference<decltype(var2)>::value << std::endl;

    // decltype(p)* var3 = &i;  // error
    decltype(p)* var3 = &p;  // int**

    auto* var4 = p;  // int*
    auto var5 = p;  // int*

    const decltype(k) var6 = 1; // redundant const, ignored
}

// track return type
//intuition verson:
//
/*
 * template<typename T1, typename T2>
 * decltype(t1 + t2) sum3(T1 &t1, T2 &t2)  // error: compiler read tokens from left to right and t1/t2 is used before declare
 * {
 *     return t1 + t2;
 * }
 * 
 * 
 */
// track return type version:
//
template<typename T1, typename T2>
auto sum3(T1 &t1, T2 &t2) -> decltype(t1 + t2) // ok: track return type declare
{
    return t1 + t2;
}

// auto (*)() -> int (*)() // a function[named a] return a function pointer
// auto pf1() -> auto (*)() -> int (*)() // a function return function named a
auto pf1() -> auto (*)() -> int (*)()
{
    return nullptr;
}

int (*(*pf2())())()
{
    return nullptr;
}

// track back type using in forward 
//
double foo(int a){ return (double)a + 0.1; }
int foo(double a){ return (int)a; }

template<class T>
auto myForward(T t) -> decltype(foo(t))
{
    return foo(t);
}

void trackReturnTypeTest()
{
    auto a = 3;
    auto b = 3L;
    auto c = sum3(a, b);
    std::cout << "a + b = " <<  c << std::endl;
    std::cout << " 'auto pf1() -> auto (*)() -> int (*)()'  and  'int (*(*pf2())())()' the same? " << std::is_same<decltype(pf1), decltype(pf2)>::value << std::endl;

    std::cout << "myForwad(3) " << myForward(3) << std::endl;
    std::cout << "myForwad(2.1) " << myForward(2.1) << std::endl;

    // function pointer 
    auto (*fp1)() -> int; // the same as below
    int (*fp2)();

    // function reference
    // auto (&fp3)() -> int = fp1; the same as below
    // int (&fp4)();
}

void decltypeTest()
{
    // basicDecltypeTest();
    // decltypeFourRulesTest();
    trackReturnTypeTest();
}

// for loop
// iterator range defined
// implement ++ and == operator
int action1(int &e) {e *=2;}
int action2(int &e) {std::cout << e << '\t';}
void forKeywordTest()
{
    int arr[5] = {1,2,3,4,5};

    // version 1: tradition traverse
    std::cout << "verson 1 traverse : " << std::endl;
    for(auto p = arr; p < arr + sizeof(arr)/sizeof(arr[0]); ++p)
    {
        *p *= 2;
    }
    for(auto p = arr; p < arr + sizeof(arr)/sizeof(arr[0]); ++p)
    {
        std::cout << *p << '\t';
    }
    std::cout << std::endl;


    // verson 2: for_each iterator traverse
    std::cout << "verson 2 traverse : " << std::endl;
    std::for_each(arr, arr + sizeof(arr)/sizeof(arr[0]), action1);
    std::for_each(arr, arr + sizeof(arr)/sizeof(arr[0]), action2);
    std::cout << std::endl;

    // verson 3: for keyword
    std::cout << "verson 3 traverse : " << std::endl;
    for(auto & e: arr)
    {
        e *=2;
    }
    for(int & e: arr)
    {
        std::cout << e << '\t';  // e NOT *e;
    }
    std::cout << std::endl;

}

int main(int argc, char** argv)
{
    // doubleRightAngleBracketTest();
    // autoKeywordTest();
    // decltypeTest();
    forKeywordTest();

    return 0;
}
