#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>
namespace openpower
{
namespace pel
{
using FFDCData = std::vector<std::pair<std::string, std::string>>;

using json = nlohmann::json;

/**
 * Create boot error PEL
 *
 * @param[in] ffdcData - failure data to append to PEL
 * @param[in] calloutData - callout data to append to PEL
 */
void createBootErrorPEL(const FFDCData& ffdcData, const json& calloutData);

/**
 * @brief Create a PEL for the specified event type
 *
 *  @param  event - the event type
 */
void createPEL(const std::string& event);

/**
 * @class FFDCFile
 * @brief This class is used to create ffdc data file and to get fd
 */
class FFDCFile
{
  public:
    FFDCFile() = delete;
    FFDCFile(const FFDCFile&) = delete;
    FFDCFile& operator=(const FFDCFile&) = delete;
    FFDCFile(FFDCFile&&) = delete;
    FFDCFile& operator=(FFDCFile&&) = delete;

    /**
     * Used to pass json object to create unique ffdc file by using
     * passed json data.
     */
    explicit FFDCFile(const json& pHALCalloutData);

    /**
     * Used to remove created ffdc file.
     */
    ~FFDCFile();

    /**
     * Used to get created ffdc file file descriptor id.
     *
     * @return file descriptor id
     */
    int getFileFD() const;

  private:
    /**
     * Used to store callout ffdc data from passed json object.
     */
    std::string calloutData;

    /**
     * Used to store unique ffdc file name.
     */
    std::string calloutFile;

    /**
     * Used to store created ffdc file descriptor id.
     */
    int fileFD;

    /**
     * Used to create ffdc file to pass PEL api for creating
     * pel records.
     *
     * @return NULL
     */
    void prepareFFDCFile();

    /**
     * Create unique ffdc file.
     *
     * @return NULL
     */
    void createCalloutFile();

    /**
     * Used write json object value into created file.
     *
     * @return NULL
     */
    void writeCalloutData();

    /**
     * Used set ffdc file seek position begining to consume by PEL
     *
     * @return NULL
     */
    void setCalloutFileSeekPos();

    /**
     * Used to remove created ffdc file.
     *
     * @return NULL
     */
    void removeCalloutFile();

}; // FFDCFile end

} // namespace pel
} // namespace openpower
