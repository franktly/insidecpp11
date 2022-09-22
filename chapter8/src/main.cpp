#include<iostream>
#include<atomic>
#include<thread>

// align - ABI attribution
// padding data: impore cache hit, some hardware throw bus error if NOT align correctly
//

struct S
{
    int  a;
    char b;
    double c;
};
/*
 *
 *  S layout (B stands for Byte[8 bits])
 *  alignof(S) = 8:
 *
 *  | B00 | B01 | B02 | B03 | B04 | B05 | B06 | B07 |
 *  |----------a(4)---------|-b(1)|----padding(3)---|
 *  | B08 | B09 | B10 | B11 | B12 | B13 | B14 | B15 |
 *  |---------------------c(8)----------------------|
 *
 * Offset:
 * a: 0
 * b: 4
 * c: 8
 *
 * sizeof(S) = 8 * 2  = 16
 * */

struct Color
{
    double r;
    double g;
    double b;
    double a;
};

struct alignas(32) ColorV2
{
    double r;
    double g;
    double b;
    double a;
};

// fixed capacity tempate array
const int FIXED_SIZE = 1024;
template<typename T>
class FixedCapArray
{
    public:
        void push_back(T t){}
        alignas(T) char data[FIXED_SIZE] = {};
        // int len = FIXED_SIZE/sizeof(T)
};

void alignTest()
{
    std::cout << "sizeof(int): " << sizeof(int) << std::endl;
    std::cout << "sizeof(char): " << sizeof(char) << std::endl;
    std::cout << "sizeof(double): " << sizeof(double) << std::endl;
    std::cout << "sizeof(S): " << sizeof(S) << std::endl;
    std::cout << "alignof(S): " << alignof(S) << std::endl;
    std::cout << "offset of a in S: " << offsetof(S, a) << std::endl;
    std::cout << "offset of b in S: " << offsetof(S, b) << std::endl;
    std::cout << "offset of c in S: " << offsetof(S, c) << std::endl;

    std::cout << "alignof(Color): " << alignof(Color) << std::endl;
    std::cout << "alignof(ColorV2): " << alignof(ColorV2) << std::endl;

    class InComplete;
    struct Completed{};
    int a;
    long long b;
    auto &c = b;
    char d[1024];
    // alignof basic and custom define type
    std::cout << "alignof(int): " << alignof(int) << std::endl;
    std::cout << "alignof(Completed): " << alignof(Completed) << std::endl;
    // alignof var , ref and array
    std::cout << "alignof(a): " << alignof(a) << std::endl;
    std::cout << "alignof(b): " << alignof(b) << std::endl;
    std::cout << "alignof(c): " << alignof(c) << std::endl; // the ref is the same type refered from
    std::cout << "alignof(d): " << alignof(d) << std::endl;
    // std::cout << "alignof(InComplete): " << alignof(InComplete) << std::endl; // error:  incomplete
    alignas(double) char e;
    alignas(alignof(double)) char f;
    std::cout << "alignof(e): " << alignof(e) << std::endl;
    std::cout << "alignof(f): " << alignof(f) << std::endl;
    std::cout << "alignof(max_align_t): " << alignof(max_align_t) << std::endl; // max scale type align: long double 

    // fixed size array  using template
    FixedCapArray<char> charArr;
    std::cout << "alignof(char): " << alignof(char) << std::endl;
    std::cout << "alignof(charArr): " << alignof(charArr) << std::endl;
    std::cout << "alignof(charArr.data): " << alignof(charArr.data) << std::endl;

    FixedCapArray<ColorV2> colorArr;
    std::cout << "alignof(color): " << alignof(ColorV2) << std::endl;
    std::cout << "alignof(colorArr): " << alignof(colorArr) << std::endl;
    std::cout << "alignof(colorArr.data): " << alignof(colorArr.data) << std::endl;

    // aligned_storage(union)
    struct IntAligned
    {
        int a;
        char b;
    };

    // 
    typedef std::aligned_storage<sizeof(IntAligned), alignof(long double)>::type StrictAligned;

    StrictAligned sa;
    IntAligned* pia = new (&sa) IntAligned;
    std::cout << "sizeof(IntAligned): " << sizeof(IntAligned) << std::endl;
    std::cout << "alignof(IntAligned): " << alignof(IntAligned) << std::endl;

    std::cout << "sizeof(long double): " << sizeof(long double) << std::endl;
    std::cout << "alignof(long double): " << alignof(long double) << std::endl;

    std::cout << "sizeof(StrictAligned): " << sizeof(StrictAligned) << std::endl;
    std::cout << "alignof(StrictAligned): " << alignof(StrictAligned) << std::endl;

    std::cout << "alignof(*pia): " << alignof(*pia) << std::endl;
    std::cout << "alignof(sa): " << alignof(sa) << std::endl;
}

