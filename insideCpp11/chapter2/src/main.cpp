//#define NDEBUG
#include <iostream>
#include <climits>
#include <cassert>
#include <stdio.h>
#include <cstring>
#include "externTemplate1.h"
#include "externTemplate2.h"

// _Pragma Operator
/*
 * 
 * #pragma once
 * // the same as 
 * #ifndef AAA_HEADER
 * #define AAA_HEADER
 * // the same as 
 * _Pragma("once")
 * 
 */

/*
 * 
 * #define CONCAT(x) PRAGMA(concat on #x)
 * #define PRAGMA(x) _Pragma(#x)
 * CONCAT(..\concat.dir)
 * // above marco defination expands in macro like this :
 * _Pragma(concat on "..\concat.dir")
 * 
 */


//Variable-length argument
#define LOG(...) {\
	fprintf(stderr, "%s: Line %d:\t", __FILE__, __LINE__);\
	fprintf(stderr, __VA_ARGS__);\
	fprintf(stderr, "\n");\
}

#if __cplusplus < 201103L
	#error "should use cpp 11 implementation"
#endif

// custom-defined-static-assert
#define my_assert_static(e) \
	do { \
		enum { my_assert_static__ = 1/(e)};\
	} while(0)

enum FeatureSupports
{
	C99       = 0x0001,
	ExtInt    = 0x0002,
	SAssert   = 0x0004,
	NoExcept  = 0x0008,
	SMAX      = 0x0010,
};

struct Compiler
{
	const char * _name; // complier name
	int          _sp; //compiler FeatureSupports
};

struct TestStruct
{
	TestStruct() : _name(__func__) {}
	const char* _name;
};


const char* hello() {return __func__;}
const char* world() {return __func__;}
void c99MacroTest()
{
    // support completed C lib: 1 otherwise 0
	std::cout << "stardard Clib: " << __STDC_HOSTED__ << std::endl; 

    // support stardarded C lib: 1 otherwise 0(compiler related)
	std::cout << "stardard C: " << __STDC__ << std::endl; 

    // support stardarded C version: (compiler related)
	std::cout << "stardard C version: " << __VERSION__ << std::endl; 

    // support ISO/10646 version 
	std::cout << "ISO/IEC: " << __STDC_ISO_10646__ << std::endl; 

    // __cplusplus macros value
	std::cout << "__cplusplus: " << __cplusplus << std::endl; 

    // int size
	std::cout << "sizeof(int): " << sizeof(int) << std::endl; 

    // system pointer size
	void *mbits = 0;
	std::cout << "sizeof(ptr): " << sizeof(mbits) << std::endl; 

    // __func__ macro
    std::cout << "function name: " << hello() << ", " << world() << std::endl;

	TestStruct ts;
	std::cout << "struct name: " << ts._name << std::endl;

    // void FuncFail(string func_name =- __func__){} // ERROR, __func__ not defined when argument declare

	int x = 4;
	LOG(" x = %d,", x);

}

void extIntTest()
{
    // long long at least 64bits 
    // LL,ll
    // ULL, ull, Ull, uLL
	long long ll_min = LLONG_MIN;
	long long ll_max = LLONG_MAX;
	unsigned long long ull_max = ULLONG_MAX;

	printf("min of long long : %lld\n", ll_min);
	printf("max of long long : %lld\n", ll_max);
	printf("max of unsigned long long : %llu\n", ull_max);

    // C++11 standard signed int type (ungsigned type: replace signed with unsigned);
    // 1. signed char, char 
    // 2. signed short int, signed short, short
    // 3. signed int, int
    // 4. signed long int, long int, long
    // 5. signed long long int, long long int, long long
    // rules:
    // 1. compiler extention type signed and unsigned type must occupy SAME memory space
    // 2. compiler extention type implicit conversion or integral promotion:
    // 2.1 the more larger the size, the higher the rank( long rank > int rank)
    // 2.2 stardard integral rank higher than compiler extension integral when they are the same size ( long long rank > _int64 rank)
    // 2.3 signed and unsigned same type have same rank ( long long rank = unsigned long long rank)
    // 2.4 implicit conversion rules: lower rank trans to higher rank and signed trans to unsigned 
}


char* ArrayAlloc(int n)
{
	assert(n > 0);
	return new char[n];
}

template <typename T, typename U> int bit_copy(T& a, U& b)
{
	//assert(sizeof(a) == sizeof(b)); dyn
	//my_assert_static(sizeof(a) == sizeof(b)); // static
	// static_assert(sizeof(a) == sizeof(b), "the argument of bit_copy should have the same width"); // static
	memcpy(&a, &b, sizeof(b));
}

