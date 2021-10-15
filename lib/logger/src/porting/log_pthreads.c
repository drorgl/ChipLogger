// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "log_config.h"

#ifdef CONFIG_LOG_PTHREADS
#include <stdint.h>
#include <assert.h>
#include "log_private.h"
#include <time.h>
#include <sys/time.h>

#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#define MAX_MUTEX_WAIT_MS 10

static sem_t mutex = NULL;

void log_impl_lock(void)
{
    if (!mutex)
    {
        sem_init(&mutex,0,1);
    }
    sem_wait(&mutex);
}


int sem_timedwait_ms(sem_t *sem, int milliseconds)
{
    if (!mutex)
    {
        sem_init(&mutex,0,1);
    }

    struct timespec ts = {
        0,(1000 * milliseconds)
    };
    // if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
    //     printf("clock_gettime\r\n");
    // }
    //ts.tv_sec += atoi(argv[2]);
    // ts.tv_nsec += (1000 * milliseconds);

    int result = sem_timedwait(sem, &ts) ;
    // printf("[semt %d %d]", result, errno);
    return result == 0;
}

bool log_impl_lock_timeout(void)
{
    return sem_timedwait_ms(&mutex, MAX_MUTEX_WAIT_MS);
}

void log_impl_unlock(void)
{
    sem_post(&mutex);
}

uint32_t log_early_timestamp(void)
{
    clock_t tick = clock();
    return tick;
}

uint32_t log_timestamp(void)
{
    clock_t tick = clock();
    return tick;
}

#endif