// general attribute
// old traditional:
//  GCC ext: __attribute__ ((attribute-list))
//  MS  ext: __declspec (extended-decl-modifier)
// C++11 standard: 
// [[attribute-list]]
//
// predefined general attribute:
// 1. [[noreturn]]: A function Never return: different from function return void
// previous means NO subsequence code will be executed, later means continue execute but curretn function hhas no return value;
// 1). terminate app
// 2). break from infinite loop
// 3). throw exception 
// 2. [[carries_dependency]]: memory_order_consume related (current atomic operation must first hanppen than subsequence related-current-atomic operation)
// mainly useful to weak-memory plateform (PowerPC)
// strong-memory plateform ignored this attribute
// using [[carries_dependency]] means will not generate memory barriar when invoke function , pass parameter or return value and improve performance

// attr1 , attr2 both take effect to func
// attr3  takes effect to parameter i
// attr4  takes effect to parameter j
// attr5  takes effect to return statement
[[attr1]] int func [[attr2]]([[attr3]] int i, [[attr4]] int j)
{
    // do something
    [[attr5]] return i + j;
}

void do1();
void do2();
[[noreturn]] void myThrow()
{
    throw "exception"; 
}

// C++ abort() function definition:
// [[noreturn]] void abort(void) noexcept;

std::atomic<int*> p1;
std::atomic<int*> p2;
std::atomic<int*> p3;
std::atomic<int*> p4;

// means need add memory barriar when invoking func_in1()
void func_in1(int* val)
{
    std::cout << *val << std::endl;
}

// means NO need add memory barriar when invoking func_in2()
// void func_in2(int* [[carries_dependency]] val)
void func_in2([[carries_dependency]] int* val)
{
    // means NO need add memory barriar when passing val
    p2.store(val, std::memory_order_release);
    std::cout << *val << std::endl;
}

// means NO need add memory barriar when invoking func_out()
[[carries_dependency]] int* func_out()
{
    // means NO need add memory barriar before return
    return (int*)p3.load(std::memory_order_consume);
}

int p1_val = 3;
int p3_val = 33;
void funcAttr()
{
    p1.store(&p1_val, std::memory_order_consume);
    p3.store(&p3_val, std::memory_order_consume);

    int* ptr1 = (int*) p1.load(std::memory_order_consume);
    std::cout << "ptr1: " << *ptr1 << std::endl;

    func_in1(ptr1);
    func_in2(ptr1);

    int *ptr2 = func_out();
    p4.store(ptr2, std::memory_order_release);
    std::cout << "ptr2: " << *ptr2 << std::endl;
}

void attributeTest()
{

    // attr1 , attr2 both take effect to arr
    [[attr1]] int arr [[attr2]][10];

    // [[attr1]] class C [[attr2]] { } [[attr3]] c1 [[attr4]], c2 [[attr5]];

    // attr1 takes effect to label L1
    [[attr1]] L1:

    // attr1 takes effect to got statement
    [[attr1]] goto L1;

    // funcAttr();

    // do1();
    // myThrow();
    // do2(); // function never reached
}

// Unicode 
//
//ASCII Charset: 7bits: 128 different chars
//Unicode Charset:  0~0x10FFFF: 1114112 different chars, only defines codepoint(number) and corresponding character,it has 3 kinds of encoding implementation(each has diffrent Bytes and Range):
//1. UTF-8: 1~4 Bytes, varidic encodiing: Web encoding, english(1 Bytes, compatible with ASCII), chinese(3 or 4 Bytes); Mac and Linux by default 
//2. UTF-16: 2 or 4Bytes, varidic encoding: NOT compatible with ASCII; english 2 Bytes, chinese 2 or 4 Bytes; Windows by default
//3. UTF-32:: 4Bytes, Fixed_width endcoding: NOT compatible with ASCII, all lang 4 Bytes, Rarely used
// other chinese charset and encode: 
// GB2312: simple chinese : a chinese charactor occupies 2 Bytes
// BIG5: traditional chinese : a chinese charactor occupies 2 Bytes
// 
// Wide Char
//
// C++98: 
// wchar_t(length depend on compiler, can be 8/16/32bits, Windows always 16bits, Linux 32 bits always
//
// C++11: 
// char: Unicode encoded by UTF-8
// char16_t: Unicode encoded by UTF-16
// char32_t: Unicode encoded by UTF-32
//
// const char prefix:(5 kinds of way to define string literal const):
// u8: UTF-8(C++11): '\uXX'(X=0~F) 
// u: UTF-16(C++11): '\uXXXX'(X=0~F) 
// U: UTF-32(C++11): '\UXXXXXXXX'(X=0~F)
// L: wchar_t
// None-Prefix: char
// Correct Unicode Charactor:  possible factor
// 1) Program Code: coder guarantee
// 2) Text Edit Save: vim ...
// 3) Compiler: Gcc, MSVC ...
// 4) Output: shell, console...
//
// Unicode convert library
// 1. char16_t & char32_t convert to mb(multi-byte): including <cuchar>
// four functions: mbrtoc16(),  c16rtomb(), mbrtoc32(), c32rtomb()
// 2. locale:  (such as: USA: en_US.UTF-8, CHA: zh_CN.GB2312): including <locale>
// 2.1 two kinds of template function each has four template functions: 
// 1) codecvt<[char, char16_t, char32_t, wchar_t], char, mbstate_t>()
// 2) has_facet<[char, char16_t, char32_t, wchar_t], char, mbstate_t>()
// 2.2 derive template classes: 
// codecvt_utf8, codecvt_utf16, codecvt_utf32,codecvt_utf8_tuf16...