// static assert
static_assert(sizeof(int) == 4, "this 32-bit machine should follow this");
static_assert((SMAX - 1) == (C99 | ExtInt | SAssert | NoExcept), "compiler feature should be support completedly");
void staticAssertTest()
{
    // dynamic assert(runtime assert)
    // using NDEBUG macro to disable this dynamic assert
	// char* a = ArrayAlloc(0);

    // check enum value is completed or not
	assert((SMAX - 1) == (C99 | ExtInt | SAssert | NoExcept));
	Compiler com = {"hello", (C99 | SAssert)};

	if(com._sp & C99)
	{
		std::cout << "support c99 and static assert" << std::endl;
	}
	else
	{
		std::cout << "not support c99 and static assert" << std::endl;
	}

    // static assert(compile-time assert)
	int a= 0x2345;
	double b;
	bit_copy(a,b);
}


void Throw() { throw 1;}
void NoBlockThrow() { Throw();}
void BlockThrow() noexcept { Throw();}

void noExceptTest()
{
	try
	{
		Throw();
	}
	catch(...)
	{
		std::cout << "found throw" << std::endl;
	}

	try
	{
		NoBlockThrow();
	}
	catch(...)
	{
		std::cout << "throw  is no blocked" << std::endl;
	}

	try
	{
		BlockThrow(); // terminate called after throwing an instance of 'int'
	}
	catch(...)
	{
		std::cout << "found throw and terminate" << std::endl;
	}
}


// cpp11 default class deconstruction function : noexcept(true)
struct A
{
	~A() { throw 1;}
};

struct B
{
	~B() noexcept(false) { throw 2;}
};

struct C
{
	B b;
};

int funcA(){ A a;}
int funcB(){ B b;}
int funcC(){ C c;}

void noExceptTest2()
{
	try
	{
		funcB();
	}
	catch(...)
	{
		std::cout << "caught funcB" << std::endl;
	}

	try
	{
		funcC();
	}
	catch(...)
	{
		std::cout << "caught funcC" << std::endl;
	}

	try
	{
		funcA();
	}
	catch(...)
	{
		std::cout << "caught funcA" << std::endl;
	}
}


struct Inner
{
public:
	Inner(int i): _c(i){};
	int _c;
};

class Init
{
	public:
		Init(): _a(0){ }
		Init(int d ): _a(d){ }
	private:
		int _a{1};  // {} init
		constexpr static int _b = 2; // const static init
		int _c = 3; // = init
		//static int _d = 4;
		static constexpr double _e = 5; // const static init
		std::string _f = "6";
		//std::string _h("7"); // c++11 NOT support ()
		std::string _i{"8"}; // c++11 support init {}
		//Inner _g(9); // c++11 NOT support ()
		Inner _j{10}; //  c++11 support init {}
	public:
		void Printf()
		{
			std::cout << "_a = " << _a << std::endl; // _a = 0
			std::cout << "_b = " << _b << std::endl; // _b = 0
			std::cout << "_c = " << _c << std::endl; // _c = 1
			//std::cout << "_d = " << _d << std::endl;
			std::cout << "_e = " << _e << std::endl;
			std::cout << "_f = " << _f << std::endl;
			//std::cout << "_h = " << _h << std::endl;
			std::cout << "_i = " << _i << std::endl;
			//std::cout << "_g = " << _g._c << std::endl;
			std::cout << "_j = " << _j._c << std::endl;
		}

};

struct Mem{
    Mem(){std::cout << "Mem Default construction, num : " << num << std::endl;}
    Mem(int i) : num(i) {std::cout << "Mem construction with argument, num : " << num << std::endl;}
    int num = 2; // use = init 
};

class Group{
    public:
        Group(){std::cout << "Group Default construction, val : " << val << std::endl;}
        Group(int i) : val('G'), a(i) {std::cout << "Group construction with argument, val : " << val << std::endl;}
        void numOfA(){ std::cout << "num of A: " << a.num << std::endl;}
        void numOfB(){ std::cout << "num of B: " << b.num << std::endl;}

    private:
        char val{'g'}; // use {} init
        Mem a;
        Mem b{33}; // use {} init
};


void memInitTest()
{
    // init list override inplace init
	Init i;
	i.Printf();

    Mem member;
    Group group;
    group.numOfA();
    group.numOfB();

    Group group2(333);
    group2.numOfA();
    group2.numOfB();
}

struct People 
{
	public:
		int hand;
		static People *all;
};

void noneStaticMemSizeofTest()
{
	People  p;
	std::cout << sizeof(p.hand) << std::endl; // c++98, c++11 both ok
	std::cout << sizeof(p.all) << std::endl;// c++98, c++11 both ok
	std::cout << sizeof(People::all) << std::endl;// c++98, c++11 both ok
	std::cout << sizeof(People::hand) << std::endl; // c++98 error but c++11 ok
	std::cout << sizeof(((People*)0)->hand) << std::endl; // c++98 trick code 
}

class Poly;
typedef Poly P;

class LiLei
{
	friend class Poly;  // c++98 and c++11 both compile ok 
};

class Jim
{
	friend Poly; // c++98 error but c++11 ok
};

class HanMeiMei
{
	friend P; // c++98 error but c++11 ok
};

template <typename T> class Person
{
	friend  T;
};

