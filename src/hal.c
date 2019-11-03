/* hal.c
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

#include "hal.h"
#include <stdio.h>
#include <stdlib.h>

static void(*__error_callback)(HAL_ERROR, const char*, const char*, const char*, i32) = NULL;

void hal_error_callback(void(*cb)(HAL_ERROR, const char*, const char*, const char*, i32)) {
  __error_callback = cb;
}

void hal_error(HAL_ERROR type, const char* file, const char* func, i32 line, const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  static i8 error[1024];
  vsprintf((char*)error, msg, args);
  va_end(args);
  
#if defined(HAL_DEBUG)
  fprintf(stderr, "[%d] from %s in %s() at %d -- %s\n", (i32)type, file, func, line, error);
#endif
  if (__error_callback) {
	__error_callback(type, (const char*)error, __FILE__, __FUNCTION__, __LINE__);
	return;
  }
  abort();
}
