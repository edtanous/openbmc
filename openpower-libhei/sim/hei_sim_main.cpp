
#include "../hei_isolator.hpp"

namespace HEI_SIM
{

int main()
{
    HEI::Isolator iso;

    iso.initialize();

    iso.isolate();

    return 0;
}

}; // end namespace HEI_SIM
