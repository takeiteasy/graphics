/* hal.h
 *
 *           _____                     _____                     _____
 *          /\    \                   /\    \                   /\    \
 *         /::\____\                 /::\    \                 /::\____\
 *        /:::/    /                /::::\    \               /:::/    /
 *       /:::/    /                /::::::\    \             /:::/    /
 *      /:::/    /                /:::/\:::\    \           /:::/    /
 *     /:::/____/                /:::/__\:::\    \         /:::/    /
 *    /::::\    \               /::::\   \:::\    \       /:::/    /
 *   /::::::\    \   _____     /::::::\   \:::\    \     /:::/    /
 *  /:::/\:::\    \ /\    \   /:::/\:::\   \:::\    \   /:::/    /
 * /:::/  \:::\    /::\____\ /:::/  \:::\   \:::\____\ /:::/____/
 * \::/    \:::\  /:::/    / \::/    \:::\  /:::/    / \:::\    \
 *  \/____/ \:::\/:::/    /   \/____/ \:::\/:::/    /   \:::\    \
 *           \::::::/    /             \::::::/    /     \:::\    \
 *            \::::/    /               \::::/    /       \:::\    \
 *            /:::/    /                /:::/    /         \:::\    \
 *           /:::/    /                /:::/    /           \:::\    \
 *          /:::/    /                /:::/    /             \:::\    \
 *         /:::/    /                /:::/    /               \:::\____\
 *         \::/    /                 \::/    /                 \::/    /
 *          \/____/                   \/____/                   \/____/
 *
 *
 * Created by Rory B. Bellows on 26/11/2017.
 * Copyright © 2017-2019 George Watson. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * *   Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * *   Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * *   Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL GEORGE WATSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef hal_h
#define hal_h
#if defined(__cplusplus)
extern "C" {
#endif
  
#define HAL_METAL

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#define HAL_EMCC
#include <emscripten/emscripten.h>
#elif defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define HAL_LINUX
#elif defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#define HAL_OSX
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define HAL_WINDOWS
#else
#define HAL_NO_WINDOW
#endif

#if defined(HAL_MALLOC) && defined(HAL_FREE) && (defined(HAL_REALLOC) || defined(HAL_REALLOC_SIZED))
#elif !defined(HAL_MALLOC) && !defined(HAL_FREE) && !defined(HAL_REALLOC) && !defined(HAL_REALLOC_SIZED)
#else
#error "Must define all or none of HAL_MALLOC, HAL_FREE, and HAL_REALLOC (or HAL_REALLOC_SIZED)."
#endif

#if !defined(HAL_MALLOC)
#define HAL_MALLOC(sz)       malloc(sz)
#define HAL_REALLOC(p,newsz) realloc(p,newsz)
#define HAL_FREE(p)          free(p)
#endif

  typedef signed char        i8;
  typedef unsigned char      u8;
  typedef signed short       i16;
  typedef unsigned short     u16;
  typedef signed int         i32;
  typedef unsigned int       u32;
  typedef signed long long   i64;
  typedef unsigned long long u64;

#if !defined(HALDEF)
#if defined(HAL_STATIC)
#define HALDEF static
#else
#define HALDEF extern
#endif
#endif

#if defined(__cplusplus)
#define HAL_EXTERN extern "C"
#else
#define HAL_EXTERN extern
#endif

#if !defined(_MSC_VER)
#if defined(__cplusplus)
#define hal_inline inline
#else
#define hal_inline
#endif
#else
#define hal_inline __forceinline
#endif

#if !defined(HAL_VERSION_MAJOR)
#define HAL_VERSION_MAJOR 0
#endif
#if !defined(HAL_VERSION_MINOR)
#define HAL_VERSION_MINOR 0
#endif
#if !defined(HAL_VERSION_REV)
#define HAL_VERSION_REV 0
#endif
#define HAL_VERSION_INT (HAL_VERSION_MAJOR << 16 | HAL_VERSION_MINOR << 8 | HAL_VERSION_REV)
#define HAL_VERSION_DOT_STR(a, b, c) a ##.## b ##.## c
#define HAL_VERSION_STR HAL_VERSION_DOT_STR(a, b, c)
#if !defined(HAL_VERSION_GIT)
#define HAL_VERSION_GIT "unknown"
#endif

#if defined(HAL_ONLY_GRAPHICS)  || \
    defined(HAL_ONLY_AUDIO)     || \
    defined(HAL_ONLY_THREADS)   || \
    defined(HAL_ONLY_SOCKETS)   || \
    defined(HAL_ONLY_FILESYSTEM)
#if !defined(HAL_ONLY_GRAPHICS)
#define HAL_NO_GRAPHICS
#endif
#if !defined(HAL_ONLY_AUDIO)
#define HAL_NO_AUDIO
#endif
#if !defined(HAL_ONLY_THREADS)
#define HAL_NO_THREADS
#endif
#if !defined(HAL_ONLY_SOCKETS)
#define HAL_NO_SOCKETS
#endif
#if !defined(HAL_ONLY_FILESYSTEM)
#define HAL_NO_FILESYSTEM
#endif
#endif

#if !defined(HAL_NO_GRAPHICS)
#include "graphics.h"
#endif

// #if !defined(HAL_NO_AUDIO)
// #include "audio.h"
// #endif

// #if !defined(HAL_NO_THREADS)
// #include "threads.h"
// #endif

// #if !defined(HAL_NO_SOCKETS)
// #include "sockets.h"
// #endif

// #if !defined(HAL_NO_FILESYSTEM)
// #include "filesystem.h"
// #endif
  
#if defined(__cplusplus)
}
#endif
#endif // hal_h

