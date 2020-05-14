#include <libpdbg.h>

#include <attn/attn_main.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <cli.hpp>
#include <listener.hpp>

/** @brief openpower-hw-diags message queue name */
static constexpr const char* mq_listener = "openpower-hw-diags-mq";

/** @brief maximum length of command line parameter */
static constexpr int max_command_len = 25;

/** @brief end of command line args message */
static const char* msg_send_end = "999999999999999";

/** @brief structure for holding main args (for threads) */
typedef struct
{
    int argc;
    char** argv;
} MainArgs_t;

/**
 * @brief Start a thread to monitor the attention GPIO
 *
 * @param i_config Attention handler configuration object
 */
void* threadGpioMon(void* i_config)
{
    // Configure and start attention monitor
    attn::attnDaemon((attn::Config*)i_config);

    pthread_exit(NULL);
}

/** @brief Start a thread to listen for attention handler messages */
void* threadListener(void* i_params)
{
    using namespace boost::interprocess;

    // remove listener message queue if exists (does not throw)
    message_queue::remove(mq_listener);

    // thread handle for gpio monitor
    pthread_t ptidGpio;

    // status of gpio monitor
    bool gpioMonEnabled = false;

    // create config
    attn::Config attnConfig;

    // This is the main listener loop. All the above code will be executed
    // only once. All other communtication with the attention handler will
    // originate from here via the message queue.
    do
    {
        // vector to hold messages sent to listener
        std::vector<std::string> messages;
        // we will catch any exceptions from thread library
        try
        {
            // create new message queue or open existing
            message_queue mq(open_or_create, mq_listener, 1, max_command_len);

            // message queue parameters
            char buffer[max_command_len + 1];
            size_t recvd_size;
            unsigned int priority;

            // We will continue receiving messages until we receive
            // a msg_send_end message to indicate all command line parameters
            // have been sent.
            do
            {
                // wait for a message to arrive
                mq.receive((void*)&buffer, max_command_len, recvd_size,
                           priority);

                // null terminate message and store
                buffer[recvd_size] = '\0';
                messages.push_back(buffer);

            } while (buffer != std::string(msg_send_end));

            messages.pop_back(); // remove msg_send_end message

            // convert messages to command line arguments
            std::vector<char*> argv;

            for (const auto& arg : messages)
            {
                argv.push_back((char*)arg.data());
            }

            int argc = argv.size();
            argv.push_back(nullptr);

            // stop attention handler daemon?
            if (true == getCliOption(argv.data(), argv.data() + argc, "--stop"))
            {
                message_queue::remove(mq_listener);
                break;
            }

            // parse config options
            parseConfig(argv.data(), argv.data() + argc, &attnConfig);

            // start attention handler daemon?
            if (true ==
                getCliOption(argv.data(), argv.data() + argc, "--start"))
            {
                if (false == gpioMonEnabled)
                {
                    if (0 == pthread_create(&ptidGpio, NULL, &threadGpioMon,
                                            &attnConfig))
                    {
                        gpioMonEnabled = true;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        catch (interprocess_exception& e)
        {
            break;
        }
    } while (1);

    // stop the gpio monitor if running
    if (true == gpioMonEnabled)
    {
        pthread_cancel(ptidGpio);
    }

    pthread_exit(NULL);
}

/** @brief Send command line to a threadi */
int sendCmdLine(int i_argc, char** i_argv)
{
    int count = 0; // number of arguments sent

    using namespace boost::interprocess;

    try
    {
        message_queue mq(open_only, mq_listener);

        while (count < i_argc)
        {
            mq.send(i_argv[count], strlen(i_argv[count]), 0);
            count++;
        }
        // indicate to listener last cmdline arg was sent
        mq.send(msg_send_end, strlen(msg_send_end), 0);
    }
    catch (interprocess_exception& e)
    {
        count = 0; // assume no arguments sent
    }
    return count;
}

/** @brief See if the listener thread message queue exists */
bool listenerMqExists()
{
    using namespace boost::interprocess;

    try
    {
        message_queue mq(open_only, mq_listener);
        return true;
    }
    catch (interprocess_exception& e)
    {
        return false;
    }
}
