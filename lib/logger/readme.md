# ChipLogger
An Embedded Logger in C extracted from [ESP-IDF](https://github.com/espressif/esp-idf/tree/master/components/log).

The logging library provides two ways for setting log verbosity:
* At compile time - setting `DEFAULT_LOG_LEVEL`
* At runtime - using `log_level_set`

This library also provides a way to completely remove log messages from the output binary using `MAXIMUM_ENABLED_LOG_LEVEL`.

The function `log_level_set` cannot set logging levels higher than specified by `CONFIG_LOG_MAXIMUM_LEVEL`. To increase log level for a specific file above this maximum at compile time, use the macro MAXIMUM_ENABLED_LOG_LEVEL (see the details below).

There are the following verbosity levels:
* None - `LOG_NONE` - No log output
* Error (lowest) - `LOG_ERROR` - Critical errors, software module can not recover on its own
* Warning - `LOG_WARN` - Error conditions from which recovery measures have been taken
* Info - `LOG_INFO` - Information messages which describe normal flow of events
* Debug - `LOG_DEBUG` - Extra information which is not necessary for normal use (values, pointers, sizes, etc)
* Verbose (highest) - `LOG_VERBOSE` -  Bigger chunks of debugging information, or frequent messages which can potentially flood the output

# How to use this library

In each module that uses logging functionality, define a tag and use it for the module name.

```c
static const char* TAG = "MyModule";
```

Then use one of logging macros to produce output, e.g:
```c
LOGW(TAG, "Baud %d", baud);
```

Several macros are available for different verbosity levels, in addition, it is possible to hex dump, character dump and a full hex dump:

* `LOGE` - error (lowest) `LOGE_BUFFER_HEX`, `LOGE_BUFFER_CHAR` and `LOGE_BUFFER_HEXDUMP`
* `LOGW` - warning `LOGW_BUFFER_HEX`, `LOGW_BUFFER_CHAR` and `LOGW_BUFFER_HEXDUMP`
* `LOGI` - info `LOGI_BUFFER_HEX`, `LOGI_BUFFER_CHAR` and `LOGI_BUFFER_HEXDUMP`
* `LOGD` - debug `LOGD_BUFFER_HEX`, `LOGD_BUFFER_CHAR` and `LOGD_BUFFER_HEXDUMP`
* `LOGV` - verbose (highest) `LOGV_BUFFER_HEX`, `LOGV_BUFFER_CHAR` and `LOGV_BUFFER_HEXDUMP`

To override default verbosity level at file or component scope, define the `MAXIMUM_ENABLED_LOG_LEVEL` macro.

At file scope, define it before including `log.h`, e.g.:

```c
#define MAXIMUM_ENABLED_LOG_LEVEL ESP_LOG_VERBOSE
#include "log.h"
```

To configure logging output per module at runtime, add calls to the function `log_level_set` as follows:

```c
log_level_set("*", LOG_ERROR);        // set all components to ERROR level
log_level_set("wifi", LOG_WARN);      // enable WARN logs from WiFi stack
log_level_set("dhcpc", LOG_INFO);     // enable INFO logs from DHCP client
```

By default, log output goes to stdout. This function can be used to redirect log
output to some other destination, such as file or network. Returns the original
log handler, which may be necessary to return output to the previous destination.
 
```c
typedef int (*vprintf_like_t)(const char *, va_list);
vprintf_like_t log_set_vprintf(vprintf_like_t func);
```

If you need fine-grained control over which log message goes to which destination
you can implement it in the writev function 
```c
typedef void (*log_writev_t)(uint8_t level, const char *tag, const char *format, va_list args);
log_writev_t log_set_writev(log_writev_t func);

//example
void mock_log_writev(uint8_t level,
                const char *tag,
                const char *format,
                va_list args)
{
    char buffer[100];
    vsnprintf(buffer, sizeof(buffer), format, args);
    //do something with level/tag
}
```


# Porting
To port the logger to a new system, you'd need to implement all functions in `log_private.h` and undefine  `CONFIG_LOG_FREERTOS`, `CONFIG_LOG_PTHREADS` and `CONFIG_LOG_NOOS`.

# Configuration

Default Log Level, can be changed by using `log_level_set("*", level)`
```c
#define DEFAULT_LOG_LEVEL (LOG_VERBOSE)
```

Maximum Log Level that should be compiled, lower levels should be optimized out by the compiler
```c
#define MAXIMUM_ENABLED_LOG_LEVEL (LOG_VERBOSE)
```

Enable built-in checks in queue.h in debug builds
```c
#define CONFIG_LOG_INVARIANTS 1
```

Enable consistency checks and cache statistics in this file.
```c
#define CONFIG_LOG_BUILTIN_CHECKS 1
```

Number of tags to be cached. Must be 2**n - 1, n >= 2.
```c
#define CONFIG_LOG_TAG_CACHE_SIZE 31
```

Log Colors
```c
#define CONFIG_LOG_COLORS 1
```

Log File Names
```c
#define CONFIG_LOG_FILENAME 1
```

Log Function names
```c
#define CONFIG_LOG_FUNCTION_NAME 1
```

print number of bytes per line for `LOGx_BUFFER_CHAR`,  `LOGx_BUFFER_HEX` and `LOGx_BUFFER_HEXDUMP`
```c
#define BYTES_PER_LINE 16
```

Log builtin checks, currently only cache misses
```c
#define LOG_BUILTIN_CHECKS
```

Use locking and timestamp from freertos/pthreads or none
```c
#define CONFIG_LOG_FREERTOS
#define CONFIG_LOG_PTHREADS
#define CONFIG_LOG_NOOS
```

# External Configuration
Copy log_config.h into "include" folder and add to `build_flags`:
```
-I include -DLOG_CONFIG=\"custom_log_config.h\"
```