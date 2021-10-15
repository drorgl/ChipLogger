Logging library
===============

Overview
--------

The logging library provides two ways for setting log verbosity:

- **At compile time**: in menuconfig, set the verbosity level using the option :envvar:`CONFIG_LOG_DEFAULT_LEVEL`. All logging statements for verbosity levels higher than :envvar:`CONFIG_LOG_DEFAULT_LEVEL` will be removed by the preprocessor.
- **At runtime**: all logs for verbosity levels lower than :envvar:`CONFIG_LOG_DEFAULT_LEVEL` are enabled by default. The function :cpp:func:`log_level_set` can be used to set a logging level on a per module basis. Modules are identified by their tags, which are human-readable ASCII zero-terminated strings.

There are the following verbosity levels:

- Error (lowest)
- Warning
- Info
- Debug
- Verbose (highest)

.. note::

    The function :cpp:func:`log_level_set` cannot set logging levels higher than specified by :envvar:`CONFIG_LOG_DEFAULT_LEVEL`. To increase log level for a specific file at compile time, use the macro `DEFAULT_LOG_LEVEL` (see the details below).


How to use this library
-----------------------

In each C file that uses logging functionality, define the TAG variable as shown below:

.. code-block:: c

   static const char* TAG = "MyModule";

Then use one of logging macros to produce output, e.g:

.. code-block:: c

   LOGW(TAG, "Baud rate error %.1f%%. Requested: %d baud, actual: %d baud", error * 100, baud_req, baud_real);

Several macros are available for different verbosity levels:

* ``LOGE`` - error (lowest)
* ``LOGW`` - warning
* ``LOGI`` - info
* ``LOGD`` - debug
* ``LOGV`` - verbose (highest)

Additionally, there are ``EARLY_LOGx`` versions for each of these macros, e.g., :c:macro:`EARLY_LOGE`. These versions have to be used explicitly in the early startup code only, before heap allocator and syscalls have been initialized. Normal ``LOGx`` macros can also be used while compiling the bootloader, but they will fall back to the same implementation as ``EARLY_LOGx`` macros.

To override default verbosity level at file or component scope, define the ``DEFAULT_LOG_LEVEL`` macro.

At file scope, define it before including ``log.h``, e.g.:

.. code-block:: c

   #define DEFAULT_LOG_LEVEL LOG_VERBOSE
   #include "log.h"

At component scope, define it in the component makefile:

.. code-block:: make

   CFLAGS += -D DEFAULT_LOG_LEVEL=LOG_DEBUG

To configure logging output per module at runtime, add calls to the function :cpp:func:`log_level_set` as follows:

.. code-block:: c

   log_level_set("*", LOG_ERROR);        // set all components to ERROR level
   log_level_set("wifi", LOG_WARN);      // enable WARN logs from WiFi stack
   log_level_set("dhcpc", LOG_INFO);     // enable INFO logs from DHCP client

Logging to Host via JTAG
^^^^^^^^^^^^^^^^^^^^^^^^

By default, the logging library uses the vprintf-like function to write formatted output to the dedicated UART. By calling a simple API, all log output may be routed to JTAG instead, making logging several times faster. For details, please refer to Section :ref:`app_trace-logging-to-host`.

