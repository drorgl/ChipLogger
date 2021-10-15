// Copyright 2015-2019 Espressif Systems (Shanghai) PTE LTD
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

/*
 * Log library implementation notes.
 *
 * Log library stores all tags provided to log_level_set as a linked
 * list. See uncached_tag_entry_t structure.
 *
 * To avoid looking up log level for given tag each time message is
 * printed, this library caches pointers to tags. Because the suggested
 * way of creating tags uses one 'TAG' constant per file, this caching
 * should be effective. Cache is a binary min-heap of cached_tag_entry_t
 * items, ordering is done on 'generation' member. In this context,
 * generation is an integer which is incremented each time an operation
 * with cache is performed. When cache is full, new item is inserted in
 * place of an oldest item (that is, with smallest 'generation' value).
 * After that, bubble-down operation is performed to fix ordering in the
 * min-heap.
 *
 * The potential problem with wrap-around of cache generation counter is
 * ignored for now. This will happen if someone happens to output more
 * than 4 billion log entries, at which point wrap-around will not be
 * the biggest problem.
 *
 */

#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h> 
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include "log.h"
#include "log_private.h"

#ifndef NDEBUG
// Enable built-in checks in queue.h in debug builds
#define INVARIANTS
// Enable consistency checks and cache statistics in this file.
#define LOG_BUILTIN_CHECKS
#endif

#include "queue.h"

// Number of tags to be cached. Must be 2**n - 1, n >= 2.
#define TAG_CACHE_SIZE 31

typedef struct
{
    const char *tag;
    uint32_t level : 3;
    uint32_t generation : 29;
} cached_tag_entry_t;

typedef struct uncached_tag_entry_
{
    SLIST_ENTRY(uncached_tag_entry_)
    entries;
    uint8_t level; // uint8_t as uint8_t
    char tag[0];   // beginning of a zero-terminated string
} uncached_tag_entry_t;

static uint8_t s_log_default_level = DEFAULT_LOG_LEVEL;
static SLIST_HEAD(log_tags_head, uncached_tag_entry_) s_log_tags = SLIST_HEAD_INITIALIZER(s_log_tags);
static cached_tag_entry_t s_log_cache[TAG_CACHE_SIZE];
static uint32_t s_log_cache_max_generation = 0;
static uint32_t s_log_cache_entry_count = 0;
static vprintf_like_t s_log_print_func = &vprintf;

#ifdef LOG_BUILTIN_CHECKS
static uint32_t s_log_cache_misses = 0;
#endif

static inline bool get_cached_log_level(const char *tag, uint8_t *level);
static inline bool get_uncached_log_level(const char *tag, uint8_t *level);
static inline void add_to_cache(const char *tag, uint8_t level);
static void heap_bubble_down(int index);
static inline void heap_swap(int i, int j);
static inline bool should_output(uint8_t level_for_message, uint8_t level_for_tag);
static inline void clear_log_level_list(void);

vprintf_like_t log_set_vprintf(vprintf_like_t func)
{
    log_impl_lock();
    vprintf_like_t orig_func = s_log_print_func;
    s_log_print_func = func;
    log_impl_unlock();
    return orig_func;
}

void log_level_set(const char *tag, uint8_t level)
{
    log_impl_lock();

    // for wildcard tag, remove all linked list items and clear the cache
    if (strcmp(tag, "*") == 0)
    {
        s_log_default_level = level;
        clear_log_level_list();
        log_impl_unlock();
        return;
    }

    // search for existing tag
    uncached_tag_entry_t *it = NULL;
    SLIST_FOREACH(it, &s_log_tags, entries)
    {
        if (strcmp(it->tag, tag) == 0)
        {
            // one tag in the linked list matched, update the level
            it->level = level;
            // quit with it != NULL
            break;
        }
    }
    // no existing tag, append new one
    if (it == NULL)
    {
        // allocate new linked list entry and append it to the head of the list
        size_t tag_len = strlen(tag) + 1;
        size_t entry_size = offsetof(uncached_tag_entry_t, tag) + tag_len;
        uncached_tag_entry_t *new_entry = (uncached_tag_entry_t *)malloc(entry_size);
        if (!new_entry)
        {
            log_impl_unlock();
            return;
        }
        new_entry->level = (uint8_t)level;
        strncpy(new_entry->tag, tag, tag_len);
        SLIST_INSERT_HEAD(&s_log_tags, new_entry, entries);
    }

    // search in the cache and update the entry it if exists
    for (uint32_t i = 0; i < s_log_cache_entry_count; ++i)
    {
#ifdef LOG_BUILTIN_CHECKS
        assert(i == 0 || s_log_cache[(i - 1) / 2].generation < s_log_cache[i].generation);
#endif
        if (strcmp(s_log_cache[i].tag, tag) == 0)
        {
            s_log_cache[i].level = level;
            break;
        }
    }
    log_impl_unlock();
}

