#pragma once

#include <attn/attn_config.hpp>

#include <string>

/*
 * @brief Search the command line arguments for an option
 *
 * @param i_begin   command line args vector begin
 * @param i_end     command line args vector end
 * @param i_option  configuration option to look for
 *
 * @return true = option found on command line
 */
bool getCliOption(char** i_begin, char** i_end, const std::string& i_option);

/*
 * @brief Search the command line arguments for a setting value
 *
 * @param i_begin   command line args vector begin
 * @param i_end     command line args vectory end
 * @param i_setting configuration setting to look for
 *
 * @return value of the setting or 0 if setting not found or value not given
 */
char* getCliSetting(char** i_begin, char** i_end, const std::string& i_setting);

/*
 *
 * @brief Get configuration flags from command line
 *
 * Parse the command line for configuration options and update the
 * attention handler configuration object as needed.
 *
 * @param i_begin       command line args vector begin
 * @param i_end         command line args vector end
 * @param o_config      pointer to attention handler configuration object
 */
void parseConfig(char** i_begin, char** i_end, attn::Config* o_config);
