/*
C89 compatible string library. Choice of public domain or MIT-0. See license statements at the end of this file.
*/

/*
This is a single file library. To define the implementation, do this in one source file:

    ```c
    #define C89STR_IMPLEMENTATION
    #include "c89str.h"
    ```

Then just include this file like any other header.

This library uses `errno_t` for result codes. It will never explicitly set the global `errno`, however calls
to standard library functions internally may update it. You should inspect the return value of the relevant
functions for determining the error rather than relying on the global `errno` variable. Compare the result
with C89STR_SUCCESS to check if the operation was successful.

This library includes an implementation of sprintf() based on stb_sprintf. A tool is used to amalgamate the
code from stb_sprintf and format it so it integrates cleanly with the namespacing and style of c89str:

    c89str_vsprintf
    c89str_vsnprintf
    c89str_sprintf
    c89str_snprintf

This is useful for when you need a consistent implementation of sprintf() without having to worry about
compatibility between different compilers.
*/

#ifndef c89str_h
#define c89str_h

#include <stddef.h> /* For NULL, size_t */
#include <stdarg.h> /* For va_list */
#include <errno.h>  /* For errno_t */

#if !defined(_WIN32)
typedef int errno_t;
#endif

#ifndef C89STR_API
#ifdef __cplusplus
#define C89STR_API extern "C"
#else
#define C89STR_API extern
#endif
#endif

#ifdef _MSC_VER
    #define C89STR_INLINE __forceinline
#elif defined(__GNUC__)
    /*
    I've had a bug report where GCC is emitting warnings about functions possibly not being inlineable. This warning happens when
    the __attribute__((always_inline)) attribute is defined without an "inline" statement. I think therefore there must be some
    case where "__inline__" is not always defined, thus the compiler emitting these warnings. When using -std=c89 or -ansi on the
    command line, we cannot use the "inline" keyword and instead need to use "__inline__". In an attempt to work around this issue
    I am using "__inline__" only when we're compiling in strict ANSI mode.
    */
    #if defined(__STRICT_ANSI__)
        #define C89STR_GNUC_INLINE_HINT __inline__
    #else
        #define C89STR_GNUC_INLINE_HINT inline
    #endif

    #if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 2)) || defined(__clang__)
        #define C89STR_INLINE C89STR_GNUC_INLINE_HINT __attribute__((always_inline))
    #else
        #define C89STR_INLINE C89STR_GNUC_INLINE_HINT
    #endif
#elif defined(__WATCOMC__)
    #define C89STR_INLINE __inline
#else
    #define C89STR_INLINE
#endif


#if defined(__has_attribute)
    #if __has_attribute(format)
        #define C89STR_ATTRIBUTE_FORMAT(fmt, va) __attribute__((format(printf, fmt, va)))
    #endif
#endif
#ifndef C89STR_ATTRIBUTE_FORMAT
#define C89STR_ATTRIBUTE_FORMAT(fmt, va)
#endif

typedef   signed char           c89str_int8;
typedef unsigned char           c89str_uint8;
typedef   signed short          c89str_int16;
typedef unsigned short          c89str_uint16;
typedef   signed int            c89str_int32;
typedef unsigned int            c89str_uint32;
#if defined(_MSC_VER) && !defined(__clang__)
    typedef   signed __int64    c89str_int64;
    typedef unsigned __int64    c89str_uint64;
#else
    #if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wlong-long"
        #if defined(__clang__)
            #pragma GCC diagnostic ignored "-Wc++11-long-long"
        #endif
    #endif
    typedef   signed long long  c89str_int64;
    typedef unsigned long long  c89str_uint64;
    #if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
        #pragma GCC diagnostic pop
    #endif
#endif
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || defined(_M_IA64) || defined(__aarch64__) || defined(_M_ARM64) || defined(__powerpc64__)
    typedef c89str_uint64       c89str_uintptr;
#else
    typedef c89str_uint32       c89str_uintptr;
#endif

typedef unsigned int            c89str_bool32;
#define C89STR_TRUE             1
#define C89STR_FALSE            0

typedef char                    c89str_utf8;
typedef c89str_uint16           c89str_utf16;
typedef c89str_uint32           c89str_utf32;

#define c89str_npos  ((size_t)-1)

/*
Custom error codes. We use errno_t types and are using negative numbers, starting from bit 14 to
try our best to avoid clashing. However, since we do not modify the global errno value and aren't
calling any system calls, it shouldn't actually matter if something else coincidentally uses the
same code. So long as we don't clash with any of the common ones, which should be unlikely as they
all seems to be positive in all of the implementations I've seen, we should be OK.
*/
#define C89STR_SUCCESS    0      /* No error. */
#define C89STR_END        1      /* Reached the end of something. Not an error. */
#define C89STR_EBOM       -16384 /* Invalid BOM */
#define C89STR_ECODEPOINT -16385 /* Invalid code point. */

typedef struct
{
    void* pUserData;
    void* (* onMalloc)(size_t sz, void* pUserData);
    void* (* onRealloc)(void* p, size_t sz, void* pUserData);
    void  (* onFree)(void* p, void* pUserData);
} c89str_allocation_callbacks;

C89STR_API void* c89str_malloc(size_t sz, const c89str_allocation_callbacks* pAllocationCallbacks);
C89STR_API void* c89str_realloc(void* p, size_t sz, const c89str_allocation_callbacks* pAllocationCallbacks);
C89STR_API void  c89str_free(void* p, const c89str_allocation_callbacks* pAllocationCallbacks);


/* Standard Library Alternatives */
/* BEG c89str_stdlib.h */
C89STR_API size_t c89str_strlen(const char* src);
C89STR_API char* c89str_strcpy(char* dst, const char* src);
C89STR_API int c89str_strncpy(char* dst, const char* src, size_t count);
C89STR_API int c89str_strcpy_s(char* dst, size_t dstCap, const char* src);
C89STR_API int c89str_strncpy_s(char* dst, size_t dstCap, const char* src, size_t count);
C89STR_API int c89str_strcat_s(char* dst, size_t dstCap, const char* src);
C89STR_API int c89str_strncat_s(char* dst, size_t dstCap, const char* src, size_t count);
C89STR_API int c89str_itoa_s(int value, char* dst, size_t dstCap, int radix);
C89STR_API int c89str_strcmp(const char* str1, const char* str2);
C89STR_API int c89str_strncmp(const char* str1, const char* str2, size_t maxLen);
C89STR_API int c89str_stricmp(const char* str1, const char* str2);
C89STR_API int c89str_strnicmp(const char* str1, const char* str2, size_t count);
/* END c89str_stdlib.h */

/* Miscellaneous Helpers */
/* BEG c89str_helpers.h */
#define c89str_is_null_or_empty(str) ((str) == NULL || (str)[0] == 0)
C89STR_API c89str_bool32 c89str_is_null_or_whitespace(const char* str, size_t strLen);
C89STR_API errno_t c89str_findn(const char* str, size_t strLen, const char* other, size_t otherLen, size_t* pResult);
C89STR_API errno_t c89str_find(const char* str, const char* other, size_t* pResult);  /* Returns NOENT if the string cannot be found, and sets pResult to c89str_npos. */
C89STR_API int c89str_strncmpn(const char* str1, size_t str1Len, const char* str2, size_t str2Len);
C89STR_API c89str_bool32 c89str_begins_with(const char* str1, size_t str1Len, const char* str2, size_t str2Len); /* Returns 0 if str1 begins with str2. */
C89STR_API c89str_bool32 c89str_ends_with(const char* str1, size_t str1Len, const char* str2, size_t str2Len); /* Returns 0 if str1 ends with str2. */
C89STR_API errno_t c89str_to_uint(const char* str, size_t strLen, unsigned int* pValue);
C89STR_API errno_t c89str_to_int(const char* str, size_t strLen, int* pValue);
C89STR_API errno_t c89str_ascii_tolower(char* dst, size_t dstCap, const char* src, size_t srcLen);
C89STR_API errno_t c89str_ascii_toupper(char* dst, size_t dstCap, const char* src, size_t srcLen);
C89STR_API c89str_bool32 c89str_is_all_digits(const char* str, size_t strLen);
/* END c89str_helpers.h */


/* Unicode API */
#define C89STR_UNICODE_MIN_CODE_POINT                        0x000000
#define C89STR_UNICODE_MAX_CODE_POINT                        0x10FFFF
#define C89STR_UNICODE_REPLACEMENT_CODE_POINT                0x00FFFD
#define C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF8    3
#define C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16   1
#define C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF32   1

#define C89STR_FORBID_BOM                                    (1 << 1)
#define C89STR_ERROR_ON_INVALID_CODE_POINT                   (1 << 2)

C89STR_API c89str_bool32 c89str_utf16_is_bom_le(const unsigned char bom[2]);
C89STR_API c89str_bool32 c89str_utf16_is_bom_be(const unsigned char bom[2]);
C89STR_API c89str_bool32 c89str_utf32_is_bom_le(const unsigned char bom[4]);
C89STR_API c89str_bool32 c89str_utf32_is_bom_be(const unsigned char bom[4]);

C89STR_API c89str_bool32 c89str_utf8_has_bom(const unsigned char* pBytes, size_t len);
C89STR_API c89str_bool32 c89str_utf16_has_bom(const unsigned char* pBytes, size_t len);
C89STR_API c89str_bool32 c89str_utf32_has_bom(const unsigned char* pBytes, size_t len);

C89STR_API void c89str_utf16_swap_endian(c89str_utf16* pUTF16, size_t count);
C89STR_API void c89str_utf32_swap_endian(c89str_utf32* pUTF32, size_t count);


/* UTF-8 */
C89STR_API errno_t c89str_utf8_to_utf16_len(size_t* pUTF16Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags);
static C89STR_INLINE errno_t c89str_utf8_to_utf16ne_len(size_t* pUTF16Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags) { return c89str_utf8_to_utf16_len(pUTF16Len, pUTF8, utf8Len, pUTF8LenProcessed, flags); }
static C89STR_INLINE errno_t c89str_utf8_to_utf16le_len(size_t* pUTF16Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags) { return c89str_utf8_to_utf16_len(pUTF16Len, pUTF8, utf8Len, pUTF8LenProcessed, flags); }
static C89STR_INLINE errno_t c89str_utf8_to_utf16be_len(size_t* pUTF16Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags) { return c89str_utf8_to_utf16_len(pUTF16Len, pUTF8, utf8Len, pUTF8LenProcessed, flags); }

C89STR_API errno_t c89str_utf8_to_utf16ne(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf8_to_utf16le(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf8_to_utf16be(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags);
static C89STR_INLINE errno_t c89str_utf8_to_utf16(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags) { return c89str_utf8_to_utf16ne(pUTF16, utf16Cap, pUTF16Len, pUTF8, utf8Len, pUTF8LenProcessed, flags); }

C89STR_API errno_t c89str_utf8_to_utf32_len(size_t* pUTF32Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags);
static C89STR_INLINE errno_t c89str_utf8_to_utf32ne_len(size_t* pUTF32Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags) { return c89str_utf8_to_utf32_len(pUTF32Len, pUTF8, utf8Len, pUTF8LenProcessed, flags); }
static C89STR_INLINE errno_t c89str_utf8_to_utf32le_len(size_t* pUTF32Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags) { return c89str_utf8_to_utf32_len(pUTF32Len, pUTF8, utf8Len, pUTF8LenProcessed, flags); }
static C89STR_INLINE errno_t c89str_utf8_to_utf32be_len(size_t* pUTF32Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags) { return c89str_utf8_to_utf32_len(pUTF32Len, pUTF8, utf8Len, pUTF8LenProcessed, flags); }

C89STR_API errno_t c89str_utf8_to_utf32ne(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf8_to_utf32le(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf8_to_utf32be(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags);
static C89STR_INLINE errno_t c89str_utf8_to_utf32(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags) { return c89str_utf8_to_utf32ne(pUTF32, utf32Cap, pUTF32Len, pUTF8, utf8Len, pUTF8LenProcessed, flags); }

C89STR_API errno_t c89str_utf8_to_wchar_len(size_t* pWCHARLen, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf8_to_wchar(wchar_t* pWCHAR, size_t wcharCap, size_t* pWCHARLen, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags);


/* UTF-16 */
C89STR_API errno_t c89str_utf16ne_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16le_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16be_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);

C89STR_API errno_t c89str_utf16ne_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16le_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16be_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);

C89STR_API errno_t c89str_utf16ne_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16le_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16be_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
static C89STR_INLINE errno_t c89str_utf16ne_to_utf32ne_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags) { return c89str_utf16ne_to_utf32_len(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags); }
static C89STR_INLINE errno_t c89str_utf16le_to_utf32le_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags) { return c89str_utf16le_to_utf32_len(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags); }
static C89STR_INLINE errno_t c89str_utf16be_to_utf32be_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags) { return c89str_utf16be_to_utf32_len(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags); }
C89STR_API errno_t c89str_utf16_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);

C89STR_API errno_t c89str_utf16ne_to_utf32ne(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16le_to_utf32le(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16be_to_utf32be(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16_to_utf32(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);


/* UTF-32 */
C89STR_API errno_t c89str_utf32ne_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32le_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32be_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);

C89STR_API errno_t c89str_utf32ne_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32le_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32be_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);

C89STR_API errno_t c89str_utf32ne_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32le_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32be_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
static C89STR_INLINE errno_t c89str_utf32ne_to_utf16ne_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags) { return c89str_utf32ne_to_utf16_len(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags); }
static C89STR_INLINE errno_t c89str_utf32le_to_utf16le_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags) { return c89str_utf32le_to_utf16_len(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags); }
static C89STR_INLINE errno_t c89str_utf32be_to_utf16be_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags) { return c89str_utf32be_to_utf16_len(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags); }
C89STR_API errno_t c89str_utf32_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);

