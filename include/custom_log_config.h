
/**
 * @brief Default Log Level, can be changed by using log_level_set("*", level);
 * 
 */
#ifndef DEFAULT_LOG_LEVEL
#define DEFAULT_LOG_LEVEL (LOG_VERBOSE)
#endif

/**
 * @brief Maximum Log Level that should be compiled, lower levels should be optimized out by the compiler
 * 
 */
#ifndef MAXIMUM_ENABLED_LOG_LEVEL
#define MAXIMUM_ENABLED_LOG_LEVEL (LOG_VERBOSE)
#endif

// #error hkflsjld

// Enable built-in checks in queue.h in debug builds
#ifndef CONFIG_LOG_INVARIANTS
#define CONFIG_LOG_INVARIANTS 0
#endif
// Enable consistency checks and cache statistics in this file.

#ifndef CONFIG_LOG_BUILTIN_CHECKS
#define CONFIG_LOG_BUILTIN_CHECKS 0
#endif


// Number of tags to be cached. Must be 2**n - 1, n >= 2.
#ifndef CONFIG_LOG_TAG_CACHE_SIZE
#define CONFIG_LOG_TAG_CACHE_SIZE 15
#endif

/**
 * @brief Log Colors
 * 
 */
#ifndef CONFIG_LOG_COLORS
#define CONFIG_LOG_COLORS 1
#endif

#ifndef CONFIG_LOG_FILENAME
#define CONFIG_LOG_FILENAME 1
#endif

#ifndef CONFIG_LOG_FUNCTION_NAME
#define CONFIG_LOG_FUNCTION_NAME 1
#endif

//print number of bytes per line for log_buffer_char and log_buffer_hex
#define BYTES_PER_LINE 16

#define LOG_BUILTIN_CHECKS

#ifdef IDF_VER
#define ESP32
#endif

#if defined(ESP32)
#define CONFIG_LOG_FREERTOS
#elif defined(__MINGW32__)
#define CONFIG_LOG_PTHREADS
#else
#define CONFIG_LOG_NOOS
#endif