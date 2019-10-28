#include <hei_main.hpp>

int main()
{
    using namespace libhei;

    void* buffer = nullptr;
    size_t sz_buffer = 0;

    initialize(buffer, sz_buffer);

    Chip c1, c2;

    std::vector<Chip> chipList;
    chipList.push_back(c1);
    chipList.push_back(c2);

    IsolationData isoData;

    isolate(chipList, isoData);

    uninitialize();

    return 0;
}