C89STR_API errno_t c89str_utf32ne_to_utf16ne(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32le_to_utf16le(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32be_to_utf16be(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32_to_utf16(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);


/* UTF-32 */
C89STR_API c89str_bool32 c89str_utf32_is_null_or_whitespace(const c89str_utf32* pUTF32, size_t utf32Len);

/* UTF-16 */

/* UTF-8 */
C89STR_API c89str_bool32 c89str_utf8_is_null_or_whitespace(const c89str_utf8* pUTF8, size_t utf8Len);
C89STR_API size_t c89str_utf8_find_next_whitespace(const c89str_utf8* pUTF8, size_t utf8Len);
C89STR_API size_t c89str_utf8_ltrim_offset(const c89str_utf8* pUTF8, size_t utf8Len);
C89STR_API size_t c89str_utf8_rtrim_offset(const c89str_utf8* pUTF8, size_t utf8Len);
C89STR_API size_t c89str_utf8_find_next_line(const c89str_utf8* pUTF8, size_t utf8Len, size_t* pThisLineLen);


/* Dynamic String API */
typedef char* c89str;
C89STR_API void    c89str_delete(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks);
C89STR_API c89str  c89str_new_with_cap(const c89str_allocation_callbacks* pAllocationCallbacks, size_t capacityNotIncludingNullTerminator);
C89STR_API c89str  c89str_new(const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther);
C89STR_API c89str  c89str_newn(const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen);
C89STR_API c89str  c89str_newv(const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args);
C89STR_API c89str  c89str_newf(const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...) C89STR_ATTRIBUTE_FORMAT(2, 3);
C89STR_API c89str  c89str_set(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther);
C89STR_API c89str  c89str_setn(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen);
C89STR_API c89str  c89str_setv(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args);
C89STR_API c89str  c89str_setf(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...) C89STR_ATTRIBUTE_FORMAT(3, 4);
C89STR_API c89str  c89str_cat(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther);
C89STR_API c89str  c89str_catn(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen);
C89STR_API c89str  c89str_catv(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args);
C89STR_API c89str  c89str_catf(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...) C89STR_ATTRIBUTE_FORMAT(3, 4);
C89STR_API c89str  c89str_prepend(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther);
C89STR_API c89str  c89str_prependn(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen);
C89STR_API c89str  c89str_prependv(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args);
C89STR_API c89str  c89str_prependf(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...) C89STR_ATTRIBUTE_FORMAT(3, 4);
C89STR_API c89str  c89str_remove(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, size_t beg, size_t end);
C89STR_API c89str  c89str_replace(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, size_t replaceOffset, size_t replaceLength, const char* pOther, size_t otherLength);
C89STR_API c89str  c89str_replace_all(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pQuery, size_t queryLen, const char* pReplacement, size_t replacementLen);
C89STR_API c89str  c89str_trim(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks);
C89STR_API void    c89str_set_len(c89str str, size_t len);  /* Do not call this manually unless you're manually changing the content of the string. */
C89STR_API size_t  c89str_len(const c89str str);
C89STR_API size_t  c89str_cap(const c89str str);
C89STR_API errno_t c89str_result(const c89str str);


/* BEG c89str_lexer.h */
/* For single characters, the UTF-32 code point will be the token. Otherwise it will be an entry in this enum. */
typedef enum
{
    c89str_token_type_eof = (C89STR_UNICODE_MAX_CODE_POINT + 1),
    c89str_token_type_error,
    c89str_token_type_whitespace,
    c89str_token_type_newline,                /* New lines are separate from whitespace as they are often treated differently by parsers. */
    c89str_token_type_comment,
    c89str_token_type_identifier,
    c89str_token_type_string_double,          /* "Example" */
    c89str_token_type_string_single,          /* '1234' */
    c89str_token_type_integer_literal_dec,    /* e.g. 1234   */
    c89str_token_type_integer_literal_hex,    /* e.g. 0x12AB */
    c89str_token_type_integer_literal_oct,    /* e.g. 01234  */
    c89str_token_type_integer_literal_bin,    /* e.g. 0b1010 */
    c89str_token_type_float_literal_dec,      /* e.g. 1.2345, 1.2345f, 1.2345d */
    c89str_token_type_float_literal_hex,      /* e.g. 0x1.12ABp1 (exponent is mandatory). */
    c89str_token_type_eqeq,                   /* == */
    c89str_token_type_noteq,                  /* != */
    c89str_token_type_lteq,                   /* <= */
    c89str_token_type_gteq,                   /* >= */
    c89str_token_type_andand,                 /* && */
    c89str_token_type_oror,                   /* || */
    c89str_token_type_plusplus,               /* ++ */
    c89str_token_type_minusminus,             /* -- */
    c89str_token_type_pluseq,                 /* += */
    c89str_token_type_minuseq,                /* -= */
    c89str_token_type_muleq,                  /* *= */
    c89str_token_type_diveq,                  /* /= */
    c89str_token_type_modeq,                  /* %= */
    c89str_token_type_shleq,                  /* <<= */
    c89str_token_type_shreq,                  /* >>= */
    c89str_token_type_shl,                    /* << */
    c89str_token_type_shr,                    /* >> */
    c89str_token_type_andeq,                  /* &= */
    c89str_token_type_oreq,                   /* |= */
    c89str_token_type_xoreq,                  /* ^= */
    c89str_token_type_coloncolon,             /* :: */
    c89str_token_type_ellipsis                /* ... */
} c89str_token_type;

typedef struct
{
    const char* pText;
    size_t textLen;     /* Set to (size_t)-1 for null terminated. */
    size_t textOff;     /* The cursor. */
    const char* pTokenStr;
    size_t tokenLen;
    c89str_utf32 token;
    size_t lineNumber;  /* One based line number. */
    struct
    {
        c89str_bool32 skipWhitespace;
        c89str_bool32 skipNewlines;
        c89str_bool32 skipComments;
        c89str_bool32 allowDashesInIdentifiers;
        const char* pLineCommentOpeningToken;
        const char* pBlockCommentOpeningToken;
        const char* pBlockCommentClosingToken;
    } options;
} c89str_lexer;

C89STR_API errno_t c89str_lexer_init(c89str_lexer* pLexer, const char* pText, size_t textLen);
C89STR_API errno_t c89str_lexer_next(c89str_lexer* pLexer);
/* END c89str_lexer.h */

C89STR_API errno_t c89str_lexer_transform_token(c89str_lexer* pLexer, c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks);



/*
Paths.

These functions are low-level functions for working with paths. The most important part of this API
is probably the iteration functions. These functions are used for iterating over each of the
segments of a path. This library will recognize both '\' and '/'. If you want to use just one or
the other, or a different separator, you'll need to use a different library. Likewise, this library
will treat paths as case sensitive. Again, you'll need to use a different library if this is not
suitable for you.
*/
typedef struct
{
    const char* pFullPath;
    size_t fullPathLength;
    size_t segmentOffset;
    size_t segmentLength;
} c89str_path_iterator;

C89STR_API errno_t c89str_path_first(const char* pPath, size_t pathLen, c89str_path_iterator* pIterator);
C89STR_API errno_t c89str_path_last(const char* pPath, size_t pathLen, c89str_path_iterator* pIterator);
C89STR_API errno_t c89str_path_next(c89str_path_iterator* pIterator);
C89STR_API errno_t c89str_path_prev(c89str_path_iterator* pIterator);
C89STR_API int c89str_path_iterators_compare(const c89str_path_iterator* pIteratorA, const c89str_path_iterator* pIteratorB);
C89STR_API const char* c89str_path_extension(const char* pPath, size_t pathLen);    /* Does *not* include the null terminator. Returns an offset of pPath. Will only be null terminated if pPath is. Returns null if the extension cannot be found. */
C89STR_API c89str_bool32 c89str_path_extension_equal(const char* pPath, size_t pathLen, const char* pExtension, size_t extensionLen); /* Returns true if the extension is equal to the given extension. */


/* sprintf() implementation via stb_sprintf(). The code between these tags is generated by a tool. Do not delete these tags. */
/* beg stb_sprintf.h */
typedef char* c89str_sprintf_callback(const char* buf, void* user, size_t len);

C89STR_API int c89str_vsprintf(char* buf, char const* fmt, va_list va);
C89STR_API int c89str_vsnprintf(char* buf, size_t count, char const* fmt, va_list va);
C89STR_API int c89str_sprintf(char* buf, char const* fmt, ...) C89STR_ATTRIBUTE_FORMAT(2, 3);
C89STR_API int c89str_snprintf(char* buf, size_t count, char const* fmt, ...) C89STR_ATTRIBUTE_FORMAT(3, 4);

C89STR_API int c89str_vsprintfcb(c89str_sprintf_callback* callback, void* user, char* buf, char const* fmt, va_list va);
C89STR_API void c89str_set_sprintf_separators(char comma, char period);
/* end stb_sprintf.h */


#endif  /* c89str_h */


#if defined(C89STR_IMPLEMENTATION)
#ifndef c89str_c
#define c89str_c

/* BEG c89str_fallthrough.h */
#if defined(__has_c_attribute) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202000L)
    #if __has_c_attribute(fallthrough)
        #define C89STR_FALLTHROUGH [[fallthrough]]
    #endif
#endif
#if !defined(C89STR_FALLTHROUGH) && defined(__has_attribute) && (defined(__clang__) || defined(__GNUC__))
    #if __has_attribute(fallthrough)
        #define C89STR_FALLTHROUGH __attribute__((fallthrough))
    #endif
#endif
#if !defined(C89STR_FALLTHROUGH)
    #define C89STR_FALLTHROUGH ((void)0)
#endif
/* END c89str_fallthrough.h */

#include <stdlib.h> /* malloc(), realloc(), free(). */
#include <string.h> /* For memcpy(). */
#include <assert.h> /* For assert(). */
#include <stdio.h>

#define C89STR_UNUSED(x) ((void)(x))

#ifndef C89STR_ASSERT
#define C89STR_ASSERT(condition)         assert(condition)
#endif

static void c89str_zero_memory_default(void* p, size_t sz)
{
    if (sz > 0) {
        memset(p, 0, sz);
    }
}

#ifndef C89STR_ZERO_MEMORY
#define C89STR_ZERO_MEMORY(p, sz)           c89str_zero_memory_default((p), (sz))
#endif

#ifndef C89STR_COPY_MEMORY
#define C89STR_COPY_MEMORY(dst, src, sz)    memcpy((dst), (src), (sz))
#endif

#ifndef C89STR_MOVE_MEMORY
#define C89STR_MOVE_MEMORY(dst, src, sz)    memmove((dst), (src), (sz))
#endif

#ifndef C89STR_MALLOC
#define C89STR_MALLOC(sz)                   malloc((sz))
#endif

#ifndef C89STR_REALLOC
#define C89STR_REALLOC(p, sz)               realloc((p), (sz))
#endif

#ifndef C89STR_FREE
#define C89STR_FREE(p)                      free((p))
#endif

#define C89STR_ZERO_OBJECT(p)               C89STR_ZERO_MEMORY((p), sizeof(*(p)))
#define C89STR_COUNTOF(x)                   (sizeof(x) / sizeof(x[0]))
#define C89STR_MAX(x, y)                    (((x) > (y)) ? (x) : (y))
#define C89STR_MIN(x, y)                    (((x) < (y)) ? (x) : (y))


static C89STR_INLINE c89str_bool32 c89str_is_little_endian(void)
{
#if defined(C89STR_X86) || defined(C89STR_X64)
    return C89STR_TRUE;
#elif defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && __BYTE_ORDER == __LITTLE_ENDIAN
    return C89STR_TRUE;
#else
    int n = 1;
    return (*(char*)&n) == 1;
#endif
}

static C89STR_INLINE c89str_bool32 c89str_is_big_endian(void)
{
    return !c89str_is_little_endian();
}

static C89STR_INLINE unsigned short c89str_swap_endian_uint16(unsigned short n)
{
#ifdef C89STR_HAS_BYTESWAP16_INTRINSIC
    #if defined(_MSC_VER)
        return _byteswap_ushort(n);
    #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap16(n);
    #else
        #error "This compiler does not support the byte swap intrinsic."
    #endif
#else
    return ((n & 0xFF00) >> 8) |
           ((n & 0x00FF) << 8);
#endif
}

static C89STR_INLINE unsigned int c89str_swap_endian_uint32(unsigned int n)
{
#ifdef C89STR_HAS_BYTESWAP32_INTRINSIC
    #if defined(_MSC_VER)
        return _byteswap_ulong(n);
    #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap32(n);
    #else
        #error "This compiler does not support the byte swap intrinsic."
    #endif
#else
    return ((n & 0xFF000000) >> 24) |
           ((n & 0x00FF0000) >>  8) |
           ((n & 0x0000FF00) <<  8) |
           ((n & 0x000000FF) << 24);
#endif
}


static C89STR_INLINE unsigned short c89str_be2host_16(unsigned short n)
{
    if (c89str_is_little_endian()) {
        return c89str_swap_endian_uint16(n);
    }

    return n;
}

static C89STR_INLINE unsigned int c89str_be2host_32(unsigned int n)
{
    if (c89str_is_little_endian()) {
        return c89str_swap_endian_uint32(n);
    }

    return n;
}

static C89STR_INLINE unsigned short c89str_le2host_16(unsigned short n)
{
    if (!c89str_is_little_endian()) {
        return c89str_swap_endian_uint16(n);
    }

    return n;
}

static C89STR_INLINE unsigned int c89str_le2host_32(unsigned int n)
{
    if (!c89str_is_little_endian()) {
        return c89str_swap_endian_uint32(n);
    }

    return n;
}


static C89STR_INLINE unsigned short c89str_host2be_16(unsigned short n)
{
    if (c89str_is_little_endian()) {
        return c89str_swap_endian_uint16(n);
    }

    return n;
}

static C89STR_INLINE unsigned int c89str_host2be_32(unsigned int n)
{
    if (c89str_is_little_endian()) {
        return c89str_swap_endian_uint32(n);
    }

    return n;
}

static C89STR_INLINE unsigned short c89str_host2le_16(unsigned short n)
{
    if (!c89str_is_little_endian()) {
        return c89str_swap_endian_uint16(n);
    }

    return n;
}

static C89STR_INLINE unsigned int c89str_host2le_32(unsigned int n)
{
    if (!c89str_is_little_endian()) {
        return c89str_swap_endian_uint32(n);
    }

    return n;
}



static void* c89str_malloc_default(size_t sz, void* pUserData)
{
    C89STR_UNUSED(pUserData);
    return C89STR_MALLOC(sz);
}

static void* c89str_realloc_default(void* p, size_t sz, void* pUserData)
{
    C89STR_UNUSED(pUserData);
    return C89STR_REALLOC(p, sz);
}

static void c89str_free_default(void* p, void* pUserData)
{
    C89STR_UNUSED(pUserData);
    C89STR_FREE(p);
}


C89STR_API void* c89str_malloc(size_t sz, const c89str_allocation_callbacks* pAllocationCallbacks)
{
    if (pAllocationCallbacks != NULL) {
        if (pAllocationCallbacks->onMalloc) {
            return pAllocationCallbacks->onMalloc(sz, pAllocationCallbacks->pUserData);
        } else {
            return NULL;    /* Do not fall back to default implementation. */
        }
    } else {
        return c89str_malloc_default(sz, NULL);
    }
}

C89STR_API void* c89str_realloc(void* p, size_t sz, const c89str_allocation_callbacks* pAllocationCallbacks)
{
    if (pAllocationCallbacks != NULL) {
        if (pAllocationCallbacks->onRealloc) {
            return pAllocationCallbacks->onRealloc(p, sz, pAllocationCallbacks->pUserData);
        } else {
            return NULL;    /* Do not fall back to default implementation. */
        }
    } else {
        return c89str_realloc_default(p, sz, NULL);
    }
}

C89STR_API void c89str_free(void* p, const c89str_allocation_callbacks* pAllocationCallbacks)
{
    if (pAllocationCallbacks != NULL) {
        if (pAllocationCallbacks->onFree) {
            pAllocationCallbacks->onFree(p, pAllocationCallbacks->pUserData);
        } else {
            return;         /* Do not fall back to default implementation. */
        }
    } else {
        c89str_free_default(p, NULL);
    }
}



/* BEG c89str_stdlib.c */
C89STR_API size_t c89str_strlen(const char* src)
{
    const char* end;

    C89STR_ASSERT(src != NULL);
    
    end = src;
    while (end[0] != '\0') {
        end += 1;
    }

    return end - src;
}

C89STR_API char* c89str_strcpy(char* dst, const char* src)
{
    char* dstorig;

    C89STR_ASSERT(dst != NULL);
    C89STR_ASSERT(src != NULL);

    dstorig = dst;

    /* No, we're not using this garbage: while (*dst++ = *src++); */
    for (;;) {
        *dst = *src;
        if (*src == '\0') {
            break;
        }

        dst += 1;
        src += 1;
    }

    return dstorig;
}

C89STR_API int c89str_strncpy(char* dst, const char* src, size_t count)
{
    size_t maxcount;
    size_t i;

    if (dst == 0) {
        return EINVAL;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    maxcount = count;

    for (i = 0; i < maxcount && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }

    if (src[i] == '\0' || i == count || count == ((size_t)-1)) {
        dst[i] = '\0';
        return 0;
    }

    dst[0] = '\0';
    return ERANGE;
}

C89STR_API int c89str_strcpy_s(char* dst, size_t dstCap, const char* src)
{
    size_t i;

    if (dst == 0) {
        return EINVAL;
    }
    if (dstCap == 0) {
        return ERANGE;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    for (i = 0; i < dstCap && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }

    if (i < dstCap) {
        dst[i] = '\0';
        return 0;
    }

    dst[0] = '\0';
    return ERANGE;
}

C89STR_API int c89str_strncpy_s(char* dst, size_t dstCap, const char* src, size_t count)
{
    size_t maxcount;
    size_t i;

    if (dst == 0) {
        return EINVAL;
    }
    if (dstCap == 0) {
        return EINVAL;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    maxcount = count;
    if (count == ((size_t)-1) || count >= dstCap) {        /* -1 = _TRUNCATE */
        maxcount = dstCap - 1;
    }

    for (i = 0; i < maxcount && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }

    if (src[i] == '\0' || i == count || count == ((size_t)-1)) {
        dst[i] = '\0';
        return 0;
    }

    dst[0] = '\0';
    return ERANGE;
}

C89STR_API int c89str_strcat_s(char* dst, size_t dstCap, const char* src)
{
    char* dstorig;

    if (dst == 0) {
        return EINVAL;
    }
    if (dstCap == 0) {
        return ERANGE;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    dstorig = dst;

    while (dstCap > 0 && dst[0] != '\0') {
        dst    += 1;
        dstCap -= 1;
    }

    if (dstCap == 0) {
        return EINVAL;  /* Unterminated. */
    }

    while (dstCap > 0 && src[0] != '\0') {
        *dst++ = *src++;
        dstCap -= 1;
    }

    if (dstCap > 0) {
        dst[0] = '\0';
    } else {
        dstorig[0] = '\0';
        return ERANGE;
    }

    return 0;
}

C89STR_API int c89str_strncat_s(char* dst, size_t dstCap, const char* src, size_t count)
{
    char* dstorig;

    if (dst == 0) {
        return EINVAL;
    }
    if (dstCap == 0) {
        return ERANGE;
    }
    if (src == 0) {
        return EINVAL;
    }

    dstorig = dst;

    while (dstCap > 0 && dst[0] != '\0') {
        dst    += 1;
        dstCap -= 1;
    }

    if (dstCap == 0) {
        return EINVAL;  /* Unterminated. */
    }

    if (count == ((size_t)-1)) {        /* _TRUNCATE */
        count = dstCap - 1;
    }

    while (dstCap > 0 && src[0] != '\0' && count > 0) {
        *dst++ = *src++;
        dstCap -= 1;
        count  -= 1;
    }

    if (dstCap > 0) {
        dst[0] = '\0';
    } else {
        dstorig[0] = '\0';
        return ERANGE;
    }

    return 0;
}

C89STR_API int c89str_itoa_s(int value, char* dst, size_t dstCap, int radix)
{
    int sign;
    unsigned int valueU;
    char* dstEnd;

    if (dst == NULL || dstCap == 0) {
        return EINVAL;
    }
    if (radix < 2 || radix > 36) {
        dst[0] = '\0';
        return EINVAL;
    }

    sign = (value < 0 && radix == 10) ? -1 : 1;     /* The negative sign is only used when the base is 10. */

    if (value < 0) {
        valueU = -value;
    } else {
        valueU = value;
    }

    dstEnd = dst;
    do
    {
        int remainder = valueU % radix;
        if (remainder > 9) {
            *dstEnd = (char)((remainder - 10) + 'a');
        } else {
            *dstEnd = (char)(remainder + '0');
        }

        dstEnd += 1;
        dstCap -= 1;
        valueU /= radix;
    } while (dstCap > 0 && valueU > 0);

    if (dstCap == 0) {
        dst[0] = '\0';
        return EINVAL;  /* Ran out of room in the output buffer. */
    }

    if (sign < 0) {
        *dstEnd++ = '-';
        dstCap -= 1;
    }

    if (dstCap == 0) {
        dst[0] = '\0';
        return EINVAL;  /* Ran out of room in the output buffer. */
    }

    *dstEnd = '\0';


    /* At this point the string will be reversed. */
    dstEnd -= 1;
    while (dst < dstEnd) {
        char temp = *dst;
        *dst = *dstEnd;
        *dstEnd = temp;

        dst += 1;
        dstEnd -= 1;
    }

    return 0;
}

C89STR_API int c89str_strcmp(const char* str1, const char* str2)
{
    if (str1 == str2) return  0;

    /* These checks differ from the standard implementation. It's not important, but I prefer it just for sanity. */
    if (str1 == NULL) return -1;
    if (str2 == NULL) return  1;

    for (;;) {
        if (str1[0] == '\0') {
            break;
        }

        if (str1[0] != str2[0]) {
            break;
        }

        str1 += 1;
        str2 += 1;
    }

    return ((unsigned char*)str1)[0] - ((unsigned char*)str2)[0];
}

C89STR_API int c89str_strncmp(const char* str1, const char* str2, size_t maxLen)
{
    if (str1 == str2) return  0;

    /* These checks differ from the standard implementation. It's not important, but I prefer it just for sanity. */
    if (str1 == NULL) return -1;
    if (str2 == NULL) return  1;

    /* This function still needs to check for null terminators even though the length has been specified. */
    for (;;) {
        if (maxLen == 0) {
            break;
        }

        if (str1[0] == '\0') {
            break;
        }

        if (str1[0] != str2[0]) {
            break;
        }

        str1 += 1;
        str2 += 1;
        maxLen -= 1;
    }

    if (maxLen == 0) {
        return 0;
    }

    return ((unsigned char*)str1)[0] - ((unsigned char*)str2)[0];
}

C89STR_API int c89str_stricmp_ascii(const char* str1, const char* str2)
{
    if (str1 == NULL || str2 == NULL) {
        return 0;
    }

    while (*str1 != '\0' && *str2 != '\0') {
        int c1 = (int)*str1;
        int c2 = (int)*str2;
    
        if (c1 >= 'A' && c1 <= 'Z') {
            c1 += 'a' - 'A';
        }
        if (c2 >= 'A' && c2 <= 'Z') {
            c2 += 'a' - 'A';
        }
    
        if (c1 != c2) {
            return c1 - c2;
        }
    
        str1 += 1;
        str2 += 1;
    }

    if (*str1 == '\0' && *str2 == '\0') {
        return 0;
    } else if (*str1 == '\0') {
        return -1;
    } else {
        return 1;
    }
}

C89STR_API int c89str_strnicmp_ascii(const char* str1, const char* str2, size_t count)
{
    if (str1 == NULL || str2 == NULL) {
        return 0;
    }

    while (*str1 != '\0' && *str2 != '\0' && count > 0) {
        int c1 = (int)*str1;
        int c2 = (int)*str2;
    
        if (c1 >= 'A' && c1 <= 'Z') {
            c1 += 'a' - 'A';
        }
        if (c2 >= 'A' && c2 <= 'Z') {
            c2 += 'a' - 'A';
        }
       
        if (c1 != c2) {
            return c1 - c2;
        }
       
        str1  += 1;
        str2  += 1;
        count -= 1;
    }

    if (count == 0) {
        return 0;
    } else if (*str1 == '\0' && *str2 == '\0') {
        return 0;
    } else if (*str1 == '\0') {
        return -1;
    } else {
        return 1;
    }
}

C89STR_API int c89str_stricmp(const char* str1, const char* str2)
{
    /* We will use the standard implementations of stricmp() and strcasecmp() if they are available. */
#if defined(_MSC_VER) && _MSC_VER >= 1400
    return _stricmp(str1, str2);
#elif defined(__GNUC__) && defined(__USE_GNU)
    return strcasecmp(str1, str2);
#else
    /* It would be good if we could use a custom implementation based on the Unicode standard here. Would require a lot of work to get that right, however. */
    return c89str_stricmp_ascii(str1, str2);
#endif
}

C89STR_API int c89str_strnicmp(const char* str1, const char* str2, size_t count)
{
    /* We will use the standard implementations of strnicmp() and strncasecmp() if they are available. */
#if defined(_MSC_VER) && _MSC_VER >= 1400
    return _strnicmp(str1, str2, count);
#elif defined(__GNUC__) && defined(__USE_GNU)
    return strncasecmp(str1, str2, count);
#else
    /* It would be good if we could use a custom implementation based on the Unicode standard here. Would require a lot of work to get that right, however. */
    return c89str_strnicmp_ascii(str1, str2, count);
#endif
}
/* END c89str_stdlib.c */


/* BEG c89str_helpers.c */
C89STR_API c89str_bool32 c89str_is_null_or_whitespace(const char* str, size_t strLen)
{
    if (str == NULL) {
        return C89STR_TRUE;
    }

    while (str[0] != '\0' && strLen > 0) {
        unsigned char c0 = (unsigned char)str[0];

        str    += 1;
        strLen -= 1;

        if (c0 >= 0x09 && c0 <= 0x0D) {
            continue;
        }
        if (c0 == 0x20) {
            continue;
        }

        if (strLen > 0) {
            unsigned char c1 = (unsigned char)str[0];

            str    += 1;
            strLen -= 1;

            if (c0 == 0xC2) {
                if (c1 == 0x85 || c1 == 0xA0) {
                    continue;   /* 0x0085, 0x00A0 */
                }
            }

            if (strLen > 1) {
                unsigned char c2 = (unsigned char)str[0];

                str    += 1;
                strLen -= 1;

                if (c0 == 0xE1) {
                    if (c1 == 0x9A) {
                        if (c2 == 0x80) {
                            continue;   /* 0x1680 */
                        }
                    }
                }

                if (c0 == 0xE2) {
                    if (c1 == 0x80) {
                        if (c2 >= 0x80 && c2 <= 0x8A) {
                            continue;   /* 0x2000 - 0x200A */
                        }

                        if (c2 == 0xA8 || c2 == 0xA9 || c2 == 0xAF) {
                            continue;   /* 0x2028, 0x2029, 0x202F */
                        }
                    }

                    if (c1 == 0x81) {
                        if (c2 == 0x9F) {
                            continue;   /* 0x205F */
                        }
                    }
                }

                if (c0 == 0xE3) {
                    if (c1 == 0x80) {
                        if (c2 == 0x80) {
                            continue;   /* 0x3000 */
                        }
                    }
                }
            }
        }

        return C89STR_FALSE;
    }

    return C89STR_TRUE;
}

C89STR_API errno_t c89str_findn(const char* str, size_t strLen, const char* other, size_t otherLen, size_t* pResult)
{
    size_t strOff;

    if (pResult == NULL) {
        return EINVAL;
    }

    *pResult = c89str_npos;

    if (str == NULL || other == NULL) {
        return EINVAL;
    }

    if (strLen == (size_t)-1) {
        strLen = c89str_strlen(str);
    }

    if (otherLen == (size_t)-1) {
        otherLen = c89str_strlen(other);
    }

    if (strLen == 0 || otherLen == 0) {
        return EINVAL;
    }

    strOff = 0;
    while ((strLen - strOff) >= otherLen) {
        c89str_bool32 found = C89STR_TRUE;
        size_t i;
        for (i = 0; i < otherLen; i += 1) {
            if (str[strOff + i] != other[i]) {
                found = C89STR_FALSE;
                break;
            }
        }

        if (found) {
            *pResult = strOff;
            return C89STR_SUCCESS;
        }

        strOff += 1;
    }

    /* Getting here means we didn't find the other string at all. */
    return ENOENT;
}

C89STR_API errno_t c89str_find(const char* str, const char* other, size_t* pResult)
{
    return c89str_findn(str, (size_t)-1, other, (size_t)-1, pResult);
}

C89STR_API int c89str_strncmpn(const char* str1, size_t str1Len, const char* str2, size_t str2Len)
{
    if (str1 == str2) return  0;

    /* These checks differ from the standard implementation. It's not important, but I prefer it just for sanity. */
    if (str1 == NULL) return -1;
    if (str2 == NULL) return  1;

    /* This function still needs to check for null terminators even though the length has been specified. */
    for (;;) {
        if (str1Len == 0 || str1[0] == '\0') {
            break;
        }
        if (str2Len == 0 || str2[0] == '\0') {
            break;
        }

        if (str1[0] != str2[0]) {
            break;
        }

        str1 += 1;
        str2 += 1;
        str1Len -= 1;
        str2Len -= 1;
    }

    /* If at this point we reached the end of both strings, they're equal. */
    if (str1Len == 0 && str2Len == 0) return 0;
    if (str1Len == 0) return -1;
    if (str2Len == 0) return  1;

    /* Getting here means we haven't reached the end according to the specified lengths, so just compare the last characters. */
    return ((unsigned char*)str1)[0] - ((unsigned char*)str2)[0];
}

C89STR_API c89str_bool32 c89str_begins_with(const char* str1, size_t str1Len, const char* str2, size_t str2Len)
{
    if (str1 == str2) {
        return C89STR_TRUE;
    }

    if (str1 == NULL || str2 == NULL) {
        return C89STR_FALSE;
    }

    /* This function still needs to check for null terminators even though the length has been specified. */
    for (;;) {
        if (str1Len == 0 || str1[0] == '\0') {
            break;
        }
        if (str2Len == 0 || str2[0] == '\0') {
            break;
        }

        if (str1[0] != str2[0]) {
            return C89STR_FALSE;
        }

        str1 += 1;
        str2 += 1;
        str1Len -= 1;
        str2Len -= 1;
    }

    /* Getting here means we've reached the end of one or both strings. If we're at the end of the second string it must mean that the first string begins with it. */
    if (str2Len == 0 || str2[0] == '\0') {
        return C89STR_TRUE;
    }

    /* Getting here means we're not at the end of the second string so we must have reached the end of the first string. */
    return C89STR_FALSE;
}

C89STR_API c89str_bool32 c89str_ends_with(const char* str1, size_t str1Len, const char* str2, size_t str2Len)
{
    if (str1 == NULL || str2 == NULL) {
        return C89STR_FALSE;
    }

    if (str1Len == (size_t)-1) {
        str1Len = c89str_strlen(str1);
    }
    if (str2Len == (size_t)-1) {
        str2Len = c89str_strlen(str2);
    }

    if (str1Len < str2Len) {
        return C89STR_FALSE;
    }

    return c89str_strncmp(str1 + str1Len - str2Len, str2, str2Len) == 0;
}

C89STR_API errno_t c89str_to_uint(const char* str, size_t len, unsigned int* pValue)
{
    unsigned int value = 0;

    if (pValue == NULL || c89str_is_null_or_whitespace(str, len)) {
        return EINVAL;
    }

    while (len > 0 && str[0] != '\0') {
        if (str[0] >= '0' && str[0] <= '9') {
            value *= 10;
            value += str[0] - '0';
        } else {
            /* Not an integer. */
            return EINVAL;
        }

        len -= 1;
        str += 1;
    }

    *pValue = value;

    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_to_int(const char* str, size_t len, int* pValue)
{
    int sign  = 1;
    int value = 0;

    if (pValue == NULL || c89str_is_null_or_whitespace(str, len)) {
        return EINVAL;
    }

    if (str[0] == '-') {
        sign = -1;
    } else {
        sign = +1;
    }

    str += 1;
    len -= 1;

    while (len > 0 && str[0] != '\0') {
        if (str[0] >= '0' && str[0] <= '9') {
            value *= 10;
            value += str[0] - '0';
        } else {
            /* Not an integer. */
            return EINVAL;
        }

        str += 1;
        len -= 1;
    }

    *pValue = value * sign;

    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_ascii_tolower(char* dst, size_t dstCap, const char* src, size_t srcLen)
{
    if (dst == NULL || src == NULL) {
        return EINVAL;
    }

    if (dstCap == 0) {
        return ERANGE;
    }

    while (srcLen > 0 && src[0] != '\0') {
        if (dstCap == 1) {
            return ERANGE;
        }

        if (src[0] >= 'A' && src[0] <= 'Z') {
            dst[0] = src[0] - 'A' + 'a';
        } else {
            dst[0] = src[0];
        }

        dst += 1;
        dstCap -= 1;
        src += 1;
        srcLen -= 1;
    }

    dst[0] = '\0';
    
    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_ascii_toupper(char* dst, size_t dstCap, const char* src, size_t srcLen)
{
    if (dst == NULL || src == NULL) {
        return EINVAL;
    }

    if (dstCap == 0) {
        return ERANGE;
    }

    while (srcLen > 0 && src[0] != '\0') {
        if (dstCap == 1) {
            return ERANGE;
        }

        if (src[0] >= 'a' && src[0] <= 'z') {
            dst[0] = src[0] - 'a' + 'A';
        } else {
            dst[0] = src[0];
        }

        dst += 1;
        dstCap -= 1;
        src += 1;
        srcLen -= 1;
    }

    dst[0] = '\0';
    
    return C89STR_SUCCESS;
}

C89STR_API c89str_bool32 c89str_is_all_digits(const char* str, size_t strLen)
{
    if (str == NULL) {
        return C89STR_FALSE;
    }

    while (strLen > 0 && str[0] != '\0') {
        if (str[0] < '0' || str[0] > '9') {
            return C89STR_FALSE;
        }

        str += 1;
        strLen -= 1;
    }

    return C89STR_TRUE;
}
/* END c89str_helpers.c */



#define C89STR_HEADER_SIZE_IN_BYTES     (sizeof(size_t) + sizeof(size_t) + sizeof(size_t)) /* cap, len, result. Result is typed as size_t here for alignment reasons. */

static size_t c89str_allocation_size(size_t cap)
{
    return C89STR_HEADER_SIZE_IN_BYTES + cap + 1; /* +1 for null terminator. */
}

static void* c89str_to_allocation_address(c89str str)
{
    return (void*)(str - C89STR_HEADER_SIZE_IN_BYTES);
}

static c89str c89str_from_allocation_address(void* pAllocationAddress)
{
    return (char*)pAllocationAddress + C89STR_HEADER_SIZE_IN_BYTES;
}

static void c89str_set_cap(c89str str, size_t cap)
{
    ((size_t*)c89str_to_allocation_address(str))[0] = cap;
}

static size_t c89str_get_cap(const c89str str)
{
    return ((size_t*)c89str_to_allocation_address(str))[0];
}

static size_t c89str_get_len(const c89str str)
{
    return ((size_t*)c89str_to_allocation_address(str))[1];
}

static void c89str_set_res(c89str str, errno_t result)
{
    if (str == NULL) {
        return;
    }

    ((size_t*)c89str_to_allocation_address(str))[2] = (size_t)result;
}

static errno_t c89str_get_res(const c89str str)
{
    /* If this is called with a null pointer, assume that NULL was returned from some function which should only happen in out-of-memory situations. */
    if (str == NULL) {
        return ENOMEM;
    }

    return (errno_t)((size_t*)c89str_to_allocation_address(str))[2];
}

static c89str c89str_realloc_string(c89str str, size_t cap, const c89str_allocation_callbacks* pAllocationCallbacks)
{
    void* pAllocation = c89str_realloc((str == NULL) ? NULL : c89str_to_allocation_address(str), c89str_allocation_size(cap), pAllocationCallbacks);
    if (pAllocation == NULL) {
        return NULL;    /* Failed */
    }

    c89str_set_cap(c89str_from_allocation_address(pAllocation), cap);
    c89str_set_res(c89str_from_allocation_address(pAllocation), C89STR_SUCCESS);

    return c89str_from_allocation_address(pAllocation);
}

static c89str c89str_realloc_string_if_necessary(c89str str, size_t requiredCap, const c89str_allocation_callbacks* pAllocationCallbacks)
{
    if (str == NULL) {
        /* Need to allocate a fresh string. */
        str = c89str_realloc_string(str, requiredCap, pAllocationCallbacks);
    } else {
        /* Don't necessarily need to allocate a fresh string, but might need to expand memory to make it fit. */
        size_t cap = c89str_get_cap(str);

        if (cap < requiredCap) {
            cap = requiredCap;
            str = c89str_realloc_string(str, cap, pAllocationCallbacks);
        }
    }

    return str;
}


C89STR_API void c89str_delete(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks)
{
    if (str == NULL) {
        return;
    }

    c89str_free(c89str_to_allocation_address(str), pAllocationCallbacks);
}

C89STR_API c89str c89str_new_with_cap(const c89str_allocation_callbacks* pAllocationCallbacks, size_t capacityNotIncludingNullTerminator)
{
    c89str str = c89str_realloc_string(NULL, capacityNotIncludingNullTerminator, pAllocationCallbacks);
    if (str == NULL) {
        return NULL;    /* Failed */
    }

    c89str_set_len(str, 0);
    str[0] = '\0';

    return str;
}

C89STR_API c89str c89str_new(const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther)
{
    return c89str_set(NULL, pAllocationCallbacks, pOther);
}

C89STR_API c89str c89str_newn(const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen)
{
    return c89str_setn(NULL, pAllocationCallbacks, pOther, otherLen);
}

C89STR_API c89str c89str_newv(const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args)
{
    return c89str_setv(NULL, pAllocationCallbacks, pFormat, args);
}

C89STR_API c89str c89str_newf(const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...)
{
    c89str str;
    va_list args;

    C89STR_ASSERT(pFormat != NULL); /* Format cannot be null. */

    va_start(args, pFormat);
    {
        str = c89str_setv(NULL, pAllocationCallbacks, pFormat, args);
    }
    va_end(args);

    return str;
}

C89STR_API c89str c89str_set(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther)
{
    return c89str_setn(str, pAllocationCallbacks, pOther, (size_t)-1);
}

C89STR_API c89str c89str_setn(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen)
{
    if (pOther == NULL) {
        pOther = "";
    }

    if (otherLen == (size_t)-1) {
        otherLen = c89str_strlen(pOther);
    }

    if (str != pOther) {
        c89str str2 = c89str_realloc_string_if_necessary(str, otherLen, pAllocationCallbacks);
        if (str2 == NULL) {
            if (str != NULL) {
                c89str_set_res(str, ENOMEM);
            }

            return str;
        }

        str = str2;
        C89STR_COPY_MEMORY(str, pOther, otherLen);   /* Will be explicitly null terminated later. */
    } else {
        /* str and pOther are the same string. No need for a data copy. */
        C89STR_ASSERT(str != NULL);
    }

    /* Null terminate and set the length. */
    str[otherLen] = '\0';
    c89str_set_len(str, otherLen);

    /* At this point we're done. */
    return str;
}

C89STR_API c89str c89str_setv(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args)
{
    va_list args2;
    int len;
    c89str str2;

    C89STR_ASSERT(pFormat != NULL); /* Format cannot be null. */

#if !defined(_MSC_VER) || _MSC_VER >= 1800
    va_copy(args2, args);
#else
    args2 = args;
#endif
    {
        len = c89str_vsnprintf(NULL, 0, pFormat, args2);
        if (len < 0) {
            if (str != NULL) {
                c89str_set_res(str, errno);
            }

            return str;
        }
    }
    va_end(args2);


    /* Make sure there's enough room for the new string. */
    str2 = c89str_realloc_string_if_necessary(str, len, pAllocationCallbacks);
    if (str2 == NULL) {
        if (str != NULL) {
            c89str_set_res(str, ENOMEM);
        }

        return str;
    }

    /* Getting here means reallocation was successfull. */
    str = str2;
    str2 = NULL;    /* <-- Just to make sure we don't try using str2 again. The compiler will optimize this away. */

    /* We have enough room in the string so now we can just format straight into it. */
    c89str_vsnprintf(str, len+1, pFormat, args);

    /* The length needs to be set explicitly. The formatting will have written the null terminator. */
    c89str_set_len(str, len);

    /* We're done. */
    return str;
}

C89STR_API c89str c89str_setf(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...)
{
    va_list args;

    C89STR_ASSERT(pFormat != NULL); /* Format cannot be null. */

    va_start(args, pFormat);
    {
        str = c89str_setv(str, pAllocationCallbacks, pFormat, args);
    }
    va_end(args);

    return str;
}

C89STR_API c89str c89str_cat(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther)
{
    return c89str_catn(str, pAllocationCallbacks, pOther, (size_t)-1);
}

C89STR_API c89str c89str_catn(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen)
{
    c89str str2;
    size_t len;

    if (str != NULL) {
        if (c89str_get_res(str) != C89STR_SUCCESS) {
            return str; /* The string is in an error state. */
        }
    }

    if (pOther == NULL) {
        pOther = "";
    }

    if (otherLen == (size_t)-1) {
        otherLen = c89str_strlen(pOther);
    }

    if (str == NULL) {
        /* Input string is null. This is just an assignment. */
        return c89str_setn(str, pAllocationCallbacks, pOther, otherLen);
    }

    /* str should never be null at this point. */
    C89STR_ASSERT(str != NULL);

    len  = c89str_get_len(str);
    str2 = c89str_realloc_string_if_necessary(str, len + otherLen, pAllocationCallbacks);
    if (str2 == NULL) {
        c89str_set_res(str, ENOMEM);
        return str;
    }

    str = str2;
    C89STR_COPY_MEMORY(str + len, pOther, otherLen);   /* Will be explicitly null terminated later. */

    /* Null terminate and set the length. */
    str[len + otherLen] = '\0';
    c89str_set_len(str, len + otherLen);

    /* At this point we're done. */
    return str;
}

C89STR_API c89str c89str_catv(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args)
{
    va_list args2;
    size_t len;
    int otherLen;
    c89str str2;

    C89STR_ASSERT(pFormat != NULL); /* Format cannot be null. */

    if (str != NULL) {
        if (c89str_get_res(str) != C89STR_SUCCESS) {
            return str; /* The string is in an error state. */
        }
    }

#if !defined(_MSC_VER) || _MSC_VER >= 1800
    va_copy(args2, args);
#else
    args2 = args;
#endif
    {
        otherLen = c89str_vsnprintf(NULL, 0, pFormat, args2);
        if (otherLen < 0) {
            if (str != NULL) {
                c89str_set_res(str, errno);
            }

            return str;
        }
    }
    va_end(args2);


    /* Make sure there's enough room for the new string. */
    len = 0;
    if (str != NULL) {
        len = c89str_get_len(str);
    }

    str2 = c89str_realloc_string_if_necessary(str, len + otherLen, pAllocationCallbacks);
    if (str2 == NULL) {
        if (str != NULL) {
            c89str_set_res(str, ENOMEM);
        }

        return str;
    }

    /* Getting here means reallocation was successfull. */
    str = str2;

    /* We have enough room in the string so now we can just format straight into it. */
    c89str_vsnprintf(str + len, otherLen+1, pFormat, args);

    /* The length needs to be set explicitly. The formatting will have written the null terminator. */
    c89str_set_len(str, len + otherLen);

    /* We're done. */
    return str;
}

C89STR_API c89str c89str_catf(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...)
{
    va_list args;

    C89STR_ASSERT(pFormat != NULL); /* Format cannot be null. */

    va_start(args, pFormat);
    {
        str = c89str_catv(str, pAllocationCallbacks, pFormat, args);
    }
    va_end(args);

    return str;
}

C89STR_API c89str c89str_prepend(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther)
{
    return c89str_prependn(str, pAllocationCallbacks, pOther, (size_t)-1);
}

C89STR_API c89str c89str_prependn(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen)
{
    c89str str2;
    size_t len;

    if (str != NULL) {
        if (c89str_get_res(str) != C89STR_SUCCESS) {
            return str; /* The string is in an error state. */
        }
    }

    if (pOther == NULL) {
        pOther = "";
    }

    if (otherLen == (size_t)-1) {
        otherLen = c89str_strlen(pOther);
    }

    if (str == NULL) {
        /* Input string is null. This is just an assignment. */
        return c89str_setn(str, pAllocationCallbacks, pOther, otherLen);
    }

    C89STR_ASSERT(str != NULL);

    len  = c89str_get_len(str);
    str2 = c89str_realloc_string_if_necessary(str, len + otherLen, pAllocationCallbacks);
    if (str2 == NULL) {
        c89str_set_res(str, ENOMEM);
        return str;
    }

    str = str2;
    C89STR_MOVE_MEMORY(str + otherLen, str, len + 1);   /* Move the existing string to the right. +1 for the null terminator. */
    C89STR_COPY_MEMORY(str, pOther, otherLen);          /* Copy the new string to the left. */

    /* Set the length (null terminator was already done. */
    c89str_set_len(str, len + otherLen);

    /* At this point we're done. */
    return str;
}

C89STR_API c89str c89str_prependv(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args)
{
    va_list args2;
    size_t len;
    int otherLen;
    c89str str2;

    C89STR_ASSERT(pFormat != NULL); /* Format cannot be null. */

    if (str != NULL) {
        if (c89str_get_res(str) != C89STR_SUCCESS) {
            return str; /* The string is in an error state. */
        }
    }

#if !defined(_MSC_VER) || _MSC_VER >= 1800
    va_copy(args2, args);
#else
    args2 = args;
#endif
    {
        otherLen = c89str_vsnprintf(NULL, 0, pFormat, args2);
        if (otherLen < 0) {
            if (str != NULL) {
                c89str_set_res(str, errno);
            }

            return str;
        }
    }
    va_end(args2);


    /* Make sure there's enough room for the new string. */
    len = 0;
    if (str != NULL) {
        len = c89str_get_len(str);
    }

    str2 = c89str_realloc_string_if_necessary(str, len + otherLen, pAllocationCallbacks);
    if (str2 == NULL) {
        if (str != NULL) {
            c89str_set_res(str, ENOMEM);
        }

        return str;
    }

    str = str2;
    str2 = NULL;    /* <-- Just to make sure we don't accidentally use str2 again. */

    /*
    Prepending is awkward here because we want to avoid as much data movement as possible which
    means it makes sense to format straight into the string. We would need to move the existing
    string down to make room for the new string, but the problem is that formatting straight into
    the output buffer will result in the null terminator overwriting the first character of the
    existing string.

    What we're doing here is grabbing the first character of the existing string and storing it.
    Then we move the existing string down to make room for the new string. We then format the new
    string straight into the buffer which will result in the null terminator overwriting the first
    character of the existing string. We then restore the first character of the existing string.
    */

    /* Grab the first character of the existing string. */
    {
        char firstChar = '\0';

        if (len > 0) {
            firstChar = str[0];
        }

        /* Move the existing string down to make room for the new string. */
        C89STR_MOVE_MEMORY(str + otherLen, str, len + 1);   /* +1 for the null terminator. */

        /* Format the new string straight into the buffer. */
        c89str_vsnprintf(str, len+1, pFormat, args);

        /* Restore the first character of the existing string. */
        if (len > 0) {
            str[otherLen] = firstChar;
        }
    }

    /* The length needs to be set explicitly. The null terminator has already been dealt with. */
    c89str_set_len(str, otherLen + len);

    /* We're done. */
    return str;
}

C89STR_API c89str c89str_prependf(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...)
{
    va_list args;

    C89STR_ASSERT(pFormat != NULL); /* Format cannot be null. */

    va_start(args, pFormat);
    {
        str = c89str_prependv(str, pAllocationCallbacks, pFormat, args);
    }
    va_end(args);

    return str;
}

C89STR_API c89str c89str_remove(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, size_t beg, size_t end)
{
    size_t len;
    size_t lenToRemove;

    C89STR_UNUSED(pAllocationCallbacks);

    if (beg > end) {
        return str;  /* The beginning must not be greater than the end. Nothing to remove. */
    }

    if (str == NULL) {
        return NULL; /* Nothing to remove from. */
    }

    if (c89str_get_res(str) != C89STR_SUCCESS) {
        return str; /* The string is in an error state. */
    }

    len = c89str_get_len(str);
    if (beg > len) {
        return str; /* Trying to remove beyond the end of the string. */
    }
    if (end > len) {
        end = len;  /* Trying to remove beyond the end of the string. Clamp the end. */
    }

    lenToRemove = end - beg;
    if (lenToRemove == 0) {
        return str; /* Nothing to remove. */
    }

    C89STR_MOVE_MEMORY(str + beg, str + end, len - end + 1); /* +1 to include the null terminator. */
    c89str_set_len(str, len - lenToRemove);

    /* We're done. */
    return str;
}

static c89str c89str_replace_ex(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, size_t replaceOffset, size_t replaceLen, const char* pOther, size_t otherLen, const char* pOtherPrepend, const char* pOtherAppend)
{
    c89str newStr;
    errno_t result;

    if (str == NULL) {
        return NULL; /* Nothing to replace. */
    }

    if (c89str_get_res(str) != C89STR_SUCCESS) {
        return str; /* The string is in an error state. */
    }

    if (replaceOffset + replaceLen > c89str_get_len(str)) {
        return str;
    }

    /* We can allow pOther to be NULL in which case it can be the same as a remove. */
    if (pOther == NULL) {
        pOther = "";
    }

    if (otherLen == (size_t)-1) {
        otherLen = c89str_strlen(pOther);
    }


    /* TODO: This can be made more efficient by doing the replacement in-place if the replacement string is the same or smaller in size. Can also optimize when there is no prepand nor append. */

    
    /* The string is split into 3 sections: the part before the replace, the replacement itself, and the part after the replacement. */

    /* Pre-replacement. */
    newStr = c89str_newn(pAllocationCallbacks, str, replaceOffset);

    /* Replacement. */
    if (pOtherPrepend != NULL) { newStr = c89str_cat(newStr, pAllocationCallbacks, pOtherPrepend); }
    {
        newStr = c89str_catn(newStr, pAllocationCallbacks, pOther, otherLen);
    }
    if (pOtherAppend  != NULL) { newStr = c89str_cat(newStr, pAllocationCallbacks, pOtherAppend);  }

    /* Post-replacement. */
    newStr = c89str_catn(newStr, pAllocationCallbacks, str + (replaceOffset + replaceLen), c89str_get_len(str) - (replaceOffset + replaceLen));

    /* Now check the result of the operation. */
    result = c89str_result(newStr);
    if (result != C89STR_SUCCESS) {
        c89str_delete(newStr, pAllocationCallbacks);
        c89str_set_res(str, result);
        return str;
    }

    /* We're done. */
    c89str_delete(str, pAllocationCallbacks);
    str = newStr;

    return str;
}

C89STR_API c89str c89str_replace(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, size_t replaceOffset, size_t replaceLength, const char* pOther, size_t otherLength)
{
    return c89str_replace_ex(str, pAllocationCallbacks, replaceOffset, replaceLength, pOther, otherLength, NULL, NULL);
}

C89STR_API c89str c89str_replace_all(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pQuery, size_t queryLen, const char* pReplacement, size_t replacementLen)
{
    size_t offset = 0;

    if (str == NULL) {
        return NULL;
    }

    if (pQuery == NULL) {
        pQuery = "";
    }

    if (queryLen == (size_t)-1) {
        queryLen = c89str_strlen(pQuery);
    }

    if (queryLen == 0) {
        return str; /* Nothing to replace. */
    }


    if (pReplacement == NULL) {
        pReplacement = "";
    }

    if (replacementLen == (size_t)-1) {
        replacementLen = c89str_strlen(pReplacement);
    }


    /* We can do an optimized implementation if we're replacing a single character. */
    if (queryLen == 1 && replacementLen == 1) {
        size_t len = c89str_get_len(str);
        size_t i;

        for (i = 0; i < len; ++i) {
            if ((str)[i] == pQuery[0]) {
                (str)[i] = pReplacement[0];
            }
        }
    
        return str;
    }


    /* We keep looping until there's no more occurrances. */
    for (;;) {
        errno_t result;
        size_t location;

        result = c89str_findn(str + offset, c89str_get_len(str) - offset, pQuery, queryLen, &location);
        if (result == ENOENT || location == c89str_npos) {
            break;  /* We're done */
        }

        /* Getting here means we found one. We just need to replace a range. */
        str = c89str_replace(str, pAllocationCallbacks, offset + location, queryLen, pReplacement, replacementLen);
        if (c89str_result(str) != C89STR_SUCCESS) {
            break;  /* We got an error. Probably out of memory. Get out of the loop. */
        }

        offset += location + replacementLen;   /* Progress past the replacement. */
    }

    return str;
}

C89STR_API c89str c89str_trim(c89str str, const c89str_allocation_callbacks* pAllocationCallbacks)
{
    size_t loff;
    size_t roff;

    C89STR_UNUSED(pAllocationCallbacks);

    if (str == NULL) {
        return NULL;
    }

    /* The length of the string will never expand which simplifies our memory management. */
    loff = c89str_utf8_ltrim_offset(str, c89str_get_len(str));
    roff = c89str_utf8_rtrim_offset(str, c89str_get_len(str));

    C89STR_MOVE_MEMORY(str, str + loff, (roff - loff));  /* Left trim by moving the string down. */
    c89str_set_len(str, roff - loff);                    /* Set the length before the right trim. */
    str[c89str_get_len(str)] = '\0';                     /* Right trim by setting the null terminator. */

    return str;
}


C89STR_API void c89str_set_len(c89str str, size_t len)
{
    if (str == NULL) {
        return;
    }

    ((size_t*)c89str_to_allocation_address(str))[1] = len;
}

C89STR_API size_t c89str_len(const c89str str)
{
    if (str == NULL) {
        return 0;
    } else {
        return c89str_get_len(str);
    }
}

C89STR_API size_t c89str_cap(const c89str str)
{
    if (str == NULL) {
        return 0;
    } else {
        return c89str_get_cap(str);
    }
}

C89STR_API errno_t c89str_result(const c89str str)
{
    return c89str_get_res(str);
}




/* Begin Unicode */
static C89STR_INLINE c89str_bool32 c89str_is_invalid_utf8_octet(c89str_utf8 utf8)
{
    /* RFC 3629 - Section 1: The octet values C0, C1, F5 to FF never appear. */
    return (unsigned char)utf8 == 0xC0 || (unsigned char)utf8 == 0xC1 || (unsigned char)utf8 >= 0xF5;
}

static C89STR_INLINE void c89str_utf32_cp_to_utf16_pair(c89str_utf32 utf32cp, c89str_utf16* pUTF16)
{
    /* RFC 2781 - Section 2.1 */
    c89str_utf32 u;

    C89STR_ASSERT(utf32cp >= 0x10000);

    u = utf32cp - 0x10000;
    pUTF16[0] = (c89str_utf16)(0xD800 | ((u & 0xFFC00) >> 10));
    pUTF16[1] = (c89str_utf16)(0xDC00 | ((u & 0x003FF) >>  0));
}

static C89STR_INLINE c89str_utf32 c89str_utf16_pair_to_utf32_cp(const c89str_utf16* pUTF16)
{
    /* RFC 2781 - Section 2.1 */
    C89STR_ASSERT(pUTF16 != NULL);

    return (((c89str_utf32)(pUTF16[0] & 0x003FF) << 10) | ((c89str_utf32)(pUTF16[1] & 0x003FF) << 0)) + 0x10000;
}

static C89STR_INLINE c89str_bool32 c89str_is_cp_in_surrogate_pair_range(c89str_utf32 utf32)
{
    return utf32 >= 0xD800 && utf32 <= 0xDFFF;
}

static C89STR_INLINE c89str_bool32 c89str_is_valid_code_point(c89str_utf32 utf32)
{
    return utf32 <= C89STR_UNICODE_MAX_CODE_POINT && !c89str_is_cp_in_surrogate_pair_range(utf32);
}

static C89STR_INLINE size_t c89str_utf32_cp_to_utf8_len(c89str_utf32 utf32)
{
    /* This API assumes the the UTF-32 code point is valid. */
    C89STR_ASSERT(utf32 <= C89STR_UNICODE_MAX_CODE_POINT);
    C89STR_ASSERT(c89str_is_cp_in_surrogate_pair_range(utf32) == C89STR_FALSE);

    if (utf32 <= 0x7F) {
        return 1;
    }
    if (utf32 <= 0x7FF) {
        return 2;
    }
    if (utf32 <= 0xFFFF) {
        return 3;
    }
    if (utf32 <= 0x10FFFF) {
        return 4;
    }

    /* Invalid. This function assume's the UTF-32 code point is valid so do an assert. */
    C89STR_ASSERT(C89STR_FALSE);
    return 0; /* Invalid. */
}

static C89STR_INLINE size_t c89str_utf32_cp_to_utf8(c89str_utf32 utf32, c89str_utf8* pUTF8, size_t utf8Cap)
{
    /* This API assumes the the UTF-32 code point is valid. */
    C89STR_ASSERT(utf32 <= C89STR_UNICODE_MAX_CODE_POINT);
    C89STR_ASSERT(c89str_is_cp_in_surrogate_pair_range(utf32) == C89STR_FALSE);
    C89STR_ASSERT(utf8Cap > 0);

    if (utf32 <= 0x7F) {
        if (utf8Cap >= 1) {
            pUTF8[0] = (utf32 & 0x7F);
            return 1;
        }
    }
    if (utf32 <= 0x7FF) {
        if (utf8Cap >= 2) {
            pUTF8[0] = 0xC0 | (c89str_utf8)((utf32 & 0x07C0) >> 6);
            pUTF8[1] = 0x80 | (c89str_utf8) (utf32 & 0x003F);
            return 2;
        }
    }
    if (utf32 <= 0xFFFF) {
        if (utf8Cap >= 3) {
            pUTF8[0] = 0xE0 | (c89str_utf8)((utf32 & 0xF000) >> 12);
            pUTF8[1] = 0x80 | (c89str_utf8)((utf32 & 0x0FC0) >>  6);
            pUTF8[2] = 0x80 | (c89str_utf8) (utf32 & 0x003F);
            return 3;
        }
    }
    if (utf32 <= 0x10FFFF) {
        if (utf8Cap >= 4) {
            pUTF8[0] = 0xF0 | (c89str_utf8)((utf32 & 0x1C0000) >> 18);
            pUTF8[1] = 0x80 | (c89str_utf8)((utf32 & 0x03F000) >> 12);
            pUTF8[2] = 0x80 | (c89str_utf8)((utf32 & 0x000FC0) >>  6);
            pUTF8[3] = 0x80 | (c89str_utf8) (utf32 & 0x00003F);
            return 4;
        }
    }

    /* Getting here means there was not enough room in the output buffer. */
    return 0;
}

static C89STR_INLINE size_t c89str_utf32_cp_to_utf16_len(c89str_utf32 utf32)
{
    /* This API assumes the the UTF-32 code point is valid. */
    C89STR_ASSERT(utf32 <= C89STR_UNICODE_MAX_CODE_POINT);
    C89STR_ASSERT(c89str_is_cp_in_surrogate_pair_range(utf32) == C89STR_FALSE);

    if (utf32 <= 0xFFFF) {
        return 1;
    } else {
        return 2;
    }

    /* Unreachable. */
#if 0
    /* Invalid. This function assume's the UTF-32 code point is valid so do an assert. */
    C89STR_ASSERT(C89STR_FALSE);
    return 0; /* Invalid. */
#endif
}

static C89STR_INLINE size_t c89str_utf32_cp_to_utf16(c89str_utf32 utf32, c89str_utf16* pUTF16, size_t utf16Cap)
{
    /* This API assumes the the UTF-32 code point is valid. */
    C89STR_ASSERT(utf32 <= C89STR_UNICODE_MAX_CODE_POINT);
    C89STR_ASSERT(c89str_is_cp_in_surrogate_pair_range(utf32) == C89STR_FALSE);
    C89STR_ASSERT(utf16Cap > 0);

    if (utf32 <= 0xFFFF) {
        if (utf16Cap >= 1) {
            pUTF16[0] = (c89str_utf16)utf32;
            return 1;
        }
    } else {
        if (utf16Cap >= 2) {
            c89str_utf32_cp_to_utf16_pair(utf32, pUTF16);
            return 2;
        }
    }

    /* Getting here means there was not enough room in the output buffer. */
    return 0;
}



C89STR_API c89str_bool32 c89str_utf16_is_bom_le(const unsigned char bom[2])
{
    /* RFC 2781 - Section 3.2 */
    return bom[0] == 0xFF && bom[1] == 0xFE;
}

C89STR_API c89str_bool32 c89str_utf16_is_bom_be(const unsigned char bom[2])
{
    /* RFC 2781 - Section 3.2 */
    return bom[0] == 0xFE && bom[1] == 0xFF;
}

C89STR_API c89str_bool32 c89str_utf32_is_bom_le(const unsigned char bom[4])
{
    return bom[0] == 0xFF && bom[1] == 0xFE && bom[2] == 0x00 && bom[3] == 0x00;
}

C89STR_API c89str_bool32 c89str_utf32_is_bom_be(const unsigned char bom[4])
{
    return bom[0] == 0x00 && bom[1] == 0x00 && bom[2] == 0xFE && bom[3] == 0xFF;
}

C89STR_API c89str_bool32 c89str_utf8_has_bom(const unsigned char* pBytes, size_t len)
{
    if (pBytes == NULL) {
        return C89STR_FALSE;
    }

    if (len < 3) {
        return C89STR_FALSE;
    }

    return (pBytes[0] == 0xEF && pBytes[1] == 0xBB && pBytes[2] == 0xBF);
}

C89STR_API c89str_bool32 c89str_utf16_has_bom(const unsigned char* pBytes, size_t len)
{
    if (pBytes == NULL) {
        return C89STR_FALSE;
    }

    if (len < 2) {
        return C89STR_FALSE;
    }

    return c89str_utf16_is_bom_le(pBytes) || c89str_utf16_is_bom_be(pBytes);
}

C89STR_API c89str_bool32 c89str_utf32_has_bom(const unsigned char* pBytes, size_t len)
{
    if (pBytes == NULL) {
        return C89STR_FALSE;
    }

    if (len < 4) {
        return C89STR_FALSE;
    }

    return c89str_utf32_is_bom_le(pBytes) || c89str_utf32_is_bom_be(pBytes);
}


C89STR_API void c89str_utf16_swap_endian(c89str_utf16* pUTF16, size_t count)
{
    if (count == (size_t)-1) {
        size_t i;
        for (i = 0; i < count; ++i) {
            pUTF16[i] = c89str_swap_endian_uint16(pUTF16[i]);
        }
    } else {
        while (pUTF16[0] != 0) {
            pUTF16[0] = c89str_swap_endian_uint16(pUTF16[0]);
            pUTF16 += 1;
        }
    }
}

C89STR_API void c89str_utf32_swap_endian(c89str_utf32* pUTF32, size_t count)
{
    if (count == (size_t)-1) {
        size_t i;
        for (i = 0; i < count; ++i) {
            pUTF32[i] = c89str_swap_endian_uint32(pUTF32[i]);
        }
    } else {
        while (pUTF32[0] != 0) {
            pUTF32[0] = c89str_swap_endian_uint32(pUTF32[0]);
            pUTF32 += 1;
        }
    }
}


C89STR_API errno_t c89str_utf8_to_utf16_len(size_t* pUTF16Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags)
{
    errno_t result = C89STR_SUCCESS;
    size_t utf16Len = 0;

    if (pUTF16Len != NULL) {
        *pUTF16Len = 0;
    }

    if (pUTF8LenProcessed != NULL) {
        *pUTF8LenProcessed = 0;
    }

    if (pUTF8 == NULL) {
        return EINVAL;   /* Invalid input. */
    }

    if (utf8Len == 0 || pUTF8[0] == 0) {
        return C89STR_SUCCESS;   /* Empty input string. Length is always 0. */
    }

    /* Check for BOM. */
    if (c89str_utf8_has_bom((const unsigned char*)pUTF8, utf8Len)) {
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        pUTF8 += 3; /* Skip past the BOM. */
        if (utf8Len != (size_t)-1) {
            utf8Len -= 3;
        }
    }

    if (utf8Len == (size_t)-1) {
        /* Null terminated string. */
        const c89str_utf8* pUTF8Original = pUTF8;
        for (;;) {
            if (pUTF8[0] == 0) {
                break;  /* Reached the end of the null terminated string. */
            }

            if ((unsigned char)pUTF8[0] < 128) {   /* ASCII character. */
                utf16Len += 1;
                pUTF8    += 1;
            } else {
                if (c89str_is_invalid_utf8_octet(pUTF8[0])) {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        /* Replacement. */
                        utf16Len += C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                        pUTF8    += 1;
                    }
                } else {
                    if ((pUTF8[0] & 0xE0) == 0xC0) {
                        if (pUTF8[1] == 0) {
                            result = EINVAL; /* Input string is too short. */
                            break;
                        }

                        utf16Len += 1;  /* Can be at most 1 UTF-16. */
                        pUTF8    += 2;
                    } else if ((pUTF8[0] & 0xF0) == 0xE0) {
                        if (pUTF8[1] == 0 || pUTF8[2] == 0) {
                            result = EINVAL; /* Input string is too short. */
                            break;
                        }

                        utf16Len += 1;  /* Can be at most 1 UTF-16.*/
                        pUTF8    += 3;
                    } else if ((pUTF8[0] & 0xF8) == 0xF0) {
                        unsigned int cp;
                        if (pUTF8[1] == 0 || pUTF8[2] == 0 || pUTF8[3] == 0) {
                            result = EINVAL; /* Input string is too short. */
                            break;
                        }

                        cp = ((c89str_utf32)(pUTF8[0] & 0x07) << 18) | ((c89str_utf32)(pUTF8[1] & 0x3F) << 12) | ((c89str_utf32)(pUTF8[2] & 0x3F) << 6) | (pUTF8[3] & 0x3F);
                        if (!c89str_is_valid_code_point(cp)) {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;
                                break;
                            } else {
                                /* Replacement. */
                                utf16Len += C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                                pUTF8    += 4;
                            }
                        } else {
                            utf16Len += 2;  /* Must be at least 2 UTF-16s */
                            pUTF8    += 4;
                        }
                    } else {
                        if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                            result = C89STR_ECODEPOINT;
                            break;
                        } else {
                            /* Replacement. */
                            utf16Len += C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                            pUTF8    += 1;
                        }
                    }
                }
            }
        }

        if (pUTF8LenProcessed != NULL) {
            *pUTF8LenProcessed = (pUTF8 - pUTF8Original);
        }
    } else {
        /* Fixed length string. */
        size_t iUTF8;
        for (iUTF8 = 0; iUTF8 < utf8Len; /* Do nothing */) {
            if ((unsigned char)pUTF8[iUTF8+0] < 128) {   /* ASCII character. */
                utf16Len += 1;
                iUTF8    += 1;
            } else {
                if (c89str_is_invalid_utf8_octet(pUTF8[iUTF8+0])) {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        /* Replacement. */
                        utf16Len += C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                        iUTF8    += 1;
                    }
                } else {
                    if ((pUTF8[iUTF8+0] & 0xE0) == 0xC0) {
                        if (iUTF8+1 > utf8Len) {
                            result = EINVAL;
                            break;
                        }

                        utf16Len += 1;  /* Can be at most 1 UTF-16.*/
                        iUTF8    += 2;
                    } else if ((pUTF8[iUTF8+0] & 0xF0) == 0xE0) {
                        if (iUTF8+2 > utf8Len) {
                            result = EINVAL;
                            break;
                        }

                        utf16Len += 1;  /* Can be at most 1 UTF-16.*/
                        iUTF8    += 3;
                    } else if ((pUTF8[iUTF8+0] & 0xF8) == 0xF0) {
                        unsigned int cp;
                        if (iUTF8+3 > utf8Len) {
                            return EINVAL;
                        }

                        cp = ((c89str_utf32)(pUTF8[0] & 0x07) << 18) | ((c89str_utf32)(pUTF8[1] & 0x3F) << 12) | ((c89str_utf32)(pUTF8[2] & 0x3F) << 6) | (pUTF8[3] & 0x3F);
                        if (!c89str_is_valid_code_point(cp)) {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;
                                break;
                            } else {
                                /* Replacement. */
                                utf16Len += C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                                iUTF8    += 4;
                            }
                        } else {
                            utf16Len += 2;  /* Must be at least 2 UTF-16s */
                            iUTF8    += 4;
                        }
                    } else {
                        if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                            result = C89STR_ECODEPOINT;
                            break;
                        } else {
                            /* Replacement. */
                            utf16Len += C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                            iUTF8    += 1;
                        }
                    }
                }
            }
        }

        if (pUTF8LenProcessed != NULL) {
            *pUTF8LenProcessed = iUTF8;
        }
    }

    if (pUTF16Len != NULL) {
        *pUTF16Len = utf16Len;
    }

    return result;
}

C89STR_API errno_t c89str_utf8_to_utf16ne(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags)
{
    errno_t result = C89STR_SUCCESS;
    size_t utf16CapOriginal = utf16Cap;

    if (pUTF16 == NULL) {
        return c89str_utf8_to_utf16_len(pUTF16Len, pUTF8, utf8Len, pUTF8LenProcessed, flags);
    }

    if (pUTF16Len != NULL) {
        *pUTF16Len = 0;
    }

    if (pUTF8LenProcessed != NULL) {
        *pUTF8LenProcessed = 0;
    }

    if (pUTF8 == NULL) {
        return EINVAL;
    }

    /* Check for BOM. */
    if (c89str_utf8_has_bom((const unsigned char*)pUTF8, utf8Len)) {
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;   /* Found a BOM, but it's forbidden. */
        }

        pUTF8 += 3; /* Skip past the BOM. */
        if (utf8Len != (size_t)-1) {
            utf8Len -= 3;
        }
    }

    if (utf8Len == (size_t)-1) {
        /* Null terminated string. */
        const c89str_utf8* pUTF8Original = pUTF8;
        while (pUTF8[0] != 0) {
            if (utf16Cap == 1) {
                result = ENOMEM;
                break;
            }

            if ((unsigned char)pUTF8[0] < 128) {   /* ASCII character. */
                pUTF16[0] = pUTF8[0];
                pUTF16   += 1;
                utf16Cap -= 1;
                pUTF8    += 1;
            } else {
                if (c89str_is_invalid_utf8_octet(pUTF8[0])) {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        /* Replacement. */
                        pUTF16[0] = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                        pUTF16   += C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                        utf16Cap -= C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                        pUTF8    += 1;
                    }
                } else {
                    if ((pUTF8[0] & 0xE0) == 0xC0) {
                        if (pUTF8[1] == 0) {
                            result = EINVAL; /* Input string is too short. */
                            break;
                        }

                        pUTF16[0] = ((c89str_utf16)(pUTF8[0] & 0x1F) <<  6) | (pUTF8[1] & 0x3F);
                        pUTF16   += 1;
                        utf16Cap -= 1;
                        pUTF8    += 2;
                    } else if ((pUTF8[0] & 0xF0) == 0xE0) {
                        if (pUTF8[1] == 0 || pUTF8[2] == 0) {
                            result = EINVAL; /* Input string is too short. */
                            break;
                        }

                        pUTF16[0] = ((c89str_utf16)(pUTF8[0] & 0x0F) << 12) | ((c89str_utf16)(pUTF8[1] & 0x3F) << 6) | (pUTF8[2] & 0x3F);
                        pUTF16   += 1;
                        utf16Cap -= 1;
                        pUTF8    += 3;
                    } else if ((pUTF8[0] & 0xF8) == 0xF0) {
                        if (utf16Cap < 2) {
                            break;  /* No enough room. */
                        } else {
                            unsigned int cp;
                            if (pUTF8[1] == 0 || pUTF8[2] == 0 || pUTF8[3] == 0) {
                                result = EINVAL; /* Input string is too short. */
                                break;
                            }

                            cp = ((c89str_utf32)(pUTF8[0] & 0x07) << 18) | ((c89str_utf32)(pUTF8[1] & 0x3F) << 12) | ((c89str_utf32)(pUTF8[2] & 0x3F) << 6) | (pUTF8[3] & 0x3F);
                            if (!c89str_is_valid_code_point(cp)) {
                                if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                    result = C89STR_ECODEPOINT;
                                    break;
                                } else {
                                    /* Replacement. */
                                    pUTF16[0] = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                                    pUTF16   += C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                                    utf16Cap -= C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                                    pUTF8    += 4;
                                }
                            } else {
                                c89str_utf32_cp_to_utf16_pair(cp, pUTF16);
                                pUTF16   += 2;
                                utf16Cap -= 2;
                                pUTF8    += 4;
                            }
                        }
                    } else {
                        if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                            result = C89STR_ECODEPOINT;
                            break;
                        } else {
                            /* Replacement. */
                            pUTF16[0] = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                            pUTF16   += C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                            utf16Cap -= C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                            pUTF8    += 1;
                        }
                    }
                }
            }
        }

        if (pUTF8LenProcessed != NULL) {
            *pUTF8LenProcessed = (pUTF8 - pUTF8Original);
        }
    } else {
        /* Fixed length string. */
        size_t iUTF8;
        for (iUTF8 = 0; iUTF8 < utf8Len; /* Do nothing */) {
            if (utf16Cap == 1) {
                result = ENOMEM;
                break;
            }

            if ((unsigned char)pUTF8[iUTF8+0] < 128) {   /* ASCII character. */
                pUTF16[0] = pUTF8[iUTF8+0];
                pUTF16   += 1;
                utf16Cap -= 1;
                iUTF8    += 1;
            } else {
                if (c89str_is_invalid_utf8_octet(pUTF8[iUTF8+0])) {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        /* Replacement. */
                        pUTF16[0] = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                        pUTF16   += C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                        utf16Cap -= C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                        iUTF8    += 1;
                    }
                } else {
                    if ((pUTF8[iUTF8+0] & 0xE0) == 0xC0) {
                        if (iUTF8+1 > utf8Len) {
                            result = EINVAL;
                            break;
                        }

                        pUTF16[0] = ((c89str_utf16)(pUTF8[iUTF8+0] & 0x1F) <<  6) | (pUTF8[iUTF8+1] & 0x3F);
                        pUTF16   += 1;
                        utf16Cap -= 1;
                        iUTF8    += 2;
                    } else if ((pUTF8[iUTF8+0] & 0xF0) == 0xE0) {
                        if (iUTF8+2 > utf8Len) {
                            result = EINVAL;
                            break;
                        }

                        pUTF16[0] = ((c89str_utf16)(pUTF8[iUTF8+0] & 0x0F) << 12) | ((c89str_utf16)(pUTF8[iUTF8+1] & 0x3F) << 6) | (pUTF8[iUTF8+2] & 0x3F);
                        pUTF16   += 1;
                        utf16Cap -= 1;
                        iUTF8    += 3;
                    } else if ((pUTF8[iUTF8+0] & 0xF8) == 0xF0) {
                        if (utf16Cap < 2) {
                            break;  /* No enough room. */
                        } else {
                            unsigned int cp;
                            if (iUTF8+3 > utf8Len) {
                                result = EINVAL;
                                break;
                            }

                            cp = ((c89str_utf32)(pUTF8[iUTF8+0] & 0x07) << 18) | ((c89str_utf32)(pUTF8[iUTF8+1] & 0x3F) << 12) | ((c89str_utf32)(pUTF8[iUTF8+2] & 0x3F) << 6) | (pUTF8[iUTF8+3] & 0x3F);
                            if (!c89str_is_valid_code_point(cp)) {
                                if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                    result = C89STR_ECODEPOINT;
                                    break;
                                } else {
                                    /* Replacement. */
                                    pUTF16[0] = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                                    pUTF16   += C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                                    utf16Cap -= C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                                    iUTF8    += 4;
                                }
                            } else {
                                c89str_utf32_cp_to_utf16_pair(cp, pUTF16);
                                pUTF16   += 2;
                                utf16Cap -= 2;
                                iUTF8    += 4;
                            }
                        }
                    } else {
                        if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                            result = C89STR_ECODEPOINT;
                            break;
                        } else {
                            /* Replacement. */
                            pUTF16[0] = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                            pUTF16   += C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                            utf16Cap -= C89STR_UNICODE_REPLACEMENT_CODE_POINT_LENGTH_UTF16;
                            iUTF8    += 1;
                        }
                    }
                }
            }
        }

        if (pUTF8LenProcessed != NULL) {
            *pUTF8LenProcessed = iUTF8;
        }
    }

    /* Null terminate. */
    if (utf16Cap == 0) {
        result = ENOMEM;    /* Not enough room in the output buffer. */
    } else {
        pUTF16[0] = 0;
    }

    if (pUTF16Len != NULL) {
        *pUTF16Len = (utf16CapOriginal - utf16Cap);
    }

    return result;
}

