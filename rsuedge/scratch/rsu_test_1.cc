#include <fstream>
#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RsuTest");


int main (int argc, char *argv[])
{
    
    uint32_t N = 2;
    /*
     * Create RSU and Vehicles
     */
    NS_LOG_INFO("Create Nodes.");
    NodeContainer rsuNode;
    NodeContainer vehicles;
    rsuNode.Create(1);
    vehicles.Create(N);
    NodeContainer allNodes = NodeContainer(rsuNode, vehicles);

    /*
     * Install network stacks on the nodes
     */
    InternetStackHelper stack;
    stack.Install(allNodes);

    /*
     * Collect an adjacency list of nodes for the p2p topology
     */
    std::vector<NodeContainer> nodeAdjacencyList(N);
    for(uint32_t i = 0;  i < nodeAdjacencyList.size(); ++i)
    {
        nodeAdjacencyList[i] = NodeContainer(rsuNode, vehicles.Get(i));
    }

    /*
     * We create the channels first without any IP addressing information
     */
    NS_LOG_INFO("Create Channels.");
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    std::vector<NetDeviceContainer> deviceAdjacencyList(N);
    for(uint32_t i = 0; i < deviceAdjacencyList.size(); ++i)
    {
        deviceAdjacencyList[i] = p2p.Install(nodeAdjacencyList[i]);
    }
    /*
     * We add IP addresses.
     */
    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    std::vector<Ipv4Address> peerAddress(N);
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    std::vector<Ipv4InterfaceContainer> interfaceAdjacencyList(N);
    for(uint32_t i = 0; i < interfaceAdjacencyList.size(); ++i)
    {
        interfaceAdjacencyList[i] = ipv4.Assign(deviceAdjacencyList[i]);
        peerAddress.push_back(interfaceAdjacencyList[i].GetAddress(1));
    }
    

    uint16_t port = 8080;
    /*
     * Create RSU Application
     */
    //Address localAddress(InetSocketAddress (Ipv4Address("255.255.255.255"), port));
    Address localAddress(InetSocketAddress (interfaceAdjacencyList[0].GetAddress(1), port));

    RsuAppHelper rsu_1(true, localAddress);
    rsu_1.SetPeersAddresses(peerAddress);
    rsu_1.SetAttribute("DataRate", DataRateValue(DataRate("5Mb/s")));
    ApplicationContainer rsuApp = rsu_1.Install(rsuNode);

    rsuApp.Start(Seconds(0.5));
    rsuApp.Stop(Seconds(10.0));

    /*
     * Create Vehicle Application
     */
    Address rsuAddress(InetSocketAddress (interfaceAdjacencyList[0].GetAddress(0), port));
    RsuVehicleHelper rsuVehi(true, rsuAddress);
    rsuVehi.SetAttribute("DataRate", DataRateValue(DataRate("5Mb/s")));
    ApplicationContainer rsuVehiApp = rsuVehi.Install(vehicles);

    rsuVehiApp.Start(Seconds(1.0));
    rsuVehiApp.Stop(Seconds(10.0));

    
    AnimationInterface anim("rsu_test_v1.xml");
    anim.SetConstantPosition(rsuNode.Get(0), 0, 5);
    anim.SetConstantPosition(vehicles.Get(0), 0, 10);
    anim.SetConstantPosition(vehicles.Get(1), 10, 10);
    
    p2p.EnablePcapAll("rsu_test_1_packet", false);

    NS_LOG_INFO("Run Simulation");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    return 0;


    


}