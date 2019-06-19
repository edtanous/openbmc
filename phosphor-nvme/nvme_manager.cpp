#include "nvme_manager.hpp"

#include "smbus.hpp"

#include <filesystem>
#include <map>
#include <nlohmann/json.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <sstream>
#include <string>

#include "i2c.h"
#define MONITOR_INTERVAL_SECONDS 1
#define NVME_SSD_SLAVE_ADDRESS 0x6a
#define GPIO_BASE_PATH "/sys/class/gpio/gpio"
#define IS_PRESENT "0"
#define POWERGD "1"

static constexpr auto configFile = "/etc/nvme/nvme_config.json";
static constexpr auto delay = std::chrono::milliseconds{100};
using Json = nlohmann::json;

static constexpr const uint8_t COMMAND_CODE_0 = 0;
static constexpr const uint8_t COMMAND_CODE_8 = 8;

static constexpr int SERIALNUMBER_START_INDEX = 3;
static constexpr int SERIALNUMBER_END_INDEX = 23;

static constexpr const int TEMPERATURE_SENSOR_FAILURE = 0x81;

namespace fs = std::filesystem;

namespace phosphor
{
namespace nvme
{

using namespace std;
using namespace phosphor::logging;

std::string intToHex(int input)
{
    std::stringstream tmp;
    tmp << std::hex << input;

    return tmp.str();
}

/** @brief Get NVMe info over smbus  */
bool getNVMeInfobyBusID(int busID, phosphor::nvme::Nvme::NVMeData& nvmeData)
{
    nvmeData.present = true;
    nvmeData.vendor = "";
    nvmeData.serialNumber = "";
    nvmeData.smartWarnings = "";
    nvmeData.statusFlags = "";
    nvmeData.driveLifeUsed = "";
    nvmeData.sensorValue = (int8_t)TEMPERATURE_SENSOR_FAILURE;

    phosphor::smbus::Smbus smbus;

    unsigned char rsp_data_command_0[I2C_DATA_MAX] = {0};
    unsigned char rsp_data_command_8[I2C_DATA_MAX] = {0};

    uint8_t tx_data = COMMAND_CODE_0;

    auto init = smbus.smbusInit(busID);

    static std::unordered_map<int, bool> isErrorSmbus;

    if (init == -1)
    {
        if (isErrorSmbus[busID] != true)
        {
            log<level::ERR>("smbusInit fail!");
            isErrorSmbus[busID] = true;
        }

        nvmeData.present = false;

        return nvmeData.present;
    }

    auto res_int =
        smbus.SendSmbusRWBlockCmdRAW(busID, NVME_SSD_SLAVE_ADDRESS, &tx_data,
                                     sizeof(tx_data), rsp_data_command_0);

    if (res_int < 0)
    {
        if (isErrorSmbus[busID] != true)
        {
            log<level::ERR>("Send command code 0 fail!");
            isErrorSmbus[busID] = true;
        }

        smbus.smbusClose(busID);
        nvmeData.present = false;
        return nvmeData.present;
    }

    tx_data = COMMAND_CODE_8;

    res_int =
        smbus.SendSmbusRWBlockCmdRAW(busID, NVME_SSD_SLAVE_ADDRESS, &tx_data,
                                     sizeof(tx_data), rsp_data_command_8);

    if (res_int < 0)
    {
        if (isErrorSmbus[busID] != true)
        {
            log<level::ERR>("Send command code 8 fail!");
            isErrorSmbus[busID] = true;
        }

        smbus.smbusClose(busID);
        nvmeData.present = false;
        return nvmeData.present;
    }

    nvmeData.vendor =
        intToHex(rsp_data_command_8[1]) + " " + intToHex(rsp_data_command_8[2]);

    for (int offset = SERIALNUMBER_START_INDEX; offset < SERIALNUMBER_END_INDEX;
         offset++)
    {
        nvmeData.serialNumber += static_cast<char>(rsp_data_command_8[offset]);
    }

    nvmeData.statusFlags = intToHex(rsp_data_command_0[1]);
    nvmeData.smartWarnings = intToHex(rsp_data_command_0[2]);
    nvmeData.driveLifeUsed = intToHex(rsp_data_command_0[4]);
    nvmeData.sensorValue = (int8_t)rsp_data_command_0[3];

    smbus.smbusClose(busID);

    isErrorSmbus[busID] = false;

    return nvmeData.present;
}

void Nvme::run()
{
    std::function<void()> callback(std::bind(&Nvme::read, this));
    try
    {
        u_int64_t interval = MONITOR_INTERVAL_SECONDS * 1000000;
        _timer.restart(std::chrono::microseconds(interval));
    }
    catch (const std::exception& e)
    {
        log<level::ERR>("Error in polling loop. "),
            entry("ERROR = %s", e.what());
    }
}

/** @brief Parsing NVMe config JSON file  */
Json parseSensorConfig()
{
    std::ifstream jsonFile(configFile);
    if (!jsonFile.is_open())
    {
        log<level::ERR>("NVMe config JSON file not found");
    }

    auto data = Json::parse(jsonFile, nullptr, false);
    if (data.is_discarded())
    {
        log<level::ERR>("NVMe config readings JSON parser failure");
    }

    return data;
}

/** @brief Obtain the initial configuration value of NVMe  */
std::vector<phosphor::nvme::Nvme::NVMeConfig> Nvme::getNvmeConfig()
{

    phosphor::nvme::Nvme::NVMeConfig nvmeConfig;
    std::vector<phosphor::nvme::Nvme::NVMeConfig> nvmeConfigs;
    int8_t criticalHigh = 0;
    int8_t criticalLow = 0;
    int8_t maxValue = 0;
    int8_t minValue = 0;
    int8_t warningHigh = 0;
    int8_t warningLow = 0;

    try
    {
        auto data = parseSensorConfig();
        static const std::vector<Json> empty{};
        std::vector<Json> readings = data.value("config", empty);
        std::vector<Json> thresholds = data.value("threshold", empty);
        if (!thresholds.empty())
        {
            for (const auto& instance : thresholds)
            {
                criticalHigh = instance.value("criticalHigh", 0);
                criticalLow = instance.value("criticalLow", 0);
                maxValue = instance.value("maxValue", 0);
                minValue = instance.value("minValue", 0);
                warningHigh = instance.value("warningHigh", 0);
                warningLow = instance.value("warningLow", 0);
            }
        }
        else
        {
            log<level::ERR>(
                "Invalid NVMe config file, thresholds dosen't exist");
        }

        if (!readings.empty())
        {
            for (const auto& instance : readings)
            {
                uint8_t index = instance.value("NVMeDriveIndex", 0);
                uint8_t busID = instance.value("NVMeDriveBusID", 0);
                uint8_t presentPin = instance.value("NVMeDrivePresentPin", 0);
                uint8_t pwrGoodPin = instance.value("NVMeDrivePwrGoodPin", 0);

                nvmeConfig.index = std::to_string(index);
                nvmeConfig.busID = busID;
                nvmeConfig.presentPin = presentPin;
                nvmeConfig.pwrGoodPin = pwrGoodPin;
                nvmeConfig.criticalHigh = criticalHigh;
                nvmeConfig.criticalLow = criticalLow;
                nvmeConfig.warningHigh = warningHigh;
                nvmeConfig.warningLow = warningLow;
                nvmeConfig.maxValue = maxValue;
                nvmeConfig.minValue = minValue;
                nvmeConfigs.push_back(nvmeConfig);
            }
        }
        else
        {
            log<level::ERR>("Invalid NVMe config file, config dosen't exist");
        }
    }
    catch (const Json::exception& e)
    {
        log<level::ERR>("Json Exception caught."), entry("MSG: %s", e.what());
    }

    return nvmeConfigs;
}

std::string Nvme::getGPIOValueOfNvme(const std::string& fullPath)
{
    std::string val;
    std::ifstream ifs;
    auto retries = 3;

    while (retries != 0)
    {
        try
        {
            if (!ifs.is_open())
                ifs.open(fullPath);
            ifs.clear();
            ifs.seekg(0);
            ifs >> val;
        }
        catch (const std::exception& e)
        {
            --retries;
            std::this_thread::sleep_for(delay);
            log<level::ERR>("Can not open gpio path.",
                            entry("MSG: %s", e.what()));
            continue;
        }
        break;
    }

    ifs.close();
    return val;
}

/** @brief Monitor NVMe drives every one second  */
void Nvme::read()
{
    std::string devPresentPath;
    std::string devPwrGoodPath;

    static std::unordered_map<std::string, bool> isErrorPower;

    for (auto config : configs)
    {
        NVMeData nvmeData;
        devPresentPath =
            GPIO_BASE_PATH + std::to_string(config.presentPin) + "/value";

        devPwrGoodPath =
            GPIO_BASE_PATH + std::to_string(config.pwrGoodPin) + "/value";

        auto iter = nvmes.find(config.index);

        if (getGPIOValueOfNvme(devPresentPath) == IS_PRESENT)
        {
            // Drive status is good, update value or create d-bus and update
            // value.
            if (getGPIOValueOfNvme(devPwrGoodPath) == POWERGD)
            {
                // get NVMe information through i2c by busID.
                getNVMeInfobyBusID(config.busID, nvmeData);
                // can not find. create dbus
                if (iter == nvmes.end())
                {
                    log<level::INFO>("SSD plug.",
                                     entry("index = %s", config.index.c_str()));

                    std::string objPath = NVME_OBJ_PATH + config.index;
                    auto nvmeSSD = std::make_shared<phosphor::nvme::NvmeSSD>(
                        bus, objPath.c_str());
                    nvmes.emplace(config.index, nvmeSSD);

                    nvmeSSD->setSensorValueToDbus(nvmeData.sensorValue);
                    nvmeSSD->setSensorThreshold(
                        config.criticalHigh, config.criticalLow,
                        config.maxValue, config.minValue, config.warningHigh,
                        config.warningLow);

                    nvmeSSD->checkSensorThreshold();
                }
                else
                {
                    iter->second->setSensorValueToDbus(nvmeData.sensorValue);
                    iter->second->checkSensorThreshold();
                }
                isErrorPower[config.index] = false;
            }
            else
            {
                // Present pin is true but power good pin is false
                // remove nvme d-bus path
                nvmeData = NVMeData();
                nvmes.erase(config.index);

                if (isErrorPower[config.index] != true)
                {
                    log<level::ERR>(
                        "Present pin is true but power good pin is false.",
                        entry("index = %s", config.index.c_str()));
                    log<level::ERR>("Erase SSD from map and d-bus.",
                                    entry("index = %s", config.index.c_str()));

                    isErrorPower[config.index] = true;
                }
            }
        }
        else
        {
            // Drive not present, remove nvme d-bus path
            nvmeData = NVMeData();
            nvmes.erase(config.index);
        }
    }
}
} // namespace nvme
} // namespace phosphor
