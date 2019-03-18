# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2018 IBM Corp.

libhiomapdir = ${libdir}/ipmid-providers
libhiomap_LTLIBRARIES = libhiomap.la

libhiomap_la_SOURCES = hiomap.cpp

libhiomap_la_LDFLAGS = $(SYSTEMD_LIBS)\
                       $(SDBUSPLUS_LIBS) \
                       $(PHOSPHOR_LOGGING_LIBS) \
                       -version-info 0:0:0 -shared

libhiomap_la_CXXFLAGS = $(SYSTEMD_CFLAGS) \
                        $(SDBUSPLUS_CFLAGS) \
                        $(PHOSPHOR_LOGGING_CFLAGS) \
                        $(AM_CXXFLAGS) \
                        -DBOOST_COROUTINES_NO_DEPRECATION_WARNING
