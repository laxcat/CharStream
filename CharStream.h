/*

CharStream
https://github.com/laxcat/CharStream/
For documentation refer to README.md in this directory or https://github.com/laxcat/CharStream/blob/master/README.md

*/

#pragma once
#include "stb_sprintf.h"
#include <stdint.h>


#ifdef _WIN32
// WINDOWS (untested)
#include <io.h>
#define stdwrite(TARGET, BUF, COUNT) ::_write(TARGET, BUF, COUNT)
#else
// MAC-OS (and other *nix platforms, untested)
#include <unistd.h>
#define stdwrite(TARGET, BUF, COUNT) ::write(TARGET, BUF, COUNT)
#define stdin STDIN_FILENO
#define stderr STDERR_FILENO
#define stdout STDOUT_FILENO
#endif


class CharStream {
// Private convenience declarations
private:

    using FI = uint8_t;
    using this_t = CharStream;

// Public declarations
public:

    static constexpr FI FORMAT_BUFFER_SIZE = 255;

    union Target {
        char * str;
        size_t value;
        Target(size_t value) : value(value) {}
        Target(char * str) : str(str) {}
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
    template <typename ... TS>
    int operator () (TS && ... params) {
        writeFormat(_sep, _trm, static_cast<TS &&>(params)...);
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
        writeFormat(sep, "", static_cast<TS &&>(params)...);
        return buffsprintf(_formatBuff, coerceToExpectedParam(static_cast<TS &&>(params))...);
    }

// Private utilities
private:

           float coerceToExpectedParam(       float t) { return t; }
          double coerceToExpectedParam(      double t) { return t; }
         uint8_t coerceToExpectedParam(     uint8_t t) { return t; }
        uint16_t coerceToExpectedParam(    uint16_t t) { return t; }
        uint32_t coerceToExpectedParam(    uint32_t t) { return t; }
        uint64_t coerceToExpectedParam(    uint64_t t) { return t; }
          int8_t coerceToExpectedParam(      int8_t t) { return t; }
         int16_t coerceToExpectedParam(     int16_t t) { return t; }
         int32_t coerceToExpectedParam(     int32_t t) { return t; }
         int64_t coerceToExpectedParam(     int64_t t) { return t; }
    char const * coerceToExpectedParam(char const * t) { return t; }

    template <typename ... TS>
    void writeFormat(char const * sep, char const * trm, TS && ... params) {
        // i = char index, j = param index, t = param count
        FI i = 0, j = 0;
        static constexpr FI t = sizeof...(params);

        // call the fn for each param:
        //  format string buffer index "i" gets modified for each call.
        //  sep or trm is sent based on param-index "j", incremented each call.
        (writeFormatItem(params, i, (++j < t) ? sep : trm), ...);

        _formatBuff[i] = '\0';


        // printf("?%s?%d", _formatBuff, (int)strlen(_formatBuff));
    }

    template <typename TS>
    void writeFormatItem(TS && param, FI & i, char const * sepOrTrm) {
        // write the format string
        _formatBuff[i + 0] =  '%';
        _formatBuff[i + 1] =  charForType(param);
        i += 2;
        // write a spacer or terminus string
        i += scpy(_formatBuff + i, sepOrTrm);
    }
    char charForType(       float) { return 'f'; }
    char charForType(      double) { return 'f'; }
    char charForType(     uint8_t) { return 'd'; }
    char charForType(    uint16_t) { return 'd'; }
    char charForType(    uint32_t) { return 'd'; }
    char charForType(    uint64_t) { return 'd'; }
    char charForType(      int8_t) { return 'd'; }
    char charForType(     int16_t) { return 'd'; }
    char charForType(     int32_t) { return 'd'; }
    char charForType(     int64_t) { return 'd'; }
    char charForType(char const *) { return 's'; }

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
    char _formatBuff[FORMAT_BUFFER_SIZE];

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