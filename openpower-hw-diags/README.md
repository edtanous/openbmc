# Hardware Diagnostics for POWER Systems

In the event of a system fatal error reported by the internal system hardware
(processor chips, memory chips, I/O chips, system memory, etc.), POWER Systems
have the ability to diagnose the root cause of the failure and perform any
service action needed to avoid repeated system failures.

Aditional details TBD.

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


