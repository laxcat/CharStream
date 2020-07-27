#define CHAR_STREAM_ENABLE_OPERATOR_MACRO
#include "CharStream.h"


struct IntPair {
    int a, b;
    IntPair(int a, int b) : a(a), b(b) {}
    CHAR_STREAM_OPERATOR(32, 8, "(%d, %d)", a, b)
};


int main() {
    CharStream Log;

    IntPair foo{56, 75};
    IntPair bar{3, 4};

    Log(foo, bar);

    return 0;
}
