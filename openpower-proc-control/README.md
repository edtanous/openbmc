Contains procedures that interact with the OpenPower nest chipset.

## To Build
```
To build this package, do the following steps:

    1. meson builddir
    2. ninja -C builddir

To build with phal feature:
    1. meson builddir -Dphal=enabled -Dopenfsi=enabled
    2. ninja -C builddir

To clean the repository run `ninja -C builddir/ clean`.
```
