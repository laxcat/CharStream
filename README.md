# CharStream

A C++17 character streaming class focussing on developer convenience. Specifically an attempt to create a simple-to-use log function (string conversion function) that accepts and automatically formats any number of parameters of any type, similar to swift's `print` or JavaScript's `console.log`. Uses stb_sprintf.h as core formatter.

Includes simple test with an example, with a simple `buildtest` script of a single command only tested on macOS. Nothing here is tested beyond that. Nothing here is production ready.



## Features

- Automatically creates a simple format string with set-once seperator and terminus strings
- Easy to write to the same output with one-off change to formating
- Can write direct to standard outputs or to a string buffer
- Works with basic types and any type that converts into a `char const *`
- Optional convenience macro to further simplify using custom types



## Examples

Create instance with default settings.

```cpp
CharStream Log;
Log(1.f, "two", 3);
Log('4');
```
Output to `stdout` (default):
```
1.00000 two 3
4
```



Set ouput target, seperator, and terminus strings once for the instance.

```cpp
CharStream Log{CharStream::STDOUT, ", ", "!\n"};
Log(1.f, "two", 3);
Log('4');
```
Ouptut to `stdout`:
```
1.00000, two, 3!
4!
```



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
Ouput to `stdout`:
```
(56, 75) (3, 4)
```



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
Ouput to `stdout`:
```
(56, 75) (3, 4)
```



## Requirements

- [`stb_sprintf.h`](https://github.com/nothings/stb/blob/master/stb_sprintf.h)
- [`<stdint.h>`](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdint.h.html)
- [`<unistd.h>` (macOS, *nix)](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/unistd.h.html) or [`<io.h>` (Windows)](https://docs.microsoft.com/en-us/cpp/c-runtime-library/low-level-i-o)



## API
**Constructor** 

`target` accepts a `char *`, `FILE *`, `size_t` (and others) reference to an output or string buffer. For each call operator call, `sep` is written between each parameter and `trm` is written after the last one.

```cpp
CharStreamer(
    CharStream::Target target = stdout,
    char const * sep = " ",
    char const * trm = "\n"
);
```



**Function Call Operator ()** 

Writes parameters to `target` buffer. `sep` is written between each parameter and `trm` is written after the last. Returns number of bytes written, not counting terminating null-byte.  

```cpp
template <typename ... TS>
int operator () (TS && ...);
```



**Format** 

Writes parameters to `target`, with provided `formatString`, ignoring instance `sep` and `trm` for this call only. Returns number of bytes written, not counting terminating null-byte.

```cpp
template <typename ... TS>
int format(char const * formatString, TS && ...);
```



**Write** 

Writes parameters to target, ignoring instance `sep` and `trm`, using `seperator` for this call only. Assumes last parameter to be terminus string, used . Returns number of bytes written, not counting terminating null-byte.

```cpp
template <typename ... TS>
int write(char const * seperator, TS && ...);
```



**CHAR_STREAM_OPERATOR**

Macro function for conveniently adding a `char const *` operator to a custom class. `SIZE` is the number of characters needed for each instance's output. `COUNT` is number of this custom-type instances that can be included as parameters of any one given call. `FORMAT` and the variadic parameters are used to construct the string.

```cpp
CHAR_STREAM_OPERATOR(SIZE, COUNT, FORMAT, ...)
```
