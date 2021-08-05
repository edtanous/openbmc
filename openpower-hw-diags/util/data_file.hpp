#include <nlohmann/json.hpp>

#include <filesystem>

namespace util
{

/**
 * @brief Returns a list of files in the given directory, matching the given
 *        search string.
 * @param i_dirPath     The target directory.
 * @param i_matchString Matching search pattern.
 * @param o_foundPaths  The returned list of found file paths.
 */
void findFiles(const std::filesystem::path& i_dirPath,
               const std::string& i_matchString,
               std::vector<std::filesystem::path>& o_foundPaths);

/**
 * @brief  Validates the given JSON file against the given schema.
 * @param  i_schema Target schema document.
 * @param  i_json   Target JSON file.
 * @return True, if validation successful. False, otherwise.
 */
bool validateJson(const nlohmann::json& i_schema, const nlohmann::json& i_json);

} // namespace util
