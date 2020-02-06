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
    typedef struct cnd_t {
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
  } cnd_t;
  
  typedef HANDLE thrd_t;
  
  typedef DWORD tss_t;
  
  typedef struct mtx_t {
	CRITICAL_SECTION cs;
  } mtx_t;
  
#ifdef EMULATED_THREADS_USE_NATIVE_CALL_ONCE
  typedef INIT_ONCE once_flag;
#else
  typedef struct once_flag {
	volatile LONG status;
  } once_flag;
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
  
  typedef pthread_cond_t  cnd_t;
  typedef pthread_t       thrd_t;
  typedef pthread_key_t   tss_t;
  typedef pthread_mutex_t mtx_t;
  typedef pthread_once_t  once_flag;
  
#else
#error Unsupported operating system, sorry
#endif
  
  typedef void (*tss_dtor_t)(void*);
  typedef int (*thrd_start_t)(void*);
  
  typedef struct xtime {
	time_t sec;
	long nsec;
  } xtime;
  
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
  
  void call_once(once_flag* flag, void (*func)(void));
  
  int cnd_broadcast(cnd_t* cond);
  void cnd_destroy(cnd_t* cond);
  int cnd_init(cnd_t* cond);
  int cnd_signal(cnd_t* cond);
  int cnd_timedwait(cnd_t* cond, mtx_t *mtx, const xtime *xt);
  int cnd_wait(cnd_t* cond, mtx_t *mtx);
  
  void mtx_destroy(mtx_t* mtx);
  int mtx_init(mtx_t* mtx, int type);
  int mtx_lock(mtx_t* mtx);
  int mtx_timedlock(mtx_t* mtx, const xtime *xt);
  int mtx_trylock(mtx_t* mtx);
  int mtx_unlock(mtx_t* mtx);
  
  int thrd_create(thrd_t* thr, thrd_start_t func, void *arg);
  thrd_t thrd_current(void);
  int thrd_detach(thrd_t thr);
  int thrd_equal(thrd_t thr0, thrd_t thr1);
  void thrd_exit(int res);
  int thrd_join(thrd_t thr, int *res);
  void thrd_sleep(const xtime *xt);
  void thrd_yield(void);
  
  int tss_create(tss_t* key, tss_dtor_t dtor);
  void tss_delete(tss_t key);
  void* tss_get(tss_t key);
  int tss_set(tss_t key, void *val);
  
  int xtime_get(xtime* xt, int base);
#if !defined(TIME_UTC)
  enum { TIME_UTC = 1 };
#endif

#if defined(__cplusplus)
}
#endif
#endif /* threads_h */