C89STR_API errno_t c89str_utf8_to_utf16le(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags)
{
    errno_t result;
    size_t utf16Len;

    /* Always do a native endian conversion first, then byte swap if necessary. */
    result = c89str_utf8_to_utf16ne(pUTF16, utf16Cap, &utf16Len, pUTF8, utf8Len, pUTF8LenProcessed, flags);

    if (pUTF16Len != NULL) {
        *pUTF16Len = utf16Len;
    }

    if (result != C89STR_SUCCESS) {
        return result;
    }

    if (pUTF16 != NULL && !c89str_is_little_endian()) {
        c89str_utf16_swap_endian(pUTF16, utf16Len);
    }

    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_utf8_to_utf16be(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags)
{
    errno_t result;
    size_t utf16Len;

    /* Always do a native endian conversion first, then byte swap if necessary. */
    result = c89str_utf8_to_utf16ne(pUTF16, utf16Cap, &utf16Len, pUTF8, utf8Len, pUTF8LenProcessed, flags);

    if (pUTF16Len != NULL) {
        *pUTF16Len = utf16Len;
    }

    if (result != C89STR_SUCCESS) {
        return result;
    }

    if (pUTF16 != NULL && !c89str_is_big_endian()) {
        c89str_utf16_swap_endian(pUTF16, utf16Len);
    }

    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_utf8_to_utf32_len(size_t* pUTF32Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags)
{
    errno_t result = C89STR_SUCCESS;
    size_t utf32Len = 0;

    if (pUTF32Len != NULL) {
        *pUTF32Len = 0;
    }

    if (pUTF8LenProcessed != NULL) {
        *pUTF8LenProcessed = 0;
    }

    if (pUTF8 == NULL) {
        return EINVAL;   /* Invalid input. */
    }

    if (utf8Len == 0 || pUTF8[0] == 0) {
        return C89STR_SUCCESS;   /* Empty input string. Length is always 0. */
    }

    /* Check for BOM. */
    if (c89str_utf8_has_bom((const unsigned char*)pUTF8, utf8Len)) {
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        pUTF8 += 3; /* Skip past the BOM. */
        if (utf8Len != (size_t)-1) {
            utf8Len -= 3;
        }
    }

    if (utf8Len == (size_t)-1) {
        /* Null terminated string. */
        const c89str_utf8* pUTF8Original = pUTF8;
        while (pUTF8[0] != 0) {
            utf32Len += 1;
            if ((unsigned char)pUTF8[0] < 128) {   /* ASCII character. */
                pUTF8 += 1;
            } else {
                if (c89str_is_invalid_utf8_octet(pUTF8[0])) {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        /* Replacement. */
                        pUTF8 += 1;
                    }
                } else {
                    if ((pUTF8[0] & 0xE0) == 0xC0) {
                        if (pUTF8[1] == 0) {
                            result = EINVAL; /* Input string is too short. */
                            break;
                        }
                        pUTF8 += 2;
                    } else if ((pUTF8[0] & 0xF0) == 0xE0) {
                        if (pUTF8[1] == 0 || pUTF8[2] == 0) {
                            result = EINVAL; /* Input string is too short. */
                            break;
                        }
                        pUTF8 += 3;
                    } else if ((pUTF8[0] & 0xF8) == 0xF0) {
                        unsigned int cp;
                        if (pUTF8[1] == 0 || pUTF8[2] == 0 || pUTF8[3] == 0) {
                            result = EINVAL; /* Input string is too short. */
                            break;
                        }
                        cp = ((c89str_utf32)(pUTF8[0] & 0x07) << 18) | ((c89str_utf32)(pUTF8[1] & 0x3F) << 12) | ((c89str_utf32)(pUTF8[2] & 0x3F) << 6) | (pUTF8[3] & 0x3F);
                        if (!c89str_is_valid_code_point(cp)) {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;
                                break;
                            } else {
                                /* Replacement. */
                            }
                        }
                        pUTF8 += 4;
                    } else {
                        if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                            result = C89STR_ECODEPOINT;
                            break;
                        } else {
                            /* Replacement. */
                            pUTF8 += 1;
                        }
                    }
                }
            }
        }

        if (pUTF8LenProcessed != NULL) {
            *pUTF8LenProcessed = (pUTF8 - pUTF8Original);
        }
    } else {
        /* Fixed length string. */
        size_t iUTF8;
        for (iUTF8 = 0; iUTF8 < utf8Len; /* Do nothing */) {
            utf32Len += 1;
            if ((unsigned char)pUTF8[iUTF8+0] < 128) {   /* ASCII character. */
                iUTF8 += 1;
            } else {
                if (c89str_is_invalid_utf8_octet(pUTF8[iUTF8+0])) {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        /* Replacement. */
                        iUTF8 += 1;
                    }
                } else {
                    if ((pUTF8[iUTF8+0] & 0xE0) == 0xC0) {
                        if (iUTF8+1 >= utf8Len) {
                            result = EINVAL;
                            break;
                        }
                        iUTF8 += 2;
                    } else if ((pUTF8[iUTF8+0] & 0xF0) == 0xE0) {
                        if (iUTF8+2 >= utf8Len) {
                            result = EINVAL;
                            break;
                        }
                        iUTF8 += 3;
                    } else if ((pUTF8[iUTF8+0] & 0xF8) == 0xF0) {
                        unsigned int cp;
                        if (iUTF8+3 >= utf8Len) {
                            result = EINVAL;
                            break;
                        }
                        cp = ((c89str_utf32)(pUTF8[0] & 0x07) << 18) | ((c89str_utf32)(pUTF8[1] & 0x3F) << 12) | ((c89str_utf32)(pUTF8[2] & 0x3F) << 6) | (pUTF8[3] & 0x3F);
                        if (!c89str_is_valid_code_point(cp)) {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;
                                break;
                            } else {
                                /* Replacement. */
                            }
                        }

                        iUTF8 += 4;
                    } else {
                        if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                            result = C89STR_ECODEPOINT;
                            break;
                        } else {
                            /* Replacement. */
                            iUTF8 += 1;
                        }
                    }
                }
            }
        }

        if (pUTF8LenProcessed != NULL) {
            *pUTF8LenProcessed = iUTF8;
        }
    }

    if (pUTF32Len != NULL) {
        *pUTF32Len = utf32Len;
    }

    return result;
}


