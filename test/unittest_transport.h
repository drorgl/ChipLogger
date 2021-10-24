#ifndef UNITTEST_TRANSPORT_H
#define UNITTEST_TRANSPORT_H

#include <stdio.h>

#ifdef ARDUINO
#include <Arduino.h>
static int uart_putchar(char c, FILE *stream);
static FILE mystdout = {0};

static int
uart_putchar(char c, FILE *stream)
{
  Serial.print(c);
  return 0;
}

#endif

void unittest_uart_begin()
{
#ifdef ARDUINO
  Serial.begin(115200);
  mystdout.put = uart_putchar;
  mystdout.get = NULL;
  mystdout.flags = _FDEV_SETUP_WRITE;
  mystdout.udata = 0;
  stdout = &mystdout;
#endif
}

void unittest_uart_putchar(char c)
{
#ifdef ARDUINO
  Serial.print(c);
#else
  putchar(c);
#endif
}

void unittest_uart_flush()
{
  // fflush(stdout);
}

void unittest_uart_end()
{
}

#endif