// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __LOG_H__
#define __LOG_H__

#include <stdint.h>
#include <stdarg.h>

#ifdef LOG_CONFIG
#include LOG_CONFIG
#else
#include "log_config.h"
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Log level
 *
 */
#define LOG_NONE 0    /*!< No log output */
#define LOG_ERROR 1   /*!< Critical errors, software module can not recover on its own */
#define LOG_WARN 2    /*!< Error conditions from which recovery measures have been taken */
#define LOG_INFO 3    /*!< Information messages which describe normal flow of events */
#define LOG_DEBUG 4   /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
#define LOG_VERBOSE 5 /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */

    typedef int (*vprintf_like_t)(const char *, va_list);
    typedef void (*log_writev_t)(uint8_t level, const char *tag, const char *format, va_list args);

    /**
 * @brief Set log level for given tag
 *
 * If logging for given component has already been enabled, changes previous setting.
 *
 * Note that this function can not raise log level above the level set using
 * CONFIG_LOG_DEFAULT_LEVEL setting in menuconfig.
 *
 * To raise log level above the default one for a given file, define
 * DEFAULT_LOG_LEVEL to one of the LOG_* values, before including
 * log.h in this file.
 *
 * @param tag Tag of the log entries to enable. Must be a non-NULL zero terminated string.
 *            Value "*" resets log level for all tags to the given value.
 *
 * @param level Selects log level to enable. Only logs at this and lower verbosity
 * levels will be shown.
 */
    void log_level_set(const char *tag, uint8_t level);



/**
 * @brief set write function
 * 
 * @param func new function used for writing
 * @return log_writev_t old function used for writing
 */
    log_writev_t log_set_writev(log_writev_t func);


    /**
 * @brief Set function used to output log entries
 *
 * By default, log output goes to UART0. This function can be used to redirect log
 * output to some other destination, such as file or network. Returns the original
 * log handler, which may be necessary to return output to the previous destination.
 *
 * @param func new Function used for output. Must have same signature as vprintf.
 *
 * @return func old Function used for output.
 */
    vprintf_like_t log_set_vprintf(vprintf_like_t func);

    /**
 * @brief Function which returns timestamp to be used in log output
 *
 * This function is used in expansion of LOGx macros.
 * In the 2nd stage bootloader, and at early application startup stage
 * this function uses CPU cycle counter as time source. Later when
 * FreeRTOS scheduler start running, it switches to FreeRTOS tick count.
 *
 * For now, we ignore millisecond counter overflow.
 *
 * @return timestamp, in milliseconds
 */
    uint32_t log_timestamp(void);

    /**
 * @brief Function which returns system timestamp to be used in log output
 *
 * This function is used in expansion of LOGx macros to print
 * the system time as "HH:MM:SS.sss". The system time is initialized to
 * 0 on startup, this can be set to the correct time with an SNTP sync,
 * or manually with standard POSIX time functions.
 *
 * Currently this will not get used in logging from binary blobs
 * (i.e WiFi & Bluetooth libraries), these will still print the RTOS tick time.
 *
 * @return timestamp, in "HH:MM:SS.sss"
 */
    char *log_system_timestamp(void);

    /**
 * @brief Function which returns timestamp to be used in log output
 *
 * This function uses HW cycle counter and does not depend on OS,
 * so it can be safely used after application crash.
 *
 * @return timestamp, in milliseconds
 */
    uint32_t log_early_timestamp(void);


/**
 * @brief checks if the tag and level should be printed out
 * 
 * @param level log level
 * @param tag tag name
 * @return true if the tag and level should be logged, false otherwise
 */
    bool is_tag_level_visible(uint8_t level, const char *tag);

    /**
 * @brief Write message into the log
 *
 * This function is not intended to be used directly. Instead, use one of
 * LOGE, LOGW, LOGI, LOGD, LOGV macros.
 *
 * This function or these macros should not be used from an interrupt.
 */
    void log_write(uint8_t level, const char *tag, const char *format, ...) __attribute__((format(printf, 3, 4)));

    /**
 * @brief Write message into the log, va_list variant
 * @see log_write()
 *
 * This function is provided to ease integration toward other logging framework,
 * so that log can be used as a log sink.
 */
    void log_writev(uint8_t level, const char *tag, const char *format, va_list args);

    void log_write_buffer_hex(uint8_t level, const char *tag, const void *buffer, uint16_t buff_len);
    void log_write_buffer_char(uint8_t level, const char *tag, const void *buffer, uint16_t buff_len);
    void log_write_buffer_hexdump(uint8_t level, const char *tag, const void *buffer, uint16_t buff_len);

    /** @cond */

#include "log_internal.h"

/** @endcond */

