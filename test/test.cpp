#define STB_SPRINTF_MIN 128
#define STB_SPRINTF_NOFLOAT
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#define CHAR_STREAM_BUFFER_SIZE STB_SPRINTF_MIN
#define CHAR_STREAM_FORMAT_BUFFER_SIZE 64
#define CHAR_STREAM_SPRINTF stbsp_sprintf
#define CHAR_STREAM_ENABLE_OPERATOR_MACRO
#include "../CharStream.h"


struct IntPair {
    int a, b;
    IntPair(int a, int b) : a(a), b(b) {}
    CHAR_STREAM_OPERATOR(32, 8, "(%d, %d)", a, b)
};




int main() {
    CharStream Log;
    Log();


    // Standard types
    Log("Standard types:\n----------------");

    auto a = -4300000000;
    auto b = 4300000000;
    auto c = 'c';
    int8_t d = c;
    char e = d;
    Log(a, b, true, false, c, d, e, -4400000000);
    Log();


    // Custom types
    Log("Custom types:\n----------------");

    IntPair foo{56, 75};
    IntPair bar{3, 4};

    Log(foo, bar);
    Log();


    // Write function
    Log("Write function:\n----------------");

    Log.write("___", 1, 2, 3, "\n");
    CharStream SameSame{stdout, "___", "\n"};
    SameSame(1, 2, 3);
    Log(1, 2, 3);
    Log();


    // Format function
    Log("Format function\n----------------");

    Log.format("(%d%d%d)\n", 1, 2, 3);
    Log(1, 2, 3);
    Log();

    return 0;
}
