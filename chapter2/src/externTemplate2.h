#pragma once

#include "externTemplate.h"
#include <iostream>

template void fun<int>(int); // c++11 extern template declare 

void externTemplateTest2()
{
    std::cout << "extern template test 2 with int argumetn 33 " << std::endl;
    fun(33);
};
