#include <analyzer/service_data.hpp>

namespace analyzer
{

void ServiceData::addCallout(const nlohmann::json& i_callout)
{
    // The new callout is either a hardware callout with a location code or a
    // procedure callout.

    std::string type{};
    if (i_callout.contains("LocationCode"))
    {
        type = "LocationCode";
    }
    else if (i_callout.contains("Procedure"))
    {
        type = "Procedure";
    }
    else
    {
        throw std::logic_error("Unsupported callout: " + i_callout.dump());
    }

    // A map to determine the priority order. All of the medium priorities,
    // including the medium group priorities, are all the same level.
    static const std::map<std::string, unsigned int> m = {
        {"H", 3}, {"M", 2}, {"A", 2}, {"B", 2}, {"C", 2}, {"L", 1},
    };

    bool addCallout = true;

    for (auto& c : iv_calloutList)
    {
        if (c.contains(type) && (c.at(type) == i_callout.at(type)))
        {
            // The new callout already exists. Don't add a new callout.
            addCallout = false;

            if (m.at(c.at("Priority")) < m.at(i_callout.at("Priority")))
            {
                // The new callout has a higher priority, update it.
                c["Priority"] = i_callout.at("Priority");
            }
        }
    }

    if (addCallout)
    {
        iv_calloutList.push_back(i_callout);
    }
}

} // namespace analyzer
