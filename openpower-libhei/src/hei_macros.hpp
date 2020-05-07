#pragma once

#include <assert.h>
#include <inttypes.h> // for PRIu64

#include <hei_user_interface.hpp>

/** @brief Common defines used throughout this library */

#define HEI_INF(...)                                                           \
    {                                                                          \
        libhei::hei_inf((char*)__VA_ARGS__);                                   \
    }

#define HEI_ERR(...)                                                           \
    {                                                                          \
        libhei::hei_err((char*)__VA_ARGS__);                                   \
    }

#define HEI_ASSERT(expression) assert(expression);
