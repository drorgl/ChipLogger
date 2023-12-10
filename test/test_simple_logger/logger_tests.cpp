#include <unity.h>

#include "log.h"
#include <string.h>
#include <stdbool.h>

// #include <avr/pgmspace.h>

void setUp(){}
void tearDown(){}

static char log_lines[4][105];
static uint8_t current_index;

struct log_writes_t{
    uint8_t level;
    char tag[50];
    char line[100];
};

static log_writes_t log_item[4];
static uint8_t current_log_item_index;

void clear_log()
{
    current_index = 0;
    current_log_item_index = 0;
}

void log_line(const char *line)
{
    snprintf(log_lines[current_index++], 105, "%s", line);
}

void run_all_tests();

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ESP_PLATFORM
    void app_main()
#elif defined(ARDUINO)
void setup()
#else
int main(/*int argc, char * argv[]*/)
#endif
    {

        run_all_tests();

#ifdef ESP_PLATFORM
#elif defined(ARDUINO)
#else
    return 0;
#endif
    }

#ifdef ARDUINO
    void loop()
    {
    }
#endif
#ifdef __cplusplus
}
#endif

bool string_contains(const char *str, const char *substr)
{
    if (strstr(str, substr))
    {
        return true;
    }

    return false;
}

int mock_vprintf(const char *format, va_list list)
{
    // vprintf(format, list);
    char buffer[105];
    int len = vsnprintf(buffer, sizeof(buffer), format, list);
    log_line(buffer);
    return len;
}

void mock_log_writev(uint8_t level,
                const char *tag,
                const char *format,
                va_list args)
{
    struct log_writes_t * next_write = &log_item[current_log_item_index++];
    next_write->level = level;
    strncpy(next_write->tag, tag, 49);
    vsnprintf(next_write->line, 99, format, args);
}

void logger_log_verbose()
{
    clear_log();
    log_level_set("*", LOG_VERBOSE);
    log_set_vprintf(mock_vprintf);

    LOGV("TAG", "hello %s", "world");

    TEST_ASSERT_TRUE_MESSAGE(current_index == 1, "index");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "V ("), "v");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "TAG"), "tag");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "logger_tests.cpp"), "filename");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "hello world"), "contents");
}

void logger_log_debug()
{
    clear_log();
    log_level_set("*", LOG_DEBUG);
    log_set_vprintf(mock_vprintf);

    LOGD("TAG", "hello %s", "world");

    TEST_ASSERT_TRUE_MESSAGE(current_index == 1, "index");
    TEST_ASSERT_TRUE(string_contains(log_lines[0], "D ("));
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "TAG"), "tag");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "logger_tests.cpp"), "filename");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "hello world"), "contents");
}

void logger_log_info()
{
    clear_log();
    log_level_set("*", LOG_INFO);
    log_set_vprintf(mock_vprintf);

    LOGI("TAG", "hello %s", "world");

    TEST_ASSERT_TRUE_MESSAGE(current_index == 1, "index");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "I ("), "level");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "TAG"), "tag");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "logger_tests.cpp"), "filename");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "hello world"), "contents");
}

void logger_log_warn()
{
    clear_log();
    log_level_set("*", LOG_WARN);
    log_set_vprintf(mock_vprintf);

    LOGW("TAG", "hello %s", "world");

    TEST_ASSERT_TRUE_MESSAGE(current_index == 1, "index");
    TEST_ASSERT_TRUE(string_contains(log_lines[0], "W ("));
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "TAG"), "tag");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "logger_tests.cpp"), "filename");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "hello world"), "contents");
}

void logger_log_error()
{
    clear_log();
    log_level_set("*", LOG_ERROR);
    log_set_vprintf(mock_vprintf);

    LOGE("TAG", "hello %s", "world");

    TEST_ASSERT_TRUE_MESSAGE(current_index == 1, "index");
    TEST_ASSERT_TRUE(string_contains(log_lines[0], "E ("));
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "TAG"), "tag");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "logger_tests.cpp"), "filename");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "hello world"), "contents");
}

