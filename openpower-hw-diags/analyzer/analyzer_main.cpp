#include <hei_main.hpp>

#include <map>
#include <string>

namespace analyzer
{

/** Analyze error condition using the hardware error isolator */
int analyzeHardware(std::map<std::string, std::string>& i_errors)
{
    using namespace libhei;

    std::vector<Chip> chipList; // data to isolator
    IsolationData isoData{};    // data from isolator

    // FIXME TEMP CODE - start

    initialize(nullptr, 0);

    chipList.emplace_back(Chip{"proc", static_cast<ChipType_t>(0xdeadbeef)});

    isolate(chipList, isoData); // isolate errors

    if (!(isoData.getSignatureList().empty()))
    {
        // Signature signature = isoData.getSignatureList().back();
    }
    else
    {
        std::string signature = "0xfeed";
        std::string chip      = "0xbeef";
        i_errors[signature]   = chip;
    }

    uninitialize();

    // FIXME TEMP CODE - end

    return 0; // FIXME - error/success from isolator or isolation data
}

} // namespace analyzer
