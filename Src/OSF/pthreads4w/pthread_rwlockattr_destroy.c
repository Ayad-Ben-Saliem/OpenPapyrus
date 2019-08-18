/*
 * pthread_rwlockattr_destroy.c
 *
 * Description:
 * This translation unit implements read/write lock primitives.
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
/*
 * ------------------------------------------------------
 * DOCPUBLIC
 *   Destroys a rwlock attributes object. The object can
 *   no longer be used.
 *
 * PARAMETERS
 *   attr
 *           pointer to an instance of pthread_rwlockattr_t
 *
 *
 * DESCRIPTION
 *   Destroys a rwlock attributes object. The object can
 *   no longer be used.
 *
 *   NOTES:
 *           1)      Does not affect rwlockss created using 'attr'
 *
 * RESULTS
 *           0               successfully released attr,
 *           EINVAL          'attr' is invalid.
 *
 * ------------------------------------------------------
 */
int pthread_rwlockattr_destroy(pthread_rwlockattr_t * attr)
{
	int result = 0;
	if(!attr || !*attr) {
		result = EINVAL;
	}
	else {
		pthread_rwlockattr_t rwa = *attr;
		*attr = NULL;
		SAlloc::F(rwa);
	}
	return result;
}
