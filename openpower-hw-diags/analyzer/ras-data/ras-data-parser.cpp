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
    RasDataParser::getResolution(const libhei::Signature&)
{
    // TODO: Default to level 2 support callout until fully implemented.
    return std::make_shared<ProcedureCalloutResolution>(
        ProcedureCallout::NEXTLVL, Callout::HIGH);
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

} // namespace analyzer
