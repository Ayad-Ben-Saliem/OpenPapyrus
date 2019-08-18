/*
 * pthread_spin_init.c
 *
 * Description:
 * This translation unit implements spin lock primitives.
 *
 * --------------------------------------------------------------------------
 *
 *   Pthreads4w - POSIX Threads for Windows
 *   Copyright 1998 John E. Bossom
 *   Copyright 1999-2018, Pthreads4w contributors
 *
 *   Homepage: https://sourceforge.net/projects/pthreads4w/
 *
 *   The current list of contributors is contained
 *   in the file CONTRIBUTORS included with the source
 *   code distribution. The list can also be seen at the
 *   following World Wide Web location:
 *
 *   https://sourceforge.net/p/pthreads4w/wiki/Contributors/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <sl_pthreads4w.h>
#pragma hdrstop

int pthread_spin_init(pthread_spinlock_t * lock, int pshared)
{
	pthread_spinlock_t s;
	int cpus = 0;
	int result = 0;
	if(lock == NULL) {
		return EINVAL;
	}
	if(0 != __ptw32_getprocessors(&cpus)) {
		cpus = 1;
	}
	if(cpus > 1) {
		if(pshared == PTHREAD_PROCESS_SHARED) {
			// Creating spinlock that can be shared between processes.
#if _POSIX_THREAD_PROCESS_SHARED >= 0
			// Not implemented yet.
	#error ERROR [__FILE__, line __LINE__]: Process shared spin locks are not supported yet.
#else
			return ENOSYS;
#endif
		}
	}
	s = static_cast<pthread_spinlock_t>(SAlloc::C(1, sizeof(*s)));
	if(s == NULL) {
		return ENOMEM;
	}
	if(cpus > 1) {
		s->u.cpus = cpus;
		s->interlock =  __PTW32_SPIN_UNLOCKED;
	}
	else {
		pthread_mutexattr_t ma;
		result = pthread_mutexattr_init(&ma);
		if(0 == result) {
			ma->pshared = pshared;
			result = pthread_mutex_init(&(s->u.mutex), &ma);
			if(0 == result) {
				s->interlock =  __PTW32_SPIN_USE_MUTEX;
			}
		}
		(void)pthread_mutexattr_destroy(&ma);
	}
	if(0 == result) {
		*lock = s;
	}
	else {
		SAlloc::F(s);
		*lock = NULL;
	}
	return result;
}
