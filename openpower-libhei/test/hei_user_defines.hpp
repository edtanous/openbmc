#pragma once

/**
 * @file hei_user_defines.hpp
 * @brief The purpose of this file is to create common defines that
 *        will be used throughout this library.
 */

#include <stdio.h>
#include <assert.h>

#define HEI_INF(...) \
    { \
      printf("I> " __VA_ARGS__); \
      printf("\n"); \
    }

#define HEI_ERR(...) \
    { \
      printf("E> " __VA_ARGS__); \
      printf("\n"); \
    }

#define HEI_ASSERT(expression) assert(expression);
