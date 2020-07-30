/*

CharStream
https://github.com/laxcat/CharStream/
For documentation refer to README.md in this directory or https://github.com/laxcat/CharStream/blob/master/README.md

*/


#pragma once
#include <stdint.h>

#ifndef CHAR_STREAM_SPRINTF
#define CHAR_STREAM_SPRINTF sprintf
#endif

#ifndef CHAR_STREAM_BUFFER_SIZE
#define CHAR_STREAM_BUFFER_SIZE 512
#endif

#ifndef CHAR_STREAM_FORMAT_BUFFER_SIZE
#define CHAR_STREAM_FORMAT_BUFFER_SIZE 128
#endif

#ifndef CHAR_STREAM_FORMAT_INDEX_TYPE
#define CHAR_STREAM_FORMAT_INDEX_TYPE uint8_t
#endif

// WINDOWS (untested)
#ifndef CHAR_STREAM_DISABLE_SYS_INCLUDE
    #ifdef _WIN32
        #include <io.h>
        #define CHAR_STREAM_SYSWRITE(DST, SRC, LEN) ::_write(DST, SRC, LEN)

    // MAC-OS (and other *nix platforms, untested)
    #else
        #include <unistd.h>
        #define CHAR_STREAM_SYSWRITE(DST, SRC, LEN) ::write(DST, SRC, LEN)

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

#else
    #define stdin 0
    #define stdout 1
    #define stderr 2
#endif


class CharStream {
// Public declarations
public:

    static constexpr char const * StringTrue            = "true";
    static constexpr char const * StringFalse           = "false";
    static constexpr char const * StringFmtFloat        = "f";
    static constexpr char const * StringFmtString       = "s";
    static constexpr char const * StringFmtChar         = "c";
    static constexpr char const * StringFmtUint         = "d";
    static constexpr char const * StringFmtUintLong     = "ld";
    static constexpr char const * StringFmtInt          = "d";
    static constexpr char const * StringFmtIntLong      = "ld";

    union Target {
        char * str;
        void * ptr;
        size_t value;
        Target(size_t value) : value(value) {}
        Target(char * str) : str(str) {}
        template <size_t N> Target(char (&str)[N]) : str((char *)str) {}
        template <typename T> Target(T * value) : ptr((void *)value) {}
        friend bool operator ==(Target const & a, size_t b) { return a.value == b; }
    };


// Instance API
public:

    // Constructor
    CharStream(Target target = stdout, char const * sep = " ", char const * trm = "\n") : 
        _target(target), 
        _targetIsSTD(_target == stdin || _target == stderr || _target == stdout), 
        _sep(sep), 
        _trm(trm) {}

    // Callop ()
    int operator () () {
        return targetSprintf("%s", _trm);
    }
    template <typename ... TS>
    int operator () (TS && ... params) {
        writeFormat(_sep, _trm, sizeof...(params), static_cast<TS &&>(params)...);
        return targetSprintf(_formatBuff, static_cast<TS &&>(params)...);
    }

    // Format
    template <typename ... TS>
    int format(char const * fmt, TS && ... params) {
        return targetSprintf(fmt, static_cast<TS &&>(params)...);
    }

    // Write
    template <typename ... TS>
    int write(char const * sep, TS && ... params) {
        writeFormat(sep, "", sizeof...(params) - 1, static_cast<TS &&>(params)...);
        return targetSprintf(_formatBuff, static_cast<TS &&>(params)...);
    }

// Private utilities
private:

    template <typename ... TS>
    void writeFormat(
        char const * sep, 
        char const * trm, 
        CHAR_STREAM_FORMAT_INDEX_TYPE paramCount, 
        TS && ... params) {

        CHAR_STREAM_FORMAT_INDEX_TYPE fbuffIndex = 0;
        CHAR_STREAM_FORMAT_INDEX_TYPE paramIndex = 0;
        (writeFormatItem(sep, trm, fbuffIndex, paramIndex, paramCount, params), ...);
    }

    template <typename TS>
    void writeFormatItem(
        char const * sep, 
        char const * trm, 
        CHAR_STREAM_FORMAT_INDEX_TYPE & fbuffIndex,
        CHAR_STREAM_FORMAT_INDEX_TYPE & paramIndex,
        CHAR_STREAM_FORMAT_INDEX_TYPE paramCount,
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
    auto coerceToExpectedParam(EXPECTED_TYPE const & VALUE) -> decltype(RETURN_VALUE) { return RETURN_VALUE; } \
    char const * charForType(EXPECTED_TYPE) { return FORMAT_CHAR; }
    //
    EXPECTED_TYPE(             float, t,                            t, StringFmtFloat)
    EXPECTED_TYPE(            double, t,                            t, StringFmtFloat)
    EXPECTED_TYPE(              bool, t, t ? StringTrue : StringFalse, StringFmtString)
    EXPECTED_TYPE(              char, t,                            t, StringFmtChar)
    EXPECTED_TYPE(           uint8_t, t,                            t, StringFmtUint)
    EXPECTED_TYPE(          uint16_t, t,                            t, StringFmtUint)
    EXPECTED_TYPE(          uint32_t, t,                            t, StringFmtUint)
    #ifndef CHAR_STREAM_DISABLE_LONG
    EXPECTED_TYPE(     unsigned long, t,                            t, StringFmtUintLong)
    #endif
    EXPECTED_TYPE(          uint64_t, t,                            t, StringFmtUintLong)
    EXPECTED_TYPE(            int8_t, t,                            t, StringFmtInt)
    EXPECTED_TYPE(           int16_t, t,                            t, StringFmtInt)
    EXPECTED_TYPE(           int32_t, t,                            t, StringFmtInt)
    #ifndef CHAR_STREAM_DISABLE_LONG
    EXPECTED_TYPE(              long, t,                            t, StringFmtIntLong)
    #endif
    EXPECTED_TYPE(           int64_t, t,                            t, StringFmtIntLong)
    EXPECTED_TYPE(      char const *, t,                            t, StringFmtString)

    template <typename ... TS>
    int targetSprintf(char const *fmt, TS && ... params) {
        int ret;
        if (_targetIsSTD) {
            #ifdef CHAR_STREAM_SYSWRITE
            ret = CHAR_STREAM_SPRINTF(_buff, fmt, coerceToExpectedParam(static_cast<TS &&>(params))...);
            CHAR_STREAM_SYSWRITE(_target.value, _buff, ret);
            #endif
        }
        else {
            ret = CHAR_STREAM_SPRINTF(_target.str, fmt, coerceToExpectedParam(static_cast<TS &&>(params))...);
        }
        return ret;
    }

// Private static utilities
private:

    static CHAR_STREAM_FORMAT_INDEX_TYPE scpy(
        char * dst, 
        char const * src, 
        CHAR_STREAM_FORMAT_INDEX_TYPE max = (CHAR_STREAM_FORMAT_INDEX_TYPE)-1) {
        
        CHAR_STREAM_FORMAT_INDEX_TYPE i = 0;
        for(; i < max; ++i) {
            dst[i] = src[i];
            if (dst[i] == '\0') break;
        }
        return i;
    }

// Instance storage
private:
    Target _target;
    bool _targetIsSTD;
    char const * _sep;
    char const * _trm;
    char _buff[CHAR_STREAM_BUFFER_SIZE];
    char _formatBuff[CHAR_STREAM_FORMAT_BUFFER_SIZE];

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
    CHAR_STREAM_SPRINTF(ret, FORMAT, __VA_ARGS__); \
    return ret.ptr(); \
}

#endif
