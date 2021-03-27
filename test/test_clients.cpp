#include <libgen.h>
#include <iostream>
#include "RtDB2.h"
#include "../comm/comm.hpp"

#define CONFIGFILE "/config/test_clients.xml"
#define NETWORK_A "network-A"
#define NETWORK_B "network-B"
#define DATABASE_1 "db1"
#define DATABASE_2 "db2"
#define DB1_KEY "DB1_ITEM"
#define DB2_KEY "DB2_ITEM"

// Simulates three clients sharing their databases each other over two
// separated networks.
// Client 1 and 2 share database 1 on network A.
// Client 1 and 3 share database 2 on network B.
// Each client uses a private storage path to store its own database and the
// database shared by the other client.
// Each client has one comm process per network that distributes the database
// linked to that network to the other comm processes.

std::vector<boost::thread> threads;

void startComm(int agent, std::string const &network, std::string const &configFile)
{
    RtDB2Context ctx = RtDB2Context::Builder(agent, RtDB2ProcessType::comm)
                                .withConfigFileName(configFile)
                                .withNetwork(network)
                                .build();
    Comm* comm = new Comm(agent, ctx);
    threads.push_back(comm->start());
    usleep(250000); // configuration is not multi-thread safe
}

int main(int argc, char **argv)
{
    // no argument parsing

    // determine path of binary
    char buffer[PATH_MAX];
    realpath(argv[0], buffer);
    char *path = dirname(buffer);
    // compose configuration file name
    std::string configFileName(path);
    configFileName += CONFIGFILE;
    std::cout << "Using configuration file: " << configFileName << std::endl;

    // client 1 on network A and B
    startComm(1, NETWORK_A, configFileName);
    startComm(1, NETWORK_B, configFileName);
    // client 2 on network A
    startComm(2, NETWORK_A, configFileName);
    // client 3 on network B
    startComm(3, NETWORK_B, configFileName);

    // client 1 - database 1
    RtDB2Context ctxClient1DB1 = RtDB2Context::Builder(1)
                                .withConfigFileName(configFileName)
                                .withDatabase(DATABASE_1)
                                .build();
    RtDB2 rtdbClient1DB1(ctxClient1DB1);
    RtDB2 rtdbClient1DB1Remote2(ctxClient1DB1, 2);
    usleep(250000); // configuration is not multi-thread safe

    // client 1 - database 2
    RtDB2Context ctxClient1DB2 = RtDB2Context::Builder(1)
                                .withConfigFileName(configFileName)
                                .withDatabase(DATABASE_2)
                                .build();
    RtDB2 rtdbClient1DB2(ctxClient1DB2);
    RtDB2 rtdbClient1DB2Remote3(ctxClient1DB2, 3);
    usleep(250000); // configuration is not multi-thread safe

    // client 2 - database 1
    RtDB2Context ctxClient2DB1 = RtDB2Context::Builder(2)
                                .withConfigFileName(configFileName)
                                .withDatabase(DATABASE_1)
                                .build();
    RtDB2 rtdbClient2DB1Remote1(ctxClient2DB1, 1);
    RtDB2 rtdbClient2DB1(ctxClient2DB1);
    usleep(250000); // configuration is not multi-thread safe

    // client 3 - database 2
    RtDB2Context ctxClient3DB2 = RtDB2Context::Builder(3)
                                .withConfigFileName(configFileName)
                                .withDatabase(DATABASE_2)
                                .build();
    RtDB2 rtdbClient3DB2Remote1(ctxClient3DB2, 1);
    RtDB2 rtdbClient3DB2(ctxClient3DB2);
    usleep(250000); // configuration is not multi-thread safe

    // rtdb
    srand(time(NULL));
    int value;
    int result;

    result = rtdbClient1DB1.get(DB1_KEY, &value);
    std::cout << "Client 1 DB1 local       (success: " << result << "): " << value << std::endl;
    result = rtdbClient1DB1Remote2.get(DB1_KEY, &value);
    std::cout << "Client 1 DB1 of Remote 2 (success: " << result << "): " << value << std::endl;
    result = rtdbClient2DB1.get(DB1_KEY, &value);
    std::cout << "Client 2 DB1 local       (success: " << result << "): " << value << std::endl;
    result = rtdbClient2DB1Remote1.get(DB1_KEY, &value);
    std::cout << "Client 2 DB1 of Client 1 (success: " << result << "): " << value << std::endl;
    result = rtdbClient1DB2.get(DB2_KEY, &value);
    std::cout << "Client 1 DB1 local       (success: " << result << "): " << value << std::endl;
    result = rtdbClient1DB2Remote3.get(DB2_KEY, &value);
    std::cout << "Client 1 DB1 of Remote 3 (success: " << result << "): " << value << std::endl;
    result = rtdbClient3DB2.get(DB2_KEY, &value);
    std::cout << "Client 3 DB1 local       (success: " << result << "): " << value << std::endl;
    result = rtdbClient3DB2Remote1.get(DB2_KEY, &value);
    std::cout << "Client 3 DB1 of Client 1 (success: " << result << "): " << value << std::endl;

    int value1 = rand() % 1000;
    std::cout << "Writing value " << value1 << " to DB1 of Client 1" << std::endl;
    rtdbClient1DB1.put(DB1_KEY, &value1);
    int value2 = rand() % 1000;
    std::cout << "Writing value " << value2 << " to DB1 of Client 2" << std::endl;
    rtdbClient2DB1.put(DB1_KEY, &value2);
    int value3 = rand() % 1000;
    std::cout << "Writing value " << value3 << " to DB2 of Client 1" << std::endl;
    rtdbClient1DB2.put(DB2_KEY, &value3);
    int value4 = rand() % 1000;
    std::cout << "Writing value " << value4 << " to DB2 of Client 3" << std::endl;
    rtdbClient3DB2.put(DB2_KEY, &value4);

    usleep(300000);

    result = rtdbClient1DB1.get(DB1_KEY, &value);
    std::cout << "Client 1 DB1 local       (success: " << result << "): " << value << std::endl;
    result = rtdbClient1DB1Remote2.get(DB1_KEY, &value);
    std::cout << "Client 1 DB1 of Remote 2 (success: " << result << "): " << value << std::endl;
    result = rtdbClient2DB1.get(DB1_KEY, &value);
    std::cout << "Client 2 DB1 local       (success: " << result << "): " << value << std::endl;
    result = rtdbClient2DB1Remote1.get(DB1_KEY, &value);
    std::cout << "Client 2 DB1 of Client 1 (success: " << result << "): " << value << std::endl;
    result = rtdbClient1DB2.get(DB2_KEY, &value);
    std::cout << "Client 1 DB2 local       (success: " << result << "): " << value << std::endl;
    result = rtdbClient1DB2Remote3.get(DB2_KEY, &value);
    std::cout << "Client 1 DB2 of Remote 3 (success: " << result << "): " << value << std::endl;
    result = rtdbClient3DB2.get(DB2_KEY, &value);
    std::cout << "Client 3 DB2 local       (success: " << result << "): " << value << std::endl;
    result = rtdbClient3DB2Remote1.get(DB2_KEY, &value);
    std::cout << "Client 3 DB2 of Client 1 (success: " << result << "): " << value << std::endl;

    Comm::shutdown();
    usleep(100000);
    // TODO: Shutdown blocks on receiver thread in comm
    // t1.join();
    // t2.join();

    // done
    return 0;
}
