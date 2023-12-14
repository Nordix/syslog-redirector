/**
 * Copyright (c) 2023 OpenInfra Foundation Europe and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License, Version 2.0
 * which accompanies this distribution, and is available at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef LOCK_H
#define LOCK_H

#define LOCK_INIT(x)                                                           \
	static pthread_mutex_t x##_mutex = PTHREAD_MUTEX_INITIALIZER;              \
	static pthread_once_t x##_once = PTHREAD_ONCE_INIT;                        \
	static void x##_init(void) { pthread_mutex_init(&x##_mutex, NULL); }

#define LOCK(x)                                                                \
	do {                                                                       \
		pthread_once(&x##_once, x##_init);                                     \
		pthread_mutex_lock(&x##_mutex);                                        \
	} while (0)

#define UNLOCK(x)                                                              \
	do {                                                                       \
		pthread_mutex_unlock(&x##_mutex);                                      \
	} while (0)

#endif