void logger_log_verbose_when_none_is_default_should_not_show()
{
    clear_log();
    log_level_set("*", LOG_NONE);
    log_set_vprintf(mock_vprintf);

    LOGV("TAG", "hello %s", "world");

    TEST_ASSERT_TRUE(current_index == 0);
}

void logger_log_debug_when_info_is_default_should_not_show()
{
    clear_log();
    log_level_set("*", LOG_INFO);
    log_set_vprintf(mock_vprintf);

    LOGD("TAG", "hello %s", "world");

    TEST_ASSERT_TRUE(current_index == 0);
}

void logger_log_info_when_warn_is_default_should_not_show()
{
    clear_log();
    log_level_set("*", LOG_WARN);
    log_set_vprintf(mock_vprintf);

    LOGI("TAG", "hello %s", "world");

    TEST_ASSERT_TRUE(current_index == 0);
}

void logger_log_warn_when_error_is_default_should_not_show()
{
    clear_log();
    log_level_set("*", LOG_ERROR);
    log_set_vprintf(mock_vprintf);

    LOGW("TAG", "hello %s", "world");

    TEST_ASSERT_TRUE(current_index == 0);
}

void logger_log_error_when_none_is_default_should_not_show()
{
    clear_log();
    log_level_set("*", LOG_NONE);
    log_set_vprintf(mock_vprintf);

    LOGE("TAG", "hello %s", "world");

    TEST_ASSERT_TRUE(current_index == 0);
}

void logger_hex_display()
{
    clear_log();
    log_level_set("*", LOG_ERROR);
    log_set_vprintf(mock_vprintf);

    const char buffer[]  = "The quick brown fox jumps over the lazy dog";

    LOGE_BUFFER_HEX("TAG", buffer, sizeof(buffer), "hello %s", "world");

    TEST_ASSERT_TRUE(current_index == 4);
    TEST_ASSERT_TRUE(string_contains(log_lines[0], "hello world"));
    TEST_ASSERT_TRUE(string_contains(log_lines[1], "54 68 65 20 71 75 69 63 6b 20 62 72 6f 77 6e 20"));
    TEST_ASSERT_TRUE(string_contains(log_lines[2], "66 6f 78 20 6a 75 6d 70 73 20 6f 76 65 72 20 74"));
    TEST_ASSERT_TRUE(string_contains(log_lines[3], "68 65 20 6c 61 7a 79 20 64 6f 67 00"));
}

void logger_char_display()
{
    clear_log();
    log_level_set("*", LOG_ERROR);
    log_set_vprintf(mock_vprintf);

    const char buffer[]  = "\1The quick brown fox jumps over the lazy dog";

    LOGE_BUFFER_CHAR("TAG", buffer, sizeof(buffer), "hello %s", "world");

    TEST_ASSERT_TRUE(current_index == 4);
    TEST_ASSERT_TRUE(string_contains(log_lines[0], "hello world"));
    TEST_ASSERT_TRUE(string_contains(log_lines[1], "The quick brown"));
    TEST_ASSERT_TRUE(string_contains(log_lines[2], "fox jumps over"));
    TEST_ASSERT_TRUE(string_contains(log_lines[3], "the lazy dog"));
}

void logger_hexdump_display()
{
    clear_log();
    log_level_set("*", LOG_ERROR);
    log_set_vprintf(mock_vprintf);

    const char buffer[]  = "\1The quick brown fox jumps over the lazy dog";

    LOGE_BUFFER_HEXDUMP("TAG", buffer, sizeof(buffer), "hello %s", "world");

    TEST_ASSERT_EQUAL_MESSAGE(4, current_index, "index");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[0], "hello world"), "message");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[1], "(00000000)  01 54 68 65 20 71 75 69  63 6b 20 62 72 6f 77 6e  |.The quick brown|"), "1st line");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[2], "(00000010)  20 66 6f 78 20 6a 75 6d  70 73 20 6f 76 65 72 20  | fox jumps over |"), "2nd line");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_lines[3], "(00000020)  74 68 65 20 6c 61 7a 79  20 64 6f 67 00           |the lazy dog.|"), "3rd line");
}

