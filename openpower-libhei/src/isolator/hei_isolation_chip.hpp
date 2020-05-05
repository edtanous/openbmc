#pragma once

#include <hei_isolation_data.hpp>
#include <isolator/hei_isolation_node.hpp>

namespace libhei
{

/**
 * @brief This class contains all information required to isolate attentions on
 *        a specific chip type.
 *
 * The primary function of this class is analyze(), which will do a depth-first
 * search of the hardware attention reporting tree structure (characterized by
 * IsolationNode objects) to find all active attentions on a chip.
 *
 * The data for each chip type is built from information in the Chip Data Files.
 *
 *  IMPORTANT:
 *   Most of the objects created and referenced by this class are stored in the
 *   flyweight factories, which are used as a space saving mechanism by
 *   preventing duplicate object creation. For example, a large set of the
 *   registers for a specific chip model will likely be common between each of
 *   chip level of that model. It is not necessary to create new objects for
 *   each level when the attributes are all the same. Because the flyweights are
 *   used, it is not possible to remove all of the data for a specific chip
 *   model/level be deleting an IsolationChip object. Instead, you must call the
 *   main uninitialize() function and clear all data for all objects.
 */
class IsolationChip
{
  public: // Constructors, destructor, assignment
    /** @brief Default constructor. */
    IsolationChip(ChipType_t i_chipType, const RootNodeMap& i_rootNodes) :
        iv_chipType(i_chipType), iv_rootNodes(i_rootNodes)
    {}

    /** @brief Destructor. */
    ~IsolationChip() = default;

    /** @brief Copy constructor. */
    IsolationChip(const IsolationChip&) = delete;

    /** @brief Assignment operator. */
    IsolationChip& operator=(const IsolationChip&) = delete;

  private: // Instance variables
    /** This chip's type. */
    const ChipType_t iv_chipType;

    /** Root nodes for this chip type. */
    const RootNodeMap iv_rootNodes;

  public: // Member functions
    /**
     * @brief  Finds all active attentions on this chip.
     * @param  i_chip     The target chip for isolation. Will assert the given
     *                    chip type matches iv_chipType.
     * @param  io_isoData The isolation data returned back to the user
     *                    application.
     * @return True, if any active attentions found on this chip.
     *         False, otherwise.
     */
    bool analyze(const Chip& i_chip, IsolationData& io_isoData) const;

    /** @brief Chip type accessor. */
    ChipType_t getChipType() const
    {
        return iv_chipType;
    }
};

} // end namespace libhei
