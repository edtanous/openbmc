#pragma once

/**
 * @brief Start a thread to listen for attention handler messages
 *
 * @param i_params command line arguments passed to main
 */
void* threadListener(void* i_params);

/**
 * @brief Send command line to a thread
 *
 * @param i_argc command line arguments count
 * @param i_argv command line arguments
 *
 * @return number of cmd line arguments sent
 */
int sendCmdLine(int i_argc, char** i_argv);

/**
 * @brief See if the listener thread message queue exists
 *
 *  @return true if message queue exists, else false
 */
bool listenerMqExists();
