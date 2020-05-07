#include <hei_main.hpp>

namespace analyzer
{

/** Analyze error condition using the hardware error isolator */
void analyzeHardware()
{
    using namespace libhei;

    std::vector<Chip> chipList; // data to isolator
    IsolationData isoData{};    // data from isolator

    // TEMP CODE - start
    initialize(nullptr, 0);

    chipList.emplace_back(Chip{"proc", static_cast<ChipType_t>(0xdeadbeef)});

    isolate(chipList, isoData);

    uninitialize();
    // TEMP CODE - end
}

} // namespace analyzer
