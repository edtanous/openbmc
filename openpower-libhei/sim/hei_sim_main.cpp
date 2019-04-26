
#include "../hei_isolator.hpp"

int main()
{
    HEI::Isolator iso;

    iso.initialize();

    iso.isolate();

    return 0;
}
