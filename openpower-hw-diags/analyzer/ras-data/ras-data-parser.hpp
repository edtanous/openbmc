#pragma once

#include <analyzer/resolution.hpp>
#include <hei_main.hpp>
#include <nlohmann/json.hpp>

#include <map>

namespace analyzer
{

/**
 * @brief Manages the RAS data files and resolves service actions required for
 *        error signatures.
 */
class RasDataParser
{
  public:
    /** @brief Default constructor. */
    RasDataParser()
    {
        initDataFiles();
    }

  private:
    /** @brief The RAS data files. */
    std::map<libhei::ChipType_t, nlohmann::json> iv_dataFiles;

  public:
    /**
     * @brief Returns a resolution for all the RAS actions needed for the given
     *        signature.
     * @param i_signature The target error signature.
     */
    std::shared_ptr<Resolution>
        getResolution(const libhei::Signature& i_signature);

  private:
    /**
     * @brief Parses all of the RAS data JSON files and validates them against
     *        the associated schema.
     */
    void initDataFiles();
};

} // namespace analyzer
