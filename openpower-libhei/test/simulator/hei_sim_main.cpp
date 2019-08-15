#include <hei_main.hpp>

int main()
{
    void * buffer = nullptr;
    size_t sz_buffer = 0;

    libhei::initialize( buffer, sz_buffer );

    void * chip = nullptr;
    libhei::ChipType_t chipType = 0;
    libhei::IsolationData isoData { chip, chipType };

    libhei::isolate( isoData );

    libhei::uninitialize();

    return 0;
}
