openpower-libhei: Hardware Error Isolation for POWER Systems
============================================================

This library is a common, portable code base for isolating errors reported by
hardware registers on POWER Systems chips.

The primary consumers (and requirements drivers) will be:
 * [OpenBMC Hardware Diagnostics][]
 * [POWER Systems Hostboot firmware][]
 * POWER Systems FSP firmware

Core API
--------

See the [primary API definitions][] for details on how to use this library.

Integration
-----------

This library can be integrated into a user application's source (either
imported, or as a git subtree/submodule) or built as static library.

User Application Requirements and APIs
--------------------------------------

 * The process to access hardware register data will vary per user application.
   Therefore, this library will declare the hardware access [user APIs][], but
   each user application must implement the APIs for their own environment.
 * This library will not contain data regarding hardware specific information.
   Instead, that information will be provided by the user application in the
   form of the [Chip Data Files][].
 * Tracing, or logging, methods will vary per user application. Therefore, this
   library will declare the tracing/logging [user APIs][], but each user
   application must implement the APIs for their own environment.

Environment configuration
-------------------------

 * `__HEI_ENABLE_HW_WRITE`: When defined, it will allow hardware write support.
   Note that the Chip Data Files will contain rules for clearing and masking
   register bits. Both of which will require modifying hardware registers,
   which is not allowed by user applications like OpenBMC or FSP firmware.

Development Notes
-----------------

 * The Hostboot environment only supports up to **C++14**. Therefore, this
   library cannot use anything newer at this time.
 * Hostboot has a very limited environment. It does not include libc or
   libstdc++. However, Hostboot has implemented select functions from those
   libraries as needed. For details, you can reference `src/include/` in the
   [POWER Systems Hostboot firmware][].

Building
--------

For a standard OpenBMC release build, you want something like:
```
meson -Dtests=disabled <build_dir>
ninja -C <build_dir>
ninja -C <build_dir> install
```

For a test / debug build, a typical configuration is:
```
meson -Dtests=enabled <build_dir>
ninja -C <build_dir> test
```

[OpenBMC Hardware Diagnostics]: https://github.com/openbmc/openpower-hw-diags
[POWER Systems Hostboot firmware]: https://github.com/open-power/hostboot
[primary API definitions]: src/hei_main.hpp
[user APIs]: src/hei_user_interface.hpp
[Chip Data Files]: src/chip_data/CHIP_DATA.md

