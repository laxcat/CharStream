/*

CharStream
https://github.com/laxcat/CharStream/
For documentation refer to README.md in this directory or https://github.com/laxcat/CharStream/blob/master/README.md

*/

#pragma once
#include "stb_sprintf.h"
#include <stdint.h>


// WINDOWS (untested)
#ifdef _WIN32
    #include <io.h>
    #define stdwrite(TARGET, BUF, COUNT) ::_write(TARGET, BUF, COUNT)
// MAC-OS (and other *nix platforms, untested)
#else
    #include <unistd.h>
    #define stdwrite(TARGET, BUF, COUNT) ::write(TARGET, BUF, COUNT)

    #ifndef stdin
    #define stdin STDIN_FILENO
    #endif

    #ifndef stderr
    #define stderr STDERR_FILENO
    #endif

    #ifndef stdout
    #define stdout STDOUT_FILENO
    #endif

#endif


class CharStream {
// Private convenience declarations
private:

    using FI = uint8_t;
    using this_t = CharStream;

// Public declarations
public:

    static constexpr char const * StringTrue = "true";
    static constexpr char const * StringFalse = "false";
    static constexpr char const * StringFmtFloat = "f";
    static constexpr char const * StringFmtString = "s";
    static constexpr char const * StringFmtChar = "c";
    static constexpr char const * StringFmtUint = "d";
    static constexpr char const * StringFmtUintLong = "ld";
    static constexpr char const * StringFmtInt = "d";
    static constexpr char const * StringFmtIntLong = "ld";

    static constexpr FI FormatBufferSize = 255;

    union Target {
        char * str;
        size_t value;
        Target(size_t value) : value(value) {}
        Target(char * str) : str(str) {}
        template <typename T> Target(T * value) : value((size_t)((void *)value)) {}
        template <size_t N> Target(char (&str)[N]) : str((char *)str) {}
    };


// Instance API
public:

    // Constructor
    CharStream(Target target = stdout, char const * sep = " ", char const * trm = "\n") : 
        _target(target), 
        _sep(sep), 
        _trm(trm) {}

    // Callop ()
    int operator () () {
        return buffsprintf("%s", _trm);
    }
    template <typename ... TS>
    int operator () (TS && ... params) {
        writeFormat(_sep, _trm, sizeof...(params), static_cast<TS &&>(params)...);
        return buffsprintf(_formatBuff, coerceToExpectedParam(static_cast<TS &&>(params))...);
    }

    // Format
    template <typename ... TS>
    int format(char const * fmt, TS && ... params) {
        return buffsprintf(fmt, coerceToExpectedParam(static_cast<TS &&>(params))...);
    }

    // Write
    template <typename ... TS>
    int write(char const * sep, TS && ... params) {
        writeFormat(sep, "", sizeof...(params) - 1, static_cast<TS &&>(params)...);
        return buffsprintf(_formatBuff, coerceToExpectedParam(static_cast<TS &&>(params))...);
    }

// Private utilities
private:

    template <typename ... TS>
    void writeFormat(char const * sep, char const * trm, FI paramCount, TS && ... params) {
        FI fbuffIndex = 0;
        FI paramIndex = 0;
        (writeFormatItem(sep, trm, fbuffIndex, paramIndex, paramCount, params), ...);
    }

    template <typename TS>
    void writeFormatItem(
        char const * sep, 
        char const * trm, 
        FI & fbuffIndex,
        FI & paramIndex,
        FI paramCount,
        TS && param) {

        // write the format string
        fbuffIndex += scpy(_formatBuff + fbuffIndex, "%", 1);
        fbuffIndex += scpy(_formatBuff + fbuffIndex, charForType(param));

        // seperator or terminator string?
        // can also be blank if past defined parameter count, which
        // can be artificially constrained
        char const * sepOrTrm = 
            (paramIndex <  paramCount-1) ? sep :
            (paramIndex == paramCount-1) ? trm :
            "";

        // write a spacer or terminus string
        fbuffIndex += scpy(_formatBuff + fbuffIndex, sepOrTrm);

        ++paramIndex;
    }

