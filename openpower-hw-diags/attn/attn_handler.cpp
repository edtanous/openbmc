#include <libpdbg.h>

#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>

#include <iomanip>

using namespace phosphor::logging;

namespace attn
{

/**
 * @brief Handle SBE vital attention
 *
 * @return 0 = success
 */
int handleVital();

/**
 * @brief Handle checkstop attention
 *
 * @return 0 = success
 */
int handleCheckstop();

/**
 * @brief Handle special attention
 *
 * @return 0 = success
 */
int handleSpecial();

/**
 * @brief Notify Cronus over dbus interface
 *
 * @param i_proc   Processor number with Special attention
 * @param i_core   Core number with special attention
 * @param i_thread Thread number with special attention
 */
void notifyCronus(uint32_t i_proc, uint32_t i_core, uint32_t i_thread);

/**
 * @brief Start host diagnostic mode
 *
 * Start the host diagnostic mode systemd unit
 */
void startHostDiagnosticMode();

/**
 * @brief The main attention handler logic
 */
void attnHandler()
{
    uint32_t isr_val, isr_mask;
    uint32_t proc;

    // loop through processors looking for active attentions
    pdbg_target* target;
    pdbg_for_each_class_target("fsi", target)
    {
        if (PDBG_TARGET_ENABLED == pdbg_target_probe(target))
        {
            proc = pdbg_target_index(target); // get processor number

            std::stringstream ss; // log message stream
            ss << "[ATTN] checking processor " << proc << std::endl;
            log<level::INFO>(ss.str().c_str());

            // get active attentions on processor
            if (0 != fsi_read(target, 0x1007, &isr_val))
            {
                std::stringstream ss; // log message stream
                ss << "[ATTN] Error! cfam read 0x1007 FAILED" << std::endl;
                log<level::INFO>(ss.str().c_str());
            }
            else
            {
                std::stringstream ss; // log message stream
                ss << "[ATTN] cfam 0x1007 = 0x";
                ss << std::hex << std::setw(8) << std::setfill('0');
                ss << isr_val << std::endl;
                log<level::INFO>(ss.str().c_str());

                // get interrupt enabled special attentions mask
                if (0 != fsi_read(target, 0x100d, &isr_mask))
                {
                    std::stringstream ss; // log message stream
                    ss << "[ATTN] Error! cfam read 0x100d FAILED" << std::endl;
                    log<level::INFO>(ss.str().c_str());
                }
                else
                {
                    std::stringstream ss; // log message stream
                    ss << "[ATTN] cfam 0x100d = 0x";
                    ss << std::hex << std::setw(8) << std::setfill('0');
                    ss << isr_mask << std::endl;
                    log<level::INFO>(ss.str().c_str());

                    // bit 0 on "left": bit 30 = SBE vital attention
                    if (isr_val & isr_mask & 0x00000002)
                    {
                        handleVital();
                    }

                    // bit 0 on "left": bit 1 = checkstop
                    if (isr_val & isr_mask & 0x40000000)
                    {
                        handleCheckstop();
                    }

                    // bit 0 on "left": bit 2 = special attention
                    if (isr_val & isr_mask & 0x20000000)
                    {
                        handleSpecial();
                    }
                } // cfam 0x100d valid
            }     // cfam 0x1007 valid
        }         // fsi target enabled
    }             // next processor

    return; // checked all processors
}

/**
 * @brief Handle SBE vital attention
 */
int handleVital()
{
    int rc = 1; // vital attention handling not yet supported

    std::stringstream ss; // log message stream
    ss << "[ATTN] vital" << std::endl;
    log<level::INFO>(ss.str().c_str());

    if (0 != rc)
    {
        std::stringstream ss; // log message stream
        ss << "[ATTN] vital NOT handled" << std::endl;
        log<level::INFO>(ss.str().c_str());
    }

    return rc;
}

/**
 * @brief Handle checkstop attention
 */
int handleCheckstop()
{
    int rc = 1; // checkstop handling not yet supported

    std::stringstream ss; // log message stream
    ss << "[ATTN] checkstop" << std::endl;
    log<level::INFO>(ss.str().c_str());

    if (0 != rc)
    {
        std::stringstream ss; // log message stream
        ss << "[ATTN] checkstop NOT handled" << std::endl;
        log<level::INFO>(ss.str().c_str());
    }

    // TODO recoverable errors?

    return rc;
}

/**
 * @brief Handle special attention
 */
int handleSpecial()
{
    int rc = 0; // special attention handling supported

    std::stringstream ss; // log message stream

    ss << "[ATTN] special" << std::endl;

    // Currently we are only handling Cronus breakpoints
    // ss << "[ATTN] breakpoint" << std::endl;
    // log<level::INFO>(ss.str().c_str());

    // Cronus will determine proc, core and thread so just notify
    // notifyCronus(0, 0, 0); // proc-0, core-0, thread-0

    // For TI special attention start host diagnostic mode
    ss << "[ATTN] TI (terminate immediately)" << std::endl;
    log<level::INFO>(ss.str().c_str());
    startHostDiagnosticMode();

    // TODO recoverable errors?

    return rc;
}

/**
 * @brief Notify Cronus over dbus interface
 */
void notifyCronus(uint32_t i_proc, uint32_t i_core, uint32_t i_thread)
{
    std::stringstream ss; // log message stream

    // log status info
    ss << "[ATTN] notify ";
    ss << i_proc << ", " << i_core << ", " << i_thread << std::endl;
    log<level::INFO>(ss.str().c_str());

    // notify Cronus over dbus
    auto bus = sdbusplus::bus::new_system();
    auto msg = bus.new_signal("/", "org.openbmc.cronus", "Brkpt");

    std::array<uint32_t, 3> params{i_proc, i_core, i_thread};
    msg.append(params);

    msg.signal_send();

    return;
}

/**
 * @brief Start host diagnostic mode
 * */
void startHostDiagnosticMode()
{
    std::stringstream ss; // log message stream

    // log status info
    ss << "[ATTN] start host diagnostic mode service" << std::endl;
    log<level::INFO>(ss.str().c_str());

    // Use the systemd service manager object interface to call the start unit
    // method with the obmc-host-diagnostic-mode target.
    auto bus    = sdbusplus::bus::new_system();
    auto method = bus.new_method_call(
        "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager", "StartUnit");

    method.append("obmc-host-diagnostic-mode@0.target"); // unit to activate
    method.append("replace"); // mode = replace conflicting queued jobs
    bus.call_noreply(method); // start the service

    return;
}

} // namespace attn
