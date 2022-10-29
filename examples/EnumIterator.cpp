/*
* EnumHelper version 0.1.0
* Licensed under the MIT License <http://opensource.org/licenses/MIT>.
* SPDX-License-Identifier: MIT
* Copyright (c) 2018 - 2022 Patrik Olesen <patrik@hemma.org>
*/
#include "EnumHelper.h"

int main(int, char *[])
{
    ENUM_HELPER(Color, Red = 23, Blue, Green, Purple, Red1 = 100, Blue1, Green1, Purple1, Red2, Blue2, Green2, Purple2);
    for(auto const& color : ColorMagicEnum)
    {
        printf("%s = %ld\n", color.toString(), color.getIntValue());
    }
}