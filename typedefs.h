#pragma once
typedef unsigned char Byte;
typedef unsigned char Bitmap;
typedef char I8;
typedef short I16;
typedef int I32;
typedef long long I64;
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned long long U64;
typedef float F32;
typedef double F64;
typedef char* pStr;

static const pStr StrEmpty = "";
static const U32 InvalidId = 0xFFFFFFFF;

typedef enum {
    False = 0,
    True
} Bool;

typedef enum {
    XPositive = 0,
    XNegative = 1,
    YPositive = 2,
    YNegative= 3,
    ZPositive = 4,
    ZNegative = 5,
} EDirection;

#ifdef __unix__
#define OS_Windows 0
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#elif defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
#define OS_WINDOWS 1
#include <windows.h>

inline int Is64BitWindows() {
#if defined(_WIN64)
    return True; // 64-bit programs run only on Win64
#elif defined(_WIN32)
    // 32-bit programs run on both 32-bit and 64-bit Windows
    // so must sniff
    BOOL f64 = False;
    return IsWow64Process(GetCurrentProcess(), &f64) && f64;
#else
    return False; // Win64 does not support Win16
#endif
}

#define COLOR_BYTE(Color) Color/255.f

#ifndef OUT
#define OUT
#endif

#endif
