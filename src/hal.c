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
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#if defined(HAL_TIME_NANO) || HAVE_CLOCK_GETTIME
#include <time.h>
#endif
#if defined(HAL_OSX)
#include <mach/mach_time.h>
#endif
#include <stdbool.h>

static void(*__error_callback)(HAL_ERROR_TYPE, const char*, const char*, const char*, int) = NULL;

void hal_error_callback(void(*cb)(HAL_ERROR_TYPE, const char*, const char*, const char*, int)) {
  __error_callback = cb;
}

void hal_error(HAL_ERROR_TYPE type, const char* file, const char* func, int line, const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  static char error[1024];
  vsprintf((char*)error, msg, args);
  va_end(args);
  
#if defined(HAL_DEBUG)
  fprintf(stderr, "[%d] from %s in %s() at %d -- %s\n", type, file, func, line, error);
#endif
  if (__error_callback) {
	__error_callback(type, (const char*)error, __FILE__, __FUNCTION__, __LINE__);
	return;
  }
  abort();
}

#if HAVE_CLOCK_GETTIME
static struct timespec start_ts;
#elif defined(HAL_OSX)
static uint64_t start_mach;
mach_timebase_info_data_t mach_base_info;
#endif
static bool has_monotonic_time = false;
static struct timeval start_tv;
static bool ticks_started = false;

static void init_ticks() {
  ticks_started = true;
#if HAVE_CLOCK_GETTIME
  if (clock_gettime(SDL_MONOTONIC_CLOCK, &start_ts) == 0)
	has_monotonic_time = SDL_TRUE;
  else
#elif defined(HAL_OSX)
	kern_return_t ret = mach_timebase_info(&mach_base_info);
  if (ret == 0) {
	has_monotonic_time = true;
	start_mach = mach_absolute_time();
  } else
#endif
	gettimeofday(&start_tv, NULL);
}

long ticks() {
  if (!ticks_started)
	init_ticks();
  
  int ret = 0;
  if (has_monotonic_time) {
#if HAVE_CLOCK_GETTIME
	struct timespec now;
	clock_gettime(SDL_MONOTONIC_CLOCK, &now);
	ret = (int)((now.tv_sec - start_ts.tv_sec) * 1000 + (now.tv_nsec - start_ts.tv_nsec) / 1000000);
#elif defined(HAL_OSX)
	uint64_t now = mach_absolute_time();
	ret = (int)((((now - start_mach) * mach_base_info.numer) / mach_base_info.denom) / 1000000);
#else
	SDL_assert(SDL_FALSE);
	ret = 0;
#endif
  } else {
	struct timeval now;
	
	gettimeofday(&now, NULL);
	ret = (int)((now.tv_sec - start_tv.tv_sec) * 1000 + (now.tv_usec - start_tv.tv_usec) / 1000);
  }
  return ret;
}

void delay(long ms) {
  int was_error;
  
#if defined(HAL_TIME_NANO)
  struct timespec elapsed, tv;
#else
  struct timeval tv;
  long then, now, elapsed;
#endif
  
  /* Set the timeout interval */
#if defined(HAL_TIME_NANO)
  elapsed.tv_sec = ms / 1000;
  elapsed.tv_nsec = (ms % 1000) * 1000000;
#else
  then = ticks();
#endif
  do {
	errno = 0;
	
#if defined(HAL_TIME_NANO)
	tv.tv_sec = elapsed.tv_sec;
	tv.tv_nsec = elapsed.tv_nsec;
	was_error = nanosleep(&tv, &elapsed);
#else
	/* Calculate the time interval left (in case of interrupt) */
	now = ticks();
	elapsed = (now - then);
	then = now;
	if (elapsed >= ms) {
	  break;
	}
	ms -= elapsed;
	tv.tv_sec = ms / 1000;
	tv.tv_usec = (ms % 1000) * 1000;
	
	was_error = select(0, NULL, NULL, NULL, &tv);
#endif /* HAVE_NANOSLEEP */
  } while (was_error && (errno == EINTR));
}

long pref_counter() {
  long ret;
  if (!ticks_started)
	init_ticks();
  
  if (has_monotonic_time) {
#if HAVE_CLOCK_GETTIME
	struct timespec now;
	
	clock_gettime(SDL_MONOTONIC_CLOCK, &now);
	ticks = now.tv_sec;
	ticks *= 1000000000;
	ticks += now.tv_nsec;
#elif defined(__APPLE__)
	ret = mach_absolute_time();
#else
	SDL_assert(SDL_FALSE);
	ticks = 0;
#endif
  } else {
	struct timeval now;
	
	gettimeofday(&now, NULL);
	ret = now.tv_sec;
	ret *= 1000000;
	ret += now.tv_usec;
  }
  return ret;
}

long pref_freq() {
  if (!ticks_started)
	init_ticks();
  
  if (has_monotonic_time) {
#if HAVE_CLOCK_GETTIME
	return 1000000000;
#elif defined(HAL_OSX)
	long freq = mach_base_info.denom;
	freq *= 1000000000;
	freq /= mach_base_info.numer;
	return freq;
#endif
  }
  
  return 1000000;
}
