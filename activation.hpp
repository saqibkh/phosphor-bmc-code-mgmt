#pragma once

#include <sdbusplus/server.hpp>
#include <xyz/openbmc_project/Software/Activation/server.hpp>
#include <xyz/openbmc_project/Software/ActivationBlocksTransition/server.hpp>
#include "xyz/openbmc_project/Software/RedundancyPriority/server.hpp"

namespace phosphor
{
namespace software
{
namespace updater
{

using ActivationInherit = sdbusplus::server::object::object<
    sdbusplus::xyz::openbmc_project::Software::server::Activation>;
using ActivationBlocksTransitionInherit = sdbusplus::server::object::object<
 sdbusplus::xyz::openbmc_project::Software::server::ActivationBlocksTransition>;
using RedundancyPriorityInherit = sdbusplus::server::object::object<
    sdbusplus::xyz::openbmc_project::Software::server::RedundancyPriority>;
    
/** @class ActivationBlocksTransition
 *  @brief OpenBMC ActivationBlocksTransition implementation.
 *  @details A concrete implementation for
 *  xyz.openbmc_project.Software.ActivationBlocksTransition DBus API.
 */
class ActivationBlocksTransition : public ActivationBlocksTransitionInherit
{
    public:
        /** @brief Constructs ActivationBlocksTransition.
         *
         *  @param[in] bus    - The Dbus bus object
         *  @param[in] path   - The Dbus object path
         */
         ActivationBlocksTransition(sdbusplus::bus::bus& bus,
                                   const std::string& path) :
                   ActivationBlocksTransitionInherit(bus, path.c_str()) {}
};

/** @class Activation
 *  @brief OpenBMC activation software management implementation.
 *  @details A concrete implementation for
 *  xyz.openbmc_project.Software.Activation DBus API.
 */
class RedundancyPriority : public RedundancyPriorityInherit
{
    public:
        /** @brief Constructs RedundancyPriority.
         *
         *  @param[in] bus    - The Dbus bus object
         *  @param[in] path   - The Dbus object path
         */
        RedundancyPriority(sdbusplus::bus::bus& bus,
                                   const std::string& path) :
                                   RedundancyPriorityInherit(bus,
                                   path.c_str(), true),
                                   bus(bus)
        {
            // Set Property
            priority(0);
            // Emit deferred signal.
            emit_object_added();
        }

        /** @brief Overloaded priority property setter function
         *
         *  @param[in] value - One of RedundancyPriority::Priority
         *
         *  @return Success or exception thrown
         */
        uint8_t priority(uint8_t value) override;

        /** @brief Persistent sdbusplus DBus bus connection */
        sdbusplus::bus::bus& bus;

        void freePriority(uint8_t basePriority);
};
    
/** @class Activation
 *  @brief OpenBMC activation software management implementation.
 *  @details A concrete implementation for
 *  xyz.openbmc_project.Software.Activation DBus API.
 */
class Activation : public ActivationInherit
{
    public:
        /** @brief Constructs Activation Software Manager
         *
         * @param[in] bus    - The Dbus bus object
         * @param[in] path   - The Dbus object path
         * @param[in] versionId  - The software version id
         * @param[in] activationStatus - The status of Activation
         */
        Activation(sdbusplus::bus::bus& bus, const std::string& path,
                   std::string& versionId,
                   sdbusplus::xyz::openbmc_project::Software::
                   server::Activation::Activations activationStatus) :
                   ActivationInherit(bus, path.c_str(), true),
                   bus(bus),
                   path(path),
                   versionId(versionId)
        {
            // Set Properties.
            activation(activationStatus);
            // Emit deferred signal.
            emit_object_added();
        }

        /** @brief Overloaded Activation property setter function
         *
         * @param[in] value - One of Activation::Activations
         *
         * @return Success or exception thrown
         */
        Activations activation(Activations value) override;

        /** @brief Overloaded requestedActivation property setter function
         *
         * @param[in] value - One of Activation::RequestedActivations
         *
         * @return Success or exception thrown
         */
        RequestedActivations requestedActivation(RequestedActivations value)
                override;

        /** @brief Persistent sdbusplus DBus bus connection */
        sdbusplus::bus::bus& bus;

        /** @brief Persistent DBus object path */
        std::string path;

        /** @brief Version id */
        std::string versionId;

        /** @brief Persistent ActivationBlocksTransition dbus object */
        std::unique_ptr<ActivationBlocksTransition> activationBlocksTransition;
        
        /** @brief Persistent RedundancyPriority dbus object */
        std::unique_ptr<RedundancyPriority> redundancyPriority;

};

} // namespace updater
} // namespace software
} // namespace phosphor
