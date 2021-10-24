
#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <log.h>

void run_main();

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

        run_main();

#ifdef ESP_PLATFORM
#elif defined(ARDUINO)
#else
    return 0;
#endif
    }

#ifdef __cplusplus
}
#endif

#ifdef ARDUINO
void loop()
{
}
#endif

// int avr_vprintf(const char *format, va_list list)
// {
//     char buffer[100];
//     int len = vsnprintf(buffer, sizeof(buffer), format, list);
//     Serial.print(buffer);
//     return len;
// }

static int
uart_putchar(char c, FILE *stream)
{
    Serial.print(c);
    return 0;
}

static FILE mystdout = {0};

int freeRam()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void run_main()
{
    Serial.begin(115200);
    Serial.println("Starting...");
    mystdout.put = uart_putchar;
    mystdout.get = NULL;
    mystdout.flags = _FDEV_SETUP_WRITE;
    mystdout.udata = 0;
    stdout = &mystdout;

    printf("---->free %d", freeRam());

    log_level_set("*", LOG_DEBUG);
    // log_set_vprintf(avr_vprintf);

    printf("---->free %d", freeRam());

    LOGD("TAG", "hello %s", "world"));
}
