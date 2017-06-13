#include "activation.hpp"
#include <string>
#include <fstream>


namespace phosphor
{
namespace software
{
namespace updater
{

namespace softwareServer = sdbusplus::xyz::openbmc_project::Software::server;

auto Activation::activation(Activations value) ->
        Activations
{
    if (value != softwareServer::Activation::Activations::Active)
    {
        redundancyPriority.reset(nullptr);
    }

    if (value == softwareServer::Activation::Activations::Activating)
    {
        if (!activationBlocksTransition)
        {
            activationBlocksTransition =
                      std::make_unique<ActivationBlocksTransition>(
                                bus,
                                path);
        }

        // Add logic for BMC Code Update
        if(1)
        {
            // Set Redundancy Priority before setting to Active
            if (!redundancyPriority)
            {
                redundancyPriority =
                          std::make_unique<RedundancyPriority>(
                                    bus,
                                    path);
            }

            return softwareServer::Activation::activation(
                    softwareServer::Activation::Activations::Active);
        }
        else
        {
            return softwareServer::Activation::activation(
                    softwareServer::Activation::Activations::Failed);
        }
    }
    else
    {
        activationBlocksTransition.reset(nullptr);
    }
    return softwareServer::Activation::activation(value);
}

auto Activation::requestedActivation(RequestedActivations value) ->
        RequestedActivations
{
    if ((value == softwareServer::Activation::RequestedActivations::Active) &&
        (softwareServer::Activation::requestedActivation() !=
                  softwareServer::Activation::RequestedActivations::Active))
    {
        if ((softwareServer::Activation::activation() ==
                    softwareServer::Activation::Activations::Ready) ||
            (softwareServer::Activation::activation() ==
                    softwareServer::Activation::Activations::Failed))
        {
            Activation::activation(
                    softwareServer::Activation::Activations::Activating);

        }
    }
    return softwareServer::Activation::requestedActivation(value);
}

uint8_t RedundancyPriority::priority(uint8_t value)
{
    RedundancyPriority::freePriority(value);
    return softwareServer::RedundancyPriority::priority(value);
}

void RedundancyPriority::freePriority(uint8_t basePriority)
{
    std::ofstream file;
    std::string BMC_BUSNAME("xyz.openbmc_project.Software.BMC.Updater");
    std::string BMC_PATH("/xyz/openbmc_project/software");
    std::string OBJECT_MANAGER("org.freedesktop.DBus.ObjectManager");
    std::string REDUNDANCY_PRIORITY("xyz.openbmc_project.Software.RedundancyPriority");

    auto method = this->bus.new_method_call(BMC_BUSNAME.c_str(),
                                            BMC_PATH.c_str(),
                                            OBJECT_MANAGER.c_str(),
                                            "GetManagedObjects");
    auto reply = this->bus.call(method);
    if (reply.is_method_error())
    {
        file.open("/tmp/errorlog.txt", std::ofstream::app);
        file << "Error in GetManagedObjects\n";
        file.close();
        return;
    }

    std::map<sdbusplus::message::object_path, std::map<std::string,
        std::map<std::string, sdbusplus::message::variant<uint8_t>>>>
        m;
    reply.read(m);

    for (const auto& intf1 : m)
    {
        for (const auto& intf2 : intf1.second)
        {
            if (intf2.first == REDUNDANCY_PRIORITY.c_str())
            {
                for (const auto& property : intf2.second)
                {
                    if (property.first == "Priority")
                    {
                        uint8_t value = sdbusplus::message::variant_ns::get<
                            uint8_t>(property.second);
                        if(basePriority == value)
                        {
                            RedundancyPriority::freePriority(value + 1);
                            // Set priority to basePriority
                            method = this->bus.new_method_call(
                                            BMC_BUSNAME.c_str(),
                                            BMC_PATH.c_str(),
                                            OBJECT_MANAGER.c_str(),
                                            "Set");
                            method.append(REDUNDANCY_PRIORITY.c_str(),
                                          "Priority",
                                          basePriority + 1);
                            reply = this->bus.call(method);
                            if (reply.is_method_error())
                            {
                                file.open("/tmp/errorlog.txt", std::ofstream::app);
                                file << "Failed to Set RedundancyPriority";
                                file.close();
                            }
                            return;
                        }
                    }
                }
            }
        }
    }
    return;

}

} // namespace updater
} // namespace software
} // namespace phosphor

