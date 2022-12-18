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
*/

#ifndef c89str_h
#define c89str_h

#include <stddef.h> /* For NULL, size_t */
#include <stdarg.h> /* For va_list */
#include <errno.h>  /* For errno_t */

#ifndef C89STR_API
#define C89STR_API extern
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


typedef unsigned int    c89str_bool32;
#define C89STR_TRUE  1
#define C89STR_FALSE 0

typedef char            c89str_utf8;
typedef unsigned short  c89str_utf16;
typedef unsigned int    c89str_utf32;

#define c89str_npos  ((size_t)-1)

/*
Custom error codes. We use errno_t types and are using negative numbers, starting from bit 14 to
try our best to avoid clashing. However, since we do not modify the global errno value and aren't
calling any system calls, it shouldn't actually matter if something else coincidentally uses the
same code. So long as we don't clash with any of the common ones, which should be unlikely as they
all seems to be positive in all of the implementations I've seen, we should be OK.
*/
#define C89STR_SUCCESS    0      /* No error. */
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
C89STR_API size_t c89str_strlen(const char* src);
C89STR_API char* c89str_strcpy(char* dst, const char* src);
C89STR_API int c89str_strcpy_s(char* dst, size_t dstCap, const char* src);
C89STR_API int c89str_strncpy_s(char* dst, size_t dstCap, const char* src, size_t count);
C89STR_API int c89str_strcat_s(char* dst, size_t dstCap, const char* src);
C89STR_API int c89str_strncat_s(char* dst, size_t dstCap, const char* src, size_t count);
C89STR_API int c89str_itoa_s(int value, char* dst, size_t dstCap, int radix);


/* Miscellaneous Helpers */
#define c89str_is_null_or_empty(str) ((str) == NULL || (str)[0] == 0)
C89STR_API errno_t c89str_find(const char* str, const char* other, size_t* pResult);  /* Returns NOENT if the string cannot be found, and sets pResult to c89str_npos. */
C89STR_API errno_t c89str_findn(const char* str, size_t strLen, const char* other, size_t otherLen, size_t* pResult);
C89STR_API int c89str_strcmp(const char* str1, const char* str2);
C89STR_API int c89str_strncmp(const char* str1, const char* str2, size_t maxLen);
C89STR_API int c89str_strncmpn(const char* str1, size_t str1Len, const char* str2, size_t str2Len);
C89STR_API c89str_bool32 c89str_begins_with(const char* str1, size_t str1Len, const char* str2, size_t str2Len); /* Returns 0 if str1 begins with str2. */
C89STR_API c89str_bool32 c89str_ends_with(const char* str1, size_t str1Len, const char* str2, size_t str2Len); /* Returns 0 if str1 ends with str2. */



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
C89STR_API errno_t utf16ne_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t utf16le_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t utf16be_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);

C89STR_API errno_t utf16ne_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t utf16le_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t utf16be_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);

C89STR_API errno_t utf16ne_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t utf16le_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t utf16be_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
static C89STR_INLINE errno_t utf16ne_to_utf32ne_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags) { return utf16ne_to_utf32_len(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags); }
static C89STR_INLINE errno_t utf16le_to_utf32le_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags) { return utf16le_to_utf32_len(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags); }
static C89STR_INLINE errno_t utf16be_to_utf32be_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags) { return utf16be_to_utf32_len(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags); }
C89STR_API errno_t c89str_utf16_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);

C89STR_API errno_t utf16ne_to_utf32ne(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t utf16le_to_utf32le(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t utf16be_to_utf32be(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf16_to_utf32(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags);


/* UTF-32 */
C89STR_API errno_t utf32ne_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t utf32le_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t utf32be_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);

C89STR_API errno_t utf32ne_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t utf32le_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t utf32be_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);

C89STR_API errno_t utf32ne_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t utf32le_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t utf32be_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
static C89STR_INLINE errno_t utf32ne_to_utf16ne_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags) { return utf32ne_to_utf16_len(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags); }
static C89STR_INLINE errno_t utf32le_to_utf16le_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags) { return utf32le_to_utf16_len(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags); }
static C89STR_INLINE errno_t utf32be_to_utf16be_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags) { return utf32be_to_utf16_len(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags); }
C89STR_API errno_t c89str_utf32_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);

