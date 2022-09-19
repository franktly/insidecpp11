#pragma once

#include "externTemplate.h"
#include <iostream>

extern template void fun<int>(int); // c++11 explicit instantiation 

void externTemplateTest1()
{
    std::cout << "extern template test 1 with int argumetn 3 " << std::endl;
    fun(3);
};
