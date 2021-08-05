#include <util/data_file.hpp>
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

#include <regex>

namespace fs = std::filesystem;

namespace util
{

void findFiles(const fs::path& i_dirPath, const std::string& i_matchString,
               std::vector<fs::path>& o_foundPaths)
{
    if (fs::exists(i_dirPath))
    {
        std::regex search{i_matchString};
        for (const auto& file : fs::directory_iterator(i_dirPath))
        {
            std::string path = file.path().string();
            if (std::regex_search(path, search))
            {
                o_foundPaths.emplace_back(file.path());
            }
        }
    }
}

bool validateJson(const nlohmann::json& i_schema, const nlohmann::json& i_json)
{
    valijson::Schema schema;
    valijson::SchemaParser parser;
    valijson::adapters::NlohmannJsonAdapter schemaAdapter(i_schema);
    parser.populateSchema(schemaAdapter, schema);

    valijson::Validator validator;
    valijson::adapters::NlohmannJsonAdapter targetAdapter(i_json);

    return validator.validate(schema, targetAdapter, nullptr);
}

} // namespace util
