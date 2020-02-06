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
 */

#ifndef hal_h
#define hal_h
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#define HAL_EMCC
#include <emscripten/emscripten.h>
#define HAL_EXTERNAL_WINDOW
#endif
  
#if defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
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
  
#if defined(DEBUG) && !defined(HAL_DEBUG)
#define HAL_DEBUG
#endif

#if !defined(HAL_MALLOC)
#define HAL_MALLOC(sz)       malloc(sz)
#define HAL_REALLOC(p,newsz) realloc(p,newsz)
#define HAL_FREE(p)          free(p)
#endif
#define HAL_SAFE_FREE(x) \
if ((x)) { \
  free((void*)(x)); \
  (x) = NULL; \
}

#if defined(_MSC_VER)
#define bool int
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif
#include <stdarg.h>
  
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

// Taken from: https://stackoverflow.com/a/1911632
#if _MSC_VER
#define STRINGISE_IMPL(x) #x
#define STRINGISE(x) STRINGISE_IMPL(x)
#define FILE_LINE_LINK __FILE__ "(" STRINGISE(__LINE__) ") : "
#define WARN(exp) (FILE_LINE_LINK "WARNING: " exp)
#else
#define WARN(exp) ("WARNING: " exp)
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
  
#define HAL_ERROR(A, ...) hal_error((A), __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
  
  /*!
   * @typedef ERROR_TYPE
   * @brief A list of different error types the library can generate
   */
  typedef enum {
	UNKNOWN_ERROR,
	OUT_OF_MEMEORY,
	FILE_OPEN_FAILED,
	INVALID_BMP,
	UNSUPPORTED_BMP,
	INVALID_PARAMETERS,
	BDF_NO_CHAR_SIZE,
	BDF_NO_CHAR_LENGTH,
	BDF_TOO_MANY_BITMAPS,
	BDF_UNKNOWN_CHAR,
	GL_SHADER_ERROR,
	GL_LOAD_DL_FAILED,
	GL_GET_PROC_ADDR_FAILED,
	CURSOR_MOD_FAILED,
	MTK_LIBRARY_ERROR,
	MTK_CREATE_PIPELINE_FAILED,
	OSX_WINDOW_CREATION_FAILED,
	OSX_APPDEL_CREATION_FAILED,
	OSX_FULLSCREEN_FAILED,
	WIN_GL_PF_ERROR,
	WIN_WINDOW_CREATION_FAILED,
	WIN_FULLSCREEN_FAILED,
	NIX_CURSOR_PIXMAP_ERROR,
	NIX_OPEN_DISPLAY_FAILED,
	NIX_GL_FB_ERROR,
	NIX_GL_CONTEXT_ERROR,
	NIX_WINDOW_CREATION_FAILED,
	WINDOW_ICON_FAILED,
	CUSTOM_CURSOR_NOT_CREATED
  } HAL_ERROR_TYPE;
  
  /*!
   * @discussion Callback for errors inside library
   * @param cb Function pointer to callback
   */
  HALDEF void hal_error_callback(void(*cb)(HAL_ERROR_TYPE, const char*, const char*, const char*, int));
  /*!
   * @discussion Internal function to send an error to the error callback
   * @param type The HAL_ERROR produced
   * @param file The file the error occured in
   * @param func The Function error occured in
   * @param line The line number the error occured on
   * @param msg Formatted error description
   */
  void hal_error(HAL_ERROR_TYPE type, const char* file, const char* func, int line, const char* msg, ...);
  /*!
   * @discussion Get current CPU time
   * @return CPU time
   */
  HALDEF long ticks(void);
  /*!
   * @discussion Sleep in milliseconds
   * @param ms Durection in milliseconds
   */
  HALDEF void delay(long ms);
  HALDEF long pref_counter(void);
  HALDEF long pref_freq(void);
  
#if defined(__cplusplus)
}
#endif
#endif // hal_h

