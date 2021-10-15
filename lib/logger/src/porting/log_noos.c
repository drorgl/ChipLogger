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

#ifdef CONFIG_LOG_NOOS

#include <assert.h>
#include <stdint.h>
#include "log_private.h"

static int s_lock = 0;

void log_impl_lock(void)
{
    assert(s_lock == 0);
    s_lock = 1;
}

bool log_impl_lock_timeout(void)
{
    log_impl_lock();
    return true;
}

void log_impl_unlock(void)
{
    assert(s_lock == 1);
    s_lock = 0;
}

static uint32_t timestamp = 0;

uint32_t log_early_timestamp(void)
{
    return timestamp++;
}

uint32_t log_timestamp(void)
{
    return timestamp++;
}

#endif