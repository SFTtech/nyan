// Copyright 2016-2017 the nyan authors, LGPLv3+. See copying.md for legal info.

#pragma once


#if defined(__GNUC__)
    /*
     * Branch prediction tuning.
     * The expression is expected to be true (=likely) or false (=unlikely).
     */
    #define likely(x)    __builtin_expect(!!(x), 1)
    #define unlikely(x)  __builtin_expect(!!(x), 0)
#else
    #define likely(x)   (x)
    #define unlikely(x) (x)
#endif


/*
 * Software breakpoint for debugging.
 */
#ifdef _WIN32
    #define BREAKPOINT __debugbreak()
#else
    #include <signal.h>
    #define BREAKPOINT raise(SIGTRAP)
#endif


/*
 * shared library symbol export declarations
 */
#if defined(_WIN32)
    #if defined(nyan_EXPORTS)
        #define NYANAPI __declspec(dllexport)     // library is built
    #else
        #define NYANAPI __declspec(dllimport)     // library is used
    #endif /* nyan_EXPORTS */
#else
    #define NYANAPI __attribute__((visibility("default")))
#endif
