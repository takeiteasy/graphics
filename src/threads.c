/*
 * C11 <threads.h> emulation library
 *
 * (C) Copyright yohhoy 2012.
 * Distributed under the Boost Software License, Version 1.0.
 * (See copy at http://www.boost.org/LICENSE_1_0.txt)
 * Modified by Rusty Shackleford, 2013
 */

/* threads.c
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

#define HAL_ONLY_THREADS
#include "hal.h"
#include <limits.h>

#if defined(HAL_USE_PTHREADS)
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>

/* Configuration macro:
 * EMULATED_THREADS_USE_NATIVE_TIMEDLOCK
 *		Use pthread_mutex_timedlock() for `mtx_timedlock()'
 *		Otherwise use mtx_trylock() + *busy loop* emulation.
 */
#if !defined(HAL_OSX) && !defined(__CYGWIN__)
#define EMULATED_THREADS_USE_NATIVE_TIMEDLOCK
#endif

/* Implementation limits:
 * - Conditionally emulation for "mutex with timeout"
 *   (see EMULATED_THREADS_USE_NATIVE_TIMEDLOCK macro)
 */
struct impl_thrd_param {
  thrd_start_t func;
  void *arg;
};

void* impl_thrd_routine(void* p);
void* impl_thrd_routine(void* p) {
  struct impl_thrd_param pack = *((struct impl_thrd_param *)p);
  free(p);
  return (void*)(long)pack.func(pack.arg);
}

void call_once(once_flag* flag, void (*func)(void)) {
  pthread_once(flag, func);
}

int cnd_broadcast(cnd_t* cond) {
  if (!cond) return THRD_ERROR;
  pthread_cond_broadcast(cond);
  return THRD_SUCCESS;
}

void cnd_destroy(cnd_t* cond) {
  assert(cond);
  pthread_cond_destroy(cond);
}

int cnd_init(cnd_t* cond) {
  if (!cond) return THRD_ERROR;
  pthread_cond_init(cond, NULL);
  return THRD_SUCCESS;
}

int cnd_signal(cnd_t* cond) {
  if (!cond) return THRD_ERROR;
  pthread_cond_signal(cond);
  return THRD_SUCCESS;
}

int cnd_timedwait(cnd_t* cond, mtx_t *mtx, const xtime *xt) {
  struct timespec abs_time;
  int rt;
  if (!cond || !mtx || !xt) return THRD_ERROR;
  rt = pthread_cond_timedwait(cond, mtx, &abs_time);
  if (rt == ETIMEDOUT)
    return THRD_BUSY;
  return (rt == 0) ? THRD_SUCCESS : THRD_ERROR;
}

int cnd_wait(cnd_t* cond, mtx_t *mtx) {
  if (!cond || !mtx) return THRD_ERROR;
  pthread_cond_wait(cond, mtx);
  return THRD_SUCCESS;
}

void mtx_destroy(mtx_t* mtx) {
  assert(mtx);
  pthread_mutex_destroy(mtx);
}

int mtx_init(mtx_t* mtx, int type) {
  pthread_mutexattr_t attr;
  if (!mtx) return THRD_ERROR;
  if (type != MTX_PLAIN
      && type != MTX_TIMED
      && type != MTX_TRY
      && type != (MTX_PLAIN|MTX_RECURSIVE)
      && type != (MTX_TIMED|MTX_RECURSIVE)
      && type != (MTX_TRY  |MTX_RECURSIVE))
    return THRD_ERROR;
  pthread_mutexattr_init(&attr);
  if ((type & MTX_RECURSIVE) != 0) {
#if defined(HAL_LINUX)
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#else
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif
  }
  pthread_mutex_init(mtx, &attr);
  pthread_mutexattr_destroy(&attr);
  return THRD_SUCCESS;
}

int mtx_lock(mtx_t* mtx) {
  if (!mtx) return THRD_ERROR;
  pthread_mutex_lock(mtx);
  return THRD_SUCCESS;
}

