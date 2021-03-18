#pragma once

#include <analyzer/service_data.hpp>

namespace analyzer
{

/** @brief An abstract class for service event resolutions. */
class Resolution
{
  public:
    /** @brief Pure virtual destructor. */
    virtual ~Resolution() = 0;

  public:
    /**
     * @brief Resolves the service actions required by this resolution.
     * @param io_sd An object containing the service data collected during
     *              hardware error analysis.
     */
    virtual void resolve(ServiceData& io_sd) const = 0;
};

// Pure virtual destructor must be defined.
inline Resolution::~Resolution() {}

/** @brief Resolves a hardware callout service event. */
class HardwareCalloutResolution : public Resolution
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_path     The devtree path of a guardable unit relative to a
     *                   chip. An empty string refers to the chip itself.
     * @param i_priority The callout priority.
     * @param i_guard    The guard type for this callout.
     */
    HardwareCalloutResolution(const std::string& i_path,
                              Callout::Priority i_priority,
                              Guard::Type i_guard) :
        iv_path(i_path),
        iv_priority(i_priority), iv_guard(i_guard)
    {}

  private:
    /** The devtree path of a guardable unit relative to a chip. An empty string
     *  refers to the chip itself. */
    const std::string iv_path;

    /** The callout priority. */
    const Callout::Priority iv_priority;

    /** The guard type for this callout. */
    const Guard::Type iv_guard;

  public:
    void resolve(ServiceData& io_sd) const override;
};

/** @brief Resolves a procedure callout service event. */
class ProcedureCalloutResolution : public Resolution
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_procedure The procedure callout type.
     * @param i_priority  The callout priority.
     */
    ProcedureCalloutResolution(ProcedureCallout::Type i_procedure,
                               Callout::Priority i_priority) :
        iv_procedure(i_procedure),
        iv_priority(i_priority)
    {}

  private:
    /** The procedure callout type. */
    const ProcedureCallout::Type iv_procedure;

    /** The callout priority. */
    const Callout::Priority iv_priority;

  public:
    void resolve(ServiceData& io_sd) const override
    {
        // Simply add the procedure to the callout list.
        io_sd.addCallout(
            std::make_shared<ProcedureCallout>(iv_procedure, iv_priority));
    }
};

/**
 * @brief Links two resolutions into a single resolution. The resolutions will
 *        will be resolved in the order they are inputted into the constructor.
 */
class LinkResolution : public Resolution
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_first  The first resolution.
     * @param i_second The second resolution.
     */
    LinkResolution(const std::shared_ptr<Resolution>& i_first,
                   const std::shared_ptr<Resolution>& i_second) :
        iv_first(i_first),
        iv_second(i_second)
    {}

  private:
    /** The first resolution. */
    const std::shared_ptr<Resolution> iv_first;

    /** The second resolution. */
    const std::shared_ptr<Resolution> iv_second;

  public:
    void resolve(ServiceData& io_sd) const override
    {
        iv_first->resolve(io_sd);
        iv_second->resolve(io_sd);
    }
};

} // namespace analyzer
