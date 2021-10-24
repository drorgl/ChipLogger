#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_THREAD_LOCAL
#include <doctest/doctest.h>

#include "log.h"

#include <vector>
#include <string>
#include <cstdio>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ESP_PLATFORM
    void app_main()
#else
int main(/*int argc, char * argv[]*/)
#endif
    {
        const int argc = 3;
        const char *argv[] = {
            "exe",
            "-d",
            "-s"};
#ifdef ESP_PLATFORM
        int result = doctest::Context(argc, argv).run();
#else
    return doctest::Context(argc, argv).run();
#endif
    }

#ifdef __cplusplus
}
#endif

std::vector<std::string> logs;

bool string_contains(std::string str, std::string substr)
{
    if (strstr(str.c_str(), substr.c_str()))
    {
        return true;
    }

    return false;
}

int mock_vprintf(const char *format, va_list list)
{
    // vprintf(format, list);
    char buffer[128];
    int len = vsnprintf(buffer, sizeof(buffer), format, list);
    logs.push_back(std::string(buffer));
    return len;
}

TEST_CASE("log verbose")
{
    logs.clear();
    log_level_set("*", LOG_VERBOSE);
    log_set_vprintf(mock_vprintf);

    LOGV("TAG", "hello %s", "world");

    CHECK(logs.size() == 1);
    CHECK(logs[0].find("V (") != std::string::npos);
    CHECK(logs[0].find("TAG") != std::string::npos);
    CHECK(logs[0].find("logger_tests.cpp") != std::string::npos);
    CHECK(logs[0].find("hello world") != std::string::npos);
}

TEST_CASE("log debug")
{
    logs.clear();
    log_level_set("*", LOG_DEBUG);
    log_set_vprintf(mock_vprintf);

    LOGD("TAG", "hello %s", "world");

    CHECK(logs.size() == 1);
    CHECK(logs[0].find("D (") != std::string::npos);
    CHECK(logs[0].find("TAG") != std::string::npos);
    CHECK(logs[0].find("logger_tests.cpp") != std::string::npos);
    CHECK(logs[0].find("hello world") != std::string::npos);
}

TEST_CASE("log info")
{
    logs.clear();
    log_level_set("*", LOG_INFO);
    log_set_vprintf(mock_vprintf);

    LOGI("TAG", "hello %s", "world");

    CHECK(logs.size() == 1);
    CHECK(logs[0].find("I (") != std::string::npos);
    CHECK(logs[0].find("TAG") != std::string::npos);
    CHECK(logs[0].find("logger_tests.cpp") != std::string::npos);
    CHECK(logs[0].find("hello world") != std::string::npos);
}

TEST_CASE("log warn")
{
    logs.clear();
    log_level_set("*", LOG_WARN);
    log_set_vprintf(mock_vprintf);

    LOGW("TAG", "hello %s", "world");

    CHECK(logs.size() == 1);
    CHECK(logs[0].find("W (") != std::string::npos);
    CHECK(logs[0].find("TAG") != std::string::npos);
    CHECK(logs[0].find("logger_tests.cpp") != std::string::npos);
    CHECK(logs[0].find("hello world") != std::string::npos);
}

TEST_CASE("log error")
{
    logs.clear();
    log_level_set("*", LOG_ERROR);
    log_set_vprintf(mock_vprintf);

    LOGE("TAG", "hello %s", "world");

    CHECK(logs.size() == 1);
    CHECK(logs[0].find("E (") != std::string::npos);
    CHECK(logs[0].find("TAG") != std::string::npos);
    CHECK(logs[0].find("logger_tests.cpp") != std::string::npos);
    CHECK(logs[0].find("hello world") != std::string::npos);
}

TEST_CASE("log verbose when none is default should not show")
{
    logs.clear();
    log_level_set("*", LOG_NONE);
    log_set_vprintf(mock_vprintf);

    LOGV("TAG", "hello %s", "world");

    CHECK(logs.size() == 0);
}

TEST_CASE("log debug when info is default should not show")
{
    logs.clear();
    log_level_set("*", LOG_INFO);
    log_set_vprintf(mock_vprintf);

    LOGD("TAG", "hello %s", "world");

    CHECK(logs.size() == 0);
}