int mtx_timedlock(mtx_t* mtx, const xtime *xt) {
  if (!mtx || !xt)
    return THRD_ERROR;
#ifdef EMULATED_THREADS_USE_NATIVE_TIMEDLOCK
  struct timespec ts;
  int rt;
  ts.tv_sec = xt->sec;
  ts.tv_nsec = xt->nsec;
  rt = pthread_mutex_timedlock(mtx, &ts);
  if (rt == 0)
    return THRD_SUCCESS;
  return (rt == ETIMEDOUT) ? THRD_BUSY : THRD_ERROR;
#else
  time_t expire = time(NULL);
  expire += xt->sec;
  while (mtx_trylock(mtx) != THRD_SUCCESS) {
    time_t now = time(NULL);
    if (expire < now)
      return THRD_BUSY;
    // busy loop!
    thrd_yield();
  }
  return THRD_SUCCESS;
#endif
}

int mtx_trylock(mtx_t* mtx) {
  if (!mtx)
    return THRD_ERROR;
  return (pthread_mutex_trylock(mtx) == 0) ? THRD_SUCCESS : THRD_BUSY;
}

int mtx_unlock(mtx_t* mtx) {
  if (!mtx)
    return THRD_ERROR;
  pthread_mutex_unlock(mtx);
  return THRD_SUCCESS;
}

int thrd_create(thrd_t* thr, thrd_start_t func, void *arg) {
  struct impl_thrd_param *pack;
  if (!thr)
    return THRD_ERROR;
  pack = malloc(sizeof(struct impl_thrd_param));
  if (!pack) return THRD_NOMEM;
  pack->func = func;
  pack->arg = arg;
  if (pthread_create(thr, NULL, impl_thrd_routine, pack) != 0) {
    free(pack);
    return THRD_ERROR;
  }
  return THRD_SUCCESS;
}

thrd_t thrd_current(void) {
  return pthread_self();
}

int thrd_detach(thrd_t thr) {
  return (pthread_detach(thr) == 0) ? THRD_SUCCESS : THRD_ERROR;
}

int thrd_equal(thrd_t thr0, thrd_t thr1) {
  return pthread_equal(thr0, thr1);
}

void thrd_exit(int res) {
  pthread_exit((void*)(long)res);
}

int thrd_join(thrd_t thr, int *res) {
  void *code;
  if (pthread_join(thr, &code) != 0)
    return THRD_ERROR;
  if (res)
    *res = (int)code;
  return THRD_SUCCESS;
}

void thrd_sleep(const xtime *xt) {
  struct timespec req;
  assert(xt);
  req.tv_sec = xt->sec;
  req.tv_nsec = xt->nsec;
  nanosleep(&req, NULL);
}

void thrd_yield(void) {
  sched_yield();
}

int tss_create(tss_t* key, tss_dtor_t dtor) {
  if (!key) return THRD_ERROR;
  return (pthread_key_create(key, dtor) == 0) ? THRD_SUCCESS : THRD_ERROR;
}

void tss_delete(tss_t key) {
  pthread_key_delete(key);
}

void* tss_get(tss_t key) {
  return pthread_getspecific(key);
}

int tss_set(tss_t key, void *val) {
  return (pthread_setspecific(key, val) == 0) ? THRD_SUCCESS : THRD_ERROR;
}
#elif defined(HAL_WINDOWS)
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include <process.h>  // MSVCRT

/* Configuration macro:
 * EMULATED_THREADS_USE_NATIVE_CALL_ONCE
 *		Use native WindowsAPI one-time initialization function.
 *		(requires WinVista or later)
 *		Otherwise emulate by mtx_trylock() + *busy loop* for WinXP.
 * EMULATED_THREADS_USE_NATIVE_CV
 *		Use native WindowsAPI condition variable object.
 *		(requires WinVista or later)
 *		Otherwise use emulated implementation for WinXP.
 * EMULATED_THREADS_TSS_DTOR_SLOTNUM
 *		Max registerable TSS dtor number.
 */
#if _WIN32_WINNT >= 0x0600
// Prefer native WindowsAPI on newer environment.
#define EMULATED_THREADS_USE_NATIVE_CALL_ONCE
#define EMULATED_THREADS_USE_NATIVE_CV
#endif
#define EMULATED_THREADS_TSS_DTOR_SLOTNUM 64  // see TLS_MINIMUM_AVAILABLE

#include "threads.h"


