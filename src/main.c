#pragma once

#include "main.h"


i32 i32_power(i32 base, i32 exponent)
{
    i32 final = 1;
    for (i8 i = 1; i <= exponent; i++)
    {
        final *= base;
    }
    return final;
}

u32 cstring_to_num(char *numeric, u32 length)
{
    u32 conversion = 0;

    char * test = "420";
    for (i8 i = 0; i < (i32)length; i++)
    {
        u8 charNum = (u8)numeric[length - i - 1] - 48;
        conversion += charNum * i32_power(10, i);
    }

    return conversion;
}

int main(int argc, char *argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    i32 x = i32_power(2, 8);
    i32 y = i32_power(2, 0);
    
    char *example = "420";
    cstring input = {0};
    input.string = example;
    input.length = 3;
    u32 conversion = cstring_to_num(example, 3);
    conversion = cstring_to_num("34", 2);
    conversion = cstring_to_num("1000", 4);
    conversion = cstring_to_num("911", 3);

    return 0;
}
