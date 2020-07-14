# CharStream

A C++17 character streaming class focussing on developer convenience. Uses stb_sprintf.h as core formatter.

Very much NOT production ready or tested.

## Features
- Automatically creates a simple format string with set-once seperator and terminus strings
- Can temporarily override seperator and terminus strings to write one-offs to same output or buffer
- Can write direct to STD outputs or to string buffer
- Works with basic types and any type that converts into a `char const *`
- Optional convenience macro to further simplify using custom types

## Examples
Set settings once for instance.
```cpp
CharStream Log;
Log(1.f, "two", 3);
Log('4');
```
```
1.00000 two 3
4
```
<br>

Set seperator and terminus strings once for the instance.
```cpp
CharStream Log{CharStream::STDOUT, ", ", "!\n"};
Log(1.f, "two", 3);
Log('4');
```
```bash
1.00000, two, 3!
4!
```
<br>

Customize output target and seperator/terminus strings with the constructor.
```cpp
char _buff[512];
CharStream Log{_buff, "::", "\n----------\n"};
Log(1.f, "two", 3);
Log('4');
```
Written to `_buff`:
```
1.00000::two::3
----------
4
----------
```
<br>

Works with custom types, any type that converts to `char const *`.
```cpp
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

IntPair foo{56, 75};
IntPair bar{3, 4};

CharStream Log;
Log(foo, bar);
```
```
(56, 75) (3, 4)
```
<br>

Use `CHAR_STREAM_OPERATOR` macro to automatically construct custom type conversion operator.
```cpp
#define CHAR_STREAM_OPERATOR_ENABLE
#include "CharStream.h"

struct IntPair {
    int a, b;
    IntPair(int a, int b) : a(a), b(b) {}
    CHAR_STREAM_OPERATOR(32, 8, "(%d, %d)", a, b)
};

IntPair foo{56, 75};
IntPair bar{3, 4};

CharStream Log;
Log(foo, bar);
```
```
(56, 75) (3, 4)
```
<br>

## Requirements:
`stb_sprintf.h` (https://github.com/nothings/stb/blob/master/stb_sprintf.h)  
`<stdint.h>`  

## API:

**Constructor**  
`target`: pointer to `char *` buffer or literal value keyword `STDIN`, `STDOUT` or `STDERR`  
`sep`: seperator `char const *` that is written between each parameter in `()` call  
`trm`: terminus `char const *` that is written after all parameters in `()` call  
```cpp
CharStreamer(
    char * target = CharStream::STDOUT,
    char const * sep = " ",
    char const * trm = "\n"
);
```

**Function Call Operator ()**  
Writes parameters to `target`. `sep` is written between each parameter. `trm` is written at end.  
Returns number of bytes written, not counting terminating null-byte.  
```cpp
template <typename ... TS>
int operator () (TS && ...);
```

**Format**  
Writes parameters to `target`, with provided `formatString`, thus instance `sep` and `trm` are ignored.  
Returns number of bytes written, not counting terminating null-byte.  
```cpp
template <typename ... TS>
int format(char const * formatString, TS && ...);
```

**Write**  
Writes parameters to target, ignoring instance `sep` and `trm`, using `seperator` for this call only.  
Returns number of bytes written, not counting terminating null-byte.  
```cpp
template <typename ... TS>
int write(char const * seperator, TS && ...);
```