C89STR_API errno_t c89str_utf8_to_utf32ne(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags)
{
    errno_t result = C89STR_SUCCESS;
    size_t utf32CapOriginal = utf32Cap;

    if (pUTF32 == NULL) {
        return c89str_utf8_to_utf32_len(pUTF32Len, pUTF8, utf8Len, pUTF8LenProcessed, flags);
    }

    if (pUTF32Len != NULL) {
        *pUTF32Len = 0;
    }

    if (pUTF8LenProcessed != NULL) {
        *pUTF8LenProcessed = 0;
    }

    if (pUTF8 == NULL) {
        return EINVAL;
    }

    /* Check for BOM. */
    if (c89str_utf8_has_bom((const unsigned char*)pUTF8, utf8Len)) {
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        pUTF8 += 3; /* Skip past the BOM. */
        if (utf8Len != (size_t)-1) {
            utf8Len -= 3;
        }
    }

    if (utf8Len == (size_t)-1) {
        /* Null terminated string. */
        const c89str_utf8* pUTF8Original = pUTF8;
        while (pUTF8[0] != 0) {
            if (utf32Cap == 0) {
                result = ENOMEM;
                break;
            }

            if ((unsigned char)pUTF8[0] < 128) {   /* ASCII character. */
                pUTF32[0] = pUTF8[0];
                pUTF8 += 1;
            } else {
                if (c89str_is_invalid_utf8_octet(pUTF8[0])) {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        /* Replacement. */
                        pUTF32[0] = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                        pUTF8 += 1;
                    }
                } else {
                    if ((pUTF8[0] & 0xE0) == 0xC0) {
                        if (pUTF8[1] == 0) {
                            result = EINVAL; /* Input string is too short. */
                            break;
                        }

                        pUTF32[0] = ((c89str_utf16)(pUTF8[0] & 0x1F) <<  6) | (pUTF8[1] & 0x3F);
                        pUTF8 += 2;
                    } else if ((pUTF8[0] & 0xF0) == 0xE0) {
                        if (pUTF8[1] == 0 || pUTF8[2] == 0) {
                            result = EINVAL; /* Input string is too short. */
                            break;
                        }

                        pUTF32[0] = ((c89str_utf16)(pUTF8[0] & 0x0F) << 12) | ((c89str_utf16)(pUTF8[1] & 0x3F) << 6) | (pUTF8[2] & 0x3F);
                        pUTF8 += 3;
                    } else if ((pUTF8[0] & 0xF8) == 0xF0) {
                        if (pUTF8[1] == 0 || pUTF8[2] == 0 || pUTF8[3] == 0) {
                            result = EINVAL; /* Input string is too short. */
                            break;
                        }

                        pUTF32[0] = ((c89str_utf32)(pUTF8[0] & 0x07) << 18) | ((c89str_utf32)(pUTF8[1] & 0x3F) << 12) | ((c89str_utf32)(pUTF8[2] & 0x3F) << 6) | (pUTF8[3] & 0x3F);
                        pUTF8 += 4;

                        if (!c89str_is_valid_code_point(pUTF32[0])) {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;   /* No characters should be in the UTF-16 surrogate pair range. */
                                break;
                            } else {
                                /* Replacement. */
                                pUTF32[0] = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                            }
                        }
                    } else {
                        if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                            result = C89STR_ECODEPOINT;
                            break;
                        } else {
                            /* Replacement. */
                            pUTF32[0] = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                            pUTF8 += 1;
                        }
                    }
                }
            }

            pUTF32   += 1;
            utf32Cap -= 1;
        }

        if (pUTF8LenProcessed != NULL) {
            *pUTF8LenProcessed = (pUTF8 - pUTF8Original);
        }
    } else {
        /* Fixed length string. */
        size_t iUTF8;
        for (iUTF8 = 0; iUTF8 < utf8Len; /* Do nothing */) {
            if (utf32Cap == 0) {
                result = ENOMEM;
                break;
            }

            if ((unsigned char)pUTF8[iUTF8+0] < 128) {   /* ASCII character. */
                pUTF32[0] = pUTF8[iUTF8+0];
                iUTF8 += 1;
            } else {
                if (c89str_is_invalid_utf8_octet(pUTF8[iUTF8+0])) {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        /* Replacement. */
                        pUTF32[0] = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                        iUTF8 += 1;
                    }
                } else {
                    if ((pUTF8[iUTF8+0] & 0xE0) == 0xC0) {
                        if (iUTF8+1 >= utf8Len) {
                            result = EINVAL;
                            break;
                        }

                        pUTF32[0] = ((c89str_utf16)(pUTF8[iUTF8+0] & 0x1F) <<  6) | (pUTF8[iUTF8+1] & 0x3F);
                        iUTF8 += 2;
                    } else if ((pUTF8[iUTF8+0] & 0xF0) == 0xE0) {
                        if (iUTF8+2 >= utf8Len) {
                            result = EINVAL;
                            break;
                        }

                        pUTF32[0] = ((c89str_utf16)(pUTF8[iUTF8+0] & 0x0F) << 12) | ((c89str_utf16)(pUTF8[iUTF8+1] & 0x3F) << 6) | (pUTF8[iUTF8+2] & 0x3F);
                        iUTF8 += 3;
                    } else if ((pUTF8[iUTF8+0] & 0xF8) == 0xF0) {
                        if (iUTF8+3 >= utf8Len) {
                            result = EINVAL;
                            break;
                        }

                        pUTF32[0] = ((c89str_utf32)(pUTF8[iUTF8+0] & 0x07) << 18) | ((c89str_utf32)(pUTF8[iUTF8+1] & 0x3F) << 12) | ((c89str_utf32)(pUTF8[iUTF8+2] & 0x3F) << 6) | (pUTF8[iUTF8+3] & 0x3F);
                        iUTF8 += 4;

                        if (!c89str_is_valid_code_point(pUTF32[0])) {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;
                                break;
                            } else {
                                /* Replacement. */
                                pUTF32[0] = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                            }
                        }
                    } else {
                        if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                            result = C89STR_ECODEPOINT;
                            break;
                        } else {
                            /* Replacement. */
                            pUTF32[0] = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                            iUTF8 += 1;
                        }
                    }
                }
            }

            pUTF32   += 1;
            utf32Cap -= 1;
        }

        if (pUTF8LenProcessed != NULL) {
            *pUTF8LenProcessed = iUTF8;
        }
    }

    /* Null terminate. */
    if (utf32Cap == 0) {
        result = ENOMEM;    /* Not enough room in the output buffer. */
    } else {
        pUTF32[0] = 0;
    }

    if (pUTF32Len != NULL) {
        *pUTF32Len = (utf32CapOriginal - utf32Cap);
    }

    return result;
}

C89STR_API errno_t c89str_utf8_to_utf32le(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags)
{
    errno_t result;
    size_t utf32Len;

    /* Always do a native endian conversion first, then byte swap if necessary. */
    result = c89str_utf8_to_utf32ne(pUTF32, utf32Cap, &utf32Len, pUTF8, utf8Len, pUTF8LenProcessed, flags);

    if (pUTF32Len != NULL) {
        *pUTF32Len = utf32Len;
    }

    if (result != C89STR_SUCCESS) {
        return result;
    }

    if (pUTF32 != NULL && !c89str_is_little_endian()) {
        c89str_utf32_swap_endian(pUTF32, utf32Len);
    }

    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_utf8_to_utf32be(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags)
{
    errno_t result;
    size_t utf32Len;

    /* Always do a native endian conversion first, then byte swap if necessary. */
    result = c89str_utf8_to_utf32ne(pUTF32, utf32Cap, &utf32Len, pUTF8, utf8Len, pUTF8LenProcessed, flags);

    if (pUTF32Len != NULL) {
        *pUTF32Len = utf32Len;
    }

    if (result != C89STR_SUCCESS) {
        return result;
    }

    if (pUTF32 != NULL && !c89str_is_big_endian()) {
        c89str_utf32_swap_endian(pUTF32, utf32Len);
    }

    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_utf8_to_wchar_len(size_t* pWCHARLen, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags)
{
#if C89STR_SIZEOF_WCHAR_T == 2
    return c89str_utf8_to_utf16_len(pWCHARLen, pUTF8, utf8Len, pUTF8LenProcessed, flags);
#else
    return c89str_utf8_to_utf32_len(pWCHARLen, pUTF8, utf8Len, pUTF8LenProcessed, flags);
#endif
}

C89STR_API errno_t c89str_utf8_to_wchar(wchar_t* pWCHAR, size_t wcharCap, size_t* pWCHARLen, const c89str_utf8* pUTF8, size_t utf8Len, size_t* pUTF8LenProcessed, unsigned int flags)
{
#if C89STR_SIZEOF_WCHAR_T == 2
    return c89str_utf8_to_utf16((c89str_utf16*)pWCHAR, wcharCap, pWCHARLen, pUTF8, utf8Len, pUTF8LenProcessed, flags);
#else
    return c89str_utf8_to_utf32((c89str_utf32*)pWCHAR, wcharCap, pWCHARLen, pUTF8, utf8Len, pUTF8LenProcessed, flags);
#endif
}



C89STR_API errno_t c89str_utf16_to_utf8_len_internal(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags, c89str_bool32 isLE)
{
    errno_t result = C89STR_SUCCESS;
    size_t utf8Len = 0;
    c89str_utf16 w1;
    c89str_utf16 w2;
    c89str_utf32 utf32;

    if (pUTF8Len != NULL) {
        *pUTF8Len = 0;
    }

    if (pUTF16LenProcessed != NULL) {
        *pUTF16LenProcessed = 0;
    }

    if (pUTF16 == NULL) {
        return EINVAL; /* Invalid input. */
    }

    if (utf16Len == 0 || pUTF16[0] == 0) {
        return C89STR_SUCCESS;  /* Empty input string. Length is always 0. */
    }

    /* Check for BOM. */
    if (c89str_utf16_has_bom((const unsigned char*)pUTF16, utf16Len)) {
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        pUTF16 += 1; /* Skip past the BOM. */
        if (utf16Len != (size_t)-1) {
            utf16Len -= 1;
        }
    }

    if (utf16Len == (size_t)-1) {
        /* Null terminated string. */
        const c89str_utf16* pUTF16Original = pUTF16;
        while (pUTF16[0] != 0) {
            if (isLE) {
                w1 = c89str_le2host_16(pUTF16[0]);
            } else {
                w1 = c89str_be2host_16(pUTF16[0]);
            }
            
            if (w1 < 0xD800 || w1 > 0xDFFF) {
                /* 1 UTF-16 code unit. */
                utf32 = w1;
                pUTF16 += 1;
            } else {
                /* 2 UTF-16 code units, or an error. */
                if (w1 >= 0xD800 && w1 <= 0xDBFF) {
                    if (pUTF16[1] == 0) {
                        result = EINVAL; /* Ran out of input data. */
                        break;
                    } else {
                        if (isLE) {
                            w2 = c89str_le2host_16(pUTF16[1]);
                        } else {
                            w2 = c89str_be2host_16(pUTF16[1]);
                        }
                    
                        if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
                            utf32 = c89str_utf16_pair_to_utf32_cp(pUTF16);
                        } else {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;
                                break;
                            } else {
                                /* Replacement. */
                                utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                            }
                        }

                        pUTF16 += 2;
                    }
                } else {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        /* Replacement. */
                        utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                    }

                    pUTF16 += 1;
                }
            }

            utf8Len += c89str_utf32_cp_to_utf8_len(utf32);
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = (pUTF16 - pUTF16Original);
        }
    } else {
        /* Fixed length string. */
        size_t iUTF16;
        for (iUTF16 = 0; iUTF16 < utf16Len; /* Do nothing */) {
            if (isLE) {
                w1 = c89str_le2host_16(pUTF16[iUTF16+0]);
            } else {
                w1 = c89str_be2host_16(pUTF16[iUTF16+0]);
            }

            if (w1 < 0xD800 || w1 > 0xDFFF) {
                /* 1 UTF-16 code unit. */
                utf32 = w1;
                iUTF16 += 1;
            } else {
                /* 2 UTF-16 code units, or an error. */
                if (w1 >= 0xD800 && w1 <= 0xDBFF) {
                    if (iUTF16+1 > utf16Len) {
                        result = EINVAL; /* Ran out of input data. */
                        break;
                    } else {
                        if (isLE) {
                            w2 = c89str_le2host_16(pUTF16[iUTF16+1]);
                        } else {
                            w2 = c89str_be2host_16(pUTF16[iUTF16+1]);
                        }
                    
                        if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
                            utf32 = c89str_utf16_pair_to_utf32_cp(pUTF16 + iUTF16);
                        } else {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;
                                break;
                            } else {
                                /* Replacement. */
                                utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                            }
                        }

                        iUTF16 += 2;
                    }
                } else {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                    }

                    iUTF16 += 1;
                }
            }

            utf8Len += c89str_utf32_cp_to_utf8_len(utf32);
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = iUTF16;
        }
    }

    if (pUTF8Len != NULL) {
        *pUTF8Len = utf8Len;
    }

    return result;
}

C89STR_API errno_t c89str_utf16ne_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return c89str_utf16le_to_utf8_len(pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    } else {
        return c89str_utf16be_to_utf8_len(pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    }
}

C89STR_API errno_t c89str_utf16le_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    return c89str_utf16_to_utf8_len_internal(pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t c89str_utf16be_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    return c89str_utf16_to_utf8_len_internal(pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, C89STR_FALSE);
}

C89STR_API errno_t c89str_utf16_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    if (pUTF8Len != NULL) {
        *pUTF8Len = 0;
    }

    /* Check for BOM. */
    if (c89str_utf16_has_bom((const unsigned char*)pUTF16, utf16Len)) {
        errno_t result;
        size_t utf16LenProcessed;

        c89str_bool32 isLE;
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        /* With this function, we need to use the endian defined by the BOM. */
        isLE = c89str_utf16_is_bom_le((const unsigned char*)pUTF16);
        
        pUTF16 += 1;    /* Skip past the BOM. */
        if (utf16Len != (size_t)-1) {
            utf16Len -= 1;
        }

        if (isLE) {
            result = c89str_utf16le_to_utf8_len(pUTF8Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = c89str_utf16be_to_utf8_len(pUTF8Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = utf16LenProcessed + 1;    /* +1 for BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return c89str_utf16ne_to_utf8_len(pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
}


C89STR_API errno_t c89str_utf16_to_utf8_internal(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags, c89str_bool32 isLE)
{
    errno_t result = C89STR_SUCCESS;
    size_t utf8CapOriginal = utf8Cap;
    c89str_utf16 w1;
    c89str_utf16 w2;
    c89str_utf32 utf32;
    size_t utf8cpLen;   /* Code point length in UTF-8 code units. */

    if (pUTF8 == NULL) {
        return c89str_utf16_to_utf8_len_internal(pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, isLE);
    }

    if (pUTF8Len != NULL) {
        *pUTF8Len = 0;
    }

    if (pUTF16LenProcessed != NULL) {
        *pUTF16LenProcessed = 0;
    }

    if (pUTF8 == NULL) {
        return EINVAL;
    }

    /* Check for BOM. */
    if (c89str_utf16_has_bom((const unsigned char*)pUTF16, utf16Len)) {
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        pUTF16 += 1;    /* Skip past the BOM. */
        if (utf16Len != (size_t)-1) {
            utf16Len -= 1;
        }
    }

    if (utf16Len == (size_t)-1) {
        /* Null terminated string. */
        const c89str_utf16* pUTF16Original = pUTF16;
        while (pUTF16[0] != 0) {
            if (utf8Cap == 0) {
                result = ENOMEM;
                break;
            }

            if (isLE) {
                w1 = c89str_le2host_16(pUTF16[0]);
            } else {
                w1 = c89str_be2host_16(pUTF16[0]);
            }
            
            if (w1 < 0xD800 || w1 > 0xDFFF) {
                /* 1 UTF-16 code unit. */
                utf32 = w1;
                pUTF16 += 1;
            } else {
                /* 2 UTF-16 code units, or an error. */
                if (w1 >= 0xD800 && w1 <= 0xDBFF) {
                    if (pUTF16[1] == 0) {
                        result = EINVAL; /* Ran out of input data. */
                        break;
                    } else {
                        if (isLE) {
                            w2 = c89str_le2host_16(pUTF16[1]);
                        } else {
                            w2 = c89str_be2host_16(pUTF16[1]);
                        }
                    
                        if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
                            utf32 = c89str_utf16_pair_to_utf32_cp(pUTF16);
                        } else {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;
                                break;
                            } else {
                                /* Replacement. */
                                utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                            }
                        }

                        pUTF16 += 2;
                    }
                } else {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        /* Replacement. */
                        utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                    }

                    pUTF16 += 1;
                }
            }

            utf8cpLen = c89str_utf32_cp_to_utf8(utf32, pUTF8, utf8Cap);
            if (utf8cpLen == 0) {
                result = ENOMEM;    /* A return value of 0 at this point means there was not enough room in the output buffer. */
                break;
            }

            pUTF8   += utf8cpLen;
            utf8Cap -= utf8cpLen;
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = (pUTF16 - pUTF16Original);
        }
    } else {
        /* Fixed length string. */
        size_t iUTF16;
        for (iUTF16 = 0; iUTF16 < utf16Len; /* Do nothing */) {
            if (utf8Cap == 0) {
                result = ENOMEM;
                break;
            }

            if (isLE) {
                w1 = c89str_le2host_16(pUTF16[iUTF16+0]);
            } else {
                w1 = c89str_be2host_16(pUTF16[iUTF16+0]);
            }

            if (w1 < 0xD800 || w1 > 0xDFFF) {
                /* 1 UTF-16 code unit. */
                utf32 = w1;
                iUTF16 += 1;
            } else {
                /* 2 UTF-16 code units, or an error. */
                if (w1 >= 0xD800 && w1 <= 0xDBFF) {
                    if (iUTF16+1 > utf16Len) {
                        result = EINVAL; /* Ran out of input data. */
                        break;
                    } else {
                        if (isLE) {
                            w2 = c89str_le2host_16(pUTF16[iUTF16+1]);
                        } else {
                            w2 = c89str_be2host_16(pUTF16[iUTF16+1]);
                        }
                    
                        if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
                            utf32 = c89str_utf16_pair_to_utf32_cp(pUTF16 + iUTF16);
                        } else {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;
                                break;
                            } else {
                                /* Replacement. */
                                utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                            }
                        }

                        iUTF16 += 2;
                    }
                } else {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        /* Replacement. */
                        utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                    }

                    iUTF16 += 1;
                }
            }

            utf8cpLen = c89str_utf32_cp_to_utf8(utf32, pUTF8, utf8Cap);
            if (utf8cpLen == 0) {
                result = ENOMEM;    /* A return value of 0 at this point means there was not enough room in the output buffer. */
                break;
            }

            pUTF8   += utf8cpLen;
            utf8Cap -= utf8cpLen;
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = iUTF16;
        }
    }
    
    /* Null terminate. */
    if (utf8Cap == 0) {
        result = ENOMEM;    /* Not enough room in the output buffer. */
    } else {
        pUTF8[0] = 0;
    }

    if (pUTF8Len != NULL) {
        *pUTF8Len = (utf8CapOriginal - utf8Cap);
    }

    return result;
}

C89STR_API errno_t c89str_utf16ne_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return c89str_utf16le_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    } else {
        return c89str_utf16be_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    }
}

C89STR_API errno_t c89str_utf16le_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    return c89str_utf16_to_utf8_internal(pUTF8, utf8Cap, pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t c89str_utf16be_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    return c89str_utf16_to_utf8_internal(pUTF8, utf8Cap, pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, C89STR_FALSE);
}