/**
 * @brief Log a buffer of hex bytes at specified level, separated into 16 bytes each line.
 *
 * @param  tag      description tag
 * @param  buffer   Pointer to the buffer array
 * @param  buff_len length of buffer in bytes
 * @param  level    level of the log
 *
 */
// #define LOG_BUFFER_HEX_LEVEL(tag, buffer, buff_len, level)
//     log_buffer_hex(tag, buffer, buff_len, level);

/**
 * @brief Log a buffer of characters at specified level, separated into 16 bytes each line. Buffer should contain only printable characters.
 *
 * @param  tag      description tag
 * @param  buffer   Pointer to the buffer array
 * @param  buff_len length of buffer in bytes
 * @param  level    level of the log
 *
 */
// #define LOG_BUFFER_CHAR_LEVEL(tag, buffer, buff_len, level)
//     log_buffer_char(tag, buffer, buff_len, level);

/**
 * @brief Dump a buffer to the log at specified level.
 *
 * The dump log shows just like the one below:
 *
 *      W (195) log_example: 0x3ffb4280   45 53 50 33 32 20 69 73  20 67 72 65 61 74 2c 20  |ESP32 is great, |
 *      W (195) log_example: 0x3ffb4290   77 6f 72 6b 69 6e 67 20  61 6c 6f 6e 67 20 77 69  |working along wi|
 *      W (205) log_example: 0x3ffb42a0   74 68 20 74 68 65 20 49  44 46 2e 00              |th the IDF..|
 *
 * It is highly recommend to use terminals with over 102 text width.
 *
 * @param tag description tag
 * @param buffer Pointer to the buffer array
 * @param buff_len length of buffer in bytes
 * @param level level of the log
 */
// #define LOG_BUFFER_HEXDUMP(tag, buffer, buff_len, level)
//     log_buffer_hexdump(tag, buffer, buff_len, level);

/**
 * @brief Log a buffer of hex bytes at Info level
 *
 * @param  tag      description tag
 * @param  buffer   Pointer to the buffer array
 * @param  buff_len length of buffer in bytes
 *
 * @see ``log_buffer_hex_level``
 *
 */
// #define LOG_BUFFER_HEX(tag, buffer, buff_len, format, ...)
//     do
//     {
//         if (DEFAULT_LOG_LEVEL >= LOG_INFO)
//         {
//             printf("(D%d,%d)", DEFAULT_LOG_LEVEL, LOG_INFO);
//             LOG_BUFFER_HEX_LEVEL(tag, buffer, buff_len, LOG_INFO);
//         }
//     } while (0)

/**
 * @brief Log a buffer of characters at Info level. Buffer should contain only printable characters.
 *
 * @param  tag      description tag
 * @param  buffer   Pointer to the buffer array
 * @param  buff_len length of buffer in bytes
 *
 * @see ``log_buffer_char_level``
 *
 */
#define LOG_BUFFER_CHAR(tag, buffer, buff_len, format, ...)         \
    do                                                              \
    {                                                               \
        if (DEFAULT_LOG_LEVEL >= LOG_INFO)                          \
        {                                                           \
            printf("(E%d,%d)", DEFAULT_LOG_LEVEL, level);           \
            LOG_BUFFER_CHAR_LEVEL(tag, buffer, buff_len, LOG_INFO); \
        }                                                           \
    } while (0)

    /** @cond */

    // //to be back compatible
    // #define log_buffer_hex      LOG_BUFFER_HEX
    // #define log_buffer_char     LOG_BUFFER_CHAR

#if CONFIG_LOG_COLORS
#define LOG_COLOR_BLACK "30"
#define LOG_COLOR_RED "31"
#define LOG_COLOR_GREEN "32"
#define LOG_COLOR_BROWN "33"
#define LOG_COLOR_BLUE "34"
#define LOG_COLOR_PURPLE "35"
#define LOG_COLOR_CYAN "36"
#define LOG_COLOR(COLOR) "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR) "\033[1;" COLOR "m"
#define LOG_RESET_COLOR "\033[0m"
#define LOG_COLOR_E LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D
#define LOG_COLOR_V
#else //CONFIG_LOG_COLORS
#define LOG_COLOR_E
#define LOG_COLOR_W
#define LOG_COLOR_I
#define LOG_COLOR_D
#define LOG_COLOR_V
#define LOG_RESET_COLOR
#endif //CONFIG_LOG_COLORS

#define LOG_FUNCTION_FILENAME_PROVIDER __FILE__
#define LOG_FUNCTION_LINE_PROVIDER __LINE__