void clear_log_level_list(void)
{
    uncached_tag_entry_t *it;
    while ((it = SLIST_FIRST(&s_log_tags)) != NULL)
    {
        SLIST_REMOVE_HEAD(&s_log_tags, entries);
        free(it);
    }
    s_log_cache_entry_count = 0;
    s_log_cache_max_generation = 0;
#ifdef LOG_BUILTIN_CHECKS
    s_log_cache_misses = 0;
#endif
}

static bool is_tag_level_visible(uint8_t level, const char *tag)
{
    if (!log_impl_lock_timeout())
    {
        // printf("timeout");
        return false;
    }
    uint8_t level_for_tag;
    // Look for the tag in cache first, then in the linked list of all tags
    if (!get_cached_log_level(tag, &level_for_tag))
    {
        if (!get_uncached_log_level(tag, &level_for_tag))
        {
            level_for_tag = s_log_default_level;
        }
        add_to_cache(tag, level_for_tag);
#ifdef LOG_BUILTIN_CHECKS
        ++s_log_cache_misses;
#endif
    }
    log_impl_unlock();
    if (!should_output(level, level_for_tag))
    {
        // printf("shouldnt output");
        return false;
    }

    return true;
}

void log_writev(uint8_t level,
                const char *tag,
                const char *format,
                va_list args)
{
    if (!is_tag_level_visible(level, tag))
    {
        return;
    }

    (*s_log_print_func)(format, args);
}

void log_write(uint8_t level,
               const char *tag,
               const char *format, ...)
{
    va_list list;
    va_start(list, format);
    log_writev(level, tag, format, list);
    va_end(list);
}

static inline bool get_cached_log_level(const char *tag, uint8_t *level)
{
    // Look for `tag` in cache
    uint32_t i;
    for (i = 0; i < s_log_cache_entry_count; ++i)
    {
#ifdef LOG_BUILTIN_CHECKS
        assert(i == 0 || s_log_cache[(i - 1) / 2].generation < s_log_cache[i].generation);
#endif
        if (s_log_cache[i].tag == tag)
        {
            break;
        }
    }
    if (i == s_log_cache_entry_count)
    { // Not found in cache
        return false;
    }
    // Return level from cache
    *level = (uint8_t)s_log_cache[i].level;
    // If cache has been filled, start taking ordering into account
    // (other options are: dynamically resize cache, add "dummy" entries
    //  to the cache; this option was chosen because code is much simpler,
    //  and the unfair behavior of cache will show it self at most once, when
    //  it has just been filled)
    if (s_log_cache_entry_count == TAG_CACHE_SIZE)
    {
        // Update item generation
        s_log_cache[i].generation = s_log_cache_max_generation++;
        // Restore heap ordering
        heap_bubble_down(i);
    }
    return true;
}

