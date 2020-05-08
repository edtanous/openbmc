#include <attn/attn_logging.hpp>

#include <iostream>

namespace attn
{

/** @brief Log message of type INFO using stdout */
template <>
void trace<INFO>(const char* i_message)
{
    std::cout << "trace: " << i_message << std::endl;
}

void eventCheckstop(std::map<std::string, std::string>& i_errors)
{
    std::string signature = i_errors.begin()->first;
    std::string chip      = i_errors.begin()->second;

    std::cout << "event: checkstop, signature = " << signature
              << ", chip = " << chip << std::endl;
}

void eventHwDiagsFail(int i_error)
{
    std::cout << "event: hwdiags fail  " << i_error << std::endl;
}

void eventAttentionFail(int i_error)
{
    std::cout << "event: attention fail" << i_error << std::endl;
}

void eventTerminate()
{
    std::cout << "event: terminate" << std::endl;
}
void eventVital()
{
    std::cout << "event: vital" << std::endl;
}

} // namespace attn