C89STR_API errno_t c89str_utf16_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    if (pUTF8 == NULL) {
        return c89str_utf16_to_utf8_len(pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    }

    if (pUTF8Len != NULL) {
        *pUTF8Len = 0;
    }

    if (pUTF8 == NULL) {
        return EINVAL;
    }

    /* Check for BOM. */
    if (c89str_utf16_has_bom((const unsigned char*)pUTF16, utf16Len)) {
        errno_t result;
        size_t utf16LenProcessed;

        c89str_bool32 isLE;
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        /* With this function, we need to use the endian defined by the BOM. */
        isLE = c89str_utf16_is_bom_le((const unsigned char*)pUTF16);
        
        pUTF16 += 1;    /* Skip past the BOM. */
        if (utf16Len != (size_t)-1) {
            utf16Len -= 1;
        }

        if (isLE) {
            result = c89str_utf16le_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = c89str_utf16be_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = utf16LenProcessed + 1;    /* +1 for BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return c89str_utf16ne_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
}


C89STR_API errno_t c89str_utf16_to_utf32_len_internal(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags, c89str_bool32 isLE)
{
    errno_t result = C89STR_SUCCESS;
    size_t utf32Len = 0;
    c89str_utf16 w1;
    c89str_utf16 w2;

    if (pUTF32Len != NULL) {
        *pUTF32Len = 0;
    }

    if (pUTF16LenProcessed != NULL) {
        *pUTF16LenProcessed = 0;
    }

    if (pUTF16 == NULL) {
        return EINVAL; /* Invalid input. */
    }

    if (utf16Len == 0 || pUTF16[0] == 0) {
        return C89STR_SUCCESS;  /* Empty input string. Length is always 0. */
    }

    /* Check for BOM. */
    if (c89str_utf16_has_bom((const unsigned char*)pUTF16, utf16Len)) {
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        pUTF16 += 1; /* Skip past the BOM. */
        if (utf16Len != (size_t)-1) {
            utf16Len -= 1;
        }
    }

    if (utf16Len == (size_t)-1) {
        /* Null terminated string. */
        const c89str_utf16* pUTF16Original = pUTF16;
        while (pUTF16[0] != 0) {
            if (isLE) {
                w1 = c89str_le2host_16(pUTF16[0]);
            } else {
                w1 = c89str_be2host_16(pUTF16[0]);
            }
            
            if (w1 < 0xD800 || w1 > 0xDFFF) {
                /* 1 UTF-16 code unit. */
                pUTF16 += 1;
            } else {
                /* 2 UTF-16 code units, or an error. */
                if (w1 >= 0xD800 && w1 <= 0xDBFF) {
                    if (pUTF16[1] == 0) {
                        result = EINVAL; /* Ran out of input data. */
                        break;
                    } else {
                        if (isLE) {
                            w2 = c89str_le2host_16(pUTF16[1]);
                        } else {
                            w2 = c89str_be2host_16(pUTF16[1]);
                        }
                    
                        /* Check for invalid code point. */
                        if (!(w2 >= 0xDC00 && w2 <= 0xDFFF)) {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;
                                break;
                            }
                        }

                        pUTF16 += 2;
                    }
                } else {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    }

                    pUTF16 += 1;
                }
            }

            utf32Len += 1;
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = (pUTF16 - pUTF16Original);
        }
    } else {
        /* Fixed length string. */
        size_t iUTF16;
        for (iUTF16 = 0; iUTF16 < utf16Len; /* Do nothing */) {
            if (isLE) {
                w1 = c89str_le2host_16(pUTF16[iUTF16+0]);
            } else {
                w1 = c89str_be2host_16(pUTF16[iUTF16+0]);
            }

            if (w1 < 0xD800 || w1 > 0xDFFF) {
                /* 1 UTF-16 code unit. */
                iUTF16 += 1;
            } else {
                /* 2 UTF-16 code units, or an error. */
                if (w1 >= 0xD800 && w1 <= 0xDBFF) {
                    if (iUTF16+1 > utf16Len) {
                        result = EINVAL; /* Ran out of input data. */
                        break;
                    } else {
                        if (isLE) {
                            w2 = c89str_le2host_16(pUTF16[iUTF16+1]);
                        } else {
                            w2 = c89str_be2host_16(pUTF16[iUTF16+1]);
                        }
                    
                        /* Check for invalid code point. */
                        if (!(w2 >= 0xDC00 && w2 <= 0xDFFF)) {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;
                                break;
                            }
                        }

                        iUTF16 += 2;
                    }
                } else {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    }

                    iUTF16 += 1;
                }
            }

            utf32Len += 1;
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = iUTF16;
        }
    }

    if (pUTF32Len != NULL) {
        *pUTF32Len = utf32Len;
    }

    return result;
}

C89STR_API errno_t c89str_utf16ne_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return c89str_utf16le_to_utf32_len(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    } else {
        return c89str_utf16be_to_utf32_len(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    }
}

C89STR_API errno_t c89str_utf16le_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    return c89str_utf16_to_utf32_len_internal(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t c89str_utf16be_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    return c89str_utf16_to_utf32_len_internal(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, C89STR_FALSE);
}

C89STR_API errno_t c89str_utf16_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    if (pUTF32Len != NULL) {
        *pUTF32Len = 0;
    }

    /* Check for BOM. */
    if (c89str_utf16_has_bom((const unsigned char*)pUTF16, utf16Len)) {
        errno_t result;
        size_t utf16LenProcessed;

        c89str_bool32 isLE;
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        /* With this function, we need to use the endian defined by the BOM. */
        isLE = c89str_utf16_is_bom_le((const unsigned char*)pUTF16);
        
        pUTF16 += 1;    /* Skip past the BOM. */
        if (utf16Len != (size_t)-1) {
            utf16Len -= 1;
        }

        if (isLE) {
            result = c89str_utf16le_to_utf32_len(pUTF32Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = c89str_utf16be_to_utf32_len(pUTF32Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = utf16LenProcessed + 1;
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return c89str_utf16ne_to_utf32_len(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
}


C89STR_API errno_t c89str_utf16_to_utf32_internal(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags, c89str_bool32 isLE)
{
    errno_t result = C89STR_SUCCESS;
    size_t utf32CapOriginal = utf32Cap;
    c89str_utf16 w1;
    c89str_utf16 w2;
    c89str_utf32 utf32;

    if (pUTF32 == NULL) {
        return c89str_utf16_to_utf32_len_internal(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, isLE);
    }

    if (pUTF32Len != NULL) {
        *pUTF32Len = 0;
    }

    if (pUTF16LenProcessed != NULL) {
        *pUTF16LenProcessed = 0;
    }

    if (pUTF32 == NULL) {
        return EINVAL;
    }

    /* Check for BOM. */
    if (c89str_utf16_has_bom((const unsigned char*)pUTF16, utf16Len)) {
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        pUTF16 += 1;    /* Skip past the BOM. */
        if (utf16Len != (size_t)-1) {
            utf16Len -= 1;
        }
    }

    if (utf16Len == (size_t)-1) {
        /* Null terminated string. */
        const c89str_utf16* pUTF16Original = pUTF16;
        while (pUTF16[0] != 0) {
            if (utf32Cap == 0) {
                result = ENOMEM;
                break;
            }

            if (isLE) {
                w1 = c89str_le2host_16(pUTF16[0]);
            } else {
                w1 = c89str_be2host_16(pUTF16[0]);
            }
            
            if (w1 < 0xD800 || w1 > 0xDFFF) {
                /* 1 UTF-16 code unit. */
                utf32 = w1;
                pUTF16 += 1;
            } else {
                /* 2 UTF-16 code units, or an error. */
                if (w1 >= 0xD800 && w1 <= 0xDBFF) {
                    if (pUTF16[1] == 0) {
                        result = EINVAL; /* Ran out of input data. */
                        break;
                    } else {
                        if (isLE) {
                            w2 = c89str_le2host_16(pUTF16[1]);
                        } else {
                            w2 = c89str_be2host_16(pUTF16[1]);
                        }
                    
                        if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
                            utf32 = c89str_utf16_pair_to_utf32_cp(pUTF16);
                        } else {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;
                                break;
                            } else {
                                /* Replacement. */
                                utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                            }
                        }

                        pUTF16 += 2;
                    }
                } else {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        /* Replacement. */
                        utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                    }

                    pUTF16 += 1;
                }
            }

            if (isLE) {
                pUTF32[0] = c89str_host2le_32(utf32);
            } else {
                pUTF32[0] = c89str_host2be_32(utf32);
            }
            
            pUTF32   += 1;
            utf32Cap -= 1;
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = (pUTF16 - pUTF16Original);
        }
    } else {
        /* Fixed length string. */
        size_t iUTF16;
        for (iUTF16 = 0; iUTF16 < utf16Len; /* Do nothing */) {
            if (utf32Cap == 0) {
                result = ENOMEM;
                break;
            }

            if (isLE) {
                w1 = c89str_le2host_16(pUTF16[iUTF16+0]);
            } else {
                w1 = c89str_be2host_16(pUTF16[iUTF16+0]);
            }

            if (w1 < 0xD800 || w1 > 0xDFFF) {
                /* 1 UTF-16 code unit. */
                utf32 = w1;
                iUTF16 += 1;
            } else {
                /* 2 UTF-16 code units, or an error. */
                if (w1 >= 0xD800 && w1 <= 0xDBFF) {
                    if (iUTF16+1 > utf16Len) {
                        result = EINVAL; /* Ran out of input data. */
                        break;
                    } else {
                        if (isLE) {
                            w2 = c89str_le2host_16(pUTF16[iUTF16+1]);
                        } else {
                            w2 = c89str_be2host_16(pUTF16[iUTF16+1]);
                        }
                    
                        if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
                            utf32 = c89str_utf16_pair_to_utf32_cp(pUTF16 + iUTF16);
                        } else {
                            if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                                result = C89STR_ECODEPOINT;
                                break;
                            } else {
                                /* Replacement. */
                                utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                            }
                        }

                        iUTF16 += 2;
                    }
                } else {
                    if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                        result = C89STR_ECODEPOINT;
                        break;
                    } else {
                        /* Replacement. */
                        utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                    }

                    iUTF16 += 1;
                }
            }

            if (isLE) {
                pUTF32[0] = c89str_host2le_32(utf32);
            } else {
                pUTF32[0] = c89str_host2be_32(utf32);
            }

            pUTF32   += 1;
            utf32Cap -= 1;
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = iUTF16;
        }
    }
    
    /* Null terminate. */
    if (utf32Cap == 0) {
        result = ENOMEM;    /* Not enough room in the output buffer. */
    } else {
        pUTF32[0] = 0;
    }

    if (pUTF32Len != NULL) {
        *pUTF32Len = (utf32CapOriginal - utf32Cap);
    }

    return result;
}

