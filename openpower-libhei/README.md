openpower-libhei: Hardware Error Isolation for POWER Systems
============================================================

This library is a common, portable code base for isolating errors reported by
hardware registers on POWER Systems chips.

The primary consumers (and requirements drivers) will be:
 * [OpenBMC Hardware Diagnostics](https://github.com/openbmc/openpower-hw-diags)
 * [POWER Systems Hostboot firmware](https://github.com/open-power/hostboot)
 * POWER Systems FSP firmware

Core API
--------

Details TBD.

Integration
-----------

This library is intended to be integrated into containing user applications as a
set of source files (either imported, or as a git subtree/submodule).

Details TBD.

User Application Requirements and APIs
--------------------------------------
 * The method to access hardware register data will vary per user application.
   Therefore, this library will declare the hardware access [user APIs], but
   each containing user application must implement the APIs for their own
   environment.
 * This library will not contain data regarding hardware specific information.
   Instead, that information will be provided by the user application in the
   form of the [Chip Data Files].
 * Tracing, or logging, methods will vary per user application. Specifically,
   FSP and Hostboot firmware utilize specialized macros as a mechanism to save
   code image space. Therefore, the user application will need to provide a
   specific header file that defines these macros. Details are included in the
   [user APIs].
 * Methods to assert programming bugs will vary per user application. Therefore,
   much like tracing, the user application will need to provide a specific
   header file that defines macros for assertion. Details are included in the
   [user APIs].

[user APIs]: src/hei_user_interface.hpp
[Chip Data Files]: src/chip_data/CHIP_DATA.md

Environment configuration
-------------------------

 * `__HEI_READ_ONLY`: When defined, it will ensure any hardware write support
   is disabled. Note that the Chip Data Files will contain rules for clearing
   and masking register bits. Both of which will require modifying hardware
   registers, which is not allowed by user applications like OpenBMC or FSP
   firmware.

Development Notes
-----------------

 * The Hostboot and FSP environments only support up to **C++11**. Therefore,
   this library cannot use anything newer at this time.
 * Hostboot has a very limited environment. It does not include libc or
   libstdc++. However, Hostboot has implemented select functions from those
   libraries as needed. For details, you can reference `src/include/` in the
   [Hostboot project](https://github.com/open-power/hostboot).

