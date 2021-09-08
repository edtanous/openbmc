#include <analyzer/ras-data/ras-data-parser.hpp>
#include <util/data_file.hpp>
#include <util/trace.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace analyzer
{

//------------------------------------------------------------------------------

std::shared_ptr<Resolution>
    RasDataParser::getResolution(const libhei::Signature& i_signature)
{
    const auto data = iv_dataFiles.at(i_signature.getChip().getType());

    const auto action = parseSignature(data, i_signature);

    return parseAction(data, action);
}

//------------------------------------------------------------------------------

void RasDataParser::initDataFiles()
{
    iv_dataFiles.clear(); // initially empty

    // Get the RAS data schema files from the package `schema` subdirectory.
    fs::path schemaDir{PACKAGE_DIR "schema"};
    auto schemaRegex = R"(ras-data-schema-v[0-9]{2}\.json)";
    std::vector<fs::path> schemaPaths;
    util::findFiles(schemaDir, schemaRegex, schemaPaths);

    // Parse each of the schema files.
    std::map<unsigned int, nlohmann::json> schemaFiles;
    for (const auto& path : schemaPaths)
    {
        // Trace each data file for debug.
        trace::inf("File found: path=%s", path.string().c_str());

        // Open the file.
        std::ifstream file{path};
        assert(file.good()); // The file must be readable.

        // Parse the JSON.
        auto schema = nlohmann::json::parse(file);

        // Get the schema version.
        auto version = schema.at("version").get<unsigned int>();

        // Keep track of the schemas.
        auto ret = schemaFiles.emplace(version, schema);
        assert(ret.second); // Should not have duplicate entries
    }

    // Get the RAS data files from the package `data` subdirectory.
    fs::path dataDir{PACKAGE_DIR "ras-data"};
    std::vector<fs::path> dataPaths;
    util::findFiles(dataDir, R"(.*\.json)", dataPaths);

    // Parse each of the data files.
    for (const auto& path : dataPaths)
    {
        // Trace each data file for debug.
        trace::inf("File found: path=%s", path.string().c_str());

        // Open the file.
        std::ifstream file{path};
        assert(file.good()); // The file must be readable.

        // Parse the JSON.
        const auto data = nlohmann::json::parse(file);

        // Get the data version.
        auto version = data.at("version").get<unsigned int>();

        // Get the schema for this file.
        auto schema = schemaFiles.at(version);

        // Validate the data against the schema.
        assert(util::validateJson(schema, data));

        // Get the chip model/EC level from the data. The value is currently
        // stored as a string representation of the hex value. So it will have
        // to be converted to an integer.
        libhei::ChipType_t chipType =
            std::stoul(data.at("model_ec").get<std::string>(), 0, 16);

        // So far, so good. Add the entry.
        auto ret = iv_dataFiles.emplace(chipType, data);
        assert(ret.second); // Should not have duplicate entries
    }
}

//------------------------------------------------------------------------------

std::string RasDataParser::parseSignature(const nlohmann::json& i_data,
                                          const libhei::Signature& i_signature)
{
    // Get the signature keys. All are hex (lower case) with no prefix.
    char buf[5];
    sprintf(buf, "%04x", i_signature.getId());
    std::string id{buf};

    sprintf(buf, "%02x", i_signature.getBit());
    std::string bit{buf};

    sprintf(buf, "%02x", i_signature.getInstance());
    std::string inst{buf};

    // Return the action.
    return i_data.at("signatures").at(id).at(bit).at(inst).get<std::string>();
}

//------------------------------------------------------------------------------

std::shared_ptr<Resolution>
    RasDataParser::parseAction(const nlohmann::json& i_data,
                               const std::string& i_action)
{
    auto o_list = std::make_shared<ResolutionList>();

    // This function will be called recursively and we want to prevent cyclic
    // recursion.
    static std::vector<std::string> stack;
    assert(stack.end() == std::find(stack.begin(), stack.end(), i_action));
    stack.push_back(i_action);

    // Iterate the action list and apply the changes.
    for (const auto& a : i_data.at("actions").at(i_action))
    {
        auto type = a.at("type").get<std::string>();

        if ("action" == type)
        {
            auto name = a.at("name").get<std::string>();

            o_list->push(parseAction(i_data, name));
        }
        else if ("callout_self" == type)
        {
            auto priority = a.at("priority").get<std::string>();
            auto guard    = a.at("guard").get<bool>();

            std::string path{}; // Must be empty to callout the chip.

            o_list->push(std::make_shared<HardwareCalloutResolution>(
                path, getPriority(priority), guard));
        }
        else if ("callout_unit" == type)
        {
            auto name     = a.at("name").get<std::string>();
            auto priority = a.at("priority").get<std::string>();
            auto guard    = a.at("guard").get<bool>();

            auto path = i_data.at("units").at(name).get<std::string>();

            o_list->push(std::make_shared<HardwareCalloutResolution>(
                path, getPriority(priority), guard));
        }
        else if ("callout_connected" == type)
        {
            auto name     = a.at("name").get<std::string>();
            auto priority = a.at("priority").get<std::string>();
            auto guard    = a.at("guard").get<bool>();

            // TODO
            trace::inf("callout_connected: name=%s priority=%s guard=%c",
                       name.c_str(), priority.c_str(), guard ? 'T' : 'F');
        }
        else if ("callout_bus" == type)
        {
            auto name = a.at("name").get<std::string>();
            // auto rx_priority = a.at("rx_priority").get<std::string>();
            // auto tx_priority = a.at("tx_priority").get<std::string>();
            auto guard = a.at("guard").get<bool>();

            // TODO
            trace::inf("callout_bus: name=%s guard=%c", name.c_str(),
                       guard ? 'T' : 'F');
        }
        else if ("callout_clock" == type)
        {
            auto name     = a.at("name").get<std::string>();
            auto priority = a.at("priority").get<std::string>();
            auto guard    = a.at("guard").get<bool>();

            // clang-format off
            static const std::map<std::string, callout::ClockType> m =
            {
                {"OSC_REF_CLOCK_0", callout::ClockType::OSC_REF_CLOCK_0},
                {"OSC_REF_CLOCK_1", callout::ClockType::OSC_REF_CLOCK_1},
            };
            // clang-format on

            o_list->push(std::make_shared<ClockCalloutResolution>(
                m.at(name), getPriority(priority), guard));
        }
        else if ("callout_procedure" == type)
        {
            auto name     = a.at("name").get<std::string>();
            auto priority = a.at("priority").get<std::string>();

            // clang-format off
            static const std::map<std::string, callout::Procedure> m =
            {
                {"LEVEL2", callout::Procedure::NEXTLVL},
            };
            // clang-format on

            o_list->push(std::make_shared<ProcedureCalloutResolution>(
                m.at(name), getPriority(priority)));
        }
        else if ("callout_part" == type)
        {
            auto name     = a.at("name").get<std::string>();
            auto priority = a.at("priority").get<std::string>();

            // TODO
            trace::inf("callout_part: name=%s priority=%s", name.c_str(),
                       priority.c_str());
        }
        else if ("plugin" == type)
        {
            auto name = a.at("name").get<std::string>();

            // TODO
            trace::inf("plugin: name=%s", name.c_str());
        }
        else
        {
            throw std::logic_error("Unsupported action type: " + type);
        }
    }

    // Done with this action pop it off the stack.
    stack.pop_back();

    return o_list;
}

//------------------------------------------------------------------------------

callout::Priority RasDataParser::getPriority(const std::string& i_priority)
{
    // clang-format off
    static const std::map<std::string, callout::Priority> m =
    {
        {"HIGH",  callout::Priority::HIGH},
        {"MED",   callout::Priority::MED},
        {"MED_A", callout::Priority::MED_A},
        {"MED_B", callout::Priority::MED_B},
        {"MED_C", callout::Priority::MED_C},
        {"LOW",   callout::Priority::LOW},
    };
    // clang-format on

    return m.at(i_priority);
}

//------------------------------------------------------------------------------

} // namespace analyzer