void logger_default_log_level_is_overwritten_by_specific_tag()
{
    clear_log();
    log_level_set("*", LOG_ERROR);
    log_level_set("TAG2", LOG_INFO);
    log_set_vprintf(mock_vprintf);

    LOGI("TAG1", "level %s", "info");
    LOGI("TAG2", "level %s", "info");

    TEST_ASSERT_TRUE_MESSAGE(current_index == 1, "index");
    TEST_ASSERT_TRUE(string_contains(log_lines[0], "TAG2"));
}

void logger_specific_tag_is_overwritten_by_default_log_level()
{
    clear_log();
    log_level_set("TAG2", LOG_INFO);
    log_level_set("*", LOG_ERROR);
    log_set_vprintf(mock_vprintf);

    LOGI("TAG1", "level %s", "info");
    LOGI("TAG2", "level %s", "info");

    TEST_ASSERT_TRUE(current_index == 0);
}





void logger_is_tag_level_visible_default_log_level_is_overwritten_by_specific_tag()
{
    log_level_set("*", LOG_ERROR);
    log_level_set("TAG2", LOG_INFO);

    TEST_ASSERT_FALSE(is_tag_level_visible(LOG_INFO, "TAG1"));
    TEST_ASSERT_TRUE(is_tag_level_visible(LOG_INFO, "TAG2"));
}

void logger_is_tag_level_visible_specific_tag_is_overwritten_by_default_log_level()
{
    log_level_set("TAG2", LOG_INFO);
    log_level_set("*", LOG_ERROR);

    TEST_ASSERT_FALSE(is_tag_level_visible(LOG_INFO, "TAG1"));
    TEST_ASSERT_FALSE(is_tag_level_visible(LOG_INFO, "TAG2"));
}


void logger_log_writev_verbose()
{
    clear_log();
    log_level_set("*", LOG_VERBOSE);
    log_set_vprintf(mock_vprintf);
    log_set_writev(mock_log_writev);

    LOGV("TAG", "hello %s", "world");

    TEST_ASSERT_TRUE_MESSAGE(current_log_item_index == 1, "index");
    TEST_ASSERT_EQUAL(LOG_VERBOSE, log_item[0].level);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("TAG", log_item[0].tag, "tag");

    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_item[0].line, "V ("), "v");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_item[0].line, "TAG"), "tag");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_item[0].line, "logger_tests.cpp"), "filename");
    TEST_ASSERT_TRUE_MESSAGE(string_contains(log_item[0].line, "hello world"), "contents");
}

void run_all_tests()
{
    UNITY_BEGIN();
    RUN_TEST(logger_log_verbose);
    RUN_TEST(logger_log_debug);
    RUN_TEST(logger_log_info);
    RUN_TEST(logger_log_warn);
    RUN_TEST(logger_log_error);
    RUN_TEST(logger_hex_display);
    RUN_TEST(logger_char_display);
    RUN_TEST(logger_hexdump_display);

    RUN_TEST(logger_log_verbose_when_none_is_default_should_not_show);
    RUN_TEST(logger_log_debug_when_info_is_default_should_not_show);
    RUN_TEST(logger_log_info_when_warn_is_default_should_not_show);
    RUN_TEST(logger_log_warn_when_error_is_default_should_not_show);
    RUN_TEST(logger_log_error_when_none_is_default_should_not_show);

    RUN_TEST(logger_default_log_level_is_overwritten_by_specific_tag);
    RUN_TEST(logger_specific_tag_is_overwritten_by_default_log_level);

    RUN_TEST(logger_is_tag_level_visible_default_log_level_is_overwritten_by_specific_tag);
    RUN_TEST(logger_is_tag_level_visible_specific_tag_is_overwritten_by_default_log_level);

    RUN_TEST(logger_log_writev_verbose);

    UNITY_END();
}