#if CONFIG_LOG_FILENAME
#define LOG_FORMAT_FILENAME "%s"
#define LOG_VALUE_FILENAME LOG_FUNCTION_FILENAME_PROVIDER
#define LOG_FORMAT_LINE ":%d"
#define LOG_VALUE_LINE LOG_FUNCTION_LINE_PROVIDER
#else
#define LOG_FORMAT_FILENAME "%s"
#define LOG_VALUE_FILENAME ""
#define LOG_FORMAT_LINE "%s"
#define LOG_VALUE_LINE ""
#endif

#define LOG_FUNCTION_VALUE_PROVIDER __FUNCTION__

//__func__ / __FUNCTION__
#if CONFIG_LOG_FUNCTION_NAME
#define LOG_FORMAT_FUNCTION_NAME " [%s]"
#define LOG_VALUE_FUNCTION_NAME LOG_FUNCTION_VALUE_PROVIDER
#else
#define LOG_FORMAT_FUNCTION_NAME "%s"
#define LOG_VALUE_FUNCTION_NAME ""
#endif

#define GET_LOG_FORMAT(letter, format) LOG_COLOR_##letter #letter " (%" PRIu32 ") %s: " LOG_FORMAT_FILENAME LOG_FORMAT_LINE LOG_FORMAT_FUNCTION_NAME " " format LOG_RESET_COLOR "\n"
#define LOG_SYSTEM_TIME_FORMAT(letter, format) LOG_COLOR_##letter #letter " (%" PRIu32 ") %s: " LOG_FORMAT_FILENAME LOG_FORMAT_LINE LOG_FORMAT_FUNCTION_NAME " " format LOG_RESET_COLOR "\n"

    /** @endcond */

    /** runtime macro to output logs at a specified level.
 *
 * @param tag tag of the log, which can be used to change the log level by ``log_level_set`` at runtime.
 * @param level level of the output log.
 * @param format format of the output log. see ``printf``
 * @param ... variables to be replaced into the log. see ``printf``
 *
 * @see ``printf``
 */

