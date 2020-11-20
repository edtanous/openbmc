extern "C"
{
#include <libpdbg.h>
}

#include "create_pel.hpp"
#include "phal_error.hpp"

#include <attributes_info.H>
#include <fmt/format.h>
#include <libekb.H>
#include <libipl.H>

#include <nlohmann/json.hpp>
#include <phosphor-logging/elog.hpp>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <list>
#include <map>
#include <sstream>
#include <string>

namespace openpower
{
namespace phal
{
using namespace phosphor::logging;

/**
 * Used to pass buffer to pdbg callback api to get required target
 * data (attributes) based on given data (attribute).
 */
struct TargetInfo
{
    ATTR_PHYS_BIN_PATH_Type physBinPath;
    ATTR_LOCATION_CODE_Type locationCode;
    ATTR_PHYS_DEV_PATH_Type physDevPath;
    ATTR_MRU_ID_Type mruId;

    bool deconfigure;

    TargetInfo()
    {
        memset(&physBinPath, '\0', sizeof(physBinPath));
        memset(&locationCode, '\0', sizeof(locationCode));
        memset(&physDevPath, '\0', sizeof(physDevPath));
        mruId = 0;
        deconfigure = false;
    }
};

/**
 * Used to return in callback function which are used to get
 * physical path value and it binary format value.
 *
 * The value for constexpr defined based on pdbg_target_traverse function usage.
 */
constexpr int continueTgtTraversal = 0;
constexpr int requireAttrFound = 1;
constexpr int requireAttrNotFound = 2;

/**
 * @brief Used to get target location code from phal device tree
 *
 * @param[in] target current device tree target
 * @param[out] appPrivData used for accessing|storing from|to application
 *
 * @return 0 to continue traverse, non-zero to stop traverse
 */
int pdbgCallbackToGetTgtReqAttrsVal(struct pdbg_target* target,
                                    void* appPrivData)
{
    TargetInfo* targetInfo = static_cast<TargetInfo*>(appPrivData);

    ATTR_PHYS_BIN_PATH_Type physBinPath;
    /**
     * TODO: Issue: phal/pdata#16
     * Should not use direct pdbg api to read attribute. Need to use DT_GET_PROP
     * macro for bmc app's and this will call libdt-api api but, it will print
     * "pdbg_target_get_attribute failed" trace if attribute is not found and
     * this callback will call recursively by using pdbg_target_traverse() until
     * find expected attribute based on return code from this callback. Because,
     * need to do target iteration to get actual attribute (ATTR_PHYS_BIN_PATH)
     * value when device tree target info doesn't know to read attribute from
     * device tree. So, Due to this error trace user will get confusion while
     * looking traces. Hence using pdbg api to avoid trace until libdt-api
     * provides log level setup.
     */
    if (!pdbg_target_get_attribute(
            target, "ATTR_PHYS_BIN_PATH",
            std::stoi(dtAttr::fapi2::ATTR_PHYS_BIN_PATH_Spec),
            dtAttr::fapi2::ATTR_PHYS_BIN_PATH_ElementCount, physBinPath))
    {
        return continueTgtTraversal;
    }

    if (std::memcmp(physBinPath, targetInfo->physBinPath,
                    sizeof(physBinPath)) != 0)
    {
        return continueTgtTraversal;
    }

    if (DT_GET_PROP(ATTR_LOCATION_CODE, target, targetInfo->locationCode))
    {
        log<level::ERR>("Could not read LOCATION_CODE attribute");
        return requireAttrNotFound;
    }

    if (DT_GET_PROP(ATTR_PHYS_DEV_PATH, target, targetInfo->physDevPath))
    {
        log<level::ERR>("Could not read PHYS_DEV_PATH attribute");
        return requireAttrNotFound;
    }

    if (DT_GET_PROP(ATTR_MRU_ID, target, targetInfo->mruId))
    {
        log<level::ERR>("Could not read MRU_ID attribute");
        return requireAttrNotFound;
    }

    if (targetInfo->deconfigure)
    {
        ATTR_HWAS_STATE_Type hwasState;
        if (DT_GET_PROP(ATTR_HWAS_STATE, target, hwasState))
        {
            log<level::ERR>("Could not read HWAS_STATE attribute");
            return requireAttrNotFound;
        }

        log<level::INFO>(fmt::format("Marking target({}) as Non-Functional",
                                     targetInfo->physDevPath)
                             .c_str());
        hwasState.functional = 0;

        if (DT_SET_PROP(ATTR_HWAS_STATE, target, hwasState))
        {
            log<level::ERR>("Could not write HWAS_STATE attribute");
            return requireAttrNotFound;
        }
    }

    return requireAttrFound;
}

/**
 * @brief Used to get target info (attributes data)
 *
 * To get target required attributes value using another attribute value
 * ("PHYS_BIN_PATH" which is present in same target attributes list) by using
 * "ipdbg_target_traverse" api because, here we have attribute value only and
 * doesn't have respective device tree target info to get required attributes
 * values from it attributes list.
 *
 * @param[in] physBinPath to pass PHYS_BIN_PATH value
 * @param[out] targetInfo to pas buufer to fill with required attributes
 *
 * @return true on success otherwise false
 */
bool getTgtReqAttrsVal(const std::vector<uint8_t>& physBinPath,
                       TargetInfo& targetInfo)
{
    std::memcpy(&targetInfo.physBinPath, physBinPath.data(),
                sizeof(targetInfo.physBinPath));

    int ret = pdbg_target_traverse(NULL, pdbgCallbackToGetTgtReqAttrsVal,
                                   &targetInfo);
    if (ret == 0)
    {
        log<level::ERR>(fmt::format("Given ATTR_PHYS_BIN_PATH value({}) "
                                    "not found in phal device tree",
                                    targetInfo.physBinPath)
                            .c_str());
        return false;
    }
    else if (ret == requireAttrNotFound)
    {
        return false;
    }

    return true;
}
} // namespace phal

namespace pel
{
using namespace phosphor::logging;

namespace detail
{
using json = nlohmann::json;

// keys need to be unique so using counter value to generate unique key
static int counter = 0;

// list of debug traces
static std::vector<std::pair<std::string, std::string>> traceLog;

void processLogTraceCallback(void*, const char* fmt, va_list ap)
{
    va_list vap;
    va_copy(vap, ap);
    std::vector<char> logData(1 + std::vsnprintf(nullptr, 0, fmt, ap));
    std::vsnprintf(logData.data(), logData.size(), fmt, vap);
    va_end(vap);
    std::string logstr(logData.begin(), logData.end());

    log<level::INFO>(logstr.c_str());

    char timeBuf[80];
    time_t t = time(0);
    tm myTm{};
    gmtime_r(&t, &myTm);
    strftime(timeBuf, 80, "%Y-%m-%d %H:%M:%S", &myTm);

    // key values need to be unique for PEL
    // TODO #openbmc/dev/issues/1563
    // If written to Json no need to worry about unique KEY
    std::stringstream str;
    str << std::setfill('0');
    str << "LOG" << std::setw(3) << counter;
    str << " " << timeBuf;
    traceLog.emplace_back(std::make_pair(str.str(), std::move(logstr)));
    counter++;
}

/**
 * @brief GET PEL priority from pHAL priority
 *
 * The pHAL callout priority is in different format than PEL format
 * so, this api is used to return current phal supported priority into
 * PEL expected format.
 *
 * @param[in] phalPriority used to pass phal priority format string
 *
 * @return pel priority format string else empty if failure
 *
 * @note For "NONE" returning "L" (LOW)
 */
static std::string getPelPriority(const std::string& phalPriority)
{
    const std::map<std::string, std::string> priorityMap = {
        {"HIGH", "H"}, {"MEDIUM", "M"}, {"LOW", "L"}, {"NONE", "L"}};

    auto it = priorityMap.find(phalPriority);
    if (it == priorityMap.end())
    {
        log<level::ERR>(fmt::format("Unsupported phal priority({}) is given "
                                    "to get pel priority format",
                                    phalPriority)
                            .c_str());
        return "H";
    }

    return it->second;
}

void processBootErrorCallback(bool status)
{
    log<level::INFO>("processBootCallback ", entry("STATUS=%d", status));
    try
    {
        // return If no failure during hwp execution
        if (status)
            return;

        // Collecting ffdc details from phal
        FFDC ffdc;
        libekb_get_ffdc(ffdc);

        log<level::INFO>(
            fmt::format("PHAL FFDC: Return Message[{}]", ffdc.message).c_str());

        // To store callouts details in json format as per pel expectation.
        json jsonCalloutDataList;
        jsonCalloutDataList = json::array();

        // To store phal trace and other additional data about ffdc.
        FFDCData pelAdditionalData;

        if (ffdc.ffdc_type == FFDC_TYPE_HWP)
        {
            // Adding hardware procedures return code details
            pelAdditionalData.emplace_back("HWP_RC", ffdc.hwp_errorinfo.rc);
            pelAdditionalData.emplace_back("HWP_RC_DESC",
                                           ffdc.hwp_errorinfo.rc_desc);

            // Adding hardware procedures required ffdc data for debug
            for_each(ffdc.hwp_errorinfo.ffdcs_data.begin(),
                     ffdc.hwp_errorinfo.ffdcs_data.end(),
                     [&pelAdditionalData](
                         std::pair<std::string, std::string>& ele) -> void {
                         std::string keyWithPrefix("HWP_FFDC_");
                         keyWithPrefix.append(ele.first);

                         pelAdditionalData.emplace_back(keyWithPrefix,
                                                        ele.second);
                     });

            // Adding hardware callout details
            int calloutCount = 0;
            for_each(ffdc.hwp_errorinfo.hwcallouts.begin(),
                     ffdc.hwp_errorinfo.hwcallouts.end(),
                     [&pelAdditionalData, &calloutCount, &jsonCalloutDataList](
                         const HWCallout& hwCallout) -> void {
                         calloutCount++;
                         std::stringstream keyPrefix;
                         keyPrefix << "HWP_HW_CO_" << std::setfill('0')
                                   << std::setw(2) << calloutCount << "_";

                         pelAdditionalData.emplace_back(
                             std::string(keyPrefix.str()).append("HW_ID"),
                             hwCallout.hwid);

                         pelAdditionalData.emplace_back(
                             std::string(keyPrefix.str()).append("PRIORITY"),
                             hwCallout.callout_priority);

                         phal::TargetInfo targetInfo;
                         phal::getTgtReqAttrsVal(hwCallout.target_entity_path,
                                                 targetInfo);

                         std::string locationCode =
                             std::string(targetInfo.locationCode);
                         pelAdditionalData.emplace_back(
                             std::string(keyPrefix.str()).append("LOC_CODE"),
                             locationCode);

                         std::string physPath =
                             std::string(targetInfo.physDevPath);
                         pelAdditionalData.emplace_back(
                             std::string(keyPrefix.str()).append("PHYS_PATH"),
                             physPath);

                         pelAdditionalData.emplace_back(
                             std::string(keyPrefix.str()).append("CLK_POS"),
                             std::to_string(hwCallout.clkPos));

                         json jsonCalloutData;
                         jsonCalloutData["LocationCode"] = locationCode;
                         std::string pelPriority =
                             getPelPriority(hwCallout.callout_priority);
                         jsonCalloutData["Priority"] = pelPriority;

                         if (targetInfo.mruId != 0)
                         {
                             jsonCalloutData["MRUs"] = json::array({
                                 {{"ID", targetInfo.mruId},
                                  {"Priority", pelPriority}},
                             });
                         }

                         jsonCalloutDataList.emplace_back(jsonCalloutData);
                     });

            // Adding CDG (callout, deconfigure and guard) targets details
            calloutCount = 0;
            for_each(ffdc.hwp_errorinfo.cdg_targets.begin(),
                     ffdc.hwp_errorinfo.cdg_targets.end(),
                     [&pelAdditionalData, &calloutCount,
                      &jsonCalloutDataList](const CDG_Target& cdg_tgt) -> void {
                         calloutCount++;
                         std::stringstream keyPrefix;
                         keyPrefix << "HWP_CDG_TGT_" << std::setfill('0')
                                   << std::setw(2) << calloutCount << "_";

                         phal::TargetInfo targetInfo;
                         targetInfo.deconfigure = cdg_tgt.deconfigure;

                         phal::getTgtReqAttrsVal(cdg_tgt.target_entity_path,
                                                 targetInfo);

                         std::string locationCode =
                             std::string(targetInfo.locationCode);
                         pelAdditionalData.emplace_back(
                             std::string(keyPrefix.str()).append("LOC_CODE"),
                             locationCode);
                         std::string physPath =
                             std::string(targetInfo.physDevPath);
                         pelAdditionalData.emplace_back(
                             std::string(keyPrefix.str()).append("PHYS_PATH"),
                             physPath);

                         pelAdditionalData.emplace_back(
                             std::string(keyPrefix.str()).append("CO_REQ"),
                             (cdg_tgt.callout == true ? "true" : "false"));

                         pelAdditionalData.emplace_back(
                             std::string(keyPrefix.str()).append("CO_PRIORITY"),
                             cdg_tgt.callout_priority);

                         pelAdditionalData.emplace_back(
                             std::string(keyPrefix.str()).append("DECONF_REQ"),
                             (cdg_tgt.deconfigure == true ? "true" : "false"));

                         pelAdditionalData.emplace_back(
                             std::string(keyPrefix.str()).append("GUARD_REQ"),
                             (cdg_tgt.guard == true ? "true" : "false"));

                         pelAdditionalData.emplace_back(
                             std::string(keyPrefix.str()).append("GUARD_TYPE"),
                             cdg_tgt.guard_type);

                         json jsonCalloutData;
                         jsonCalloutData["LocationCode"] = locationCode;
                         std::string pelPriority =
                             getPelPriority(cdg_tgt.callout_priority);
                         jsonCalloutData["Priority"] = pelPriority;

                         if (targetInfo.mruId != 0)
                         {
                             jsonCalloutData["MRUs"] = json::array({
                                 {{"ID", targetInfo.mruId},
                                  {"Priority", pelPriority}},
                             });
                         }
                         jsonCalloutData["Deconfigured"] = cdg_tgt.deconfigure;
                         jsonCalloutData["Guarded"] = cdg_tgt.guard;

                         jsonCalloutDataList.emplace_back(jsonCalloutData);
                     });
        }
        else if ((ffdc.ffdc_type != FFDC_TYPE_NONE) &&
                 (ffdc.ffdc_type != FFDC_TYPE_UNSUPPORTED))
        {
            log<level::ERR>(
                fmt::format("Unsupported phal FFDC type to create PEL. "
                            "MSG: {}",
                            ffdc.message)
                    .c_str());
        }

        // Adding collected phal logs into PEL additional data
        for_each(traceLog.begin(), traceLog.end(),
                 [&pelAdditionalData](
                     std::pair<std::string, std::string>& ele) -> void {
                     pelAdditionalData.emplace_back(ele.first, ele.second);
                 });

        // TODO: #ibm-openbmc/dev/issues/2595 : Once enabled this support,
        // callout details is not required to sort in H,M and L orders which
        // are expected by pel because, pel will take care for sorting callouts
        // based on priority so, now adding support to send callout in order
        // i.e High -> Medium -> Low.
        std::sort(
            jsonCalloutDataList.begin(), jsonCalloutDataList.end(),
            [](const json& aEle, const json& bEle) -> bool {
                // Considering b element having higher priority than a element
                // or Both element will be same priorty (to keep same order
                // which are given by phal when two callouts are having same
                // priority)
                if (((aEle["Priority"] == "M") && (bEle["Priority"] == "H")) ||
                    ((aEle["Priority"] == "L") &&
                     ((bEle["Priority"] == "H") ||
                      (bEle["Priority"] == "M"))) ||
                    (aEle["Priority"] == bEle["Priority"]))
                {
                    return false;
                }

                // Considering a element having higher priority than b element
                return true;
            });

        openpower::pel::createBootErrorPEL(pelAdditionalData,
                                           jsonCalloutDataList);
    }
    catch (std::exception& ex)
    {
        reset();
        throw ex;
    }
    reset();
}

void reset()
{
    // reset the trace log and counter
    traceLog.clear();
    counter = 0;
}

void pDBGLogTraceCallbackHelper(int, const char* fmt, va_list ap)
{
    processLogTraceCallback(NULL, fmt, ap);
}
} // namespace detail

static inline uint8_t getLogLevelFromEnv(const char* env, const uint8_t dValue)
{
    auto logLevel = dValue;
    try
    {
        if (const char* env_p = std::getenv(env))
        {
            logLevel = std::stoi(env_p);
        }
    }
    catch (std::exception& e)
    {
        log<level::ERR>(("Conversion Failure"), entry("ENVIRONMENT=%s", env),
                        entry("EXCEPTION=%s", e.what()));
    }
    return logLevel;
}

void addBootErrorCallbacks()
{
    // Get individual phal repos log level from environment variable
    // and update the  log level.
    pdbg_set_loglevel(getLogLevelFromEnv("PDBG_LOG", PDBG_INFO));
    libekb_set_loglevel(getLogLevelFromEnv("LIBEKB_LOG", LIBEKB_LOG_IMP));
    ipl_set_loglevel(getLogLevelFromEnv("IPL_LOG", IPL_INFO));

    // add callback for debug traces
    pdbg_set_logfunc(detail::pDBGLogTraceCallbackHelper);
    libekb_set_logfunc(detail::processLogTraceCallback, NULL);
    ipl_set_logfunc(detail::processLogTraceCallback, NULL);

    // add callback for ipl failures
    ipl_set_error_callback_func(detail::processBootErrorCallback);
}

} // namespace pel
} // namespace openpower