void UnicodeTest()
{
    char c1[] = "hello";
    char c2[] = "你好啊";
    wchar_t wc1[] = L"hello";
    wchar_t wc2[] = L"你好啊";
    char utf8[] = u8"\u4F60\u597D\u554A";
    char16_t utf16[] = u"hello";   // linux does not support UTF-16 output
    char32_t utf32[] = U"hello equals \u4F60\u597D\u554A";   // linux does not support UTF-32 output

    std::cout << "char[]: " << c1 << std::endl; 
    std::cout << "char[]: " << c2 << std::endl;
    std::cout << "wchar[]: " << wc1 << std::endl;
    std::cout << "wchar[]: " << wc2 << std::endl;
    std::cout << "utf-8: " << utf8 << std::endl;
    std::cout << "utf-16: " << utf16 << std::endl;
    std::cout << "utf-32: " << utf32 << std::endl;

    std::cout <<  "c1 size: " << sizeof(c1) << std::endl;// 6: every english charactor 1 Bytes
    std::cout <<  "c2 size: " << sizeof(c2) << std::endl; //10 every chinese charactor 3 Bytes + '\0' 
    std::cout <<  "wc1 size: " << sizeof(wc1) << std::endl;
    std::cout <<  "wc2 size: " << sizeof(wc2) << std::endl;
    std::cout <<  "utf8 size: " << sizeof(utf8) << std::endl;
    std::cout <<  "utf16 size: " << sizeof(utf16) << std::endl;
    std::cout <<  "utf32 size: " << sizeof(utf32) << std::endl;


    // char32_t u2[] =u"hello"; // error: type mismatch
    // char u3[] =U"hello"; // error: type mismatch
    // char16_t u4[] =u8"hello"; // error: type mismatch
    //
    char utf8_2[] = u8"\u4F60\u597D\u554A";
    char16_t utf16_2[] = u"\u4F60\u597D\u554A";
    char32_t utf32_2[] = U"\u4F60\u597D\u554A";
    wchar_t wchar[] = L"\u4F60\u597D\u554A";
    std::cout <<  "u8 size: " << sizeof(utf8_2) << std::endl; //10 every chinese charactor 3 Bytes + '\0' 
    std::cout <<  "u16 size: " << sizeof(utf16_2) << std::endl;
    std::cout <<  "u32 size: " << sizeof(utf32_2) << std::endl;
    std::cout <<  "wchar size: " << sizeof(wchar) << std::endl;
}

// raw string literal
// R"XXX";
// u8R-- UTF8, uR-- UTF16, UR-- UTF32
//

void rawStringLiteralTest()
{
    // raw string literal output
    std::cout << R"(hello, \n
        world)" << std::endl;

    std::cout << u8R"(\u4F60, \n
    \u597D)" << std::endl;
    std::cout << u8R"(你好)" << std::endl;
    std::cout << u8R"(hello)" << std::endl;
    std::cout << uR"(hello)" << std::endl;
    std::cout << UR"(hello)" << std::endl;

    // raw string literal size
    std::cout << "sizeof(u8R-zh): "  << sizeof(u8R"(你好)") << std::endl;  // 7  = 3 * 2 + 1
    std::cout << "sizeof(u8R-en): " << sizeof(u8R"(hello)") << std::endl;  // 6 = 1 * 5 + 1
    std::cout << "sizeof(uR): " << sizeof(uR"(hello)") << std::endl;
    std::cout << "sizeof(UR): " << sizeof(UR"(hello)") << std::endl;

    // raw string literal concat
    char u8string[] = u8R"(你好)"  " = hello";
    std::cout << u8string << std::endl;
    std::cout << "sizeof(concat-u8R): " << sizeof(u8string) << std::endl; // 15 = 3 * 2 + 8 + 1 
}

int main(int argc, char** argv)
{
    // alignTest();
    // attributeTest();
    // UnicodeTest();
    rawStringLiteralTest();
    return 0;
}
