#include <stdarg.h>
#include <stdio.h>

namespace libhei
{

// prints a single line to stdout
void hei_inf(char* format, ...)
{
    va_list args;
    fprintf(stdout, "I> ");
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fprintf(stdout, "\n");
}

// prints a single line to stderr
void hei_err(char* format, ...)
{
    va_list args;
    fprintf(stderr, "E> ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

} // namespace libhei
