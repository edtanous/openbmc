#include <libpdbg.h>

#include <attn/attention.hpp>
#include <attn/attn_config.hpp>
#include <attn/attn_handler.hpp>
#include <cli.hpp>
#include <util/trace.hpp>

#include <vector>

namespace attn
{
// these are in the attn_lib but not all exposed via headers
int handleSpecial(Attention* i_attention);
int handleCheckstop(Attention* i_attention);
int handleVital(Attention* i_attention);
} // namespace attn

/** @brief Attention handler test application */
int main(int argc, char* argv[])
{
    int rc = 0; // return code

    // initialize pdbg targets
    pdbg_targets_init(nullptr);

    // create attention handler config object
    attn::Config attnConfig;

    // convert cmd line args to config values
    parseConfig(argv, argv + argc, &attnConfig);

    // exercise attention gpio event path
    attn::attnHandler(&attnConfig);

    // Get first enabled proc for testing
    pdbg_target* target = nullptr;
    pdbg_for_each_class_target("proc", target)
    {
        trace::inf("proc: %u", pdbg_target_index(target));
        if (PDBG_TARGET_ENABLED == pdbg_target_probe(target))
        {
            trace::inf("target enabled");
            break;
        }
    }

    // Exercise special, checkstop and vital attention handler paths
    if ((nullptr != target) &&
        (PDBG_TARGET_ENABLED == pdbg_target_probe(target)))
    {
        std::vector<attn::Attention> attentions;

        attentions.emplace_back(attn::Attention::AttentionType::Special,
                                attn::handleSpecial, target, &attnConfig);

        attentions.emplace_back(attn::Attention::AttentionType::Checkstop,
                                attn::handleCheckstop, target, &attnConfig);

        attentions.emplace_back(attn::Attention::AttentionType::Vital,
                                attn::handleVital, target, &attnConfig);

        std::for_each(std::begin(attentions), std::end(attentions),
                      [](attn::Attention attention) {
                          trace::inf("calling handler");
                          attention.handle();
                      });
    }

    return rc;
}
