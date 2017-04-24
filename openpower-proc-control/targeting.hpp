#pragma once

#include <memory>
#include <vector>
#include "filedescriptor.hpp"

namespace openpower
{
namespace targeting
{

constexpr auto fsiMasterDevPath =
    "/sys/devices/platform/gpio-fsi/fsi0/slave@00:00/raw";
constexpr auto fsiMasterDevPathOld =
    "/sys/devices/platform/fsi-master/slave@00:00/raw";

constexpr auto fsiSlaveBaseDir =
    "/sys/devices/platform/gpio-fsi/fsi0/slave@00:00/00:00:00:0a/";
constexpr auto fsiSlaveBaseDirOld =
    "/sys/devices/platform/fsi-master/slave@00:00/hub@00/";

typedef uint32_t (*swap_endian_t)(uint32_t);

/**
 * Represents a specific P9 processor in the system.  Used by
 * the access APIs to specify the chip to operate on.
 */
class Target
{
    public:

        /**
         * Constructor
         *
         * @param[in] - The logical position of the target
         * @param[in] - The sysfs device path
         * @param[in] - The function pointer for swapping endianness
         */
        Target(size_t position, const std::string& devPath,
               const swap_endian_t swapper) :
            pos(position), cfamPath(devPath), doSwapEndian(swapper)
        {
        }

        Target() = delete;
        ~Target() = default;
        Target(const Target&) = default;
        Target(Target&&) = default;
        Target& operator=(Target&&) = default;

        /**
         * Returns the position
         */
        inline auto getPos() const
        {
            return pos;
        }

        /**
         * Returns the CFAM sysfs path
         */
        inline auto getCFAMPath() const
        {
            return cfamPath;
        }

        /**
         * Returns the file descriptor to use
         * for read/writeCFAM operations.
         */
        int getCFAMFD();

        /**
         * Returns correct byte-order data. (May or may not swap it depending
         * on the function received during construction from Targeting and the
         * host endianness).
         */
        inline uint32_t swapEndian(uint32_t data) const
        {
            return doSwapEndian(data);
        }

    private:

        /**
         * The logical position of this target
         */
        size_t pos;

        /**
         * The sysfs device path for the CFAM
         */
        const std::string cfamPath;

        /**
         * The file descriptor to use for read/writeCFAMReg
         */
        std::unique_ptr<openpower::util::FileDescriptor> cfamFD;

        /**
         * The function pointer for swapping endianness
         */
        const swap_endian_t doSwapEndian;
};


/**
 * Class that manages processor targeting for FSI operations.
 */
class Targeting
{
    public:

        /**
         * Scans sysfs to find all processors and creates Target objects
         * for them.
         * @param[in] fsiMasterDev - the sysfs device for the master
         * @param[in] fsiSlaveDirectory - the base sysfs dir for slaves
         */
        Targeting(const std::string& fsiMasterDev,
                  const std::string& fsiSlaveDir);

        Targeting() : Targeting(fsiMasterDevPath, fsiSlaveBaseDir) {}

        ~Targeting() = default;
        Targeting(const Targeting&) = default;
        Targeting(Targeting&&) = default;
        Targeting& operator=(Targeting&&) = default;

        /**
         * Returns a const iterator to the first target
         */
        inline auto begin()
        {
            return targets.cbegin();
        }

        /**
         * Returns a const iterator to the last (highest position) target.
         */
        inline auto end()
        {
            return targets.cend();
        }

        /**
         * Returns the number of targets
         */
        inline auto size()
        {
            return targets.size();
        }

        /**
         * Returns a target by position.
         */
        std::unique_ptr<Target>& getTarget(size_t pos);

    private:

        /**
         * The path to the fsi-master sysfs device to access
         */
        std::string fsiMasterPath;

        /**
         * The path to the fsi slave sysfs base directory
         */
        std::string fsiSlaveBasePath;

        /**
         * A container of Targets in the system
         */
        std::vector<std::unique_ptr<Target>> targets;
};

}
}