static inline void add_to_cache(const char *tag, uint8_t level)
{
    uint32_t generation = s_log_cache_max_generation++;
    // First consider the case when cache is not filled yet.
    // In this case, just add new entry at the end.
    // This happens to satisfy binary min-heap ordering.
    if (s_log_cache_entry_count < TAG_CACHE_SIZE)
    {
        s_log_cache[s_log_cache_entry_count] = (cached_tag_entry_t){
            .generation = generation,
            .level = level,
            .tag = tag};
        ++s_log_cache_entry_count;
        return;
    }

    // Cache is full, so we replace the oldest entry (which is at index 0
    // because this is a min-heap) with the new one, and do bubble-down
    // operation to restore min-heap ordering.
    s_log_cache[0] = (cached_tag_entry_t){
        .tag = tag,
        .level = level,
        .generation = generation};
    heap_bubble_down(0);
}

static inline bool get_uncached_log_level(const char *tag, uint8_t *level)
{
    // Walk the linked list of all tags and see if given tag is present in the list.
    // This is slow because tags are compared as strings.
    uncached_tag_entry_t *it;
    SLIST_FOREACH(it, &s_log_tags, entries)
    {
        if (strcmp(tag, it->tag) == 0)
        {
            *level = it->level;
            return true;
        }
    }
    return false;
}

static inline bool should_output(uint8_t level_for_message, uint8_t level_for_tag)
{
    // printf("should output %d <= %d\r\n", level_for_message, level_for_tag);
    return level_for_message <= level_for_tag;
}

static void heap_bubble_down(int index)
{
    while (index < TAG_CACHE_SIZE / 2)
    {
        int left_index = index * 2 + 1;
        int right_index = left_index + 1;
        int next = (s_log_cache[left_index].generation < s_log_cache[right_index].generation) ? left_index : right_index;
        heap_swap(index, next);
        index = next;
    }
}

static inline void heap_swap(int i, int j)
{
    cached_tag_entry_t tmp = s_log_cache[i];
    s_log_cache[i] = s_log_cache[j];
    s_log_cache[j] = tmp;
}

// static void log_buffer(uint8_t level, const char *tag, const char *value)
// {
//     switch (level)
//     {
//     case LOG_NONE:
//         return;
//     case LOG_ERROR:
//         LOGE(tag, "%s", value);
//         return;
//     case LOG_WARN:
//         LOGW(tag, "%s", value);
//         return;
//     case LOG_INFO:
//         LOGI(tag, "%s", value);
//         return;
//     case LOG_DEBUG:
//         LOGD(tag, "%s", value);
//         return;
//     case LOG_VERBOSE:
//         LOGV(tag, "%s", value);
//         return;
//     }
// }

static void log_buffer_hex_internal(const char *tag, const void *buffer, uint16_t buff_len,
                                    uint8_t log_level)
{
    if (buff_len == 0)
    {
        return;
    }
    char temp_buffer[BYTES_PER_LINE + 3]; //for not-byte-accessible memory
    char hex_buffer[3 * BYTES_PER_LINE + 1];
    const char *ptr_line;
    int bytes_cur_line;

    do
    {
        if (buff_len > BYTES_PER_LINE)
        {
            bytes_cur_line = BYTES_PER_LINE;
        }
        else
        {
            bytes_cur_line = buff_len;
        }
        //use memcpy to get around alignment issue
        memcpy(temp_buffer, buffer, (bytes_cur_line + 3) / 4 * 4);
        ptr_line = temp_buffer;

        for (int i = 0; i < bytes_cur_line; i++)
        {
            sprintf(hex_buffer + 3 * i, "%02x ", ptr_line[i]);
        }
        log_write(log_level, tag, "%s\n", hex_buffer);
        // LOG_LEVEL(log_level, tag, "%s", hex_buffer);
        buffer += bytes_cur_line;
        buff_len -= bytes_cur_line;
    } while (buff_len);
}