/* Implementation limits:
 * - Conditionally emulation for "Initialization functions"
 *   (see EMULATED_THREADS_USE_NATIVE_CALL_ONCE macro)
 * - Emulated `mtx_timelock()' with mtx_trylock() + *busy loop*
 */
static void impl_tss_dtor_invoke();  // forward decl.

struct impl_thrd_param {
  thrd_start_t func;
  void *arg;
};

static unsigned __stdcall impl_thrd_routine(void* p) {
  struct impl_thrd_param pack;
  int code;
  memcpy(&pack, p, sizeof(struct impl_thrd_param));
  free(p);
  code = pack.func(pack.arg);
  impl_tss_dtor_invoke();
  return (unsigned)code;
}

static DWORD impl_xtime2msec(const xtime *xt) {
  return (DWORD)((xt->sec * 1000u) + (xt->nsec / 1000));
}

#ifdef EMULATED_THREADS_USE_NATIVE_CALL_ONCE
struct impl_call_once_param { void (*func)(void); };
static BOOL CALLBACK impl_call_once_callback(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *Context) {
  struct impl_call_once_param *param = (struct impl_call_once_param*)Parameter;
  (param->func)();
  ((void)InitOnce); ((void)Context);  // suppress warning
  return TRUE;
}
#endif  // ifdef EMULATED_THREADS_USE_NATIVE_CALL_ONCE

#ifndef EMULATED_THREADS_USE_NATIVE_CV
/* Note:
 * The implementation of condition variable is ported from Boost.Interprocess
 * See http://www.boost.org/boost/interprocess/sync/windows/condition.hpp
 */
static void impl_cond_do_signal(cnd_t* cond, int broadcast) {
  int nsignal = 0;
  
  EnterCriticalSection(&cond->monitor);
  if (cond->to_unblock != 0) {
    if (cond->blocked == 0) {
      LeaveCriticalSection(&cond->monitor);
      return;
    }
    if (broadcast) {
      cond->to_unblock += nsignal = cond->blocked;
      cond->blocked = 0;
    } else {
      nsignal = 1;
      cond->to_unblock++;
      cond->blocked--;
    }
  } else if (cond->blocked > cond->gone) {
    WaitForSingleObject(cond->sem_gate, INFINITE);
    if (cond->gone != 0) {
      cond->blocked -= cond->gone;
      cond->gone = 0;
    }
    if (broadcast) {
      nsignal = cond->to_unblock = cond->blocked;
      cond->blocked = 0;
    } else {
      nsignal = cond->to_unblock = 1;
      cond->blocked--;
    }
  }
  LeaveCriticalSection(&cond->monitor);
  
  if (0 < nsignal)
    ReleaseSemaphore(cond->sem_queue, nsignal, NULL);
}

static int impl_cond_do_wait(cnd_t* cond, mtx_t *mtx, const xtime *xt) {
  int nleft = 0;
  int ngone = 0;
  int timeout = 0;
  DWORD w;
  
  WaitForSingleObject(cond->sem_gate, INFINITE);
  cond->blocked++;
  ReleaseSemaphore(cond->sem_gate, 1, NULL);
  
  mtx_unlock(mtx);
  
  w = WaitForSingleObject(cond->sem_queue, xt ? impl_xtime2msec(xt) : INFINITE);
  timeout = (w == WAIT_TIMEOUT);
  
  EnterCriticalSection(&cond->monitor);
  if ((nleft = cond->to_unblock) != 0) {
    if (timeout) {
      if (cond->blocked != 0) {
        cond->blocked--;
      } else {
        cond->gone++;
      }
    }
    if (--cond->to_unblock == 0) {
      if (cond->blocked != 0) {
        ReleaseSemaphore(cond->sem_gate, 1, NULL);
        nleft = 0;
      }
      else if ((ngone = cond->gone) != 0) {
        cond->gone = 0;
      }
    }
  } else if (++cond->gone == INT_MAX/2) {
    WaitForSingleObject(cond->sem_gate, INFINITE);
    cond->blocked -= cond->gone;
    ReleaseSemaphore(cond->sem_gate, 1, NULL);
    cond->gone = 0;
  }
  LeaveCriticalSection(&cond->monitor);
  
  if (nleft == 1) {
    while (ngone--)
      WaitForSingleObject(cond->sem_queue, INFINITE);
    ReleaseSemaphore(cond->sem_gate, 1, NULL);
  }
  
  mtx_lock(mtx);
  return timeout ? thrd_busy : thrd_success;
}
#endif  // ifndef EMULATED_THREADS_USE_NATIVE_CV

