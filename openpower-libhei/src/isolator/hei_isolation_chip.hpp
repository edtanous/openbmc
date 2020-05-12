#pragma once

#include <hei_isolation_data.hpp>
#include <isolator/hei_isolation_node.hpp>
#include <register/hei_hardware_register.hpp>

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
    /** @brief Constructor. */
    explicit IsolationChip(ChipType_t i_chipType) : iv_chipType(i_chipType) {}

    /** @brief Destructor. */
    ~IsolationChip() = default;

    /** @brief Copy constructor. */
    IsolationChip(const IsolationChip&) = delete;

    /** @brief Assignment operator. */
    IsolationChip& operator=(const IsolationChip&) = delete;

  private: // Instance variables
    /** This chip's type. */
    const ChipType_t iv_chipType;

    /** All hardware registers for this chip type. */
    using RegisterKey_t = std::pair<RegisterId_t, Instance_t>;
    std::map<RegisterKey_t, const HardwareRegisterPtr> iv_regs;

    /** All isolation nodes for this chip type. */
    using NodeKey_t = std::pair<NodeId_t, Instance_t>;
    std::map<NodeKey_t, const IsolationNodePtr> iv_nodes;

    /** Root nodes for this chip type. */
    std::map<AttentionType_t, const IsolationNodePtr> iv_rootNodes;

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

    /**
     * @brief Adds a hardware register to this chip.
     * @param i_hwReg The target hardware register. Will assert that a different
     *                register with the same ID and instance does not exist.
     */
    void addHardwareRegister(HardwareRegisterPtr i_hwReg);

    /**
     * @brief Adds an isolation node to this chip.
     * @param i_isoNode The target isolation node. Will assert that a different
     *                  node with the same ID and instance does not exist.
     */
    void addIsolationNode(IsolationNodePtr i_isoNode);

    /**
     * @brief Adds a root node to this chip.
     * @param i_attnType The target attention type. Will assert this type does
     *                   not already exist in iv_rootNodes.
     * @param i_rootNode The target isolation node for this attention type.
     */
    void addRootNode(AttentionType_t i_attnType, IsolationNodePtr i_rootNode);

    /**
     * @brief  Retrieves a hardware register from this chip, if it exists.
     * @param  i_regId   The register ID.
     * @param  i_regInst The register instance.
     * @return The target hardware register. Will assert that the target
     *         register exists in iv_regs.
     */
    HardwareRegisterPtr getHardwareRegister(RegisterId_t i_regId,
                                            Instance_t i_regInst) const;

    /**
     * @brief  Retrieves an isolation node from this chip, if it exists.
     * @param  i_nodeId   The node ID.
     * @param  i_nodeInst The node instance.
     * @return The target isolation node. Will assert that the target node
     *         exists in iv_nodes.
     */
    IsolationNodePtr getIsolationNode(NodeId_t i_nodeId,
                                      Instance_t i_nodeInst) const;
};

/** Pointer management for isolation chips. */
using IsolationChipPtr = std::unique_ptr<IsolationChip>;

/** A simple map to ensure only one IsolationChip exists per chip type. */
using IsolationChipMap =
    std::map<ChipType_t, const std::unique_ptr<const IsolationChip>>;

} // end namespace libhei
