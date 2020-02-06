#pragma once

/**
 * @file hei_user_defines.hpp
 * @brief The purpose of this file is to create common defines that
 *        will be used throughout this library.
 */

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#define HEI_INF(...)                                                           \
    {                                                                          \
        printf("HWDIAGS I> " __VA_ARGS__);                                     \
        printf("\n");                                                          \
    }

#define HEI_ERR(...)                                                           \
    {                                                                          \
        printf("HWDIAGS E> " __VA_ARGS__);                                     \
        printf("\n");                                                          \
    }

#define HEI_ASSERT(expression) assert(expression);