static struct impl_tss_dtor_entry {
  tss_t key;
  tss_dtor_t dtor;
} impl_tss_dtor_tbl[EMULATED_THREADS_TSS_DTOR_SLOTNUM];

static int impl_tss_dtor_register(tss_t key, tss_dtor_t dtor) {
  int i;
  for (i = 0; i < EMULATED_THREADS_TSS_DTOR_SLOTNUM; i++) {
    if (!impl_tss_dtor_tbl[i].dtor)
      break;
  }
  if (i == EMULATED_THREADS_TSS_DTOR_SLOTNUM)
    return 1;
  impl_tss_dtor_tbl[i].key = key;
  impl_tss_dtor_tbl[i].dtor = dtor;
  return 0;
}

static void impl_tss_dtor_invoke() {
  int i;
  for (i = 0; i < EMULATED_THREADS_TSS_DTOR_SLOTNUM; i++) {
    if (impl_tss_dtor_tbl[i].dtor) {
      void* val = tss_get(impl_tss_dtor_tbl[i].key);
      if (val)
        (impl_tss_dtor_tbl[i].dtor)(val);
    }
  }
}

void call_once(once_flag* flag, void (*func)(void)) {
  assert(!flag && !func);
#ifdef EMULATED_THREADS_USE_NATIVE_CALL_ONCE
  struct impl_call_once_param param;
  param.func = func;
  InitOnceExecuteOnce(flag, impl_call_once_callback, (PVOID)&param, NULL);
#else
  if (InterlockedCompareExchange(&flag->status, 1, 0) == 0) {
    (func)();
    InterlockedExchange(&flag->status, 2);
  } else {
    while (flag->status == 1) {
      // busy loop!
      thrd_yield();
    }
  }
#endif
}

int cnd_broadcast(cnd_t* cond) {
  if (!cond) return thrd_error;
#ifdef EMULATED_THREADS_USE_NATIVE_CV
  WakeAllConditionVariable(&cond->condvar);
#else
  impl_cond_do_signal(cond, 1);
#endif
  return thrd_success;
}

void cnd_destroy(cnd_t* cond) {
  assert(cond);
#ifdef EMULATED_THREADS_USE_NATIVE_CV
  // do nothing
#else
  CloseHandle(cond->sem_queue);
  CloseHandle(cond->sem_gate);
  DeleteCriticalSection(&cond->monitor);
#endif
}

int cnd_init(cnd_t* cond) {
  if (!cond) return thrd_error;
#ifdef EMULATED_THREADS_USE_NATIVE_CV
  InitializeConditionVariable(&cond->condvar);
#else
  cond->blocked = 0;
  cond->gone = 0;
  cond->to_unblock = 0;
  cond->sem_queue = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
  cond->sem_gate = CreateSemaphore(NULL, 1, 1, NULL);
  InitializeCriticalSection(&cond->monitor);
#endif
  return thrd_success;
}

int cnd_signal(cnd_t* cond) {
  if (!cond) return thrd_error;
#ifdef EMULATED_THREADS_USE_NATIVE_CV
  WakeConditionVariable(&cond->condvar);
#else
  impl_cond_do_signal(cond, 0);
#endif
  return thrd_success;
}

int cnd_timedwait(cnd_t* cond, mtx_t *mtx, const xtime *xt) {
  if (!cond || !mtx || !xt) return thrd_error;
#ifdef EMULATED_THREADS_USE_NATIVE_CV
  if (SleepConditionVariableCS(&cond->condvar, &mtx->cs, impl_xtime2msec(xt)))
    return thrd_success;
  return (GetLastError() == ERROR_TIMEOUT) ? thrd_busy : thrd_error;
#else
  return impl_cond_do_wait(cond, mtx, xt);
#endif
}

int cnd_wait(cnd_t* cond, mtx_t *mtx) {
  if (!cond || !mtx) return thrd_error;
#ifdef EMULATED_THREADS_USE_NATIVE_CV
  SleepConditionVariableCS(&cond->condvar, &mtx->cs, INFINITE);
#else
  impl_cond_do_wait(cond, mtx, NULL);
#endif
  return thrd_success;
}

