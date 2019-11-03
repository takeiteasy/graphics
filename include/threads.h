/*
 * C11 <threads.h> emulation library
 *
 * (C) Copyright yohhoy 2012.
 * Distributed under the Boost Software License, Version 1.0.
 * (See copy at http://www.boost.org/LICENSE_1_0.txt)
 * Modified by Rusty Shackleford, 2013
 */

/* threads.h
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

#ifndef threads_h
#define threads_h
#if defined(__cplusplus)
extern "C" {
#endif
  
#include "hal.h"
#include <time.h>

#if defined(HAL_OSX) || defined(HAL_LINUX)
#define HAL_USE_PTHREADS
#endif
  
#if defined(HAL_WINDOWS) && !defined(HAL_USE_PTHREADS)
#include <windows.h>
  
#if defined(EMULATED_THREADS_USE_NATIVE_CALL_ONCE) && (_WIN32_WINNT < 0x0600)
#error EMULATED_THREADS_USE_NATIVE_CALL_ONCE requires _WIN32_WINNT>=0x0600
#endif
  
#if defined(EMULATED_THREADS_USE_NATIVE_CV) && (_WIN32_WINNT < 0x0600)
#error EMULATED_THREADS_USE_NATIVE_CV requires _WIN32_WINNT>=0x0600
#endif
  
#ifdef EMULATED_THREADS_USE_NATIVE_CALL_ONCE
#define ONCE_FLAG_INIT INIT_ONCE_STATIC_INIT
#else
#define ONCE_FLAG_INIT {0}
#endif
#define TSS_DTOR_ITERATIONS 1
    typedef struct hal_cnd_t {
#ifdef EMULATED_THREADS_USE_NATIVE_CV
	CONDITION_VARIABLE condvar;
#else
	int blocked;
	int gone;
	int to_unblock;
	HANDLE sem_queue;
	HANDLE sem_gate;
	CRITICAL_SECTION monitor;
#endif
  } hal_cnd_t;
  
  typedef HANDLE hal_thrd_t;
  
  typedef DWORD hal_tss_t;
  
  typedef struct hal_mtx_t {
	CRITICAL_SECTION cs;
  } hal_mtx_t;
  
#ifdef EMULATED_THREADS_USE_NATIVE_CALL_ONCE
  typedef INIT_ONCE hal_once_flag;
#else
  typedef struct hal_once_flag {
	volatile LONG status;
  } hal_once_flag;
#endif
  
#elif defined(HAL_USE_PTHREADS)
#include <pthread.h>

/* OSX doesn't support timed mutex locking */
#if defined(HAL_OSX) && defined(EMULATED_THREADS_USE_NATIVE_TIMEDLOCK)
#undef EMULATED_THREADS_USE_NATIVE_TIMEDLOCK
#endif
  
#define ONCE_FLAG_INIT PTHREAD_ONCE_INIT
#ifdef INIT_ONCE_STATIC_INIT
#define TSS_DTOR_ITERATIONS PTHREAD_DESTRUCTOR_ITERATIONS
#else
#define TSS_DTOR_ITERATIONS 1  // assume TSS dtor MAY be called at least once.
#endif
  
  typedef pthread_cond_t  hal_cnd_t;
  typedef pthread_t       hal_thrd_t;
  typedef pthread_key_t   hal_tss_t;
  typedef pthread_mutex_t hal_mtx_t;
  typedef pthread_once_t  hal_once_flag;
  
#else
#error Unsupported operating system, sorry
#endif
  
  typedef void (*tss_dtor_t)(void*);
  typedef int (*thrd_start_t)(void*);
  
  typedef struct xtime {
	time_t sec;
	long nsec;
  } hal_xtime;
  
  enum {
	MTX_PLAIN     = 0,
	MTX_TRY       = 1,
	MTX_TIMED     = 2,
	MTX_RECURSIVE = 4
  };
  
  enum {
	THRD_SUCCESS = 0, // succeeded
	THRD_TIMEOUT,     // timeout
	THRD_ERROR,       // failed
	THRD_BUSY,        // resource busy
	THRD_NOMEM        // out of memory
  };
  
  void hal_call_once(hal_once_flag* flag, void (*func)(void));
  
  int hal_cnd_broadcast(hal_cnd_t* cond);
  void hal_cnd_destroy(hal_cnd_t* cond);
  int hal_cnd_init(hal_cnd_t* cond);
  int hal_cnd_signal(hal_cnd_t* cond);
  int hal_cnd_timedwait(hal_cnd_t* cond, hal_mtx_t *mtx, const hal_xtime *xt);
  int hal_cnd_wait(hal_cnd_t* cond, hal_mtx_t *mtx);
  
  void hal_mtx_destroy(hal_mtx_t* mtx);
  int hal_mtx_init(hal_mtx_t* mtx, int type);
  int hal_mtx_lock(hal_mtx_t* mtx);
  int hal_mtx_timedlock(hal_mtx_t* mtx, const hal_xtime *xt);
  int hal_mtx_trylock(hal_mtx_t* mtx);
  int hal_mtx_unlock(hal_mtx_t* mtx);
  
  int hal_thrd_create(hal_thrd_t* thr, thrd_start_t func, void *arg);
  hal_thrd_t hal_thrd_current(void);
  int hal_thrd_detach(hal_thrd_t thr);
  int hal_thrd_equal(hal_thrd_t thr0, hal_thrd_t thr1);
  void hal_thrd_exit(int res);
  int hal_thrd_join(hal_thrd_t thr, int *res);
  void hal_thrd_sleep(const hal_xtime *xt);
  void hal_thrd_yield(void);
  
  int hal_tss_create(hal_tss_t* key, tss_dtor_t dtor);
  void hal_tss_delete(hal_tss_t key);
  void* hal_tss_get(hal_tss_t key);
  int hal_tss_set(hal_tss_t key, void *val);
  
  int hal_xtime_get(hal_xtime* xt, int base);
#if !defined(TIME_UTC)
  enum { TIME_UTC = 1 };
#endif

#if defined(__cplusplus)
}
#endif
#endif /* threads_h */