C89STR_API errno_t c89str_utf16ne_to_utf32ne(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return c89str_utf16le_to_utf32le(pUTF32, utf32Cap, pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    } else {
        return c89str_utf16be_to_utf32be(pUTF32, utf32Cap, pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    }
}

C89STR_API errno_t c89str_utf16le_to_utf32le(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    return c89str_utf16_to_utf32_internal(pUTF32, utf32Cap, pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t c89str_utf16be_to_utf32be(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    return c89str_utf16_to_utf32_internal(pUTF32, utf32Cap, pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, C89STR_FALSE);
}

C89STR_API errno_t c89str_utf16_to_utf32(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    if (pUTF32 == NULL) {
        return c89str_utf16_to_utf32_len(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    }

    if (pUTF32Len != NULL) {
        *pUTF32Len = 0;
    }

    if (pUTF32 == NULL) {
        return EINVAL;
    }

    /* Check for BOM. */
    if (c89str_utf16_has_bom((const unsigned char*)pUTF16, utf16Len)) {
        errno_t result;
        size_t utf16LenProcessed;

        c89str_bool32 isLE;
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        /* With this function, we need to use the endian defined by the BOM. */
        isLE = c89str_utf16_is_bom_le((const unsigned char*)pUTF16);
        
        pUTF16 += 1;    /* Skip past the BOM. */
        if (utf16Len != (size_t)-1) {
            utf16Len -= 1;
        }

        if (isLE) {
            result = c89str_utf16le_to_utf32le(pUTF32, utf32Cap, pUTF32Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = c89str_utf16be_to_utf32be(pUTF32, utf32Cap, pUTF32Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = utf16LenProcessed + 1;    /* +1 for BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return c89str_utf16ne_to_utf32ne(pUTF32, utf32Cap, pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
}


C89STR_API errno_t c89str_utf32_to_utf8_len_internal(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags, c89str_bool32 isLE)
{
    errno_t result = C89STR_SUCCESS;
    size_t utf8Len = 0;
    c89str_utf32 utf32;

    if (pUTF8Len != NULL) {
        *pUTF8Len = 0;
    }

    if (pUTF32LenProcessed != NULL) {
        *pUTF32LenProcessed = 0;
    }

    if (pUTF32 == NULL) {
        return EINVAL; /* Invalid input. */
    }

    if (utf32Len == 0 || pUTF32[0] == 0) {
        return C89STR_SUCCESS;  /* Empty input string. Length is always 0. */
    }

    /* Check for BOM. */
    if (c89str_utf32_has_bom((const unsigned char*)pUTF32, utf32Len)) {
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        pUTF32 += 1;    /* Skip past the BOM. */
        if (utf32Len != (size_t)-1) {
            utf32Len -= 1;
        }
    }

    if (utf32Len == (size_t)-1) {
        /* Null terminated string. */
        const c89str_utf32* pUTF32Original = pUTF32;
        while (pUTF32[0] != 0) {
            if (isLE) {
                utf32 = c89str_le2host_32(pUTF32[0]);
            } else {
                utf32 = c89str_be2host_32(pUTF32[0]);
            }

            if (!c89str_is_valid_code_point(utf32)) {
                if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                    result = C89STR_ECODEPOINT;
                    break;
                } else {
                    utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                }
            }

            utf8Len += c89str_utf32_cp_to_utf8_len(utf32);
            pUTF32  += 1;
        }

        if (pUTF32LenProcessed != NULL) {
            *pUTF32LenProcessed = (pUTF32 - pUTF32Original);
        }
    } else {
        /* Fixed length string. */
        size_t iUTF32;
        for (iUTF32 = 0; iUTF32 < utf32Len; iUTF32 += 1) {
            if (isLE) {
                utf32 = c89str_le2host_32(pUTF32[iUTF32]);
            } else {
                utf32 = c89str_be2host_32(pUTF32[iUTF32]);
            }

            if (!c89str_is_valid_code_point(utf32)) {
                if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                    result = C89STR_ECODEPOINT;
                    break;
                } else {
                    utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                }
            }

            utf8Len += c89str_utf32_cp_to_utf8_len(utf32);
        }

        if (pUTF32LenProcessed != NULL) {
            *pUTF32LenProcessed = iUTF32;
        }
    }

    if (pUTF8Len != NULL) {
        *pUTF8Len = utf8Len;
    }

    return result;
}

C89STR_API errno_t c89str_utf32ne_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return c89str_utf32le_to_utf8_len(pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    } else {
        return c89str_utf32be_to_utf8_len(pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    }
}

C89STR_API errno_t c89str_utf32le_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    return c89str_utf32_to_utf8_len_internal(pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t c89str_utf32be_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    return c89str_utf32_to_utf8_len_internal(pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, C89STR_FALSE);
}

C89STR_API errno_t c89str_utf32_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    if (pUTF8Len != NULL) {
        *pUTF8Len = 0;
    }

    /* Check for BOM. */
    if (c89str_utf32_has_bom((const unsigned char*)pUTF32, utf32Len)) {
        errno_t result;
        size_t utf32LenProcessed;

        c89str_bool32 isLE;
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        /* With this function, we need to use the endian defined by the BOM. */
        isLE = c89str_utf32_is_bom_le((const unsigned char*)pUTF32);
        
        pUTF32 += 1;    /* Skip past the BOM. */
        if (utf32Len != (size_t)-1) {
            utf32Len -= 1;
        }

        if (isLE) {
            result = c89str_utf32le_to_utf8_len(pUTF8Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = c89str_utf32be_to_utf8_len(pUTF8Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF32LenProcessed) {
            *pUTF32LenProcessed = utf32LenProcessed + 1;    /* +1 for the BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return c89str_utf32ne_to_utf8_len(pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
}


C89STR_API errno_t c89str_utf32_to_utf8_internal(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags, c89str_bool32 isLE)
{
    errno_t result = C89STR_SUCCESS;
    size_t utf8CapOriginal = utf8Cap;
    size_t utf8cpLen;   /* Code point length in UTF-8 code units. */
    c89str_utf32 utf32;

    if (pUTF8 == NULL) {
        return c89str_utf32_to_utf8_internal(pUTF8, utf8Cap, pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, isLE);
    }

    if (pUTF8Len != NULL) {
        *pUTF8Len = 0;
    }

    if (pUTF32LenProcessed != NULL) {
        *pUTF32LenProcessed = 0;
    }

    if (pUTF8 == NULL) {
        return EINVAL;
    }

    /* Check for BOM. */
    if (c89str_utf32_has_bom((const unsigned char*)pUTF32, utf32Len)) {
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        pUTF32 += 1;    /* Skip past the BOM. */
        if (utf32Len != (size_t)-1) {
            utf32Len -= 1;
        }
    }

    if (utf32Len == (size_t)-1) {
        /* Null terminated string. */
        const c89str_utf32* pUTF32Original = pUTF32;
        while (pUTF32[0] != 0) {
            if (utf8Cap == 0) {
                result = ENOMEM;
                break;
            }

            if (isLE) {
                utf32 = c89str_le2host_32(pUTF32[0]);
            } else {
                utf32 = c89str_be2host_32(pUTF32[0]);
            }

            if (!c89str_is_valid_code_point(utf32)) {
                if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                    result = C89STR_ECODEPOINT;
                    break;
                } else {
                    utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                }
            }

            utf8cpLen = c89str_utf32_cp_to_utf8(utf32, pUTF8, utf8Cap);
            if (utf8cpLen == 0) {
                result = ENOMEM;    /* A return value of 0 at this point means there was not enough room in the output buffer. */
                break;
            }

            pUTF8   += utf8cpLen;
            utf8Cap -= utf8cpLen;
            pUTF32  += 1;
        }

        if (pUTF32LenProcessed != NULL) {
            *pUTF32LenProcessed = (pUTF32 - pUTF32Original);
        }
    } else {
        /* Fixed length string. */
        size_t iUTF32;
        for (iUTF32 = 0; iUTF32 < utf32Len; iUTF32 += 1) {
            if (utf8Cap == 0) {
                result = ENOMEM;
                break;
            }

            if (isLE) {
                utf32 = c89str_le2host_32(pUTF32[iUTF32]);
            } else {
                utf32 = c89str_be2host_32(pUTF32[iUTF32]);
            }

            if (!c89str_is_valid_code_point(utf32)) {
                if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                    result = C89STR_ECODEPOINT;
                    break;
                } else {
                    utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                }
            }

            utf8cpLen = c89str_utf32_cp_to_utf8(utf32, pUTF8, utf8Cap);
            if (utf8cpLen == 0) {
                result = ENOMEM;    /* A return value of 0 at this point means there was not enough room in the output buffer. */
                break;
            }

            pUTF8   += utf8cpLen;
            utf8Cap -= utf8cpLen;
        }

        if (pUTF32LenProcessed != NULL) {
            *pUTF32LenProcessed = iUTF32;
        }
    }

    /* Null terminate. */
    if (utf8Cap == 0) {
        result = ENOMEM;    /* Not enough room in the output buffer. */
    } else {
        pUTF8[0] = 0;
    }

    if (pUTF8Len != NULL) {
        *pUTF8Len = (utf8CapOriginal - utf8Cap);
    }

    return result;
}

C89STR_API errno_t c89str_utf32ne_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return c89str_utf32le_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    } else {
        return c89str_utf32be_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    }
}

C89STR_API errno_t c89str_utf32le_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    return c89str_utf32_to_utf8_internal(pUTF8, utf8Cap, pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t c89str_utf32be_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    return c89str_utf32_to_utf8_internal(pUTF8, utf8Cap, pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, C89STR_FALSE);
}

C89STR_API errno_t c89str_utf32_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    if (pUTF8Len != NULL) {
        *pUTF8Len = 0;
    }

    /* Check for BOM. */
    if (c89str_utf32_has_bom((const unsigned char*)pUTF32, utf32Len)) {
        errno_t result;
        size_t utf32LenProcessed;

        c89str_bool32 isLE;
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        /* With this function, we need to use the endian defined by the BOM. */
        isLE = c89str_utf32_is_bom_le((const unsigned char*)pUTF32);
        
        pUTF32 += 1;    /* Skip past the BOM. */
        if (utf32Len != (size_t)-1) {
            utf32Len -= 1;
        }

        if (isLE) {
            result = c89str_utf32le_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = c89str_utf32be_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF32LenProcessed) {
            *pUTF32LenProcessed = utf32LenProcessed + 1;    /* +1 for the BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return c89str_utf32ne_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
}



C89STR_API errno_t c89str_utf32_to_utf16_len_internal(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags, c89str_bool32 isLE)
{
    errno_t result = C89STR_SUCCESS;
    size_t utf16Len = 0;
    c89str_utf32 utf32;

    if (pUTF16Len != NULL) {
        *pUTF16Len = 0;
    }

    if (pUTF32LenProcessed != NULL) {
        *pUTF32LenProcessed = 0;
    }

    if (pUTF32 == NULL) {
        return EINVAL; /* Invalid input. */
    }

    if (utf32Len == 0 || pUTF32[0] == 0) {
        return C89STR_SUCCESS;  /* Empty input string. Length is always 0. */
    }

    /* Check for BOM. */
    if (c89str_utf32_has_bom((const unsigned char*)pUTF32, utf32Len)) {
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        pUTF32 += 1;    /* Skip past the BOM. */
        if (utf32Len != (size_t)-1) {
            utf32Len -= 1;
        }
    }

    if (utf32Len == (size_t)-1) {
        /* Null terminated string. */
        const c89str_utf32* pUTF32Original = pUTF32;
        while (pUTF32[0] != 0) {
            if (isLE) {
                utf32 = c89str_le2host_32(pUTF32[0]);
            } else {
                utf32 = c89str_be2host_32(pUTF32[0]);
            }

            if (!c89str_is_valid_code_point(utf32)) {
                if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                    result = C89STR_ECODEPOINT;
                    break;
                } else {
                    utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                }
            }

            utf16Len += c89str_utf32_cp_to_utf16_len(utf32);
            pUTF32   += 1;
        }

        if (pUTF32LenProcessed != NULL) {
            *pUTF32LenProcessed = (pUTF32 - pUTF32Original);
        }
    } else {
        /* Fixed length string. */
        size_t iUTF32;
        for (iUTF32 = 0; iUTF32 < utf32Len; iUTF32 += 1) {
            if (isLE) {
                utf32 = c89str_le2host_32(pUTF32[iUTF32]);
            } else {
                utf32 = c89str_be2host_32(pUTF32[iUTF32]);
            }

            if (!c89str_is_valid_code_point(utf32)) {
                if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                    result = C89STR_ECODEPOINT;
                    break;
                } else {
                    utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                }
            }

            utf16Len += c89str_utf32_cp_to_utf16_len(utf32);
        }

        if (pUTF32LenProcessed != NULL) {
            *pUTF32LenProcessed = iUTF32;
        }
    }

    if (pUTF16Len != NULL) {
        *pUTF16Len = utf16Len;
    }

    return result;
}

errno_t c89str_utf32ne_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return c89str_utf32le_to_utf16_len(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    } else {
        return c89str_utf32be_to_utf16_len(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    }
}

C89STR_API errno_t c89str_utf32le_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    return c89str_utf32_to_utf16_len_internal(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t c89str_utf32be_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    return c89str_utf32_to_utf16_len_internal(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, C89STR_FALSE);
}

C89STR_API errno_t c89str_utf32_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    if (pUTF16Len != NULL) {
        *pUTF16Len = 0;
    }

    /* Check for BOM. */
    if (c89str_utf32_has_bom((const unsigned char*)pUTF32, utf32Len)) {
        errno_t result;
        size_t utf32LenProcessed;

        c89str_bool32 isLE;
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        /* With this function, we need to use the endian defined by the BOM. */
        isLE = c89str_utf32_is_bom_le((const unsigned char*)pUTF32);
        
        pUTF32 += 1;    /* Skip past the BOM. */
        if (utf32Len != (size_t)-1) {
            utf32Len -= 1;
        }

        if (isLE) {
            result = c89str_utf32le_to_utf16_len(pUTF16Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = c89str_utf32be_to_utf16_len(pUTF16Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF32LenProcessed) {
            *pUTF32LenProcessed = utf32LenProcessed + 1;    /* +1 for the BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return c89str_utf32ne_to_utf16_len(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
}


C89STR_API errno_t c89str_utf32_to_utf16_internal(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags, c89str_bool32 isLE)
{
    errno_t result = C89STR_SUCCESS;
    size_t utf16CapOriginal = utf16Cap;
    size_t utf16cpLen;  /* Code point length in UTF-8 code units. */
    c89str_utf32 utf32;

    if (pUTF16 == NULL) {
        return c89str_utf32_to_utf16_internal(pUTF16, utf16Cap, pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, isLE);
    }

    if (pUTF16Len != NULL) {
        *pUTF16Len = 0;
    }

    if (pUTF32LenProcessed != NULL) {
        *pUTF32LenProcessed = 0;
    }

    if (pUTF16 == NULL) {
        return EINVAL;
    }

    /* Check for BOM. */
    if (c89str_utf32_has_bom((const unsigned char*)pUTF32, utf32Len)) {
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        pUTF32 += 1;    /* Skip past the BOM. */
        if (utf32Len != (size_t)-1) {
            utf32Len -= 1;
        }
    }

    if (utf32Len == (size_t)-1) {
        /* Null terminated string. */
        const c89str_utf32* pUTF32Original = pUTF32;
        while (pUTF32[0] != 0) {
            if (utf16Cap == 0) {
                result = ENOMEM;
                break;
            }

            if (isLE) {
                utf32 = c89str_le2host_32(pUTF32[0]);
            } else {
                utf32 = c89str_be2host_32(pUTF32[0]);
            }

            if (!c89str_is_valid_code_point(utf32)) {
                if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                    result = C89STR_ECODEPOINT;
                    break;
                } else {
                    utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                }
            }

            utf16cpLen = c89str_utf32_cp_to_utf16(utf32, pUTF16, utf16Cap);
            if (utf16cpLen == 0) {
                result = ENOMEM;    /* A return value of 0 at this point means there was not enough room in the output buffer. */
                break;
            }

            if (isLE) {
                if (utf16cpLen == 1) {
                    pUTF16[0] = c89str_host2le_16(pUTF16[0]);
                } else {
                    pUTF16[0] = c89str_host2le_16(pUTF16[0]);
                    pUTF16[1] = c89str_host2le_16(pUTF16[1]);
                }
            } else {
                if (utf16cpLen == 1) {
                    pUTF16[0] = c89str_host2be_16(pUTF16[0]);
                } else {
                    pUTF16[0] = c89str_host2be_16(pUTF16[0]);
                    pUTF16[1] = c89str_host2be_16(pUTF16[1]);
                }
            }

            pUTF16   += utf16cpLen;
            utf16Cap -= utf16cpLen;
            pUTF32   += 1;
        }

        if (pUTF32LenProcessed != NULL) {
            *pUTF32LenProcessed = (pUTF32 - pUTF32Original);
        }
    } else {
        /* Fixed length string. */
        size_t iUTF32;
        for (iUTF32 = 0; iUTF32 < utf32Len; iUTF32 += 1) {
            if (utf16Cap == 0) {
                result = ENOMEM;
                break;
            }

            if (isLE) {
                utf32 = c89str_le2host_32(pUTF32[iUTF32]);
            } else {
                utf32 = c89str_be2host_32(pUTF32[iUTF32]);
            }

            if (!c89str_is_valid_code_point(utf32)) {
                if ((flags & C89STR_ERROR_ON_INVALID_CODE_POINT) != 0) {
                    result = C89STR_ECODEPOINT;
                    break;
                } else {
                    utf32 = C89STR_UNICODE_REPLACEMENT_CODE_POINT;
                }
            }

            utf16cpLen = c89str_utf32_cp_to_utf16(utf32, pUTF16, utf16Cap);
            if (utf16cpLen == 0) {
                result = ENOMEM;    /* A return value of 0 at this point means there was not enough room in the output buffer. */
                break;
            }

            if (isLE) {
                if (utf16cpLen == 1) {
                    pUTF16[0] = c89str_host2le_16(pUTF16[0]);
                } else {
                    pUTF16[0] = c89str_host2le_16(pUTF16[0]);
                    pUTF16[1] = c89str_host2le_16(pUTF16[1]);
                }
            } else {
                if (utf16cpLen == 1) {
                    pUTF16[0] = c89str_host2be_16(pUTF16[0]);
                } else {
                    pUTF16[0] = c89str_host2be_16(pUTF16[0]);
                    pUTF16[1] = c89str_host2be_16(pUTF16[1]);
                }
            }

            pUTF16   += utf16cpLen;
            utf16Cap -= utf16cpLen;
        }

        if (pUTF32LenProcessed != NULL) {
            *pUTF32LenProcessed = iUTF32;
        }
    }

    /* Null terminate. */
    if (utf16Cap == 0) {
        result = ENOMEM;    /* Not enough room in the output buffer. */
    } else {
        pUTF16[0] = 0;
    }

    if (pUTF16Len != NULL) {
        *pUTF16Len = (utf16CapOriginal - utf16Cap);
    }

    return result;
}

C89STR_API errno_t c89str_utf32ne_to_utf16ne(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return c89str_utf32le_to_utf16le(pUTF16, utf16Cap, pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    } else {
        return c89str_utf32be_to_utf16be(pUTF16, utf16Cap, pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    }
}

C89STR_API errno_t c89str_utf32le_to_utf16le(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    return c89str_utf32_to_utf16_internal(pUTF16, utf16Cap, pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t c89str_utf32be_to_utf16be(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    return c89str_utf32_to_utf16_internal(pUTF16, utf16Cap, pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, C89STR_FALSE);
}

C89STR_API errno_t c89str_utf32_to_utf16(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    if (pUTF16Len != NULL) {
        *pUTF16Len = 0;
    }

    /* Check for BOM. */
    if (c89str_utf32_has_bom((const unsigned char*)pUTF32, utf32Len)) {
        errno_t result;
        size_t utf32LenProcessed;

        c89str_bool32 isLE;
        if ((flags & C89STR_FORBID_BOM) != 0) {
            return C89STR_EBOM;  /* Found a BOM, but it's forbidden. */
        }

        /* With this function, we need to use the endian defined by the BOM. */
        isLE = c89str_utf32_is_bom_le((const unsigned char*)pUTF32);
        
        pUTF32 += 1;    /* Skip past the BOM. */
        if (utf32Len != (size_t)-1) {
            utf32Len -= 1;
        }

        if (isLE) {
            result = c89str_utf32le_to_utf16le(pUTF16, utf16Cap, pUTF16Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = c89str_utf32be_to_utf16be(pUTF16, utf16Cap, pUTF16Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF32LenProcessed) {
            *pUTF32LenProcessed = utf32LenProcessed + 1;    /* +1 for the BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return c89str_utf32ne_to_utf16ne(pUTF16, utf16Cap, pUTF16Len, pUTF32+1, utf32Len-1, pUTF32LenProcessed, flags);
}



C89STR_API c89str_bool32 c89str_utf32_is_null_or_whitespace(const c89str_utf32* pUTF32, size_t utf32Len)
{
    if (pUTF32 == NULL) {
        return C89STR_TRUE;
    }

    while (pUTF32[0] != 0 && utf32Len > 0) {
        c89str_utf32 cp = pUTF32[0];

        pUTF32   += 1;
        utf32Len -= 1;
        
        if (cp >= 0x09 && cp <= 0x0D) {
            continue;
        }

        if (cp >= 0x2000 && cp <= 0x200A) {
            continue;
        }

        switch (cp) {
            case 0x0020:
            case 0x0085:
            case 0x00A0:
            case 0x1680:
            case 0x2028:
            case 0x2029:
            case 0x202F:
            case 0x205F:
            case 0x3000:
                continue;

            default:
                return C89STR_FALSE;
        }
    }

    /* Getting here means we reached the end of the string without finding anything other than whitespace which means the string is entire whitespace. */
    return C89STR_TRUE;
}

C89STR_API c89str_bool32 c89str_utf32_is_newline(c89str_utf32 utf32)
{
    if (utf32 >= 0x0A && utf32 <= 0x0D) {
        return C89STR_TRUE;
    }

    if (utf32 == 0x85) {
        return C89STR_TRUE;
    }

    if (utf32 >= 0x2028 && utf32 <= 0x2029) {
        return C89STR_TRUE;
    }

    return C89STR_FALSE;
}

C89STR_API c89str_bool32 c89str_utf8_is_null_or_whitespace(const c89str_utf8* pUTF8, size_t utf8Len)
{
    return c89str_is_null_or_whitespace((const char*)pUTF8, utf8Len);
}

C89STR_API size_t c89str_utf8_find_next_whitespace(const c89str_utf8* pUTF8, size_t utf8Len)
{
    size_t utf8RunningOffset = 0;

    if (pUTF8 == NULL) {
        return c89str_npos;
    }

    while (pUTF8[0] != '\0' && utf8Len > 0) {
        c89str_utf32 utf32;
        size_t utf8Processed;
        int err;

        err = c89str_utf8_to_utf32(&utf32, 1, NULL, pUTF8, utf8Len, &utf8Processed, 0);
        if (err != 0 && err != ENOMEM) {
            break;
        }

        if (utf8Processed == 0) {
            break;
        }

        if (c89str_utf32_is_null_or_whitespace(&utf32, 1) == C89STR_TRUE) {
            return utf8RunningOffset;
        }

        utf8RunningOffset += utf8Processed;
        pUTF8             += utf8Processed;
        utf8Len           -= utf8Processed;
    }

    return c89str_npos;
}


C89STR_API size_t c89str_utf8_ltrim_offset(const c89str_utf8* pUTF8, size_t utf8Len)
{
    size_t utf8RunningOffset = 0;

    if (pUTF8 == NULL) {
        return c89str_npos;
    }

    while (pUTF8[0] != '\0' && utf8Len > 0) {
        c89str_utf32 utf32;
        size_t utf8Processed;
        int err;

        err = c89str_utf8_to_utf32(&utf32, 1, NULL, pUTF8, utf8Len, &utf8Processed, 0);
        if (err != 0 && err != ENOMEM) {
            break;
        }

        if (utf8Processed == 0) {
            break;
        }

        if (c89str_utf32_is_null_or_whitespace(&utf32, 1) == C89STR_FALSE) {
            break;
        }

        utf8RunningOffset += utf8Processed;
        pUTF8             += utf8Processed;
        utf8Len           -= utf8Processed;
    }

    return utf8RunningOffset;
}

C89STR_API size_t c89str_utf8_rtrim_offset(const c89str_utf8* pUTF8, size_t utf8Len)
{
    size_t utf8RunningOffset = 0;
    size_t utf8LastNonWhitespaceOffset = utf8Len;

    if (pUTF8 == NULL) {
        return c89str_npos;
    }

    for (;;) {
        c89str_utf32 utf32;
        size_t utf8Processed;
        int err;

        err = c89str_utf8_to_utf32(&utf32, 1, NULL, pUTF8, utf8Len, &utf8Processed, 0);
        if (err != 0 && err != ENOMEM) {
            break;
        }

        if (utf8Processed == 0) {
            break;
        }

        utf8RunningOffset += utf8Processed;

        if (c89str_utf32_is_null_or_whitespace(&utf32, 1) == C89STR_FALSE) {
            utf8LastNonWhitespaceOffset = utf8RunningOffset;
        }

        pUTF8   += utf8Processed;
        utf8Len -= utf8Processed;
    }

    return utf8LastNonWhitespaceOffset;
}

C89STR_API size_t c89str_utf8_find_next_line(const c89str_utf8* pUTF8, size_t utf8Len, size_t* pThisLineLen)
{
    size_t thisLen = 0;
    size_t nextBeg = 0;

    if (pThisLineLen != NULL) {
        *pThisLineLen = 0;
    }

    if (pUTF8 == NULL) {
        return c89str_npos;
    }

    /* This could be faster, but it's practical. */
    while (pUTF8[0] != '\0' && utf8Len > 0) {
        c89str_utf32 utf32;
        size_t utf8Processed;
        int err;

        /* We expect ENOMEM to be returned, but we should still have a valid utf32 character. */
        err = c89str_utf8_to_utf32(&utf32, 1, NULL, pUTF8, utf8Len, &utf8Processed, 0);
        if (err != 0 && err != ENOMEM) {
            break;
        }

        if (utf8Processed == 0) {
            break;
        }

        nextBeg += utf8Processed;

        if (c89str_utf32_is_newline(utf32) == C89STR_TRUE) {
            /* Special case for \r\n. This needs to be treated as one line. The \r by itself should also be treated as a new line, however. */
            if (utf32 == '\r' && utf8Len + utf8Processed > 0 && pUTF8[utf8Processed] == '\n') {
                nextBeg += 1;
            }

            break;
        }

        C89STR_ASSERT(utf8Len >= utf8Processed);  /* If there wasn't enough data in the UTF-8 string, c89str_utf8_to_utf32() should have failed. */

        thisLen += utf8Processed;
        pUTF8   += utf8Processed;
        utf8Len -= utf8Processed;
    }

    if (pThisLineLen != NULL) {
        *pThisLineLen = thisLen;
    }

    return nextBeg;
}
/* End Unicode */








/* BEG c89str_lexer.c */
C89STR_API errno_t c89str_lexer_init(c89str_lexer* pLexer, const char* pText, size_t textLen)
{
    if (pLexer == NULL) {
        return EINVAL;  /* Invalid arguments. */
    }

    C89STR_ZERO_OBJECT(pLexer);

    if (pText == NULL) {
        return EINVAL;  
    }

    pLexer->pText      = pText;
    pLexer->textLen    = textLen;
    pLexer->textOff    = 0;
    pLexer->lineNumber = 1;

    /* We do not use the null terminator to know the end of the string so we need to calculate it now. */
    if (pLexer->textLen == (size_t)-1) {
        pLexer->textLen = c89str_strlen(pText);
    }

    pLexer->options.pLineCommentOpeningToken  = "//";
    pLexer->options.pBlockCommentOpeningToken = "/*";
    pLexer->options.pBlockCommentClosingToken = "*/";

    return 0;
}


static errno_t c89str_lexer_set_token(c89str_lexer* pLexer, c89str_utf32 token, size_t tokenLen)
{
    C89STR_ASSERT(pLexer != NULL);

    pLexer->token     = token;
    pLexer->pTokenStr = pLexer->pText + pLexer->textOff;
    pLexer->tokenLen  = tokenLen;
    pLexer->textOff  += tokenLen;

    if (token == c89str_token_type_newline) {
        pLexer->lineNumber += 1;
    }

    /* We need to parse comments and strings to find line numbers. */
    if (token == c89str_token_type_comment || token == c89str_token_type_string_double || token == c89str_token_type_string_single) {
        const char* pRunningStr = pLexer->pTokenStr;
        for (;;) {
            size_t thisLineLen;
            size_t nextLineOff = c89str_utf8_find_next_line(pRunningStr, pLexer->tokenLen - (pRunningStr - pLexer->pTokenStr), &thisLineLen);
            if (nextLineOff == thisLineLen) {
                break;  /* Reached the end. */
            }

            pRunningStr += nextLineOff;
            pLexer->lineNumber += 1;
        }
    }

    return 0;
}

static errno_t c89str_lexer_set_single_char(c89str_lexer* pLexer, c89str_utf32 c)
{
    return c89str_lexer_set_token(pLexer, c, 1);
}

static errno_t c89str_lexer_set_error(c89str_lexer* pLexer, size_t tokenLen)
{
    c89str_lexer_set_token(pLexer, c89str_token_type_error, tokenLen);
    return EINVAL;
}

static size_t c89str_lexer_parse_integer_suffix(c89str_lexer* pLexer, size_t off) /* Returns the new offset. */
{
    const char* txt;
    size_t len;

    C89STR_ASSERT(pLexer != NULL);

    txt = pLexer->pText;
    len = pLexer->textLen;

    if (off < len) {
        if (txt[off] == 'u' || txt[off] == 'U') {
            off += 1;
            if (off < len && (txt[off] == 'l' || txt[off] == 'L')) {
                off += 1;
                if (off < len && (txt[off] == 'l' || txt[off] == 'L')) {
                    off += 1;   /* Suffix is ull/ULL. */
                } else {
                    /* Suffix is ul/UL. */
                }
            } else {
                /* Suffix is u/U. */
            }
        } else if (txt[off] == 'l' || txt[off] == 'L') {
            off += 1;
            if (off < len && (txt[off] == 'l' || txt[off] == 'L')) {
                off += 1;
                if (off < len && (txt[off] == 'u' || txt[off] == 'U')) {
                    off += 1;   /* Suffix is llu/LLU. */
                } else {
                    /* Suffix is ll/LL. */
                }
            } else if (off < len && (txt[off] == 'u' || txt[off] == 'U')) {
                off += 1;   /* Suffix is lu/LU. */
            } else {
                /* Suffix is l/L. */
            }
        } else {
            /* No suffix. */
        }
    } else {
        /* No suffix (EOF). */
    }

    return off;
}

static size_t c89str_lexer_parse_float_suffix(c89str_lexer* pLexer, size_t off) /* Returns the new offset. */
{
    const char* txt;
    size_t len;

    C89STR_ASSERT(pLexer != NULL);

    txt = pLexer->pText;
    len = pLexer->textLen;

    /* Supported suffixes: 'f'/'F' (float), 'd'/'D' (double) and 'l'/'L' (long double). */
    if (off < len) {
        if (txt[off] == 'f' || txt[off] == 'F' || txt[off] == 'd' || txt[off] == 'D' || txt[off] == 'l' || txt[off] == 'L') {
            off += 1;
        }
    } else {
        /* No suffix (EOF). */
    }

    return off;
}

static errno_t c89str_lexer_parse_suffix_and_set_token(c89str_lexer* pLexer, c89str_utf32 token, size_t off)
{
    /* Get past the suffix first. */
    if (token == c89str_token_type_float_literal_dec || token == c89str_token_type_float_literal_hex) {
        /* Floating point suffix. */
        off = c89str_lexer_parse_float_suffix(pLexer, off);
    } else {
        /* Assume an integer suffix. */
        off = c89str_lexer_parse_integer_suffix(pLexer, off);
    }

    return c89str_lexer_set_token(pLexer, token, (off - pLexer->textOff));
}

C89STR_API errno_t c89str_lexer_next(c89str_lexer* pLexer)
{
    int result;
    const char* txt;
    size_t off;
    size_t len;

    if (pLexer == NULL) {
        return EINVAL;  /* Invalid arguments. */
    }

    /* We need to run this in a loop because we may be wanting to skip certain tokens such as whitespace, newlines and comments. */
    for (;;) {
        /*
        When off == len, the end has been reached. The remaining number of bytes is (len - off). txt[off] is the current character. off is variable and can move forward
        whereas len is constant and remains the same (it represents the length of the string and is required for calculating the number of bytes remaining).
        */
        txt = pLexer->pText;
        off = pLexer->textOff;  /* Moves forward. */
        len = pLexer->textLen;  /* Constant. */

        if (off == len) {
            c89str_lexer_set_token(pLexer, c89str_token_type_eof, 0);
            return ENOMEM;  /* Out of input data. */
        }

        /* First check if we're on whitespace. */
        {
            size_t whitespaceLen = c89str_utf8_ltrim_offset(txt + off, (len - off));
            if (whitespaceLen > 0) {
                /* It's whitespace. Our lexer makes a distrinction between whitespace and new line characters so we need to check that too. */
                size_t thisLineLen;
                size_t nextLineOff = c89str_utf8_find_next_line(txt + off, (len - off), &thisLineLen);
                if (thisLineLen > whitespaceLen) {
                    /* There's no new line character within the whitespace area. */
                    result = c89str_lexer_set_token(pLexer, c89str_token_type_whitespace, whitespaceLen);
                    if (pLexer->options.skipWhitespace) {
                        continue;
                    } else {
                        return result;
                    }
                } else {
                    /* There's a new line somewhere in the whitespace. */
                    if (thisLineLen <= whitespaceLen && thisLineLen > 0) {
                        /* There's a new line character within the whitespace area. */
                        result = c89str_lexer_set_token(pLexer, c89str_token_type_whitespace, thisLineLen);
                        if (pLexer->options.skipWhitespace) {
                            continue;
                        } else {
                            return result;
                        }
                    } else {
                        /* It's a new line character. */
                        C89STR_ASSERT(thisLineLen == 0);
                        result = c89str_lexer_set_token(pLexer, c89str_token_type_newline, (nextLineOff - thisLineLen));
                        if (pLexer->options.skipNewlines) {
                            continue;
                        } else {
                            return result;
                        }
                    }
                }
            }
        }

        /* It's not whitespace or a new line. Check if it's a line comment. */
        if (c89str_begins_with(&txt[off], len - off, pLexer->options.pLineCommentOpeningToken, (size_t)-1)) {
            /* Found the beginning of a line comment. Note that we do *not* include the new line in the returned token. */
            size_t thisLineLen;
            size_t openingLen;

            openingLen = c89str_strlen(pLexer->options.pLineCommentOpeningToken);
            
            off += openingLen;
            c89str_utf8_find_next_line(txt + off, (len - off), &thisLineLen);
            result = c89str_lexer_set_token(pLexer, c89str_token_type_comment, thisLineLen + openingLen);
            if (pLexer->options.skipComments) {
                continue;
            } else {
                return result;
            }
        }

        if (c89str_begins_with(&txt[off], len - off, pLexer->options.pBlockCommentOpeningToken, (size_t)-1)) {
            /* It's the beginning of a block comment. */
            errno_t searchResult;
            size_t tokenLen;
            size_t openingLen;
            size_t closingLen;

            openingLen = c89str_strlen(pLexer->options.pBlockCommentOpeningToken);
            closingLen = c89str_strlen(pLexer->options.pBlockCommentClosingToken);
            
            off += openingLen;
            searchResult = c89str_findn(txt + off, (len - off), pLexer->options.pBlockCommentClosingToken, closingLen, &tokenLen);
            if (searchResult != C89STR_SUCCESS) {
                /* The closing token could not be found. Treat the entire rest of the content as a comment. */
                result = c89str_lexer_set_token(pLexer, c89str_token_type_comment, (len - off) + openingLen);
            } else {
                /* We found the closing token. */
                result = c89str_lexer_set_token(pLexer, c89str_token_type_comment, tokenLen + openingLen + closingLen);
            }

            if (pLexer->options.skipComments) {
                continue;
            } else {
                return result;
            }
        }

        /* It's not whitespace, new line nor a comment. Check if it's a string. We support both double and single quoted strings. */
        if (txt[off] == '\"') {
            off += 1;
            for (; off < len; off += 1) {
                if (txt[off] == '\"') {
                    /* Could be the end of the string. Need to check that this double-quote is escaped. If so we continue, otherwise we have reached the end. */
                    C89STR_ASSERT(off > 0);
                    if (txt[off-1] != '\\') {
                        /* It's not an escaped double quote which means we've reached the end. */
                        off += 1;
                        return c89str_lexer_set_token(pLexer, c89str_token_type_string_double, (off - pLexer->textOff));
                    }
                }
            }
        }

        if (txt[off] == '\'') {
            off += 1;
            for (; off < len; off += 1) {
                if (txt[off] == '\'') {
                    /* Could be the end of the string. Need to check that this double-quote is escaped. If so we continue, otherwise we have reached the end. */
                    C89STR_ASSERT(off > 0);
                    if (txt[off-1] != '\\') {
                        /* It's not an escaped double quote which means we've reached the end. */
                        off += 1;
                        return c89str_lexer_set_token(pLexer, c89str_token_type_string_double, (off - pLexer->textOff));
                    }
                }
            }
        }

        /* It's not whitespace, new line, comment, nor a string. Check if it's a number. Using a switch here so we can do a convenient fall-through for handling the 0 special case. */
        switch (txt[off]) {
            case '0':
            {
                size_t tokenBeg = off;  /* <-- Will be used to calculate the length of the token. */

                if ((off+1) < len) {
                    if (txt[off+1] == 'x' || txt[off+1] == 'X') {
                        /* Hex integer or float literal. If we find a '.', 'p' or 'P' it means we're looking at a floating-point literal. */
                        off += 2;   /* +1 for the '0' and +1 for the 'x/X'. */
                        while (off < len && ((txt[off] >= '0' && txt[off] <= '9') || (txt[off] >= 'a' && txt[off] <= 'f') || (txt[off] >= 'A' && txt[off] <= 'F'))) {
                            off += 1;
                        }

                        if (txt[off] == '.') {
                            off += 1;
                            while (off < len && ((txt[off] >= '0' && txt[off] <= '9') || (txt[off] >= 'a' && txt[off] <= 'f') || (txt[off] >= 'A' && txt[off] <= 'F'))) {
                                off += 1;
                            }
                        }

                        /* If our next character is an 'p' or 'P' it means we're using scientific notation. */
                        if (txt[off] == 'p' || txt[off] == 'P') {
                            /* Scientific notation. */
                            off += 1;
                            if (off < len && (txt[off] == '-' || txt[off] == '+')) {
                                off += 1;
                            }

                            /* We must have at least one digit. */
                            if ((txt[off] >= '0' && txt[off] <= '9') || (txt[off] >= 'a' && txt[off] <= 'f') || (txt[off] >= 'A' && txt[off] <= 'F')) {
                                off += 1;
                            } else {
                                /* Invalid float literal. */
                                return c89str_lexer_set_error(pLexer, (off - tokenBeg));
                            }

                            /* Now we just need to go until we hit the last digit. */
                            while (off < len && ((txt[off] >= '0' && txt[off] <= '9') || (txt[off] >= 'a' && txt[off] <= 'f') || (txt[off] >= 'A' && txt[off] <= 'F'))) {
                                off += 1;
                            }
                        }

                        /* We've reached the end of the literal. Check for a suffix and set the token. */
                        return c89str_lexer_parse_suffix_and_set_token(pLexer, c89str_token_type_float_literal_hex, off);
                    } else if (txt[off+1] == 'b' || txt[off+1] == 'B') {
                        /* Binary literal. */
                        off += 2;   /* +1 for '0' and +1 for 'b/B'. */
                        while (off < len && (txt[off] >= '0' && txt[off] <= '1')) {
                            off += 1;
                        }

                        /* We've reached the end of the literal. */
                        return c89str_lexer_parse_suffix_and_set_token(pLexer, c89str_token_type_integer_literal_bin, off);
                    } else {
                        /* Maybe an octal literal, but could also just be a float starting with 0. If it's float we fall through to the next case statement which will treat it as decimal. */
                        size_t newOff = off+1;

                        /* First get past all leading zeros. */
                        while (newOff < len && txt[newOff] == '0') {
                            newOff += 1;
                        }

                        /* If the next character is between 1 and 7 it means we have an octal constant. Otherwise we need to fall through and treat it as a decimal literal. */
                        if (txt[newOff] >= '1' && txt[newOff] <= '7') {
                            /* It's an octal integer literal. */
                            off = newOff;
                            while (off < len && (txt[off] >= '0' && txt[off] <= '7')) {
                                off += 1;
                            }

                            /* We've reached the end of the literal. */
                            return c89str_lexer_parse_suffix_and_set_token(pLexer, c89str_token_type_integer_literal_oct, off);
                        } else {
                            /* It's not an octal literal. Just fall through and treat it as a decimal literal. Note that we have not incremented 'off' at this point. */
                        }
                    }
                }
            } C89STR_FALLTHROUGH /* fallthrough */;

            case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            {
                /* Decimal integer or float literal. We keep looping until we find something that's not a number. If it is a '.', 'e' or 'E' it means we're looking at a floating-point literal. */
                size_t tokenBeg = off;  /* <-- Will be used to calculate the length of the token. */
                off += 1;
                while (off < len && (txt[off] >= '0' && txt[off] <= '9')) {
                    off += 1;
                }

                /* Not a digit. If it's a dot it means we're processing a floating point literal. */
                if (txt[off] == '.' || txt[off] == 'e' || txt[off] == 'E') {
                    /* It's a floating point literal. We need to do another digit iteration. */
                    if (txt[off] == '.') {
                        off += 1;
                        while (off < len && (txt[off] >= '0' && txt[off] <= '9')) {
                            off += 1;
                        }
                    }

                    /* If our next character is an 'e' or 'E' it means we're using scientific notation. */
                    if (txt[off] == 'e' || txt[off] == 'E') {
                        /* Scientific notation. */
                        off += 1;
                        if (off < len && (txt[off] == '-' || txt[off] == '+')) {
                            off += 1;
                        }

                        /* We must have at least one digit. */
                        if (txt[off] >= '0' && txt[off] <= '9') {
                            off += 1;
                        } else {
                            /* Invalid float literal. */
                            return c89str_lexer_set_error(pLexer, (off - tokenBeg));
                        }

                        /* Now we just need to go until we hit the last digit. */
                        while (off < len && (txt[off] >= '0' && txt[off] <= '9')) {
                            off += 1;
                        }
                    }

                    /* We've reached the end of the literal. Check for a suffix and set the token. */
                    return c89str_lexer_parse_suffix_and_set_token(pLexer, c89str_token_type_float_literal_dec, off);
                } else {
                    /* It's a decimal integer literal. Check fo a suffix and set the token. */
                    return c89str_lexer_parse_suffix_and_set_token(pLexer, c89str_token_type_integer_literal_dec, off);
                }
            } break;

            case '=':
            {
                if (off+1 < len) {
                    if (txt[off+1] == '=') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_eqeq, 2);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            };

            case '!':
            {
                if (off+1 < len) {
                    if (txt[off+1] == '=') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_noteq, 2);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            };

            case '<':
            {
                if (off+1 < len) {
                    if (txt[off+1] == '=') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_lteq, 2);
                    }
                    if (txt[off+1] == '<') {
                        if (off+2 < len) {
                            if (txt[off+2] == '=') {
                                return c89str_lexer_set_token(pLexer, c89str_token_type_shleq, 3);
                            }
                        }
                        return c89str_lexer_set_token(pLexer, c89str_token_type_shl, 2);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            };

            case '>':
            {
                if (off+1 < len) {
                    if (txt[off+1] == '=') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_gteq, 2);
                    }
                    if (txt[off+1] == '>') {
                        if (off+2 < len) {
                            if (txt[off+2] == '=') {
                                return c89str_lexer_set_token(pLexer, c89str_token_type_shreq, 3);
                            }
                        }
                        return c89str_lexer_set_token(pLexer, c89str_token_type_shr, 2);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            };

            case '&':
            {
                if (off+1 < len) {
                    if (txt[off+1] == '&') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_andand, 2);
                    }
                    if (txt[off+1] == '=') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_andeq, 2);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            };

            case '|':
            {
                if (off+1 < len) {
                    if (txt[off+1] == '|') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_oror, 2);
                    }
                    if (txt[off+1] == '=') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_oreq, 2);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            };

            case '+':
            {
                if (off+1 < len) {
                    if (txt[off+1] == '+') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_plusplus, 2);
                    }
                    if (txt[off+1] == '=') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_pluseq, 2);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            };

            case '-':
            {
                if (off+1 < len) {
                    if (txt[off+1] == '-') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_minusminus, 2);
                    }
                    if (txt[off+1] == '=') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_minuseq, 2);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            };

            case '*':
            {
                if (off+1 < len) {
                    if (txt[off+1] == '=') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_muleq, 2);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            };

            case '/':
            {
                if (off+1 < len) {
                    if (txt[off+1] == '=') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_diveq, 2);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);   /* Should never hit this because the '/' character is handled when handling comments. */
            };

            case '%':
            {
                if (off+1 < len) {
                    if (txt[off+1] == '=') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_modeq, 2);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            };

            case '^':
            {
                if (off+1 < len) {
                    if (txt[off+1] == '=') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_xoreq, 2);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            };

            case ':':
            {
                if (off+1 < len) {
                    if (txt[off+1] == ':') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_coloncolon, 2);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            };

            case '.':
            {
                if (off+2 < len) {
                    if(txt[off+1] == '.' && txt[off+2] == '.') {
                        return c89str_lexer_set_token(pLexer, c89str_token_type_ellipsis, 3);
                    }
                }
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            };

            /*
            By default we must have an identifier. In our lexer, all special operators are represented using ASCII characters. This means we can use *most* Unicode code points
            in our identifiers. What we cannot use, however, is any of the Unicode defined whitespace characters (these encompass new line characters). We know that it won't
            start with a whitespace character because we checked that at the top. However, we need to make sure we don't include any Unicode whitespace characters.
            */
            default:
            {
                /* We just need to loop until we hit the first disallowed character. We support "_", "a-z", "A-Z", "0-9" and all Unicode characters outside of ASCII except whitespace. */
                if ((txt[off] >= 'a' && txt[off] <= 'z') ||
                    (txt[off] >= 'A' && txt[off] <= 'Z') ||
                    (txt[off] == '_')                    ||
                    ((unsigned char)txt[off] >= 0x80)) {
                    size_t tokenMaxLen = c89str_utf8_find_next_whitespace(txt + off, (len - off));   /* <-- We'll be using this to ensure we don't include any Unicode whitespace characters. */
                    size_t tokenLen = 0;

                    while (tokenLen < (len - off)) {
                        tokenLen += 1;
                        if (tokenLen == tokenMaxLen) {
                            break;
                        }

                        if ((txt[off+tokenLen] >= 'a' && txt[off+tokenLen] <= 'z')                 ||
                            (txt[off+tokenLen] >= 'A' && txt[off+tokenLen] <= 'Z')                 ||
                            (txt[off+tokenLen] >= '0' && txt[off+tokenLen] <= '9')                 ||
                            (txt[off+tokenLen] == '_')                                             ||
                            (txt[off+tokenLen] == '-' && pLexer->options.allowDashesInIdentifiers) ||   /* Enables support for kabab-case. */
                            ((unsigned char)txt[off+tokenLen] >= 0x80)) {
                            continue;   /* Still valid. */
                        } else {
                            break;      /* Not a valid character for an identifier. We're done. */
                        }
                    }

                    return c89str_lexer_set_token(pLexer, c89str_token_type_identifier, tokenLen);
                } else {
                    return c89str_lexer_set_single_char(pLexer, txt[off]);
                }
            };
        }
    }

    /* Shouldn't get here. */
    /*return 0;*/
}
/* END c89str_lexer.c */


static c89str c89str_lexer_unescape_string(const c89str_allocation_callbacks* pAllocationCallbacks, const char* pToken, size_t tokenLen)
{
    errno_t result;
    c89str str = NULL;

    if (pToken == NULL) {
        return NULL;
    }

    /* We need to remove the surrounding quotes. */
    if ((pToken[0] == '\"' || pToken[0] == '\'') && tokenLen >= 2) {
        str = c89str_newn(pAllocationCallbacks, pToken + 1, tokenLen - 2);
    } else {
        str = c89str_newn(pAllocationCallbacks, pToken, tokenLen);
    }

    result = c89str_get_res(str);
    if (result != C89STR_SUCCESS) {
        return str;
    }


    /*
    From here on we won't ever be making the string larger. We can therefore do an efficient iteration
    with the assumption that we won't ever need to expand. We need to transform the following:

        \\r -> \r
        \\n -> \n
        \\t -> \t
        \\f -> \f
        \\" -> \"
        \\' -> \'
        \\\ -> \\
        \\0 -> \0
    */
    {
        size_t i = 0;
        while (i < c89str_get_len(str)) {
            if (str[i] == '\\' && i+1 < c89str_get_len(str)) {
                if (str[i+1] == '\r' ||
                    str[i+1] == '\n' ||
                    str[i+1] == '\t' ||
                    str[i+1] == '\f' ||
                    str[i+1] == '\"' ||
                    str[i+1] == '\'' ||
                    str[i+1] == '\\' ||
                    str[i+1] == '\0') {
                    str = c89str_remove(str, pAllocationCallbacks, i, i + 1);
                    continue;   /* Continue without incrementing the counter. */
                }
            }

            i += 1;
        }
    }

    /* TODO: Handle unicode constants with '\u'. */
    /* TODO: Handle hex constants with '\x'. */
    /* TODO: Handle octal constants with '\0[0..7]'. */

    /* We're done. */
    return str;
}

C89STR_API errno_t c89str_lexer_transform_token(c89str_lexer* pLexer, c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks)
{
    if (pStr == NULL) {
        return EINVAL;
    }

    *pStr = NULL;

    if (pLexer == NULL) {
        return EINVAL;
    }

    if (pLexer->token == c89str_token_type_error) {
        return EINVAL;
    }

    if (pLexer->token == c89str_token_type_string_double || pLexer->token == c89str_token_type_string_single) {
        *pStr = c89str_lexer_unescape_string(pAllocationCallbacks, pLexer->pTokenStr, pLexer->tokenLen);
        return c89str_result(*pStr);
    } else if (pLexer->token == c89str_token_type_comment) {
        if (c89str_begins_with(pLexer->pTokenStr, pLexer->tokenLen, pLexer->options.pLineCommentOpeningToken, (size_t)-1)) {
            /* Line comment. */
            size_t openingLen = c89str_strlen(pLexer->options.pLineCommentOpeningToken);

            *pStr = c89str_newn(pAllocationCallbacks, pLexer->pTokenStr + openingLen, pLexer->tokenLen - openingLen);
            return c89str_result(*pStr);
        } else if (c89str_begins_with(pLexer->pTokenStr, pLexer->tokenLen, pLexer->options.pBlockCommentOpeningToken, (size_t)-1)) {
            /* Block Comment. Keep in mind here that the end of the token might be the end of the contents and not necessarily the closing block comment token. */
            size_t openingLen = c89str_strlen(pLexer->options.pBlockCommentOpeningToken);
            size_t closingLen = c89str_strlen(pLexer->options.pBlockCommentClosingToken);
            size_t transformedLen = pLexer->tokenLen - openingLen;

            /*
            If the token does not end with the block comment terminator it means the end of the contents marks the
            end in which case we don't want to subtrack the length of the closing block comment token.
            */
            if (c89str_ends_with(pLexer->pTokenStr + openingLen, transformedLen, pLexer->options.pBlockCommentClosingToken, closingLen)) {
                transformedLen -= closingLen;
            }
            
            *pStr = c89str_newn(pAllocationCallbacks, pLexer->pTokenStr + openingLen, transformedLen);
            return c89str_result(*pStr);
        }
    }

    /* Getting here means it's not a string nor a comment. We don't need to transform anything so we just create a new string. */
    *pStr = c89str_newn(pAllocationCallbacks, pLexer->pTokenStr, pLexer->tokenLen);
    return c89str_result(*pStr);
}



/*
Paths
*/
C89STR_API errno_t c89str_path_first(const char* pPath, size_t pathLen, c89str_path_iterator* pIterator)
{
    if (pIterator == NULL) {
        return EINVAL;
    }

    C89STR_ZERO_OBJECT(pIterator);

    if (pPath == NULL || pPath[0] == '\0' || pathLen == 0) {
        return EINVAL;
    }

    pIterator->pFullPath      = pPath;
    pIterator->fullPathLength = pathLen;
    pIterator->segmentOffset  = 0;
    pIterator->segmentLength  = 0;

    /* We need to find the first separator, or the end of the string. */
    while (pIterator->segmentLength < pathLen && pPath[pIterator->segmentLength] != '\0' && (pPath[pIterator->segmentLength] != '\\' && pPath[pIterator->segmentLength] != '/')) {
        pIterator->segmentLength += 1;
    }

    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_path_last(const char* pPath, size_t pathLen, c89str_path_iterator* pIterator)
{
    if (pIterator == NULL) {
        return EINVAL;
    }

    C89STR_ZERO_OBJECT(pIterator);

    if (pathLen == 0 || pPath == NULL || pPath[0] == '\0') {
        return EINVAL;
    }

    /* Little trick here. Not *quite* as optimal as it could be, but just go to the end of the string, and then go to the previous segment. */
    pIterator->pFullPath      = pPath;
    pIterator->fullPathLength = (pathLen == (size_t)-1) ? c89str_strlen(pPath) : pathLen;
    pIterator->segmentOffset  = pathLen;
    pIterator->segmentLength  = 0;

    return c89str_path_prev(pIterator);
}

C89STR_API errno_t c89str_path_next(c89str_path_iterator* pIterator)
{
    char c;

    if (pIterator == NULL) {
        return EINVAL;
    }

    C89STR_ASSERT(pIterator->pFullPath != NULL);

    /* Move the offset to the end of the previous segment and reset the length. */
    pIterator->segmentOffset = pIterator->segmentOffset + pIterator->segmentLength;
    pIterator->segmentLength = 0;

    /*
    At this point we'll be sitting on either a separator or the end of the string. We need to get
    past this.
    */
    for (;;) {
        if (pIterator->segmentOffset >= pIterator->fullPathLength) {
            return C89STR_END;
        }

        c = pIterator->pFullPath[pIterator->segmentOffset];
        if (c == '\0') {
            return C89STR_END;
        }

        if (c != '\\' && c != '/') {
            break;
        }

        /* Getting here means we're on a separator. Go to the next character. */
        pIterator->segmentOffset += 1;
    }

    /* We should not be sitting on the end at this point. */
    C89STR_ASSERT(pIterator->segmentOffset < pIterator->fullPathLength);
    C89STR_ASSERT(pIterator->pFullPath[pIterator->segmentOffset] != '\0');

    /* Now we just need to find the end of the segment. */
    for (;;) {
        if ((pIterator->segmentOffset + pIterator->segmentLength) == pIterator->fullPathLength) {
            break;  /* Reached the end of the path. */
        }

        c = pIterator->pFullPath[pIterator->segmentOffset + pIterator->segmentLength];
        if (c == '\0' || c == '\\' || c == '/') {
            break;
        }

        pIterator->segmentLength += 1;
    }

    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_path_prev(c89str_path_iterator* pIterator)
{
    size_t offsetEnd;

    if (pIterator == NULL) {
        return EINVAL;
    }

    C89STR_ASSERT(pIterator->pFullPath != NULL);

    if (pIterator->segmentOffset == 0) {
        return C89STR_END;  /* If we're already at the start it must mean we're finished iterating. */
    }

    pIterator->segmentLength = 0;

    /*
    We need to find the end of the previous segment. We do this by starting at the offset of the
    current segment and working backwards until we reach the start or a non-separator.
    */
    offsetEnd = pIterator->segmentOffset - 1;
    for (;;) {
        if (offsetEnd == 0) {
            break;  /* We reached the start. */
        }

        if (pIterator->pFullPath[offsetEnd] == '\\' || pIterator->pFullPath[offsetEnd] == '/') {
            break;
        }

        pIterator->segmentOffset -= 1;
    }

    /*
    The offset is just before a separator, or on a separator at the start of the path. If we're at
    the start of the path, but the end is sitting on a separator, it means we have a path like this:

        /some/path

    In this case, it's sitting on the root directory. We want to iterate this so we have enough
    information to fully reconstruct a path, but also to distinguish between an absolute path and
    a relative path.
    */
    if (offsetEnd == 0) {
        /* It's sitting at the start of the path. */
        if (pIterator->pFullPath[offsetEnd] == '\\' || pIterator->pFullPath[offsetEnd] == '/') {
            /* It's the root segment. */
            pIterator->segmentLength = 0;
            return C89STR_SUCCESS;
        } else {
            /* It's sitting at the start of the path, but it's not a root directory. This just means we've reached the end. */
            return C89STR_END;
        }
    } else {
        /* We're not at the start of the path. We need to keep iterating backwards until we find another separator. */
        pIterator->segmentOffset = offsetEnd;

        /*
        The end offset needs to be increment by one so that it's sitting on the separator. This way
        when we calculate the length, we can just do "offsetEnd - offset"
        */
        offsetEnd += 1;

        /* Just keep iterating backwards until we find the start of the path or another separator. */
        for (;;) {
            if (pIterator->segmentOffset == 0) {
                break;
            }

            if (pIterator->pFullPath[pIterator->segmentOffset] == '\\' || pIterator->pFullPath[pIterator->segmentOffset] == '/') {
                pIterator->segmentOffset += 1;  /* <-- Don't include the separator. */
                break;
            }

            /* Getting here means we didn't find the separator. */
            pIterator->segmentOffset -= 1;
        }

        pIterator->segmentLength = offsetEnd - pIterator->segmentOffset;

        return C89STR_SUCCESS;
    }
}

C89STR_API int c89str_path_iterators_compare(const c89str_path_iterator* pIteratorA, const c89str_path_iterator* pIteratorB)
{
    C89STR_ASSERT(pIteratorA != NULL);
    C89STR_ASSERT(pIteratorB != NULL);

    return c89str_strncmp(pIteratorA->pFullPath + pIteratorA->segmentOffset, pIteratorB->pFullPath + pIteratorB->segmentOffset, C89STR_MIN(pIteratorA->segmentLength, pIteratorB->segmentLength));
}

C89STR_API const char* c89str_path_extension(const char* pPath, size_t pathLen)
{
    const char* pDot = NULL;
    const char* pLastSlash = NULL;
    size_t i;

    if (pPath == NULL) {
        return NULL;
    }

    /* We need to find the last dot after the last slash. */
    for (i = 0; i < pathLen; ++i) {
        if (pPath[i] == '.') {
            pDot = pPath + i;
        } else if (pPath[i] == '\\' || pPath[i] == '/') {
            pLastSlash = pPath + i;
        }
    }

    /* If the last dot is after the last slash, we've found it. Otherwise, it's not there and we need to return null. */
    if (pDot != NULL && pDot > pLastSlash) {
        return pDot + 1;
    } else {
        return NULL;
    }
}

C89STR_API c89str_bool32 c89str_path_extension_equal(const char* pPath, size_t pathLen, const char* pExtension, size_t extensionLen)
{
    size_t pathExtensionLen;
    const char* pPathExtension = c89str_path_extension(pPath, pathLen);
    if (pPathExtension == NULL) {
        return C89STR_FALSE;
    }

    if (pathLen == (size_t)-1) {
        pathExtensionLen = c89str_strlen(pPathExtension);
    } else {
        pathExtensionLen = pathLen - (pPathExtension - pPath);
    }

    if (extensionLen == (size_t)-1) {
        extensionLen = c89str_strlen(pExtension);
    }

    if (pathExtensionLen != extensionLen) {
        return C89STR_FALSE;
    }

    return c89str_strnicmp(pPathExtension, pExtension, extensionLen);
}




/* ===== Amalgamations Below ===== */

/*
Disabling unaligned access for safety. TODO: Look at a way to make this configurable. Will require reversing the
logic in stb_sprintf() which we might be able to do via the amalgamator.
*/
#ifndef C89STR_SPRINTF_NOUNALIGNED
#define C89STR_SPRINTF_NOUNALIGNED
#endif

/* We need to disable the implicit-fallthrough warning on GCC. */
#if defined(__GNUC__) && (__GNUC__ >= 7 || (__GNUC__ == 6 && __GNUC_MINOR__ >= 1))
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

/* beg stb_sprintf.c */
#if defined(__clang__)
 #if defined(__has_feature) && defined(__has_attribute)
  #if __has_feature(address_sanitizer)
   #if __has_attribute(__no_sanitize__)
    #define C89STR_ASAN __attribute__((__no_sanitize__("address")))
   #elif __has_attribute(__no_sanitize_address__)
    #define C89STR_ASAN __attribute__((__no_sanitize_address__))
   #elif __has_attribute(__no_address_safety_analysis__)
    #define C89STR_ASAN __attribute__((__no_address_safety_analysis__))
   #endif
  #endif
 #endif
#elif defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
 #if defined(__SANITIZE_ADDRESS__) && __SANITIZE_ADDRESS__
  #define C89STR_ASAN __attribute__((__no_sanitize_address__))
 #endif
#elif defined(_MSC_VER)
 #if defined(__SANITIZE_ADDRESS__) && __SANITIZE_ADDRESS__
  #define C89STR_ASAN __declspec(no_sanitize_address)
 #endif
#endif

#ifndef C89STR_ASAN
#define C89STR_ASAN
#endif

#ifndef C89STR_API_SPRINTF_DEF
#define C89STR_API_SPRINTF_DEF C89STR_API C89STR_ASAN
#endif

#ifndef C89STR_SPRINTF_MIN
#define C89STR_SPRINTF_MIN 512 
#endif

#ifndef C89STR_SPRINTF_MSVC_MODE 
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define C89STR_SPRINTF_MSVC_MODE
#endif
#endif

#ifdef C89STR_SPRINTF_NOUNALIGNED 
#define C89STR_UNALIGNED(code)
#else
#define C89STR_UNALIGNED(code) code
#endif

#ifndef C89STR_SPRINTF_NOFLOAT

static c89str_int32 c89str_real_to_str(char const* *start, c89str_uint32 *len, char* out, c89str_int32 *decimal_pos, double value, c89str_uint32 frac_digits);
static c89str_int32 c89str_real_to_parts(c89str_int64 *bits, c89str_int32 *expo, double value);
#define C89STR_SPECIAL 0x7000
#endif

static char c89str_period = '.';
static char c89str_comma = ',';
static struct
{
   short temp; 
   char pair[201];
} c89str_digitpair =
{
  0,
   "00010203040506070809101112131415161718192021222324"
   "25262728293031323334353637383940414243444546474849"
   "50515253545556575859606162636465666768697071727374"
   "75767778798081828384858687888990919293949596979899"
};

C89STR_API_SPRINTF_DEF void c89str_set_sprintf_separators(char pcomma, char pperiod)
{
   c89str_period = pperiod;
   c89str_comma = pcomma;
}

#define C89STR_LEFTJUST 1
#define C89STR_LEADINGPLUS 2
#define C89STR_LEADINGSPACE 4
#define C89STR_LEADING_0X 8
#define C89STR_LEADINGZERO 16
#define C89STR_INTMAX 32
#define C89STR_TRIPLET_COMMA 64
#define C89STR_NEGATIVE 128
#define C89STR_METRIC_SUFFIX 256
#define C89STR_HALFWIDTH 512
#define C89STR_METRIC_NOSPACE 1024
#define C89STR_METRIC_1024 2048
#define C89STR_METRIC_JEDEC 4096

static void c89str_lead_sign(c89str_uint32 fl, char* sign)
{
   sign[0] = 0;
   if (fl & C89STR_NEGATIVE) {
      sign[0] = 1;
      sign[1] = '-';
   } else if (fl & C89STR_LEADINGSPACE) {
      sign[0] = 1;
      sign[1] = ' ';
   } else if (fl & C89STR_LEADINGPLUS) {
      sign[0] = 1;
      sign[1] = '+';
   }
}

static C89STR_ASAN c89str_uint32 c89str_strlen_limited(char const* s, c89str_uint32 limit)
{
   char const*  sn = s;

   
   for (;;) {
      if (((c89str_uintptr)sn & 3) == 0)
         break;

      if (!limit || *sn == 0)
         return (c89str_uint32)(sn - s);

      ++sn;
      --limit;
   }

   
   
   
   
   
   while (limit >= 4) {
      c89str_uint32 v = *(c89str_uint32 *)sn;
      
      if ((v - 0x01010101) & (~v) & 0x80808080UL)
         break;

      sn += 4;
      limit -= 4;
   }

   
   while (limit && *sn) {
      ++sn;
      --limit;
   }

   return (c89str_uint32)(sn - s);
}

C89STR_API_SPRINTF_DEF int c89str_vsprintfcb(c89str_sprintf_callback* callback, void* user, char* buf, char const* fmt, va_list va)
{
   static char hex[] = "0123456789abcdefxp";
   static char hexu[] = "0123456789ABCDEFXP";
   char* bf;
   char const* f;
   int tlen = 0;

   bf = buf;
   f = fmt;
   for (;;) {
      c89str_int32 fw, pr, tz;
      c89str_uint32 fl;

      
      #define c89str_chk_cb_bufL(bytes)                        \
         {                                                     \
            int len = (int)(bf - buf);                         \
            if ((len + (bytes)) >= C89STR_SPRINTF_MIN) {          \
               tlen += len;                                    \
               if (0 == (bf = buf = callback(buf, user, len))) \
                  goto done;                                   \
            }                                                  \
         }
      #define c89str_chk_cb_buf(bytes)    \
         {                                \
            if (callback) {               \
               c89str_chk_cb_bufL(bytes); \
            }                             \
         }
      #define c89str_flush_cb()                      \
         {                                           \
            c89str_chk_cb_bufL(C89STR_SPRINTF_MIN - 1); \
         } 
      #define c89str_cb_buf_clamp(cl, v)                \
         cl = v;                                        \
         if (callback) {                                \
            int lg = C89STR_SPRINTF_MIN - (int)(bf - buf); \
            if (cl > lg)                                \
               cl = lg;                                 \
         }

      
      for (;;) {
         while (((c89str_uintptr)f) & 3) {
         schk1:
            if (f[0] == '%')
               goto scandd;
         schk2:
            if (f[0] == 0)
               goto endfmt;
            c89str_chk_cb_buf(1);
            *bf++ = f[0];
            ++f;
         }
         for (;;) {
            
            
            
            c89str_uint32 v, c;
            v = *(c89str_uint32 *)f;
            c = (~v) & 0x80808080;
            if (((v ^ 0x25252525) - 0x01010101) & c)
               goto schk1;
            if ((v - 0x01010101) & c)
               goto schk2;
            if (callback)
               if ((C89STR_SPRINTF_MIN - (int)(bf - buf)) < 4)
                  goto schk1;
            #ifdef C89STR_SPRINTF_NOUNALIGNED
                if(((c89str_uintptr)bf) & 3) {
                    bf[0] = f[0];
                    bf[1] = f[1];
                    bf[2] = f[2];
                    bf[3] = f[3];
                } else
            #endif
            {
                *(c89str_uint32 *)bf = v;
            }
            bf += 4;
            f += 4;
         }
      }
   scandd:

      ++f;

      
      fw = 0;
      pr = -1;
      fl = 0;
      tz = 0;

      
      for (;;) {
         switch (f[0]) {
         
         case '-':
            fl |= C89STR_LEFTJUST;
            ++f;
            continue;
         
         case '+':
            fl |= C89STR_LEADINGPLUS;
            ++f;
            continue;
         
         case ' ':
            fl |= C89STR_LEADINGSPACE;
            ++f;
            continue;
         
         case '#':
            fl |= C89STR_LEADING_0X;
            ++f;
            continue;
         
         case '\'':
            fl |= C89STR_TRIPLET_COMMA;
            ++f;
            continue;
         
         case '$':
            if (fl & C89STR_METRIC_SUFFIX) {
               if (fl & C89STR_METRIC_1024) {
                  fl |= C89STR_METRIC_JEDEC;
               } else {
                  fl |= C89STR_METRIC_1024;
               }
            } else {
               fl |= C89STR_METRIC_SUFFIX;
            }
            ++f;
            continue;
         
         case '_':
            fl |= C89STR_METRIC_NOSPACE;
            ++f;
            continue;
         
         case '0':
            fl |= C89STR_LEADINGZERO;
            ++f;
            goto flags_done;
         default: goto flags_done;
         }
      }
   flags_done:

      
      if (f[0] == '*') {
         fw = va_arg(va, c89str_uint32);
         ++f;
      } else {
         while ((f[0] >= '0') && (f[0] <= '9')) {
            fw = fw * 10 + f[0] - '0';
            f++;
         }
      }
      
      if (f[0] == '.') {
         ++f;
         if (f[0] == '*') {
            pr = va_arg(va, c89str_uint32);
            ++f;
         } else {
            pr = 0;
            while ((f[0] >= '0') && (f[0] <= '9')) {
               pr = pr * 10 + f[0] - '0';
               f++;
            }
         }
      }

      
      switch (f[0]) {
      
      case 'h':
         fl |= C89STR_HALFWIDTH;
         ++f;
         if (f[0] == 'h')
            ++f;  
         break;
      
      case 'l':
         fl |= ((sizeof(long) == 8) ? C89STR_INTMAX : 0);
         ++f;
         if (f[0] == 'l') {
            fl |= C89STR_INTMAX;
            ++f;
         }
         break;
      
      case 'j':
         fl |= (sizeof(size_t) == 8) ? C89STR_INTMAX : 0;
         ++f;
         break;
      
      case 'z':
         fl |= (sizeof(ptrdiff_t) == 8) ? C89STR_INTMAX : 0;
         ++f;
         break;
      case 't':
         fl |= (sizeof(ptrdiff_t) == 8) ? C89STR_INTMAX : 0;
         ++f;
         break;
      
      case 'I':
         if ((f[1] == '6') && (f[2] == '4')) {
            fl |= C89STR_INTMAX;
            f += 3;
         } else if ((f[1] == '3') && (f[2] == '2')) {
            f += 3;
         } else {
            fl |= ((sizeof(void* ) == 8) ? C89STR_INTMAX : 0);
            ++f;
         }
         break;
      default: break;
      }

      
      switch (f[0]) {
         #define C89STR_NUMSZ 512 
         char num[C89STR_NUMSZ];
         char lead[8];
         char tail[8];
         char* s;
         char const* h;
         c89str_uint32 l, n, cs;
         c89str_uint64 n64;
#ifndef C89STR_SPRINTF_NOFLOAT
         double fv;
#endif
         c89str_int32 dp;
         char const* sn;

      case 's':
         
         s = va_arg(va, char* );
         if (s == 0)
            s = (char* )"null";
         
         
         l = c89str_strlen_limited(s, (pr >= 0) ? (c89str_uint32)pr : ~0u);
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         
         goto scopy;

      case 'c': 
         
         s = num + C89STR_NUMSZ - 1;
         *s = (char)va_arg(va, int);
         l = 1;
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         goto scopy;

      case 'n': 
      {
         int *d = va_arg(va, int *);
         *d = tlen + (int)(bf - buf);
      } break;

#ifdef C89STR_SPRINTF_NOFLOAT
      case 'A':              
      case 'a':              
      case 'G':              
      case 'g':              
      case 'E':              
      case 'e':              
      case 'f':              
         va_arg(va, double); 
         s = (char* )"No float";
         l = 8;
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         cs = 0;
         C89STR_UNUSED(dp);
         goto scopy;
#else
      case 'A': 
      case 'a': 
         h = (f[0] == 'A') ? hexu : hex;
         fv = va_arg(va, double);
         if (pr == -1)
            pr = 6; 
         
         if (c89str_real_to_parts((c89str_int64 *)&n64, &dp, fv))
            fl |= C89STR_NEGATIVE;

         s = num + 64;

         c89str_lead_sign(fl, lead);

         if (dp == -1023)
            dp = (n64) ? -1022 : 0;
         else
            n64 |= (((c89str_uint64)1) << 52);
         n64 <<= (64 - 56);
         if (pr < 15)
            n64 += ((((c89str_uint64)8) << 56) >> (pr * 4));


#ifdef C89STR_SPRINTF_MSVC_MODE
         *s++ = '0';
         *s++ = 'x';
#else
         lead[1 + lead[0]] = '0';
         lead[2 + lead[0]] = 'x';
         lead[0] += 2;
#endif
         *s++ = h[(n64 >> 60) & 15];
         n64 <<= 4;
         if (pr)
            *s++ = c89str_period;
         sn = s;

         
         n = pr;
         if (n > 13)
            n = 13;
         if (pr > (c89str_int32)n)
            tz = pr - n;
         pr = 0;
         while (n--) {
            *s++ = h[(n64 >> 60) & 15];
            n64 <<= 4;
         }

         
         tail[1] = h[17];
         if (dp < 0) {
            tail[2] = '-';
            dp = -dp;
         } else
            tail[2] = '+';
         n = (dp >= 1000) ? 6 : ((dp >= 100) ? 5 : ((dp >= 10) ? 4 : 3));
         tail[0] = (char)n;
         for (;;) {
            tail[n] = '0' + dp % 10;
            if (n <= 3)
               break;
            --n;
            dp /= 10;
         }

         dp = (int)(s - sn);
         l = (int)(s - (num + 64));
         s = num + 64;
         cs = 1 + (3 << 24);
         goto scopy;

      case 'G': 
      case 'g': 
         h = (f[0] == 'G') ? hexu : hex;
         fv = va_arg(va, double);
         if (pr == -1)
            pr = 6;
         else if (pr == 0)
            pr = 1; 
         
         if (c89str_real_to_str(&sn, &l, num, &dp, fv, (pr - 1) | 0x80000000))
            fl |= C89STR_NEGATIVE;

         
         n = pr;
         if (l > (c89str_uint32)pr)
            l = pr;
         while ((l > 1) && (pr) && (sn[l - 1] == '0')) {
            --pr;
            --l;
         }

         
         if ((dp <= -4) || (dp > (c89str_int32)n)) {
            if (pr > (c89str_int32)l)
               pr = l - 1;
            else if (pr)
               --pr; 
            goto doexpfromg;
         }
         
         if (dp > 0) {
            pr = (dp < (c89str_int32)l) ? l - dp : 0;
         } else {
            pr = -dp + ((pr > (c89str_int32)l) ? (c89str_int32) l : pr);
         }
         goto dofloatfromg;

      case 'E': 
      case 'e': 
         h = (f[0] == 'E') ? hexu : hex;
         fv = va_arg(va, double);
         if (pr == -1)
            pr = 6; 
         
         if (c89str_real_to_str(&sn, &l, num, &dp, fv, pr | 0x80000000))
            fl |= C89STR_NEGATIVE;
      doexpfromg:
         tail[0] = 0;
         c89str_lead_sign(fl, lead);
         if (dp == C89STR_SPECIAL) {
            s = (char* )sn;
            cs = 0;
            pr = 0;
            goto scopy;
         }
         s = num + 64;
         
         *s++ = sn[0];

         if (pr)
            *s++ = c89str_period;

         
         if ((l - 1) > (c89str_uint32)pr)
            l = pr + 1;
         for (n = 1; n < l; n++)
            *s++ = sn[n];
         
         tz = pr - (l - 1);
         pr = 0;
         
         tail[1] = h[0xe];
         dp -= 1;
         if (dp < 0) {
            tail[2] = '-';
            dp = -dp;
         } else
            tail[2] = '+';
#ifdef C89STR_SPRINTF_MSVC_MODE
         n = 5;
#else
         n = (dp >= 100) ? 5 : 4;
#endif
         tail[0] = (char)n;
         for (;;) {
            tail[n] = '0' + dp % 10;
            if (n <= 3)
               break;
            --n;
            dp /= 10;
         }
         cs = 1 + (3 << 24); 
         goto flt_lead;

      case 'f': 
         fv = va_arg(va, double);
      doafloat:
         
         if (fl & C89STR_METRIC_SUFFIX) {
            double divisor;
            divisor = 1000.0f;
            if (fl & C89STR_METRIC_1024)
               divisor = 1024.0;
            while (fl < 0x4000000) {
               if ((fv < divisor) && (fv > -divisor))
                  break;
               fv /= divisor;
               fl += 0x1000000;
            }
         }
         if (pr == -1)
            pr = 6; 
         
         if (c89str_real_to_str(&sn, &l, num, &dp, fv, pr))
            fl |= C89STR_NEGATIVE;
      dofloatfromg:
         tail[0] = 0;
         c89str_lead_sign(fl, lead);
         if (dp == C89STR_SPECIAL) {
            s = (char* )sn;
            cs = 0;
            pr = 0;
            goto scopy;
         }
         s = num + 64;

         
         if (dp <= 0) {
            c89str_int32 i;
            
            *s++ = '0';
            if (pr)
               *s++ = c89str_period;
            n = -dp;
            if ((c89str_int32)n > pr)
               n = pr;
            i = n;
            while (i) {
               if ((((c89str_uintptr)s) & 3) == 0)
                  break;
               *s++ = '0';
               --i;
            }
            while (i >= 4) {
               *(c89str_uint32 *)s = 0x30303030;
               s += 4;
               i -= 4;
            }
            while (i) {
               *s++ = '0';
               --i;
            }
            if ((c89str_int32)(l + n) > pr)
               l = pr - n;
            i = l;
            while (i) {
               *s++ = *sn++;
               --i;
            }
            tz = pr - (n + l);
            cs = 1 + (3 << 24); 
         } else {
            cs = (fl & C89STR_TRIPLET_COMMA) ? ((600 - (c89str_uint32)dp) % 3) : 0;
            if ((c89str_uint32)dp >= l) {
               
               n = 0;
               for (;;) {
                  if ((fl & C89STR_TRIPLET_COMMA) && (++cs == 4)) {
                     cs = 0;
                     *s++ = c89str_comma;
                  } else {
                     *s++ = sn[n];
                     ++n;
                     if (n >= l)
                        break;
                  }
               }
               if (n < (c89str_uint32)dp) {
                  n = dp - n;
                  if ((fl & C89STR_TRIPLET_COMMA) == 0) {
                     while (n) {
                        if ((((c89str_uintptr)s) & 3) == 0)
                           break;
                        *s++ = '0';
                        --n;
                     }
                     while (n >= 4) {
                        *(c89str_uint32 *)s = 0x30303030;
                        s += 4;
                        n -= 4;
                     }
                  }
                  while (n) {
                     if ((fl & C89STR_TRIPLET_COMMA) && (++cs == 4)) {
                        cs = 0;
                        *s++ = c89str_comma;
                     } else {
                        *s++ = '0';
                        --n;
                     }
                  }
               }
               cs = (int)(s - (num + 64)) + (3 << 24); 
               if (pr) {
                  *s++ = c89str_period;
                  tz = pr;
               }
            } else {
               
               n = 0;
               for (;;) {
                  if ((fl & C89STR_TRIPLET_COMMA) && (++cs == 4)) {
                     cs = 0;
                     *s++ = c89str_comma;
                  } else {
                     *s++ = sn[n];
                     ++n;
                     if (n >= (c89str_uint32)dp)
                        break;
                  }
               }
               cs = (int)(s - (num + 64)) + (3 << 24); 
               if (pr)
                  *s++ = c89str_period;
               if ((l - dp) > (c89str_uint32)pr)
                  l = pr + dp;
               while (n < l) {
                  *s++ = sn[n];
                  ++n;
               }
               tz = pr - (l - dp);
            }
         }
         pr = 0;

         
         if (fl & C89STR_METRIC_SUFFIX) {
            char idx;
            idx = 1;
            if (fl & C89STR_METRIC_NOSPACE)
               idx = 0;
            tail[0] = idx;
            tail[1] = ' ';
            {
               if (fl >> 24) { 
                  if (fl & C89STR_METRIC_1024)
                     tail[idx + 1] = "_KMGT"[fl >> 24];
                  else
                     tail[idx + 1] = "_kMGT"[fl >> 24];
                  idx++;
                  
                  if (fl & C89STR_METRIC_1024 && !(fl & C89STR_METRIC_JEDEC)) {
                     tail[idx + 1] = 'i';
                     idx++;
                  }
                  tail[0] = idx;
               }
            }
         };

      flt_lead:
         
         l = (c89str_uint32)(s - (num + 64));
         s = num + 64;
         goto scopy;
#endif

      case 'B': 
      case 'b': 
         h = (f[0] == 'B') ? hexu : hex;
         lead[0] = 0;
         if (fl & C89STR_LEADING_0X) {
            lead[0] = 2;
            lead[1] = '0';
            lead[2] = h[0xb];
         }
         l = (8 << 4) | (1 << 8);
         goto radixnum;

      case 'o': 
         h = hexu;
         lead[0] = 0;
         if (fl & C89STR_LEADING_0X) {
            lead[0] = 1;
            lead[1] = '0';
         }
         l = (3 << 4) | (3 << 8);
         goto radixnum;

      case 'p': 
         fl |= (sizeof(void* ) == 8) ? C89STR_INTMAX : 0;
         pr = sizeof(void* ) * 2;
         fl &= ~C89STR_LEADINGZERO; 
                                    

      case 'X': 
      case 'x': 
         h = (f[0] == 'X') ? hexu : hex;
         l = (4 << 4) | (4 << 8);
         lead[0] = 0;
         if (fl & C89STR_LEADING_0X) {
            lead[0] = 2;
            lead[1] = '0';
            lead[2] = h[16];
         }
      radixnum:
         
         if (fl & C89STR_INTMAX)
            n64 = va_arg(va, c89str_uint64);
         else
            n64 = va_arg(va, c89str_uint32);

         s = num + C89STR_NUMSZ;
         dp = 0;
         
         tail[0] = 0;
         if (n64 == 0) {
            lead[0] = 0;
            if (pr == 0) {
               l = 0;
               cs = 0;
               goto scopy;
            }
         }
         
         for (;;) {
            *--s = h[n64 & ((1 << (l >> 8)) - 1)];
            n64 >>= (l >> 8);
            if (!((n64) || ((c89str_int32)((num + C89STR_NUMSZ) - s) < pr)))
               break;
            if (fl & C89STR_TRIPLET_COMMA) {
               ++l;
               if ((l & 15) == ((l >> 4) & 15)) {
                  l &= ~15;
                  *--s = c89str_comma;
               }
            }
         };
         
         cs = (c89str_uint32)((num + C89STR_NUMSZ) - s) + ((((l >> 4) & 15)) << 24);
         
         l = (c89str_uint32)((num + C89STR_NUMSZ) - s);
         
         goto scopy;

      case 'u': 
      case 'i':
      case 'd': 
         
         if (fl & C89STR_INTMAX) {
            c89str_int64 i64 = va_arg(va, c89str_int64);
            n64 = (c89str_uint64)i64;
            if ((f[0] != 'u') && (i64 < 0)) {
               n64 = (c89str_uint64)-i64;
               fl |= C89STR_NEGATIVE;
            }
         } else {
            c89str_int32 i = va_arg(va, c89str_int32);
            n64 = (c89str_uint32)i;
            if ((f[0] != 'u') && (i < 0)) {
               n64 = (c89str_uint32)-i;
               fl |= C89STR_NEGATIVE;
            }
         }

#ifndef C89STR_SPRINTF_NOFLOAT
         if (fl & C89STR_METRIC_SUFFIX) {
            if (n64 < 1024)
               pr = 0;
            else if (pr == -1)
               pr = 1;
            fv = (double)(c89str_int64)n64;
            goto doafloat;
         }
#endif

         
         s = num + C89STR_NUMSZ;
         l = 0;

         for (;;) {
            
            char* o = s - 8;
            if (n64 >= 100000000) {
               n = (c89str_uint32)(n64 % 100000000);
               n64 /= 100000000;
            } else {
               n = (c89str_uint32)n64;
               n64 = 0;
            }
            if ((fl & C89STR_TRIPLET_COMMA) == 0) {
               do {
                  s -= 2;
                  *(c89str_uint16 *)s = *(c89str_uint16 *)&c89str_digitpair.pair[(n % 100) * 2];
                  n /= 100;
               } while (n);
            }
            while (n) {
               if ((fl & C89STR_TRIPLET_COMMA) && (l++ == 3)) {
                  l = 0;
                  *--s = c89str_comma;
                  --o;
               } else {
                  *--s = (char)(n % 10) + '0';
                  n /= 10;
               }
            }
            if (n64 == 0) {
               if ((s[0] == '0') && (s != (num + C89STR_NUMSZ)))
                  ++s;
               break;
            }
            while (s != o)
               if ((fl & C89STR_TRIPLET_COMMA) && (l++ == 3)) {
                  l = 0;
                  *--s = c89str_comma;
                  --o;
               } else {
                  *--s = '0';
               }
         }

         tail[0] = 0;
         c89str_lead_sign(fl, lead);

         
         l = (c89str_uint32)((num + C89STR_NUMSZ) - s);
         if (l == 0) {
            *--s = '0';
            l = 1;
         }
         cs = l + (3 << 24);
         if (pr < 0)
            pr = 0;

      scopy:
         
         if (pr < (c89str_int32)l)
            pr = l;
         n = pr + lead[0] + tail[0] + tz;
         if (fw < (c89str_int32)n)
            fw = n;
         fw -= n;
         pr -= l;

         
         if ((fl & C89STR_LEFTJUST) == 0) {
            if (fl & C89STR_LEADINGZERO) 
            {
               pr = (fw > pr) ? fw : pr;
               fw = 0;
            } else {
               fl &= ~C89STR_TRIPLET_COMMA; 
            }
         }

         
         if (fw + pr) {
            c89str_int32 i;
            c89str_uint32 c;

            
            if ((fl & C89STR_LEFTJUST) == 0)
               while (fw > 0) {
                  c89str_cb_buf_clamp(i, fw);
                  fw -= i;
                  while (i) {
                     if ((((c89str_uintptr)bf) & 3) == 0)
                        break;
                     *bf++ = ' ';
                     --i;
                  }
                  while (i >= 4) {
                     *(c89str_uint32 *)bf = 0x20202020;
                     bf += 4;
                     i -= 4;
                  }
                  while (i) {
                     *bf++ = ' ';
                     --i;
                  }
                  c89str_chk_cb_buf(1);
               }

            
            sn = lead + 1;
            while (lead[0]) {
               c89str_cb_buf_clamp(i, lead[0]);
               lead[0] -= (char)i;
               while (i) {
                  *bf++ = *sn++;
                  --i;
               }
               c89str_chk_cb_buf(1);
            }

            
            c = cs >> 24;
            cs &= 0xffffff;
            cs = (fl & C89STR_TRIPLET_COMMA) ? ((c89str_uint32)(c - ((pr + cs) % (c + 1)))) : 0;
            while (pr > 0) {
               c89str_cb_buf_clamp(i, pr);
               pr -= i;
               if ((fl & C89STR_TRIPLET_COMMA) == 0) {
                  while (i) {
                     if ((((c89str_uintptr)bf) & 3) == 0)
                        break;
                     *bf++ = '0';
                     --i;
                  }
                  while (i >= 4) {
                     *(c89str_uint32 *)bf = 0x30303030;
                     bf += 4;
                     i -= 4;
                  }
               }
               while (i) {
                  if ((fl & C89STR_TRIPLET_COMMA) && (cs++ == c)) {
                     cs = 0;
                     *bf++ = c89str_comma;
                  } else
                     *bf++ = '0';
                  --i;
               }
               c89str_chk_cb_buf(1);
            }
         }

         
         sn = lead + 1;
         while (lead[0]) {
            c89str_int32 i;
            c89str_cb_buf_clamp(i, lead[0]);
            lead[0] -= (char)i;
            while (i) {
               *bf++ = *sn++;
               --i;
            }
            c89str_chk_cb_buf(1);
         }

         
         n = l;
         while (n) {
            c89str_int32 i;
            c89str_cb_buf_clamp(i, n);
            n -= i;
            C89STR_UNALIGNED(while (i >= 4) {
               *(c89str_uint32 volatile *)bf = *(c89str_uint32 volatile *)s;
               bf += 4;
               s += 4;
               i -= 4;
            })
            while (i) {
               *bf++ = *s++;
               --i;
            }
            c89str_chk_cb_buf(1);
         }

         
         while (tz) {
            c89str_int32 i;
            c89str_cb_buf_clamp(i, tz);
            tz -= i;
            while (i) {
               if ((((c89str_uintptr)bf) & 3) == 0)
                  break;
               *bf++ = '0';
               --i;
            }
            while (i >= 4) {
               *(c89str_uint32 *)bf = 0x30303030;
               bf += 4;
               i -= 4;
            }
            while (i) {
               *bf++ = '0';
               --i;
            }
            c89str_chk_cb_buf(1);
         }

         
         sn = tail + 1;
         while (tail[0]) {
            c89str_int32 i;
            c89str_cb_buf_clamp(i, tail[0]);
            tail[0] -= (char)i;
            while (i) {
               *bf++ = *sn++;
               --i;
            }
            c89str_chk_cb_buf(1);
         }

         
         if (fl & C89STR_LEFTJUST)
            if (fw > 0) {
               while (fw) {
                  c89str_int32 i;
                  c89str_cb_buf_clamp(i, fw);
                  fw -= i;
                  while (i) {
                     if ((((c89str_uintptr)bf) & 3) == 0)
                        break;
                     *bf++ = ' ';
                     --i;
                  }
                  while (i >= 4) {
                     *(c89str_uint32 *)bf = 0x20202020;
                     bf += 4;
                     i -= 4;
                  }
                  while (i--)
                     *bf++ = ' ';
                  c89str_chk_cb_buf(1);
               }
            }
         break;

      default: 
         s = num + C89STR_NUMSZ - 1;
         *s = f[0];
         l = 1;
         fw = fl = 0;
         lead[0] = 0;
         tail[0] = 0;
         pr = 0;
         dp = 0;
         cs = 0;
         goto scopy;
      }
      ++f;
   }
endfmt:

   if (!callback)
      *bf = 0;
   else
      c89str_flush_cb();

done:
   return tlen + (int)(bf - buf);
}


#undef C89STR_LEFTJUST
#undef C89STR_LEADINGPLUS
#undef C89STR_LEADINGSPACE
#undef C89STR_LEADING_0X
#undef C89STR_LEADINGZERO
#undef C89STR_INTMAX
#undef C89STR_TRIPLET_COMMA
#undef C89STR_NEGATIVE
#undef C89STR_METRIC_SUFFIX
#undef C89STR_NUMSZ
#undef c89str_chk_cb_bufL
#undef c89str_chk_cb_buf
#undef c89str_flush_cb
#undef c89str_cb_buf_clamp




C89STR_API_SPRINTF_DEF int c89str_sprintf(char* buf, char const* fmt, ...)
{
   int result;
   va_list va;
   va_start(va, fmt);
   result = c89str_vsprintfcb(0, 0, buf, fmt, va);
   va_end(va);
   return result;
}

typedef struct c89str_sprintf_context {
   char* buf;
   size_t count;
   size_t length;
   char tmp[C89STR_SPRINTF_MIN];
} c89str_sprintf_context;

static char* c89str_clamp_callback(const char* buf, void* user, size_t len)
{
   c89str_sprintf_context *c = (c89str_sprintf_context *)user;
   c->length += len;

   if (len > c->count)
      len = c->count;

   if (len) {
      if (buf != c->buf) {
         const char* s, *se;
         char* d;
         d = c->buf;
         s = buf;
         se = buf + len;
         do {
            *d++ = *s++;
         } while (s < se);
      }
      c->buf += len;
      c->count -= len;
   }

   if (c->count <= 0)
      return c->tmp;
   return (c->count >= C89STR_SPRINTF_MIN) ? c->buf : c->tmp; 
}

static char*  c89str_count_clamp_callback( const char*  buf, void*  user, size_t len )
{
   c89str_sprintf_context * c = (c89str_sprintf_context*)user;
   (void) sizeof(buf);

   c->length += len;
   return c->tmp; 
}

C89STR_API_SPRINTF_DEF int c89str_vsnprintf( char*  buf, size_t count, char const*  fmt, va_list va )
{
   c89str_sprintf_context c;

   if ( (count == 0) && !buf )
   {
      c.length = 0;

      c89str_vsprintfcb( c89str_count_clamp_callback, &c, c.tmp, fmt, va );
   }
   else
   {
      size_t l;

      c.buf = buf;
      c.count = count;
      c.length = 0;

      c89str_vsprintfcb( c89str_clamp_callback, &c, c89str_clamp_callback(0,&c,0), fmt, va );

      
      l = (size_t)( c.buf - buf );
      if ( l >= count ) 
         l = count - 1;
      buf[l] = 0;
   }

   return (int)c.length;
}

C89STR_API_SPRINTF_DEF int c89str_snprintf(char* buf, size_t count, char const* fmt, ...)
{
   int result;
   va_list va;
   va_start(va, fmt);

   result = c89str_vsnprintf(buf, count, fmt, va);
   va_end(va);

   return result;
}

C89STR_API_SPRINTF_DEF int c89str_vsprintf(char* buf, char const* fmt, va_list va)
{
   return c89str_vsprintfcb(0, 0, buf, fmt, va);
}




#ifndef C89STR_SPRINTF_NOFLOAT


#define C89STR_COPYFP(dest, src)                   \
   {                                               \
      int cn;                                      \
      for (cn = 0; cn < 8; cn++)                   \
         ((char* )&dest)[cn] = ((char* )&src)[cn]; \
   }


static c89str_int32 c89str_real_to_parts(c89str_int64 *bits, c89str_int32 *expo, double value)
{
   double d;
   c89str_int64 b = 0;

   
   d = value;

   C89STR_COPYFP(b, d);

   *bits = b & ((((c89str_uint64)1) << 52) - 1);
   *expo = (c89str_int32)(((b >> 52) & 2047) - 1023);

   return (c89str_int32)((c89str_uint64) b >> 63);
}

static double const c89str_bot[23] = {
   1e+000, 1e+001, 1e+002, 1e+003, 1e+004, 1e+005, 1e+006, 1e+007, 1e+008, 1e+009, 1e+010, 1e+011,
   1e+012, 1e+013, 1e+014, 1e+015, 1e+016, 1e+017, 1e+018, 1e+019, 1e+020, 1e+021, 1e+022
};
static double const c89str_negbot[22] = {
   1e-001, 1e-002, 1e-003, 1e-004, 1e-005, 1e-006, 1e-007, 1e-008, 1e-009, 1e-010, 1e-011,
   1e-012, 1e-013, 1e-014, 1e-015, 1e-016, 1e-017, 1e-018, 1e-019, 1e-020, 1e-021, 1e-022
};
static double const c89str_negboterr[22] = {
   -5.551115123125783e-018,  -2.0816681711721684e-019, -2.0816681711721686e-020, -4.7921736023859299e-021, -8.1803053914031305e-022, 4.5251888174113741e-023,
   4.5251888174113739e-024,  -2.0922560830128471e-025, -6.2281591457779853e-026, -3.6432197315497743e-027, 6.0503030718060191e-028,  2.0113352370744385e-029,
   -3.0373745563400371e-030, 1.1806906454401013e-032,  -7.7705399876661076e-032, 2.0902213275965398e-033,  -7.1542424054621921e-034, -7.1542424054621926e-035,
   2.4754073164739869e-036,  5.4846728545790429e-037,  9.2462547772103625e-038,  -4.8596774326570872e-039
};
static double const c89str_top[13] = {
   1e+023, 1e+046, 1e+069, 1e+092, 1e+115, 1e+138, 1e+161, 1e+184, 1e+207, 1e+230, 1e+253, 1e+276, 1e+299
};
static double const c89str_negtop[13] = {
   1e-023, 1e-046, 1e-069, 1e-092, 1e-115, 1e-138, 1e-161, 1e-184, 1e-207, 1e-230, 1e-253, 1e-276, 1e-299
};
static double const c89str_toperr[13] = {
   8388608,
   6.8601809640529717e+028,
   -7.253143638152921e+052,
   -4.3377296974619174e+075,
   -1.5559416129466825e+098,
   -3.2841562489204913e+121,
   -3.7745893248228135e+144,
   -1.7356668416969134e+167,
   -3.8893577551088374e+190,
   -9.9566444326005119e+213,
   6.3641293062232429e+236,
   -5.2069140800249813e+259,
   -5.2504760255204387e+282
};
static double const c89str_negtoperr[13] = {
   3.9565301985100693e-040,  -2.299904345391321e-063,  3.6506201437945798e-086,  1.1875228833981544e-109,
   -5.0644902316928607e-132, -6.7156837247865426e-155, -2.812077463003139e-178,  -5.7778912386589953e-201,
   7.4997100559334532e-224,  -4.6439668915134491e-247, -6.3691100762962136e-270, -9.436808465446358e-293,
   8.0970921678014997e-317
};

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
static c89str_uint64 const c89str_powten[20] = {
   1,
   10,
   100,
   1000,
   10000,
   100000,
   1000000,
   10000000,
   100000000,
   1000000000,
   10000000000,
   100000000000,
   1000000000000,
   10000000000000,
   100000000000000,
   1000000000000000,
   10000000000000000,
   100000000000000000,
   1000000000000000000,
   10000000000000000000U
};
#define c89str_tento19th ((c89str_uint64)1000000000000000000)
#else
static c89str_uint64 const c89str_powten[20] = {
   1,
   10,
   100,
   1000,
   10000,
   100000,
   1000000,
   10000000,
   100000000,
   1000000000,
   10000000000ULL,
   100000000000ULL,
   1000000000000ULL,
   10000000000000ULL,
   100000000000000ULL,
   1000000000000000ULL,
   10000000000000000ULL,
   100000000000000000ULL,
   1000000000000000000ULL,
   10000000000000000000ULL
};
#define c89str_tento19th (1000000000000000000ULL)
#endif

#define c89str_ddmulthi(oh, ol, xh, yh)                            \
   {                                                               \
      double ahi = 0, alo, bhi = 0, blo;                           \
      c89str_int64 bt;                                             \
      oh = xh * yh;                                                \
      C89STR_COPYFP(bt, xh);                                       \
      bt &= ((~(c89str_uint64)0) << 27);                           \
      C89STR_COPYFP(ahi, bt);                                      \
      alo = xh - ahi;                                              \
      C89STR_COPYFP(bt, yh);                                       \
      bt &= ((~(c89str_uint64)0) << 27);                           \
      C89STR_COPYFP(bhi, bt);                                      \
      blo = yh - bhi;                                              \
      ol = ((ahi * bhi - oh) + ahi * blo + alo * bhi) + alo * blo; \
   }

#define c89str_ddtoS64(ob, xh, xl)          \
   {                                        \
      double ahi = 0, alo, vh, t;           \
      ob = (c89str_int64)xh;                \
      vh = (double)ob;                      \
      ahi = (xh - vh);                      \
      t = (ahi - xh);                       \
      alo = (xh - (ahi - t)) - (vh + t);    \
      ob += (c89str_int64)(ahi + alo + xl); \
   }

#define c89str_ddrenorm(oh, ol) \
   {                            \
      double s;                 \
      s = oh + ol;              \
      ol = ol - (s - oh);       \
      oh = s;                   \
   }

#define c89str_ddmultlo(oh, ol, xh, xl, yh, yl) ol = ol + (xh * yl + xl * yh);

#define c89str_ddmultlos(oh, ol, xh, yl) ol = ol + (xh * yl);

static void c89str_raise_to_power10(double *ohi, double *olo, double d, c89str_int32 power) 
{
   double ph, pl;
   if ((power >= 0) && (power <= 22)) {
      c89str_ddmulthi(ph, pl, d, c89str_bot[power]);
   } else {
      c89str_int32 e, et, eb;
      double p2h, p2l;

      e = power;
      if (power < 0)
         e = -e;
      et = (e * 0x2c9) >> 14; 
      if (et > 13)
         et = 13;
      eb = e - (et * 23);

      ph = d;
      pl = 0.0;
      if (power < 0) {
         if (eb) {
            --eb;
            c89str_ddmulthi(ph, pl, d, c89str_negbot[eb]);
            c89str_ddmultlos(ph, pl, d, c89str_negboterr[eb]);
         }
         if (et) {
            c89str_ddrenorm(ph, pl);
            --et;
            c89str_ddmulthi(p2h, p2l, ph, c89str_negtop[et]);
            c89str_ddmultlo(p2h, p2l, ph, pl, c89str_negtop[et], c89str_negtoperr[et]);
            ph = p2h;
            pl = p2l;
         }
      } else {
         if (eb) {
            e = eb;
            if (eb > 22)
               eb = 22;
            e -= eb;
            c89str_ddmulthi(ph, pl, d, c89str_bot[eb]);
            if (e) {
               c89str_ddrenorm(ph, pl);
               c89str_ddmulthi(p2h, p2l, ph, c89str_bot[e]);
               c89str_ddmultlos(p2h, p2l, c89str_bot[e], pl);
               ph = p2h;
               pl = p2l;
            }
         }
         if (et) {
            c89str_ddrenorm(ph, pl);
            --et;
            c89str_ddmulthi(p2h, p2l, ph, c89str_top[et]);
            c89str_ddmultlo(p2h, p2l, ph, pl, c89str_top[et], c89str_toperr[et]);
            ph = p2h;
            pl = p2l;
         }
      }
   }
   c89str_ddrenorm(ph, pl);
   *ohi = ph;
   *olo = pl;
}





static c89str_int32 c89str_real_to_str(char const* *start, c89str_uint32 *len, char* out, c89str_int32 *decimal_pos, double value, c89str_uint32 frac_digits)
{
   double d;
   c89str_int64 bits = 0;
   c89str_int32 expo, e, ng, tens;

   d = value;
   C89STR_COPYFP(bits, d);
   expo = (c89str_int32)((bits >> 52) & 2047);
   ng = (c89str_int32)((c89str_uint64) bits >> 63);
   if (ng)
      d = -d;

   if (expo == 2047) 
   {
      *start = (bits & ((((c89str_uint64)1) << 52) - 1)) ? "NaN" : "Inf";
      *decimal_pos = C89STR_SPECIAL;
      *len = 3;
      return ng;
   }

   if (expo == 0) 
   {
      if (((c89str_uint64) bits << 1) == 0) 
      {
         *decimal_pos = 1;
         *start = out;
         out[0] = '0';
         *len = 1;
         return ng;
      }
      
      {
         c89str_int64 v = ((c89str_uint64)1) << 51;
         while ((bits & v) == 0) {
            --expo;
            v >>= 1;
         }
      }
   }

   
   {
      double ph, pl;

      
      tens = expo - 1023;
      tens = (tens < 0) ? ((tens * 617) / 2048) : (((tens * 1233) / 4096) + 1);

      
      c89str_raise_to_power10(&ph, &pl, d, 18 - tens);

      
      c89str_ddtoS64(bits, ph, pl);

      
      if (((c89str_uint64)bits) >= c89str_tento19th)
         ++tens;
   }

   
   frac_digits = (frac_digits & 0x80000000) ? ((frac_digits & 0x7ffffff) + 1) : (tens + frac_digits);
   if ((frac_digits < 24)) {
      c89str_uint32 dg = 1;
      if ((c89str_uint64)bits >= c89str_powten[9])
         dg = 10;
      while ((c89str_uint64)bits >= c89str_powten[dg]) {
         ++dg;
         if (dg == 20)
            goto noround;
      }
      if (frac_digits < dg) {
         c89str_uint64 r;
         
         e = dg - frac_digits;
         if ((c89str_uint32)e >= 24)
            goto noround;
         r = c89str_powten[e];
         bits = bits + (r / 2);
         if ((c89str_uint64)bits >= c89str_powten[dg])
            ++tens;
         bits /= r;
      }
   noround:;
   }

   
   if (bits) {
      c89str_uint32 n;
      for (;;) {
         if (bits <= 0xffffffff)
            break;
         if (bits % 1000)
            goto donez;
         bits /= 1000;
      }
      n = (c89str_uint32)bits;
      while ((n % 1000) == 0)
         n /= 1000;
      bits = n;
   donez:;
   }

   
   out += 64;
   e = 0;
   for (;;) {
      c89str_uint32 n;
      char* o = out - 8;
      
      if (bits >= 100000000) {
         n = (c89str_uint32)(bits % 100000000);
         bits /= 100000000;
      } else {
         n = (c89str_uint32)bits;
         bits = 0;
      }
      while (n) {
         out -= 2;
         *(c89str_uint16 *)out = *(c89str_uint16 *)&c89str_digitpair.pair[(n % 100) * 2];
         n /= 100;
         e += 2;
      }
      if (bits == 0) {
         if ((e) && (out[0] == '0')) {
            ++out;
            --e;
         }
         break;
      }
      while (out != o) {
         *--out = '0';
         ++e;
      }
   }

   *decimal_pos = tens;
   *start = out;
   *len = e;
   return ng;
}

#undef c89str_ddmulthi
#undef c89str_ddrenorm
#undef c89str_ddmultlo
#undef c89str_ddmultlos
#undef C89STR_SPECIAL
#undef C89STR_COPYFP

#endif 


#undef C89STR_UNALIGNED
/* end stb_sprintf.c */

#if defined(__GNUC__) && (__GNUC__ >= 7 || (__GNUC__ == 6 && __GNUC_MINOR__ >= 1))
    #pragma GCC diagnostic pop
#endif



#endif  /* c89str_c */
#endif  /* C89STR_IMPLEMENTATION */

/*
This software is available as a choice of the following licenses. Choose
whichever you prefer.

===============================================================================
ALTERNATIVE 1 - Public Domain (www.unlicense.org)
===============================================================================
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>

===============================================================================
ALTERNATIVE 2 - MIT No Attribution
===============================================================================
Copyright 2023 David Reid

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
