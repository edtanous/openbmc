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
void parseConfig(char** i_begin, char** i_end, bool& o_vital, bool& o_checkstop,
                 bool& o_terminate, bool& o_breakpoints)
{
    char* setting;

    // --all on/off takes precedence over individual settings
    setting = getCliSetting(i_begin, i_end, "--all");
    if (nullptr != setting)
    {
        if (std::string("off") == setting)
        {
            o_vital       = false;
            o_checkstop   = false;
            o_terminate   = false;
            o_breakpoints = false;
        }

        if (std::string("on") == setting)
        {
            o_vital       = true;
            o_checkstop   = true;
            o_terminate   = true;
            o_breakpoints = true;
        }
    }
    // Parse individual options
    else
    {
        setting = getCliSetting(i_begin, i_end, "--vital");
        if (std::string("off") == setting)
        {
            o_vital = false;
        }
        if (std::string("on") == setting)
        {
            o_vital = true;
        }

        setting = getCliSetting(i_begin, i_end, "--checkstop");
        if (std::string("off") == setting)
        {
            o_checkstop = false;
        }
        if (std::string("on") == setting)
        {
            o_checkstop = true;
        }

        setting = getCliSetting(i_begin, i_end, "--terminate");
        if (std::string("off") == setting)
        {
            o_terminate = false;
        }
        if (std::string("on") == setting)
        {
            o_terminate = true;
        }

        setting = getCliSetting(i_begin, i_end, "--breakpoints");
        if (std::string("off") == setting)
        {
            o_breakpoints = false;
        }
        if (std::string("on") == setting)
        {
            o_breakpoints = true;
        }
    }
}
