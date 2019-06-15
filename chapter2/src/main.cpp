//#define NDEBUG
#include <iostream>
#include <climits>
#include <cassert>
#include <stdio.h>
#include <cstring>

#define LOG(...) {\
	fprintf(stderr, "%s: Line %d:\t", __FILE__, __LINE__);\
	fprintf(stderr, __VA_ARGS__);\
	fprintf(stderr, "\n");\
}

#if __cplusplus < 201103L
	#error "should use cpp 11 implementation"
#endif

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
	const char * _name;
	int          _sp;
};

struct TestStruct
{
	TestStruct() : _name(__func__) {}
	const char* _name;
};

void macroTest()
{
	std::cout << "stardard Clib: " << __STDC_HOSTED__ << std::endl; 
	std::cout << "stardard C: " << __STDC__ << std::endl; 
	std::cout << "stardard C version: " << __VERSION__ << std::endl; 
	std::cout << "ISO/IEC: " << __STDC_ISO_10646__ << std::endl; 
	std::cout << "__cplusplus: " << __cplusplus << std::endl; 
	std::cout << "sizeof(int): " << sizeof(int) << std::endl; 
	void *mbits = 0;
	std::cout << "sizeof(ptr): " << sizeof(mbits) << std::endl; 

	TestStruct ts;
	std::cout << ts._name << std::endl;

	int x = 4;
	LOG(" x = %d,", x);

}

void extIntTest()
{
	long long ll_min = LLONG_MIN;
	long long ll_max = LLONG_MAX;
	unsigned long long ull_max = ULLONG_MAX;

	printf("min of long long : %lld\n", ll_min);
	printf("max of long long : %lld\n", ll_max);
	printf("max of unsigned long long : %llu\n", ull_max);
}


char* ArrayAlloc(int n)
{
	assert(n > 0);
	return new char[n];
}

template <typename T, typename U> int bit_copy(T& a, U& b)
{
	//assert(sizeof(a) == sizeof(b));
	//my_assert_static(sizeof(a) == sizeof(b));
	//static_assert(sizeof(a) == sizeof(b), "the parameter of bit_copy should have the same width");
	memcpy(&a, &b, sizeof(b));
}
static_assert(sizeof(int) == 4, "this 32-bit machine should follow this");
void staticAssertTest()
{
	// char* a = ArrayAlloc(0);

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
		std::cout << "throw  is blocked" << std::endl;
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
		int _a{2};  // {} init
		constexpr static int _b = 0; // const static init
		int _c = 1; // = init
		//static int _d = 0;
		static constexpr double _e = 1.2; // const static init
		std::string _f = "hello";
		//std::string _h("world");
		std::string _i{"tly"};
		//Inner _g(1);
		Inner _j{1}; // {} init aggration type
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

void memInitTest()
{
	Init i;
	i.Printf();
}

struct People 
{
	public:
		int hand;
		static People *all;
};

void noneStaticSizeofTest()
{
	People  p;
	std::cout << sizeof(p.hand) << std::endl;
	std::cout << sizeof(p.all) << std::endl;
	std::cout << sizeof(People::all) << std::endl;
	std::cout << sizeof(People::hand) << std::endl; // c98 compile error but c11 ok
	std::cout << sizeof(((People*)0)->hand) << std::endl; // c98 trick code 
}

class Poly;
typedef Poly P;

class LiLei
{
	friend class Poly;  // c98 and c11 both compile ok 
};

class Jim
{
	friend Poly; // c98 compile error but c11 ok
};

class HanMeiMei
{
	friend P; // c98 compile error but c11 ok
};

template <typename T> class Man
{
	friend  T;
};

using Mp = Man<P>;  // P is the friend class of Man
using Mi = Man<int>;

template <typename T> class DefenderT
{
	public:
		friend T;
		DefenderT(int x , int y): _x(x), _y(y) {}
	private:
		int _x = 1;
		int _y = 1;
};

using Defender = DefenderT<int>;

class Validator;
using DefenderTest = DefenderT<Validator>;
class Validator
{
	public:
		void Validate(int x, int y, DefenderTest &d)
	       	{
			std::string res1 = (d._x == x) ? "true" : "false";
			std::string res2 = (d._y == y) ? "true" : "false";
			std::cout << "x.res  = "  << res1  << std::endl;
			std::cout << "y.res  = "  << res2  << std::endl;
		}
};


void extFriendTest()
{
	Mp p;
	Mi i;
	Validator v;
	DefenderTest d(1,2);
	v.Validate(1,2, d);
}

int main()
{
	macroTest();
	extIntTest();
	staticAssertTest();
	//noExceptTest();
	//noExceptTest2();
	memInitTest();
	noneStaticSizeofTest();
	extFriendTest();
}

