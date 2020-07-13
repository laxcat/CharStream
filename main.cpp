#define CHAR_STREAM_OPERATOR_ENABLE
#include "CharStream.h"


struct IntPair {
    int a, b;
    IntPair(int a, int b) : a(a), b(b) {}
    operator char const * () {
        stbsp_sprintf(_buff, "(%d, %d)", a, b);
        return _buff;
    }
private:
    char _buff[32];
};

struct IntPair {
    int a, b;
    IntPair(int a, int b) : a(a), b(b) {}
    CHAR_STREAM_OPERATOR(32, 8, "(%d, %d)", a, b);
};


int main() {
    IntPair foo{56, 75};
    IntPair bar{3, 4};

    Log(foo, bar);

    return 0;
}