static void log_buffer_char_internal(const char *tag, const void *buffer, uint16_t buff_len,
                                     uint8_t log_level)
{
    if (buff_len == 0)
    {
        return;
    }
    char temp_buffer[BYTES_PER_LINE + 3]; //for not-byte-accessible memory
    char char_buffer[BYTES_PER_LINE + 1];
    const char *ptr_line;
    int bytes_cur_line;

    do
    {
        if (buff_len > BYTES_PER_LINE)
        {
            bytes_cur_line = BYTES_PER_LINE;
        }
        else
        {
            bytes_cur_line = buff_len;
        }
        //use memcpy to get around alignment issue
        memcpy(temp_buffer, buffer, (bytes_cur_line + 3) / 4 * 4);
        ptr_line = temp_buffer;

        for (int i = 0; i < bytes_cur_line; i++)
        {
            sprintf(char_buffer + i, "%c", ptr_line[i]);
        }
        log_write(log_level, tag, "%s\n", char_buffer);
        // LOG_LEVEL(log_level, tag, "%s", char_buffer);
        buffer += bytes_cur_line;
        buff_len -= bytes_cur_line;
    } while (buff_len);
}

static void log_buffer_hexdump_internal(const char *tag, const void *buffer,
                                        uint16_t buff_len, uint8_t log_level)
{

    if (buff_len == 0)
    {
        return;
    }
    char temp_buffer[BYTES_PER_LINE + 3]; //for not-byte-accessible memory
    const char *ptr_line;
    //format: field[length]
    // ADDR[10]+"   "+DATA_HEX[8*3]+" "+DATA_HEX[8*3]+"  |"+DATA_CHAR[8]+"|"
    char hd_buffer[sizeof(void*) + 3 + 10 + 3 + BYTES_PER_LINE * 3 + 3 + 1 + BYTES_PER_LINE + 1 + 3];
    char *ptr_hd;
    int bytes_cur_line;

    const void * buffer_start = buffer;

    do
    {
        if (buff_len > BYTES_PER_LINE)
        {
            bytes_cur_line = BYTES_PER_LINE;
        }
        else
        {
            bytes_cur_line = buff_len;
        }
        //use memcpy to get around alignment issue
        memcpy(temp_buffer, buffer, (bytes_cur_line + 3) / 4 * 4);
        ptr_line = temp_buffer;
        ptr_hd = hd_buffer;

        ptr_hd += sprintf(ptr_hd, "%p (%08X)", buffer, buffer - buffer_start);
        for (int i = 0; i < BYTES_PER_LINE; i++)
        {
            if ((i & 7) == 0)
            {
                ptr_hd += sprintf(ptr_hd, " ");
            }
            if (i < bytes_cur_line)
            {
                ptr_hd += sprintf(ptr_hd, " %02x", ptr_line[i]);
            }
            else
            {
                ptr_hd += sprintf(ptr_hd, "   ");
            }
        }
        ptr_hd += sprintf(ptr_hd, "  |");
        for (int i = 0; i < bytes_cur_line; i++)
        {
            if (isprint((int)ptr_line[i]))
            {
                ptr_hd += sprintf(ptr_hd, "%c", ptr_line[i]);
            }
            else
            {
                ptr_hd += sprintf(ptr_hd, ".");
            }
        }
        ptr_hd += sprintf(ptr_hd, "|");
        printf("printing %d out of %d\r\n", ptr_hd - hd_buffer, sizeof(hd_buffer));
        // (char * )*va_arg(list, char * ) =  hd_buffer;
        // log_writev(log_level, tag, format, list);
        log_write(log_level, tag, "%s\n", hd_buffer);

        // LOG_LEVEL(log_level, tag, "%s", hd_buffer);
        buffer += bytes_cur_line;
        buff_len -= bytes_cur_line;
    } while (buff_len);
}

void log_write_buffer_hex(uint8_t level, const char *tag, const void *buffer, uint16_t buff_len)
{
    if (!is_tag_level_visible(level, tag))
    {
        return;
    }

    log_buffer_hex_internal(tag, buffer, buff_len, level);
}

void log_write_buffer_char(uint8_t level, const char *tag, const void *buffer, uint16_t buff_len)
{
    if (!is_tag_level_visible(level, tag))
    {
        return;
    }
    log_buffer_char_internal(tag, buffer, buff_len, level);
}

void log_write_buffer_hexdump(uint8_t level, const char *tag, const void *buffer, uint16_t buff_len)
{
    if (!is_tag_level_visible(level, tag))
    {
        return;
    }
    log_buffer_hexdump_internal(tag, buffer, buff_len, level);
}
