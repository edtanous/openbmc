#include <analyzer/service_data.hpp>

namespace analyzer
{

//------------------------------------------------------------------------------

void applyRasActions(ServiceData& io_servData)
{
    // TODO: finish implementing

    // The default action is to callout level 2 support.
    io_servData.addCallout(std::make_shared<ProcedureCallout>(
        ProcedureCallout::NEXTLVL, Callout::Priority::HIGH));
}

//------------------------------------------------------------------------------

} // namespace analyzer
