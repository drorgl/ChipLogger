# Chiplogger

See [library](lib/logger/readme.md)

# ESP32
Works and tests pass

# Native
Works and tests pass

# ATMEGA328
While using this logger on the atmega328 is not completely impossible, its usage of strings might use too much RAM. Possible solution would be to surround every string with PSTR.

# Publishing
```
pio package pack lib/logger
```
after examining logger-*.tar.gz, publish:
```
pio package publish lib/logger
```

# Changelog

* 1.0.2
- add log_set_writev for more fine-grained logging


* 1.0.1
- add external configuration option
- fix log level printf data type mismatch



* 1.0.0 
- initial version, extract from esp-idf logger
- add pointer for hexdump and fix stack overflow in hd_buffer
