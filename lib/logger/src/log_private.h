#pragma once
#include <stdbool.h>

void log_impl_lock(void);
bool log_impl_lock_timeout(void);
void log_impl_unlock(void);
