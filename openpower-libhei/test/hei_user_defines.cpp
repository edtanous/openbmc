#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

namespace libhei
{

void hei_inf(char* format, ...)
{
    va_list args;

    printf("I> ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void hei_err(char* format, ...)
{
    va_list args;

    printf("E> ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

} // namespace libhei
