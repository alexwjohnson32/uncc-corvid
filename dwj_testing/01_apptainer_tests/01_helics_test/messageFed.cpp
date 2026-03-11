#include "helics/MessageFederates.hpp"
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char* argv[])
{
    helics::FederateInfo fi;
    fi.loadInfoFromArgs(argc, argv);
    
    if (fi.defName.empty()) {
        fi.defName = "fed";
    }

    // Use a numeric property ID to avoid macro name issues
    fi.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, 5);

    auto mFed = std::make_unique<helics::MessageFederate>(fi.defName, fi);
    
    // Using a global endpoint so federates can find each other easily
    auto& ept = mFed->registerGlobalEndpoint(mFed->getName() + "/endpoint");

    std::cout << "Federate " << mFed->getName() << " entering Execution Mode..." << std::endl;
    mFed->enterExecutingMode();

    // Logic to determine who to talk to
    std::string target = (mFed->getName() == "fed1") ? "fed2/endpoint" : "fed1/endpoint";

    for (int i = 1; i < 5; ++i) {
        std::string payload = "Message from " + mFed->getName() + " count " + std::to_string(i);
        
        // HELICS 3: Use the endpoint object directly. 
        // We use .data() and .size() to match the 'const char*, size_t' signature
        ept.sendTo(payload, target);
        
        std::cout << "Sent to " << target << ": " << payload << std::endl;

        auto newTime = mFed->requestTime(static_cast<double>(i));

        while (ept.hasMessage()) {
            auto nmessage = ept.getMessage();
            if (nmessage) {
                std::cout << "Received from " << nmessage->source 
                          << ": " << nmessage->data.to_string() << std::endl;
            }
        }
    }

    mFed->finalize();
    return 0;
}