using PP = Person<P>;  // class P is the friend class of Person
using Pi = Person<int>; // friend declare is ignored when using int type

template <typename T> class DefenderT
{
	public:
		friend T;
        void defence(int x, int y){_x = x; _y = y;}
	private:
		int _x = 1;
		int _y = 1;
};

// generic class using int as argument
using Defender = DefenderT<int>;

// Unit Test Validator Class
class Validator;
using DefenderTest = DefenderT<Validator>; // Unit Test Specialized Class 
class Validator
{
	public:
		void Validate(int x, int y, DefenderTest &d)
        {
            std::string res1 = (d._x == x) ? "true" : "false";
            std::string res2 = (d._y == y) ? "true" : "false";
            std::cout << "unit test x.res  = "  << res1  << std::endl;
            std::cout << "unit test y.res  = "  << res2  << std::endl;
        }
};


void extFriendTest()
{
	PP p;
	Pi i;

	Validator v;
	DefenderTest d;
    d.defence(1,2);
	v.Validate(1,1, d);
}

class Base{
    virtual void turning() = 0;
    virtual void dijkstra(){};
    virtual void vneumann() final {}  // final means prevent from override(ONLY supports virtual function)
    virtual void dKnuth() const {};
    void print() {};
};

class Derive : Base{
    public: void turning() override { std::cout << "Dervie override turning function" << std::endl; }
    public: void dijkstra() { std::cout << "Dervie override dijkstra function" << std::endl; } // override can be ignored
    // void vneumann() override {} // error: final not support override

    // void print() override {} // error: None-virtual function override
};

void finalOverrideKeywordTest()
{
    Derive d;
    d.turning();
    d.dijkstra();
}

// template function and template class default argument
void defParam(int m = 3){}  // c++98 and c++11 both ok

template <typename T = int> 
    class defTemplateClassArgmumenti {}; // c++98 and c++11 both ok

template <typename T = int>
    void defTemplateFuncArgument() {}; // c++98 error but c++11 ok

// template <typename T = int, typename U>
    // class defTemplateClassMultiArgument {}; // c++98 and c++11 both error

// template <int i = 0, typename T>
     // class defTemplateClassMultiArgument {}; // c++98 and c++11 both error

template <typename T, typename U = int>
    class defTemplateClassMultiArgument {}; // c++98 and c++11 both ok

template <typename T, int i = 0>
    class defTemplateClassMultiArgument2 {}; // c++98 and c++11 both ok


template <typename T = int, typename U>
    void defTemplateFuncMultiArgument() {}; // c++98 and c++11 both ok

template <int i = 0, typename T>
    void defTemplateFuncMultiArgument2() {}; // c++98 and c++11 both ok

template <typename T, typename U = int>
    void defTemplateFuncMultiArgument3() {}; // c++98 and c++11 both ok

template <typename T, int i = 0>
    void defTemplateFuncMultiArgument4() {}; // c++98 and c++11 both ok


template <class T, class U = double>
void templateFuncInfer(T t = 0, U u = 0){};

// function template argument selection is inferred by function actual parameter NOT formal argument
void templateFuncDefaultArgumentTest()
{
    templateFuncInfer(1, 'c'); // templateFuncInfer<int, char> instantiation
    templateFuncInfer(1); // templateFuncInfer<int, double> instantiation using default 
    // templateFuncInfer(); // error: templateFuncInfer<int, double> instantiation error
    templateFuncInfer<int>(); // templateFuncInfer<int, double> instantiation using default
    templateFuncInfer<int, char>(); // templateFuncInfer<int, char> instantiation
}

// add extern template function or class declare to decrease linking time remarkably
void externTemplateTest()
{
    externTemplateTest1();
    externTemplateTest2();
}


template<typename T> class TemplateClass {};
template<typename T> void TemplateFunc(T t) {};
class namedClass{} namedClassVar;   // named class 
class {int a; }unnamedClassVar;     // unamed class var
typedef struct {int i;} unamedTypedefStruct;  // typedef unamed struct type 

void localAndUnameTypeTemplateArgumentTest()
{
    class localClass {} localClassVar;  // local class

    TemplateClass<namedClass> c1;    // c++98 and c++11 both ok
    TemplateClass<unamedTypedefStruct> c2;    // c++98 error but c++11 ok
    TemplateClass<localClass> c3;    // c++98 error but c++11 ok

    TemplateFunc(namedClassVar);    // c++98 and c++11 both ok
    TemplateFunc(unnamedClassVar);    // c++98 error but c++11 ok
    TemplateFunc(localClassVar);    // c++98 error but c++11 ok
}

int main(int argc, char** argv)
{
	// c99MacroTest();
	// extIntTest();
	// staticAssertTest();
	// noExceptTest();
	// noExceptTest2();
	// memInitTest();
    // noneStaticMemSizeofTest();
	// extFriendTest();
    // finalOverrideKeywordTest();
    // templateFuncDefaultArgumentTest();
    // externTemplateTest();
    localAndUnameTypeTemplateArgumentTest();

    return 0;
}