/* definition to expand macro then apply to pragma message */
#if (MAXIMUM_ENABLED_LOG_LEVEL >= LOG_VERBOSE)
#define LOGV(tag, format, ...) log_write(LOG_VERBOSE, tag, GET_LOG_FORMAT(V, format), log_timestamp(), tag, LOG_VALUE_FILENAME, LOG_VALUE_LINE, LOG_VALUE_FUNCTION_NAME, ##__VA_ARGS__);
#define LOGV_BUFFER_HEX(tag, buffer, buff_len, format, ...) \
    LOGV(tag, format, ##__VA_ARGS__);                       \
    log_write_buffer_hex(LOG_VERBOSE, tag, buffer, buff_len);
#define LOGV_BUFFER_CHAR(tag, buffer, buff_len, format, ...) \
    LOGV(tag, format, ##__VA_ARGS__);                        \
    log_write_buffer_char(LOG_VERBOSE, tag, buffer, buff_len);
#define LOGV_BUFFER_HEXDUMP(tag, buffer, buff_len, format, ...) \
    LOGV(tag, format, ##__VA_ARGS__);                           \
    log_write_buffer_hexdump(LOG_VERBOSE, tag, buffer, buff_len);
#else
#define LOGV(tag, format, ...)
#define LOGV_BUFFER_HEX(tag, buffer, buff_len, format, ...)
#define LOGV_BUFFER_CHAR(tag, buffer, buff_len, format, ...)
#define LOGV_BUFFER_HEXDUMP(tag, buffer, buff_len, format, ...)
#endif

#if (MAXIMUM_ENABLED_LOG_LEVEL >= LOG_DEBUG)
#define LOGD(tag, format, ...) log_write(LOG_DEBUG, tag, GET_LOG_FORMAT(D, format), log_timestamp(), tag, LOG_VALUE_FILENAME, LOG_VALUE_LINE, LOG_VALUE_FUNCTION_NAME, ##__VA_ARGS__);
#define LOGD_BUFFER_HEX(tag, buffer, buff_len, format, ...) \
    LOGD(tag, format, ##__VA_ARGS__);                       \
    log_write_buffer_hex(LOG_DEBUG, tag, buffer, buff_len);
#define LOGD_BUFFER_CHAR(tag, buffer, buff_len, format, ...) \
    LOGD(tag, format, ##__VA_ARGS__);                        \
    log_write_buffer_char(LOG_DEBUG, tag, buffer, buff_len);
#define LOGD_BUFFER_HEXDUMP(tag, buffer, buff_len, format, ...) \
    LOGD(tag, format, ##__VA_ARGS__);                           \
    log_write_buffer_hexdump(LOG_DEBUG, tag, buffer, buff_len);
#else
#define LOGD(tag, format, ...)
#define LOGD_BUFFER_HEX(tag, buffer, buff_len, format, ...)
#define LOGD_BUFFER_CHAR(tag, buffer, buff_len, format, ...)
#define LOGD_BUFFER_HEXDUMP(tag, buffer, buff_len, format, ...)
#endif

#if (MAXIMUM_ENABLED_LOG_LEVEL >= LOG_INFO)
#define LOGI(tag, format, ...) log_write(LOG_INFO, tag, GET_LOG_FORMAT(I, format), log_timestamp(), tag, LOG_VALUE_FILENAME, LOG_VALUE_LINE, LOG_VALUE_FUNCTION_NAME, ##__VA_ARGS__);
#define LOGI_BUFFER_HEX(tag, buffer, buff_len, format, ...) \
    LOGI(tag, format, ##__VA_ARGS__);                       \
    log_write_buffer_hex(LOG_INFO, tag, buffer, buff_len);
#define LOGI_BUFFER_CHAR(tag, buffer, buff_len, format, ...) \
    LOGI(tag, format, ##__VA_ARGS__);                        \
    log_write_buffer_char(LOG_INFO, tag, buffer, buff_len);
#define LOGI_BUFFER_HEXDUMP(tag, buffer, buff_len, format, ...) \
    LOGI(tag, format, ##__VA_ARGS__);                           \
    log_write_buffer_hexdump(LOG_INFO, tag, buffer, buff_len);
#else
#define LOGI(tag, format, ...)
#define LOGI_BUFFER_HEX(tag, buffer, buff_len, format, ...)
#define LOGI_BUFFER_CHAR(tag, buffer, buff_len, format, ...)
#define LOGI_BUFFER_HEXDUMP(tag, buffer, buff_len, format, ...)
#endif

#if (MAXIMUM_ENABLED_LOG_LEVEL >= LOG_WARN)
#define LOGW(tag, format, ...) log_write(LOG_WARN, tag, GET_LOG_FORMAT(W, format), log_timestamp(), tag, LOG_VALUE_FILENAME, LOG_VALUE_LINE, LOG_VALUE_FUNCTION_NAME, ##__VA_ARGS__);
#define LOGW_BUFFER_HEX(tag, buffer, buff_len, format, ...) \
    LOGW(tag, format, ##__VA_ARGS__);                       \
    log_write_buffer_hex(LOG_WARN, tag, buffer, buff_len);
#define LOGW_BUFFER_CHAR(tag, buffer, buff_len, format, ...) \
    LOGW(tag, format, ##__VA_ARGS__);                        \
    log_write_buffer_char(LOG_WARN, tag, buffer, buff_len);
#define LOGW_BUFFER_HEXDUMP(tag, buffer, buff_len, format, ...) \
    LOGW(tag, format, ##__VA_ARGS__);                           \
    log_write_buffer_hexdump(LOG_WARN, tag, buffer, buff_len);
#else
#define LOGW(tag, format, ...)
#define LOGW_BUFFER_HEX(tag, buffer, buff_len, format, ...)
#define LOGW_BUFFER_CHAR(tag, buffer, buff_len, format, ...)
#define LOGW_BUFFER_HEXDUMP(tag, buffer, buff_len, format, ...)
#endif

#if (MAXIMUM_ENABLED_LOG_LEVEL >= LOG_ERROR)
#define LOGE(tag, format, ...) log_write(LOG_ERROR, tag, GET_LOG_FORMAT(E, format), log_timestamp(), tag, LOG_VALUE_FILENAME, LOG_VALUE_LINE, LOG_VALUE_FUNCTION_NAME, ##__VA_ARGS__);
#define LOGE_BUFFER_HEX(tag, buffer, buff_len, format, ...) \
    LOGE(tag, format, ##__VA_ARGS__);                       \
    log_write_buffer_hex(LOG_ERROR, tag, buffer, buff_len);
#define LOGE_BUFFER_CHAR(tag, buffer, buff_len, format, ...) \
    LOGE(tag, format, ##__VA_ARGS__);                        \
    log_write_buffer_char(LOG_ERROR, tag, buffer, buff_len);
#define LOGE_BUFFER_HEXDUMP(tag, buffer, buff_len, format, ...) \
    LOGE(tag, format, ##__VA_ARGS__);                           \
    log_write_buffer_hexdump(LOG_ERROR, tag, buffer, buff_len);
#else
#define LOGE(tag, format, ...)
#define LOGE_BUFFER_HEX(tag, buffer, buff_len, format, ...)
#define LOGE_BUFFER_CHAR(tag, buffer, buff_len, format, ...)
#define LOGE_BUFFER_HEXDUMP(tag, buffer, buff_len, format, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LOG_H__ */
