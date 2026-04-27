/*
* MIT License
 *
 * Copyright (c) 2026 Silph5
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TRYCHAINLIB_LIBRARY_H
#define TRYCHAINLIB_LIBRARY_H
#include <stdio.h>

typedef enum {
    ltc_success = 0,
    _ltc_chain_fail, //for macro only
    ltc_fail,
    ltc_fail_no_mem,
    ltc_fail_invalid_arg,
    ltc_fail_invalid_state,
    ltc_fail_not_found,
    ltc_fail_not_supported,
    ltc_fail_io,
    ltc_fail_file_open,
    ltc_fail_file_close,
    ltc_fail_parse,
    ltc_fail_timeout,
} ltc_status;
//The above enums should be returned from any function interacting with the below ltc_TRY and ltc_TRYROOT macros. They correspond to
//...a series of constant lib error messages such as "ltc: INVALID STATE" (ltc_fail_invalid_state)
//ltc_success should only be returned on success. If any other status is returned, the library will interpret it as a fail.
//Some statuses have additional functionality or rules as listed below:
//          _ltc_chain_fail         - only to be returned by the TRY macro. Indicates a return from a fail chain. Basically, don't touch it.
//          ltc_fail_invalid_arg    - ltc_setArgFailSubject() can be used to give context to which arg was invalid in a function

void ltc_setArgFailSubject(int argNum);
//^Sets the number of the argument which is reported back as invalid.
//This should only be used direltcy before a ltc_fail_invalid_arg return.
//if the subject is not set, no specific argument will be reported as invalid.

void ltc_captureErrno(int newErrno);
//^Captures the current state of errno.
//Use this function immediately before returning a relevant fail ltc_status.
//this will make the log of the fail report the errno at that point.

//Below: macro funcs. do not use these.
void _ltc_onTry();
void _ltc_onTryFail(const char* errMsg, int line, const char* fileName, ltc_status status);
void _ltc_onTrySuccess();
void _ltc_onTryRootFail(const char* errMsg, int line, const char* fileName, ltc_status status);

//Below are the fundamental macros for the lib. Without these, the lib will do nothing. This also means you can easily selectively choose
//...what parts of your program you want to use this lib for
#define LTC_TRY(func, errorMsg) do { \
    _ltc_onTry(); \
    ltc_status status = (func); \
    if (status != ltc_success) { \
        _ltc_onTryFail(errorMsg, __LINE__, __FILE_NAME__, status); \
        return _ltc_chain_fail; \
    } \
    _ltc_onTrySuccess(); \
} while (0);

#define LTC_TRY_ROOT(func, errorMsg, onError) do { \
    _ltc_onTry(); \
    ltc_status status = (func); \
    if (status != ltc_success) { \
        _ltc_onTryRootFail(errorMsg, __LINE__, __FILE_NAME__, status); \
        onError; \
    } \
    _ltc_onTrySuccess(); \
} while (0);

//The ltc_TRY_ROOT macro indicates the "root" of a fail chain, where the chain report will terminate.
//Unlike ltc_TRY, the onError macro parameter here can be used to execute code in response to the fail chain

//view libtests/main.c for a clear look at how the above macros are used

#define LTC_CHECK_IO_ERROR(file) do { \
    if (ferror(file)) { void ltc_captureErrno(errno); return ltc_fail_io; } \
} while (0);
//^This can be used to check for io fails on a stream.

void ltc_setOutStream(FILE* stream);
//^Sets the stream where try chains and other warnings will be written. If not set, will default to stderr.

ltc_status ltc_malloc(void** outPtr, size_t size);
ltc_status ltc_calloc(void** outPtr, size_t nItems, size_t itemSize);
ltc_status ltc_realloc(void** outPtr, size_t size);
ltc_status ltc_fopen(FILE** outFile, const char* path, const char* mode);
ltc_status ltc_fclose(FILE** file);
ltc_status ltc_getenv(const char* name, char** outVal);
//Above are wrappers for some standard C functions. To avoid obfuscation, only specific funcs have been selected.
//Possible errors from other funcs should be handled manually when used in ltc_status-returning functions.

#endif // TRYTRACELIB_LIBRARY_H