TEST_CASE("log info when warn is default should not show")
{
    logs.clear();
    log_level_set("*", LOG_WARN);
    log_set_vprintf(mock_vprintf);

    LOGI("TAG", "hello %s", "world");

    CHECK(logs.size() == 0);
}

TEST_CASE("log warn when error is default should not show")
{
    logs.clear();
    log_level_set("*", LOG_ERROR);
    log_set_vprintf(mock_vprintf);

    LOGW("TAG", "hello %s", "world");

    CHECK(logs.size() == 0);
}

TEST_CASE("log error when none is default should not show")
{
    logs.clear();
    log_level_set("*", LOG_NONE);
    log_set_vprintf(mock_vprintf);

    LOGE("TAG", "hello %s", "world");

    CHECK(logs.size() == 0);
}

TEST_CASE("hex display")
{
    logs.clear();
    log_level_set("*", LOG_ERROR);
    log_set_vprintf(mock_vprintf);

    const char buffer[] = "The quick brown fox jumps over the lazy dog";

    LOGE_BUFFER_HEX("TAG", buffer, sizeof(buffer), "hello %s", "world");

    CHECK(logs.size() == 4);
    CHECK(logs[0].find("hello world") != std::string::npos);
    CHECK(logs[1].find("54 68 65 20 71 75 69 63 6b 20 62 72 6f 77 6e 20") != std::string::npos);
    CHECK(logs[2].find("66 6f 78 20 6a 75 6d 70 73 20 6f 76 65 72 20 74") != std::string::npos);
    CHECK(logs[3].find("68 65 20 6c 61 7a 79 20 64 6f 67 00") != std::string::npos);
}

TEST_CASE("char display")
{
    logs.clear();
    log_level_set("*", LOG_ERROR);
    log_set_vprintf(mock_vprintf);

    const char buffer[] = "\1The quick brown fox jumps over the lazy dog";

    LOGE_BUFFER_CHAR("TAG", buffer, sizeof(buffer), "hello %s", "world");

    CHECK(logs.size() == 4);
    CHECK(string_contains(logs[0], "hello world"));
    CHECK(string_contains(logs[1], "The quick brown"));
    CHECK(string_contains(logs[2], "fox jumps over"));
    CHECK(string_contains(logs[3], "the lazy dog"));
}

TEST_CASE("hexdump display")
{
    logs.clear();
    log_level_set("*", LOG_ERROR);
    log_set_vprintf(mock_vprintf);

    const char buffer[] = "\1The quick brown fox jumps over the lazy dog";

    LOGE_BUFFER_HEXDUMP("TAG", buffer, sizeof(buffer), "hello %s", "world");

    CHECK(logs.size() == 4);
    CHECK(string_contains(logs[0], "hello world"));
    CHECK(string_contains(logs[1], "(00000000)  01 54 68 65 20 71 75 69  63 6b 20 62 72 6f 77 6e  |.The quick brown|"));
    CHECK(string_contains(logs[2], "(00000010)  20 66 6f 78 20 6a 75 6d  70 73 20 6f 76 65 72 20  | fox jumps over |"));
    CHECK(string_contains(logs[3], "(00000020)  74 68 65 20 6c 61 7a 79  20 64 6f 67 00           |the lazy dog.|"));
}

TEST_CASE("default log level is overwritten by specific tag")
{
    logs.clear();
    log_level_set("*", LOG_ERROR);
    log_level_set("TAG2", LOG_INFO);
    log_set_vprintf(mock_vprintf);

    LOGI("TAG1", "level %s", "info");
    LOGI("TAG2", "level %s", "info");

    CHECK(logs.size() == 1);
    CHECK(logs[0].find("TAG2") != std::string::npos);
}

TEST_CASE("specific tag is overwritten by default log level")
{
    logs.clear();
    log_level_set("TAG2", LOG_INFO);
    log_level_set("*", LOG_ERROR);
    log_set_vprintf(mock_vprintf);

    LOGI("TAG1", "level %s", "info");
    LOGI("TAG2", "level %s", "info");

    CHECK(logs.size() == 0);
}