    #define EXPECTED_TYPE(EXPECTED_TYPE, VALUE, RETURN_VALUE, FORMAT_CHAR) \
    auto coerceToExpectedParam(EXPECTED_TYPE const & VALUE) { return RETURN_VALUE; } \
    char const * charForType(EXPECTED_TYPE) { return FORMAT_CHAR; }
    //
    EXPECTED_TYPE(             float, t,                            t, StringFmtFloat)
    EXPECTED_TYPE(            double, t,                            t, StringFmtFloat)
    EXPECTED_TYPE(              bool, t, t ? StringTrue : StringFalse, StringFmtString)
    EXPECTED_TYPE(              char, t,                            t, StringFmtChar)
    EXPECTED_TYPE(           uint8_t, t,                            t, StringFmtUint)
    EXPECTED_TYPE(          uint16_t, t,                            t, StringFmtUint)
    EXPECTED_TYPE(          uint32_t, t,                            t, StringFmtUint)
    EXPECTED_TYPE(     unsigned long, t,                            t, StringFmtUintLong)
    EXPECTED_TYPE(          uint64_t, t,                            t, StringFmtUintLong)
    EXPECTED_TYPE(            int8_t, t,                            t, StringFmtInt)
    EXPECTED_TYPE(           int16_t, t,                            t, StringFmtInt)
    EXPECTED_TYPE(           int32_t, t,                            t, StringFmtInt)
    EXPECTED_TYPE(              long, t,                            t, StringFmtIntLong)
    EXPECTED_TYPE(           int64_t, t,                            t, StringFmtIntLong)
    EXPECTED_TYPE(      char const *, t,                            t, StringFmtString)

    int buffsprintf(char const *fmt, ...) {
        _callbackBuffIndex = 0;
        va_list va;
        va_start(va, fmt);
        int ret = stbsp_vsprintfcb(callback, (void *)this, _buff, fmt, va);
        va_end(va);
        callback("\0", (void *)this, 1);
        return ret;
    }

    bool isTargetStd() const {
        size_t t = _target.value;
        return (
            t == stdout ||
            t == stdin  ||
            t == stderr
        );
    }

    char * callbackTargetStr() const  {
        return _target.str + _callbackBuffIndex;
    }

// Private static utilities
private:

    static size_t scpy(char * dst, char const * src, size_t max = -1) {
        size_t i = 0;
        for(; i < max; ++i) {
            dst[i] = src[i];
            if (dst[i] == '\0') break;
        }
        return i;
    }

    static char * callback(char const * buf, void * user, int len) {
        this_t & cs = *(reinterpret_cast<this_t *>(user));
        int i = 0;
        if (cs.isTargetStd()) {
            i = stdwrite(cs._target.value, buf, len);
        }
        else {
            i = scpy(cs.callbackTargetStr(), buf, len);
            // write null byte, but don't include it in new index
            scpy(cs.callbackTargetStr() + i, "\0", 1);
        }
        cs._callbackBuffIndex += i;
        return cs._buff;
    }

// Instance storage
private:
    Target _target;
    char const * _sep;
    char const * _trm;
    size_t _callbackBuffIndex;
    char _buff[STB_SPRINTF_MIN];
    char _formatBuff[FormatBufferSize];

};


#ifdef CHAR_STREAM_ENABLE_OPERATOR_MACRO

template <typename ST, ST N>
class CharLoop {
public:
    class Str {
    private:
        Str(CharLoop * parent, ST index, ST size) : parent(parent), index(index), size(size) {}
        CharLoop * parent;
        ST index;
        ST size;
    public:
        char const * ptr () const { return (char const *)(parent->_buff + index); }
        char * ptr () { return (char *)(parent->_buff + index); }
        operator char const * () const { return ptr(); }
        operator char * () { return ptr(); }
        friend class CharLoop;
    };

    Str claim(ST size) {
        if (_next + size > N) _next = 0;
        _buff[_next + size - 1] = '\0';
        auto ret = Str{this, _next, size};
        _next += size;
        return ret;
    }
private:
    char _buff[N] = {'\0'};
    ST _next = 0;
};

#define CHAR_STREAM_OPERATOR(SIZE, COUNT, FORMAT, ...) operator char const * () { \
    static CharLoop<uint16_t, SIZE * COUNT> buff; \
    auto ret = buff.claim(SIZE); \
    stbsp_sprintf(ret, FORMAT, __VA_ARGS__); \
    return ret.ptr(); \
}

#endif