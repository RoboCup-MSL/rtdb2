#include <iostream>
#include "RtDB2Configuration.h"

void print_comm_settings(RtDB2Configuration &config)
{
    CommunicationSettings cs = config.get_communication_settings();
    std::cout << "\tmultiCastIP = " << cs.multiCastIP << std::endl;
    std::cout << "\tfrequency   = " << cs.frequency << std::endl;
    std::cout << "\tport        = " << cs.port << std::endl;
    std::cout << "\tloopback    = " << cs.loopback << std::endl;
    std::cout << "\tsend        = " << cs.send << std::endl;
}

int main(int argc, char **argv)
{
    std::cout << "=== Default configuration ===" << std::endl;
    RtDB2Context defaultContext = RtDB2Context::Builder().build();
    RtDB2Configuration config1(defaultContext);
    print_comm_settings(config1);
    std::cout << config1 << std::endl;

    for (int i = 1; i < argc; i++)
    {
        std::string configFile = argv[i];
        std::cout << "=== " << configFile << " ===" << std::endl;
        RtDB2Context context = RtDB2Context::Builder(RtDB2ProcessType::comm)
                                   .withConfigFileName(configFile)
                                   .build();
        RtDB2Configuration config(context);
        config.load_configuration();
        print_comm_settings(config);
        std::cout << config << std::endl;
    }

    // done
    return 0;
}
