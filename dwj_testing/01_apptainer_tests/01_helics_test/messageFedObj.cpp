#include "helics/MessageFederates.hpp"
#include "helics/application_api/BrokerApp.hpp"
#include "helics/core/helicsCLI11.hpp"
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char* argv[])
{
    helics::helicsCLI11App app("Message Fed Obj", "MessageFedObj");
    std::string targetFederate = "fed";
    std::string targetEndpoint = "endpoint";
    std::string myendpoint = "endpoint";

    app.add_option("--target,-t", targetFederate, "name of the target federate");
    app.add_option("--endpoint,-e", targetEndpoint, "name of the target endpoint");
    app.add_option("--source,-s", myendpoint, "name of the source endpoint");

    auto ret = app.helics_parse(argc, argv);
    
    // Simple check: If ret is less than 0, it was a help call or an error.
    if (static_cast<int>(ret) < 0) {
        return 0; 
    }

    helics::FederateInfo fi;
    fi.loadInfoFromArgs(app.remainArgs());
    
    // Using numeric ID for log level to ensure 2.x and 3.x compatibility
    fi.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, 5);

    auto mFed = std::make_unique<helics::MessageFederate>(fi.defName, fi);
    
    // Object-oriented endpoint registration
    helics::Endpoint endpoint(mFed.get(), myendpoint);

    std::cout << "Federate " << mFed->getName() << " entering Execution Mode..." << std::endl;
    mFed->enterExecutingMode();

    std::string target = targetFederate + "/" + targetEndpoint;

    for (int i = 1; i < 5; ++i) {
        std::string payload = "Message from " + mFed->getName() + " count " + std::to_string(i);
        endpoint.sendTo(payload, target);
        
        std::cout << "Sent: " << payload << std::endl;
        auto newTime = mFed->requestTime(static_cast<double>(i));

        while (endpoint.hasMessage()) {
            auto nmessage = endpoint.getMessage();
            if (nmessage) {
                std::cout << "Received: " << nmessage->data.to_string() << std::endl;
            }
        }
    }

    mFed->finalize();
    return 0;
}