#include <stdint.h>

/**
 * @brief Get the current boot count for the host
 *
 * The boot count indicates how many more times the bmc will try to
 * boot the host.
 *
 * @return Number of boot attempts left
 **/
uint32_t getBootCount();
