#include "hei_isolator.hpp"

int main()
{
    libhei::Isolator iso;

    void * buffer = nullptr;
    size_t sz_buffer = 0;

    iso.initialize( buffer, sz_buffer );

    void * chip = nullptr;
    libhei::ChipType_t chipType = 0;
    libhei::IsolationData isoData;

    iso.isolate( chip, chipType, isoData );

    return 0;
}
