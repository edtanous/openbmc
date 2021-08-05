#include <analyzer/service_data.hpp>
#include <hei_main.hpp>
#include <nlohmann/json.hpp>
#include <util/data_file.hpp>
#include <util/trace.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace analyzer
{

//------------------------------------------------------------------------------

// Returns the RAS data schema file. Will throw exceptions if the file does not
// exist, is unreadable, or is malformed JSON.
nlohmann::json __getSchemaFile()
{
    // Search all the files in the package `schema` subdirectory and find the
    // file named `ras-data-schema.json`.
    fs::path dirPath{PACKAGE_DIR "schema"};
    std::vector<fs::path> files;
    util::findFiles(dirPath, R"(ras-data-schema\.json)", files);
    assert(1 == files.size()); // Should be one, and only one, file.

    // Trace the file for debug.
    trace::inf("File found: path=%s", files.front().string().c_str());

    // Open the file.
    std::ifstream file{files.front()};
    assert(file.good()); // The file must be readable.

    // Parse and return the JSON. Note that this will throw an exception if the
    // file is not properly formatted.
    return nlohmann::json::parse(file);
}

//------------------------------------------------------------------------------

void __getDataFiles(std::map<libhei::ChipType_t, nlohmann::json>& o_files)
{
    o_files.clear(); // initially empty

    // Get the schema document for the data files.
    const auto schema = __getSchemaFile();

    // Get all JSON files in the package `data` subdirectory.
    fs::path dirPath{PACKAGE_DIR "ras-data"};
    std::vector<fs::path> files;
    util::findFiles(dirPath, R"(.*\.json)", files);

    for (const auto& path : files)
    {
        // Trace each data file for debug.
        trace::inf("File found: path=%s", path.string().c_str());

        // Open the file.
        std::ifstream file{path};
        assert(file.good()); // The file must be readable.

        // Parse the JSON. Note that this will throw an exception if the
        // file is not properly formatted.
        const auto data = nlohmann::json::parse(file);

        // Validate the data against the schema.
        assert(util::validateJson(schema, data));

        // Get the chip model/EC level from the data. The value is currently
        // stored as a string representation of the hex value. So it will have
        // to be converted to an integer.
        libhei::ChipType_t chipType =
            std::stoul(data.at("model_ec").get<std::string>(), 0, 16);

        // So far, so good. Add the entry.
        auto ret = o_files.emplace(chipType, data);
        assert(ret.second); // Should not have duplicate entries
    }
}

//------------------------------------------------------------------------------

void applyRasActions(ServiceData& io_servData)
{
    // Find all of the existing chip data files.
    std::map<libhei::ChipType_t, nlohmann::json> files;
    __getDataFiles(files);

    // Get the RAS data file for this signature's chip type.
    auto rootCause   = io_servData.getRootCause();
    auto rasData_itr = files.find(rootCause.getChip().getType());
    assert(files.end() != rasData_itr); // the data file must exist

    // TODO: finish implementing

    // The default action is to callout level 2 support.
    io_servData.addCallout(std::make_shared<ProcedureCallout>(
        ProcedureCallout::NEXTLVL, Callout::Priority::HIGH));
}

//------------------------------------------------------------------------------

} // namespace analyzer