C89STR_API errno_t utf32ne_to_utf16ne(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t utf32le_to_utf16le(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t utf32be_to_utf16be(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);
C89STR_API errno_t c89str_utf32_to_utf16(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags);


/* UTF-32 */
C89STR_API c89str_bool32 c89str_utf32_is_null_or_whitespace(const c89str_utf32* pUTF32, size_t utf32Len);

/* UTF-16 */

/* UTF-8 */
C89STR_API c89str_bool32 c89str_utf8_is_null_or_whitespace(const c89str_utf8* pUTF8, size_t utf8Len);
C89STR_API size_t c89str_utf8_next_whitespace(const c89str_utf8* pUTF8, size_t utf8Len);
C89STR_API size_t c89str_utf8_ltrim_offset(const c89str_utf8* pUTF8, size_t utf8Len);
C89STR_API size_t c89str_utf8_rtrim_offset(const c89str_utf8* pUTF8, size_t utf8Len);
C89STR_API size_t c89str_utf8_next_line(const c89str_utf8* pUTF8, size_t utf8Len, size_t* pThisLineLen);


/* Dynamic String API */
typedef char* c89str;
C89STR_API errno_t c89str_delete(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks);
C89STR_API errno_t c89str_new(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther);
C89STR_API errno_t c89str_newn(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen);
C89STR_API errno_t c89str_newv(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args);
C89STR_API errno_t c89str_newf(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...) C89STR_ATTRIBUTE_FORMAT(3, 4);
C89STR_API errno_t c89str_set(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther);
C89STR_API errno_t c89str_setn(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen);
C89STR_API errno_t c89str_setv(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args);
C89STR_API errno_t c89str_setf(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...) C89STR_ATTRIBUTE_FORMAT(3, 4);
C89STR_API errno_t c89str_cat(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther);
C89STR_API errno_t c89str_catn(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen);
C89STR_API errno_t c89str_catv(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args);
C89STR_API errno_t c89str_catf(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...) C89STR_ATTRIBUTE_FORMAT(3, 4);
C89STR_API errno_t c89str_remove(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, size_t beg, size_t end);
C89STR_API errno_t c89str_replace(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, size_t replaceOffset, size_t replaceLength, const char* pOther, size_t otherLength);
C89STR_API errno_t c89str_replace_all(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pQuery, size_t queryLen, const char* pReplacement, size_t replacementLen);
C89STR_API errno_t c89str_trim(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks);
C89STR_API errno_t c89str_len(const c89str* pStr, size_t* pLen);
C89STR_API errno_t c89str_cap(const c89str* pStr, size_t* pCap);


/* Lexer */

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
C89STR_API errno_t c89str_lexer_transform_token(c89str_lexer* pLexer, c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks);



#endif  /* c89str_h */


#if defined(C89STR_IMPLEMENTATION)
#ifndef c89str_c
#define c89str_c

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
#define C89STR_ZERO_MEMORY(p, sz)        c89str_zero_memory_default((p), (sz))
#endif

#ifndef C89STR_COPY_MEMORY
#define C89STR_COPY_MEMORY(dst, src, sz) memcpy((dst), (src), (sz))
#endif

#ifndef C89STR_MOVE_MEMORY
#define C89STR_MOVE_MEMORY(dst, src, sz) memmove((dst), (src), (sz))
#endif

#ifndef C89STR_MALLOC
#define C89STR_MALLOC(sz)                malloc((sz))
#endif

#ifndef C89STR_REALLOC
#define C89STR_REALLOC(p, sz)            realloc((p), (sz))
#endif

#ifndef C89STR_FREE
#define C89STR_FREE(p)                   free((p))
#endif

#define C89STR_ZERO_OBJECT(p)            C89STR_ZERO_MEMORY((p), sizeof(*(p)))
#define C89STR_COUNTOF(x)                (sizeof(x) / sizeof(x[0]))


static C89STR_INLINE c89str_bool32 c89str_is_little_endian()
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

static C89STR_INLINE c89str_bool32 c89str_is_big_endian()
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



C89STR_API errno_t c89str_find(const char* str, const char* other, size_t* pResult)
{
    return c89str_findn(str, (size_t)-1, other, (size_t)-1, pResult);
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
        C89STR_FALSE;
    }

    return c89str_strncmp(str1 + str1Len - str2Len, str2, str2Len) == 0;
}



static int c89str_vscprintf_internal(const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args)
{
#if defined(_MSC_VER)
    #if _MSC_VER > 1200
        return _vscprintf(pFormat, args);
    #else
        /*
        We need to emulate _vscprintf() for the VC6 build. This can be made more efficient, but since it's only VC6 I'm happy to keep this simple. In the VC6
        build we can implement this in terms of _vsnprintf().
        */
        int result;
        char* pTempBuffer = NULL;
        size_t tempBufferCap = 1024;

        if (pFormat == NULL) {
            errno = EINVAL;
            return -1;
        }

	    for (;;) {
            char* pNewTempBuffer = (char*)c89str_realloc(pTempBuffer, tempBufferCap, pAllocationCallbacks);
            if (pNewTempBuffer == NULL) {
                c89str_free(pTempBuffer, pAllocationCallbacks);
                errno = ENOMEM;
                return -1;  /* Out of memory. */
            }

            pTempBuffer = pNewTempBuffer;

            result = _vsnprintf(pTempBuffer, tempBufferCap, pFormat, args);
            c89str_free(pTempBuffer, pAllocationCallbacks);
        
            if (result != -1) {
                break;  /* Got it. */
            }

            /* Buffer wasn't big enough. Ideally it'd be nice to use an error code to know the reason for sure, but this is reliable enough. */
            tempBufferCap *= 2;
	    }

        return result;
    #endif
#else
    return vsnprintf(NULL, 0, pFormat, args);
#endif
}

static int c89str_vsprintf_internal(const c89str_allocation_callbacks* pAllocationCallbacks, char* pOutput, const char* pFormat, va_list args)
{
    /* WARNING: This should only be used by first measuring the output string by setting pOutput to NULL. */

    if (pOutput == NULL) {
        return c89str_vscprintf_internal(pAllocationCallbacks, pFormat, args);
    } else {
    #if (!defined(_MSC_VER) || _MSC_VER >= 1900) && !defined(__STRICT_ANSI__)
        return vsnprintf(pOutput, (size_t)-1, pFormat, args);   /* We're lying about the length here. We should only be calling this internally, and only when computing the length beforehand, so it should be safe. */
    #else
        return vsprintf(pOutput, pFormat, args);
    #endif
    }
}


#define C89STR_HEADER_SIZE_IN_BYTES     (sizeof(size_t) + sizeof(size_t))

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

static void c89str_set_len(c89str str, size_t len)
{
    ((size_t*)c89str_to_allocation_address(str))[1] = len;
}

static size_t c89str_get_len(const c89str str)
{
    return ((size_t*)c89str_to_allocation_address(str))[1];
}

static c89str c89str_realloc_string(c89str str, size_t cap, const c89str_allocation_callbacks* pAllocationCallbacks)
{
    void* pAllocation = c89str_realloc((str == NULL) ? NULL : c89str_to_allocation_address(str), c89str_allocation_size(cap), pAllocationCallbacks);
    if (pAllocation == NULL) {
        return NULL;    /* Failed */
    }

    c89str_set_cap(c89str_from_allocation_address(pAllocation), cap);

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


C89STR_API errno_t c89str_delete(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks)
{
    if (pStr == NULL) {
        return EINVAL;
    }

    c89str_free(c89str_to_allocation_address(*pStr), pAllocationCallbacks);

    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_new(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther)
{
    if (pStr == NULL) {
        return EINVAL;
    }

    /* new() is just a wrapper around set(), but ensuring the input string is NULL. It's assumed that pStr does not point to an existing string, unlike set(). */
    *pStr = NULL;
    return c89str_set(pStr, pAllocationCallbacks, pOther);
}

C89STR_API errno_t c89str_newn(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen)
{
    if (pStr == NULL) {
        return EINVAL;
    }

    *pStr = NULL;
    return c89str_setn(pStr, pAllocationCallbacks, pOther, otherLen);
}

C89STR_API errno_t c89str_newv(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args)
{
    if (pStr == NULL) {
        return EINVAL;
    }

    *pStr = NULL;
    return c89str_setv(pStr, pAllocationCallbacks, pFormat, args);
}

C89STR_API errno_t c89str_newf(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...)
{
    errno_t result;
    va_list args;

    if (pStr == NULL || pFormat == NULL) {
        return EINVAL;
    }

    *pStr = NULL;

    va_start(args, pFormat);
    {
        result = c89str_setv(pStr, pAllocationCallbacks, pFormat, args);
    }
    va_end(args);

    return result;
}

C89STR_API errno_t c89str_set(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther)
{
    if (pOther == NULL) {
        pOther = "";
    }

    return c89str_setn(pStr, pAllocationCallbacks, pOther, c89str_strlen(pOther));
}

C89STR_API errno_t c89str_setn(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen)
{
    c89str str;

    if (pStr == NULL) {
        return EINVAL;
    }

    if (pOther == NULL) {
        pOther = "";
    }

    if (otherLen == (size_t)-1) {
        otherLen = c89str_strlen(pOther);
    }

    str = *pStr;
    if (str != pOther) {
        str = c89str_realloc_string_if_necessary(str, otherLen, pAllocationCallbacks);
        if (str == NULL) {
            return ENOMEM;
        }

        C89STR_COPY_MEMORY(str, pOther, otherLen);   /* Will be explicitly null terminated later. */
    } else {
        /* str and pOther are the same string. No need for a data copy. */
    }

    /* Null terminate and set the length. */
    str[otherLen] = '\0';
    c89str_set_len(str, otherLen);

    /* At this point we're done. */
    *pStr = str;
    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_setv(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args)
{
    va_list args2;
    size_t  len;
    c89str  str;

    if (pFormat == NULL) {
        return EINVAL;
    }

#if !defined(_MSC_VER) || _MSC_VER >= 1800
    va_copy(args2, args);
#else
    args2 = args;
#endif

    len = c89str_vsprintf_internal(pAllocationCallbacks, NULL, pFormat, args2);
    if (len < 0) {
        return errno;  /* Error occurred with formatting. */
    }

    va_end(args2);


    /* Make sure there's enough room for the new string. */
    str = c89str_realloc_string_if_necessary(*pStr, len, pAllocationCallbacks);
    if (str == NULL) {
        return ENOMEM;
    }

    /* We have enough room in the string so now we can just format straight into it. */
    c89str_vsprintf_internal(pAllocationCallbacks, str, pFormat, args);

    /* The length needs to be set explicitly. The formatting will have written the null terminator. */
    c89str_set_len(str, len);

    /* We're done. */
    *pStr = str;
    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_setf(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...)
{
    errno_t result;
    va_list args;

    if (pStr == NULL || pFormat == NULL) {
        return EINVAL;
    }

    va_start(args, pFormat);
    {
        result = c89str_setv(pStr, pAllocationCallbacks, pFormat, args);
    }
    va_end(args);

    return result;
}

C89STR_API errno_t c89str_cat(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther)
{
    if (pOther == NULL) {
        pOther = "";
    }

    return c89str_catn(pStr, pAllocationCallbacks, pOther, c89str_strlen(pOther));
}

C89STR_API errno_t c89str_catn(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pOther, size_t otherLen)
{
    c89str str;
    size_t len;

    if (pStr == NULL) {
        return EINVAL;
    }

    if (pOther == NULL) {
        pOther = "";
    }

    if (otherLen == (size_t)-1) {
        otherLen = c89str_strlen(pOther);
    }

    str = *pStr;
    if (str == NULL) {
        /* Input string is null. This is just an assignment. */
        return c89str_setn(pStr, pAllocationCallbacks, pOther, otherLen);
    }

    len = c89str_get_len(str);
    str = c89str_realloc_string_if_necessary(str, len + otherLen, pAllocationCallbacks);
    if (str == NULL) {
        return ENOMEM;
    }

    C89STR_COPY_MEMORY(str + len, pOther, otherLen);   /* Will be explicitly null terminated later. */

    /* Null terminate and set the length. */
    str[len + otherLen] = '\0';
    c89str_set_len(str, len + otherLen);

    /* At this point we're done. */
    *pStr = str;
    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_catv(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, va_list args)
{
    va_list args2;
    size_t len;
    size_t otherLen;
    c89str str;

    if (pFormat == NULL) {
        return EINVAL;
    }

#if !defined(_MSC_VER) || _MSC_VER >= 1800
    va_copy(args2, args);
#else
    args2 = args;
#endif

    otherLen = c89str_vsprintf_internal(pAllocationCallbacks, NULL, pFormat, args2);
    if (otherLen < 0) {
        return errno;  /* Error occurred with formatting. */
    }

    va_end(args2);


    /* Make sure there's enough room for the new string. */
    len = 0;
    str = *pStr;
    if (str != NULL) {
        len = c89str_get_len(str);
    }

    str = c89str_realloc_string_if_necessary(*pStr, len + otherLen, pAllocationCallbacks);
    if (str == NULL) {
        return ENOMEM;
    }

    /* We have enough room in the string so now we can just format straight into it. */
    c89str_vsprintf_internal(pAllocationCallbacks, str + len, pFormat, args);

    /* The length needs to be set explicitly. The formatting will have written the null terminator. */
    c89str_set_len(str, len + otherLen);

    /* We're done. */
    *pStr = str;
    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_catf(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pFormat, ...)
{
    errno_t result;
    va_list args;

    if (pStr == NULL || pFormat == NULL) {
        return EINVAL;
    }

    va_start(args, pFormat);
    {
        result = c89str_catv(pStr, pAllocationCallbacks, pFormat, args);
    }
    va_end(args);

    return result;
}

C89STR_API errno_t c89str_remove(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, size_t beg, size_t end)
{
    c89str str;
    size_t len;
    size_t lenToRemove;

    if (pStr == NULL) {
        return EINVAL;
    }

    if (beg > end) {
        return ERANGE;  /* The beginning must not be greater than the end. */
    }

    str = *pStr;
    if (str == NULL) {
        return EINVAL;
    }

    lenToRemove = end - beg;
    if (lenToRemove == 0) {
        return C89STR_SUCCESS;   /* Nothing to remove. */
    }

    len = c89str_get_len(str);
    if (beg > len || end > len) {
        return ERANGE;  /* Trying to remove beyond the end of the string. */
    }

    C89STR_MOVE_MEMORY(str + beg, str + end, len - end + 1); /* +1 to include the null terminator. */
    c89str_set_len(str, len - lenToRemove);

    /* We're done. */
    *pStr = str;
    return C89STR_SUCCESS;
}

static errno_t c89str_replace_ex(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, size_t replaceOffset, size_t replaceLen, const char* pOther, size_t otherLen, const char* pOtherPrepend, const char* pOtherAppend)
{
    c89str str;
    c89str newStr;
    errno_t result;

    if (pStr == NULL) {
        return EINVAL;
    }

    str = *pStr;
    if (str == NULL) {
        return EINVAL;
    }

    if (replaceOffset + replaceLen > c89str_get_len(str)) {
        return C89STR_SUCCESS;
    }

    if (replaceLen == 0) {
        return C89STR_SUCCESS; /* Nothing to replace. */
    }

    /* We can allow pOther to be NULL in which case it can be the same as a remove. */
    if (pOther == NULL) {
        pOther = "";
    }

    if (otherLen == (size_t)-1) {
        otherLen = c89str_strlen(pOther);
    }

    
    /* The string is split into 3 sections: the part before the replace, the replacement itself, and the part after the replacement. */

    /* Pre-replacement. */
    result = c89str_newn(&newStr, pAllocationCallbacks, str, replaceOffset);
    if (result != C89STR_SUCCESS) {
        return result;
    }

    /* Replacement. */
    if (pOtherPrepend != NULL) {
        result = c89str_cat(&newStr, pAllocationCallbacks, pOtherPrepend);
        if (result != C89STR_SUCCESS) {
            c89str_delete(&newStr, pAllocationCallbacks);
            return result;
        }
    }

    result = c89str_catn(&newStr, pAllocationCallbacks, pOther, otherLen);
    if (result != C89STR_SUCCESS) {
        c89str_delete(&newStr, pAllocationCallbacks);
        return result;
    }

    if (pOtherAppend != NULL) {
        result = c89str_cat(&newStr, pAllocationCallbacks, pOtherAppend);
        if (result != C89STR_SUCCESS) {
            c89str_delete(&newStr, pAllocationCallbacks);
            return result;
        }
    }

    /* Post-replacement. */
    result = c89str_catn(&newStr, pAllocationCallbacks, str + (replaceOffset + replaceLen), c89str_get_len(str) - (replaceOffset + replaceLen));
    if (result != C89STR_SUCCESS) {
        c89str_delete(&newStr, pAllocationCallbacks);
        return result;
    }

    /* We're done. */
    c89str_delete(&str, pAllocationCallbacks);

    *pStr = newStr;
    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_replace(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, size_t replaceOffset, size_t replaceLength, const char* pOther, size_t otherLength)
{
    return c89str_replace_ex(pStr, pAllocationCallbacks, replaceOffset, replaceLength, pOther, otherLength, NULL, NULL);
}

C89STR_API errno_t c89str_replace_all(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pQuery, size_t queryLen, const char* pReplacement, size_t replacementLen)
{
    /* This function could be made safer by using an intermediary string. */

    size_t offset = 0;

    if (pStr == NULL) {
        return EINVAL;
    }

    if (*pStr == NULL) {
        return EINVAL;
    }

    if (pQuery == NULL || pReplacement == NULL) {
        return EINVAL;
    }

    if (queryLen == (size_t)-1) {
        queryLen = c89str_strlen(pQuery);
    }

    if (replacementLen == (size_t)-1) {
        replacementLen = c89str_strlen(pReplacement);
    }

    /* We keep looping until there's no more occurrances. */
    for (;;) {
        errno_t result;
        size_t location;

        result = c89str_findn(*pStr + offset, c89str_get_len(*pStr) - offset, pQuery, queryLen, &location);
        if (result == ENOENT || location == c89str_npos) {
            break;  /* We're done */
        }

        /* Getting here means we found one. We just need to replace a range. */
        result = c89str_replace(pStr, pAllocationCallbacks, offset + location, queryLen, pReplacement, replacementLen);
        if (result != C89STR_SUCCESS) {
            return result;
        }

        offset += location + replacementLen;   /* Progress past the replacement. */
    }

    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_trim(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks)
{
    c89str str;
    size_t loff;
    size_t roff;

    C89STR_UNUSED(pAllocationCallbacks);

    if (pStr == NULL) {
        return EINVAL;
    }

    str = *pStr;
    if (str == NULL) {
        return EINVAL;
    }

    /* The length of the string will never expand which simplifies our memory management. */
    loff = c89str_utf8_ltrim_offset(str, c89str_get_len(str));
    roff = c89str_utf8_rtrim_offset(str, c89str_get_len(str));

    C89STR_MOVE_MEMORY(str, str + loff, (roff - loff));  /* Left trim by moving the string down. */
    c89str_set_len(str, roff - loff);                    /* Set the length before the right trim. */
    str[c89str_get_len(str)] = '\0';                     /* Right trim by setting the null terminator. */

    return C89STR_SUCCESS;
}


C89STR_API errno_t c89str_len(const c89str* pStr, size_t* pLen)
{
    if (pStr == NULL || pLen == NULL) {
        return EINVAL;
    }

    if (*pStr == NULL) {
        *pLen = 0;
    } else {
        *pLen = c89str_get_len(*pStr);
    }

    return C89STR_SUCCESS;
}

C89STR_API errno_t c89str_cap(const c89str* pStr, size_t* pCap)
{
    if (pStr == NULL || pCap == NULL) {
        return EINVAL;
    }

    if (*pStr == NULL) {
        *pCap = 0;
    } else {
        *pCap = c89str_get_cap(*pStr);
    }

    return C89STR_SUCCESS;
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

C89STR_API errno_t utf16ne_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return utf16le_to_utf8_len(pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    } else {
        return utf16be_to_utf8_len(pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    }
}

C89STR_API errno_t utf16le_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    return c89str_utf16_to_utf8_len_internal(pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t utf16be_to_utf8_len(size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
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
            result = utf16le_to_utf8_len(pUTF8Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = utf16be_to_utf8_len(pUTF8Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = utf16LenProcessed + 1;    /* +1 for BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return utf16ne_to_utf8_len(pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
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

C89STR_API errno_t utf16ne_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return utf16le_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    } else {
        return utf16be_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    }
}

C89STR_API errno_t utf16le_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    return c89str_utf16_to_utf8_internal(pUTF8, utf8Cap, pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t utf16be_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
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
            result = utf16le_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = utf16be_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = utf16LenProcessed + 1;    /* +1 for BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return utf16ne_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
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

C89STR_API errno_t utf16ne_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return utf16le_to_utf32_len(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    } else {
        return utf16be_to_utf32_len(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    }
}

C89STR_API errno_t utf16le_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    return c89str_utf16_to_utf32_len_internal(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t utf16be_to_utf32_len(size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
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
            result = utf16le_to_utf32_len(pUTF32Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = utf16be_to_utf32_len(pUTF32Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = utf16LenProcessed + 1;
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return utf16ne_to_utf32_len(pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
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

C89STR_API errno_t utf16ne_to_utf32ne(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return utf16le_to_utf32le(pUTF32, utf32Cap, pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    } else {
        return utf16be_to_utf32be(pUTF32, utf32Cap, pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
    }
}

C89STR_API errno_t utf16le_to_utf32le(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
{
    return c89str_utf16_to_utf32_internal(pUTF32, utf32Cap, pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t utf16be_to_utf32be(c89str_utf32* pUTF32, size_t utf32Cap, size_t* pUTF32Len, const c89str_utf16* pUTF16, size_t utf16Len, size_t* pUTF16LenProcessed, unsigned int flags)
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
            result = utf16le_to_utf32le(pUTF32, utf32Cap, pUTF32Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = utf16be_to_utf32be(pUTF32, utf32Cap, pUTF32Len, pUTF16+1, utf16Len-1, &utf16LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF16LenProcessed != NULL) {
            *pUTF16LenProcessed = utf16LenProcessed + 1;    /* +1 for BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return utf16ne_to_utf32ne(pUTF32, utf32Cap, pUTF32Len, pUTF16, utf16Len, pUTF16LenProcessed, flags);
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

C89STR_API errno_t utf32ne_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return utf32le_to_utf8_len(pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    } else {
        return utf32be_to_utf8_len(pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    }
}

C89STR_API errno_t utf32le_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    return c89str_utf32_to_utf8_len_internal(pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t utf32be_to_utf8_len(size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
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
            result = utf32le_to_utf8_len(pUTF8Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = utf32be_to_utf8_len(pUTF8Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF32LenProcessed) {
            *pUTF32LenProcessed = utf32LenProcessed + 1;    /* +1 for the BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return utf32ne_to_utf8_len(pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
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

C89STR_API errno_t utf32ne_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return utf32le_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    } else {
        return utf32be_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    }
}

C89STR_API errno_t utf32le_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    return c89str_utf32_to_utf8_internal(pUTF8, utf8Cap, pUTF8Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t utf32be_to_utf8(c89str_utf8* pUTF8, size_t utf8Cap, size_t* pUTF8Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
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
            result = utf32le_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = utf32be_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF32LenProcessed) {
            *pUTF32LenProcessed = utf32LenProcessed + 1;    /* +1 for the BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return utf32ne_to_utf8(pUTF8, utf8Cap, pUTF8Len, pUTF32+1, utf32Len-1, pUTF32LenProcessed, flags);
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

errno_t utf32ne_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return utf32le_to_utf16_len(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    } else {
        return utf32be_to_utf16_len(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    }
}

C89STR_API errno_t utf32le_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    return c89str_utf32_to_utf16_len_internal(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t utf32be_to_utf16_len(size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
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
            result = utf32le_to_utf16_len(pUTF16Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = utf32be_to_utf16_len(pUTF16Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF32LenProcessed) {
            *pUTF32LenProcessed = utf32LenProcessed + 1;    /* +1 for the BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return utf32ne_to_utf16_len(pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
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

C89STR_API errno_t utf32ne_to_utf16ne(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    if (c89str_is_little_endian()) {
        return utf32le_to_utf16le(pUTF16, utf16Cap, pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    } else {
        return utf32be_to_utf16be(pUTF16, utf16Cap, pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags);
    }
}

C89STR_API errno_t utf32le_to_utf16le(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
{
    return c89str_utf32_to_utf16_internal(pUTF16, utf16Cap, pUTF16Len, pUTF32, utf32Len, pUTF32LenProcessed, flags, C89STR_TRUE);
}

C89STR_API errno_t utf32be_to_utf16be(c89str_utf16* pUTF16, size_t utf16Cap, size_t* pUTF16Len, const c89str_utf32* pUTF32, size_t utf32Len, size_t* pUTF32LenProcessed, unsigned int flags)
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
            result = utf32le_to_utf16le(pUTF16, utf16Cap, pUTF16Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        } else {
            result = utf32be_to_utf16be(pUTF16, utf16Cap, pUTF16Len, pUTF32+1, utf32Len-1, &utf32LenProcessed, flags | C89STR_FORBID_BOM); /* <-- We already found a BOM, so we don't want to allow another occurance. */
        }

        if (pUTF32LenProcessed) {
            *pUTF32LenProcessed = utf32LenProcessed + 1;    /* +1 for the BOM. */
        }

        return result;
    }

    /* Getting here means there was no BOM, so assume native endian. */
    return utf32ne_to_utf16ne(pUTF16, utf16Cap, pUTF16Len, pUTF32+1, utf32Len-1, pUTF32LenProcessed, flags);
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
    if (pUTF8 == NULL) {
        return C89STR_TRUE;
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

        if (c89str_utf32_is_null_or_whitespace(&utf32, 1) == C89STR_FALSE) {
            return C89STR_FALSE;
        }

        pUTF8   += utf8Processed;
        utf8Len -= utf8Processed;
    }

    return C89STR_TRUE;
}

C89STR_API size_t c89str_utf8_next_whitespace(const c89str_utf8* pUTF8, size_t utf8Len)
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

C89STR_API size_t c89str_utf8_next_line(const c89str_utf8* pUTF8, size_t utf8Len, size_t* pThisLineLen)
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
            if (utf8Len + utf8Processed > 0 && pUTF8[utf8Processed] == '\n') {
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








/*
Lexer
*/
static errno_t c89str_lexer_unescape_string(c89str* pStr, const c89str_allocation_callbacks* pAllocationCallbacks, const char* pToken, size_t tokenLen)
{
    errno_t result;
    c89str str = NULL;

    if (pStr == NULL) {
        return EINVAL;
    }

    *pStr = NULL;

    if (pToken == NULL) {
        return EINVAL;
    }

    /* We need to remove the surrounding quotes. */
    if (pToken[0] == '\"' || pToken[0] == '\'' && tokenLen >= 2) {
        result = c89str_newn(&str, pAllocationCallbacks, pToken + 1, tokenLen - 2);
    } else {
        result = c89str_newn(&str, pAllocationCallbacks, pToken, tokenLen);
    }

    if (result != C89STR_SUCCESS) {
        return result;
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
                    c89str_remove(&str, pAllocationCallbacks, i, i + 1);
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
    *pStr = str;
    return 0;
}

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
            size_t nextLineOff = c89str_utf8_next_line(pRunningStr, pLexer->tokenLen - (pRunningStr - pLexer->pTokenStr), &thisLineLen);
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
                size_t nextLineOff = c89str_utf8_next_line(txt + off, (len - off), &thisLineLen);
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
            c89str_utf8_next_line(txt + off, (len - off), &thisLineLen);
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

#if 0
        /* It's not whitespace or a new line. */
        if (txt[off] == '/') {
            /* Might be an opening comment. */
            if (((off+1) < len) && txt[off+1] == '*') {
                /* It's a block comment. We need to include everything up to and including the block comment terminator token. */
                errno_t searchResult;
                size_t tokenLen;
            
                off += 2;
                searchResult = c89str_findn(txt + off, (len - off), "*/", 2, &tokenLen);
                if (searchResult != C89STR_SUCCESS) {
                    /* The closing token could not be found. Treat the entire rest of the file as a comment. */
                    result = c89str_lexer_set_token(pLexer, c89str_token_type_comment, (len - off) + 2);  /* +2 for the opening. */
                } else {
                    /* We found the closing token. */
                    result = c89str_lexer_set_token(pLexer, c89str_token_type_comment, tokenLen + 4);     /* +2 for the opening, +2 for the closing. */
                }

                if (pLexer->options.skipComments) {
                    continue;
                } else {
                    return result;
                }
            } else if ((off+1 < len) && txt[off+1] == '/') {
                /* It's a line comment. Note that we do *not* include the new line in the returned token. */
                size_t thisLineLen;
            
                off += 2;
                c89str_utf8_next_line(txt + off, (len - off), &thisLineLen);
                result = c89str_lexer_set_token(pLexer, c89str_token_type_comment, thisLineLen + 2);      /* +2 for the opening. */
                if (pLexer->options.skipComments) {
                    continue;
                } else {
                    return result;
                }
            } else {
                /* It's just a general token. */
                return c89str_lexer_set_single_char(pLexer, txt[off]);
            }
        }
#endif

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
            } /* fallthrough */;

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
                    (txt[off] >= 0x80)) {
                    size_t tokenMaxLen = c89str_utf8_next_whitespace(txt + off, (len - off));   /* <-- We'll be using this to ensure we don't include any Unicode whitespace characters. */
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
                            (txt[off+tokenLen] >= 0x80)) {
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
        return c89str_lexer_unescape_string(pStr, pAllocationCallbacks, pLexer->pTokenStr, pLexer->tokenLen);
    } else if (pLexer->token == c89str_token_type_comment) {
        if (c89str_begins_with(pLexer->pTokenStr, pLexer->tokenLen, pLexer->options.pLineCommentOpeningToken, (size_t)-1)) {
            /* Line comment. */
            size_t openingLen = c89str_strlen(pLexer->options.pLineCommentOpeningToken);

            return c89str_newn(pStr, pAllocationCallbacks, pLexer->pTokenStr + openingLen, pLexer->tokenLen - openingLen);
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
            
            return c89str_newn(pStr, pAllocationCallbacks, pLexer->pTokenStr + openingLen, transformedLen);
        }
    }

    /* Getting here means it's not a string nor a comment. We don't need to transform anything so we just create a new string. */
    return c89str_newn(pStr, pAllocationCallbacks, pLexer->pTokenStr, pLexer->tokenLen);
}



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
