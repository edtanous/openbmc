#include <hei_main.hpp>

namespace analyzer
{

// Quick test that we can call the core libhei functions. This function
// is called from the attention handler when a checkstop event is active.
void analyzeHardware()
{
    using namespace libhei;

    // Add some chips for error isolation
    std::vector<Chip> chipList;
    chipList.emplace_back(Chip{"proc", static_cast<ChipType_t>(0xdeadbeef)});

    // Isolation data that will be populated by isolator
    IsolationData isoData{};

    // Isolate active hardware errors in chips
    initialize(nullptr, 0);
    isolate(chipList, isoData);
    uninitialize();
}

} // namespace analyzer