void mtx_destroy(mtx_t* mtx) {
  assert(mtx);
  DeleteCriticalSection(&mtx->cs);
}

int mtx_init(mtx_t* mtx, int type) {
  if (!mtx) return thrd_error;
  if (type != mtx_plain && type != mtx_timed && type != mtx_try
      && type != (mtx_plain|mtx_recursive)
      && type != (mtx_timed|mtx_recursive)
      && type != (mtx_try|mtx_recursive))
    return thrd_error;
  InitializeCriticalSection(&mtx->cs);
  return thrd_success;
}

int mtx_lock(mtx_t* mtx) {
  if (!mtx) return thrd_error;
  EnterCriticalSection(&mtx->cs);
  return thrd_success;
}

int mtx_timedlock(mtx_t* mtx, const xtime *xt) {
  time_t expire, now;
  if (!mtx || !xt) return thrd_error;
  expire = time(NULL);
  expire += xt->sec;
  while (mtx_trylock(mtx) != thrd_success) {
    now = time(NULL);
    if (expire < now)
      return thrd_busy;
    // busy loop!
    thrd_yield();
  }
  return thrd_success;
}

int mtx_trylock(mtx_t* mtx) {
  if (!mtx) return thrd_error;
  return TryEnterCriticalSection(&mtx->cs) ? thrd_success : thrd_busy;
}

int mtx_unlock(mtx_t* mtx) {
  if (!mtx) return thrd_error;
  LeaveCriticalSection(&mtx->cs);
  return thrd_success;
}

int thrd_create(thrd_t* thr, thrd_start_t func, void *arg) {
  struct impl_thrd_param *pack;
  uintptr_t handle;
  if (!thr) return thrd_error;
  pack = malloc(sizeof(struct impl_thrd_param));
  if (!pack) return thrd_nomem;
  pack->func = func;
  pack->arg = arg;
  handle = _beginthreadex(NULL, 0, impl_thrd_routine, pack, 0, NULL);
  if (handle == 0) {
    if (errno == EAGAIN || errno == EACCES)
      return thrd_nomem;
    return thrd_error;
  }
  *thr = (thrd_t)handle;
  return thrd_success;
}

thrd_t thrd_current(void) {
  return GetCurrentThread();
}

int thrd_detach(thrd_t thr) {
  CloseHandle(thr);
  return thrd_success;
}

int thrd_equal(thrd_t thr0, thrd_t thr1) {
  return (thr0 == thr1);
}

void thrd_exit(int res) {
  impl_tss_dtor_invoke();
  _endthreadex((unsigned)res);
}

int thrd_join(thrd_t thr, int *res) {
  DWORD w, code;
  w = WaitForSingleObject(thr, INFINITE);
  if (w != WAIT_OBJECT_0)
    return thrd_error;
  if (res) {
    if (!GetExitCodeThread(thr, &code)) {
      CloseHandle(thr);
      return thrd_error;
    }
    *res = (int)code;
  }
  CloseHandle(thr);
  return thrd_success;
}

void thrd_sleep(const xtime *xt) {
  assert(xt);
  Sleep(impl_xtime2msec(xt));
}

void thrd_yield(void) {
  SwitchToThread();
}

int tss_create(tss_t* key, tss_dtor_t dtor) {
  if (!key) return thrd_error;
  *key = TlsAlloc();
  if (dtor) {
    if (impl_tss_dtor_register(*key, dtor)) {
      TlsFree(*key);
      return thrd_error;
    }
  }
  return (*key != 0xFFFFFFFF) ? thrd_success : thrd_error;
}

void tss_delete(tss_t key) {
  TlsFree(key);
}

void* tss_get(tss_t key) {
  return TlsGetValue(key);
}

int tss_set(tss_t key, void *val) {
  return TlsSetValue(key, val) ? thrd_success : thrd_error;
}
#else
#error Unsupported operating system, sorry
#endif

int xtime_get(xtime* xt, int base) {
  if (!xt) return 0;
  if (base == TIME_UTC) {
    xt->sec = time(NULL);
    xt->nsec = 0;
    return base;
  }
  return 0;
}
