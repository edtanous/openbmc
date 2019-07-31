#include "hei_isolator.hpp"

int main()
{
    libhei::Isolator iso;

    iso.initialize();

    iso.isolate();

    return 0;
}
