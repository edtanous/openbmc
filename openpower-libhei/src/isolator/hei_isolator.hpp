#pragma once

#include <hei_includes.hpp>
#include <hei_isolation_data.hpp>
#include <isolator/hei_isolation_node.hpp>

namespace libhei
{

/**
 * @brief This class is a complement to the main APIs. Its purpose is to store
 *        and maintain all of the objects necessary for isolation.
 *
 * The intended flow is to:
 *  - Create a singleton instance of an Isolator object via getSingleton().
 *  - Use initialize() to input each necessary Chip Data File provided by the
 *    user application.
 *  - Call isolate() to find all active errors being reported by the given list
 *    of chips.
 *  - Once isolation is no longer needed, use uninitialize() to free up
 *    internal resources.
 *
 * The purpose of the singleton instance is to avoid initializing the object
 * each time isolation is required. The data provided by the Chip Data Files is
 * static. So reinitializing would be a waste of time, unless for some reason
 * the Chip Data Files themselves are updated, which would require
 * reinitialization anyway. Of course, leaving the object in memory chews up
 * resources. So, some users may need to weigh performance vs. memory usage.
 */
class Isolator
{
  private: // This class cannot be instantiated. Use getSingleton() instead.
    /** @brief Default constructor. */
    Isolator() = default;

    /** @brief Destructor. */
    ~Isolator()
    {
        // Clear out all of the internal isolation objects.
        uninitialize();
    }

    /** @brief Copy constructor. */
    Isolator(const Isolator&) = delete;

    /** @brief Assignment operator. */
    Isolator& operator=(const Isolator&) = delete;

  public:
    /** @brief Provides access to a singleton instance of this object. */
    static Isolator& getSingleton()
    {
        static Isolator theIsolator;
        return theIsolator;
    }

    /** @brief See API wrapper description in hei_main.hpp. */
    void initialize(void* i_buffer, size_t i_bufferSize);

    /**
     * @brief See API wrapper description in hei_main.hpp.
     *
     * This function is called in the destructor. Therefore, it should never
     * throw an exception.
     */
    void uninitialize();

    /** @brief See API wrapper description in hei_main.hpp. */
    void isolate(const std::vector<Chip>& i_chipList,
                 IsolationData& o_isoData) const;

}; // end class Isolator

} // end namespace libhei
