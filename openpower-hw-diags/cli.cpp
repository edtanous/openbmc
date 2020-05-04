#include <attn/attn_config.hpp>

#include <algorithm>
#include <string>

/** @brief Search the command line arguments for an option */
bool getCliOption(char** i_begin, char** i_end, const std::string& i_option)
{
    return (i_end != std::find(i_begin, i_end, i_option));
}

/** @brief Search the command line arguments for a setting value */
char* getCliSetting(char** i_begin, char** i_end, const std::string& i_setting)
{
    char** value = std::find(i_begin, i_end, i_setting);
    return (value != i_end && ++value != i_end) ? *value : 0;
}

/** @brief Parse command line for configuration flags */
void parseConfig(char** i_begin, char** i_end, attn::Config* o_config)
{
    char* setting;

    // --all on/off takes precedence over individual settings
    setting = getCliSetting(i_begin, i_end, "--all");
    if (nullptr != setting)
    {
        if (std::string("off") == setting)
        {
            o_config->clearFlagAll();
        }

        if (std::string("on") == setting)
        {
            o_config->setFlagAll();
        }
    }
    // Parse individual options
    else
    {
        setting = getCliSetting(i_begin, i_end, "--vital");
        if (nullptr != setting)
        {
            if (std::string("off") == setting)
            {
                o_config->clearFlag(attn::enVital);
            }
            if (std::string("on") == setting)
            {
                o_config->setFlag(attn::enVital);
            }
        }

        setting = getCliSetting(i_begin, i_end, "--checkstop");
        if (nullptr != setting)
        {
            if (std::string("off") == setting)
            {
                o_config->clearFlag(attn::enCheckstop);
            }
            if (std::string("on") == setting)
            {
                o_config->setFlag(attn::enCheckstop);
            }
        }

        setting = getCliSetting(i_begin, i_end, "--terminate");
        if (nullptr != setting)
        {
            if (std::string("off") == setting)
            {
                o_config->clearFlag(attn::enTerminate);
            }
            if (std::string("on") == setting)
            {
                o_config->setFlag(attn::enTerminate);
            }
        }

        setting = getCliSetting(i_begin, i_end, "--breakpoints");
        if (nullptr != setting)
        {
            if (std::string("off") == setting)
            {
                o_config->clearFlag(attn::enBreakpoints);
            }
            if (std::string("on") == setting)
            {
                o_config->setFlag(attn::enBreakpoints);
            }
        }
    }
}
