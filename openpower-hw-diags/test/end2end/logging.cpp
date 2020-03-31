#include <attn/logging.hpp>

#include <iostream>

namespace attn
{

/** @brief Log message of type INFO using stdout */
template <>
void log<INFO>(const char* i_message)
{
    std::cout << i_message << std::endl;
}

} // namespace attn
