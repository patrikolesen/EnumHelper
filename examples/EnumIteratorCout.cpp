/*
* EnumHelper version 0.1.0
* Licensed under the MIT License <http://opensource.org/licenses/MIT>.
* SPDX-License-Identifier: MIT
* Copyright (c) 2018 - 2022 Patrik Olesen <patrik@hemma.org>
*/
#include "EnumHelper.h"
#include <iostream>

int main(int, char *[])
{
    EnumHelper(Color, Red = 23, Blue, Purple2 = 100 + 100);
    for(auto const& color : ColorMagicEnum)
    {
         std::cout << color <<", ";
    }
    std::cout << std::endl;
}