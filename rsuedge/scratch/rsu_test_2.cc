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

NS_LOG_COMPONENT_DEFINE("RsuTest_v2");


int main (int argc, char *argv[])
{
    
    uint32_t N = 1;
    /*
     * Create RSU and Vehicles
     */
    NS_LOG_INFO("Create Nodes.");
    NodeContainer allNodes;
    allNodes.Create(2);

    /*
     * Install network stacks on the nodes
     */
    InternetStackHelper stack;
    stack.Install(allNodes);

    /*
     * We create the channels first without any IP addressing information
     */
    NS_LOG_INFO("Create Channels.");
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer devices = p2p.Install(allNodes);
    /*
     * We add IP addresses.
     */
    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    std::vector<Ipv4Address> peerAddress(N);
    std::vector<Ipv4Address> rsupeerAddress;
    peerAddress.clear();
    rsupeerAddress.clear();
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);
    for(uint32_t i = 0; i < N ; ++i)
    {
        NS_LOG_INFO("Add IP (" << interfaces.GetAddress(i+1) << ") address to vector");
        peerAddress.push_back(interfaces.GetAddress(i+1));
    }
    rsupeerAddress.push_back(interfaces.GetAddress(0));
    NS_LOG_INFO("number of vehicles: " << peerAddress.size());
    
    /*
    uint16_t port = 8080;
    Address dest(InetSocketAddress(interfaces.GetAddress(1), port));
    OnOffHelper onoff("ns3::UdpSocketFactory", dest);
    onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
    onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
    onoff.SetAttribute("DataRate", DataRateValue(5000000));

    ApplicationContainer rsuApp = onoff.Install(allNodes.Get(0));

    rsuApp.Start(Seconds(1.0));
    rsuApp.Stop(Seconds(10.0));

    
    PacketSinkHelper psk("ns3::UdpSocketFactory", 
            InetSocketAddress (Ipv4Address::GetAny(), port));
    ApplicationContainer rsuVehiApp = psk.Install(allNodes.Get(1));

    rsuVehiApp.Start(Seconds(0.0));
    rsuVehiApp.Stop(Seconds(10.0));
    */
    
    uint16_t port = 8080;
    
    //Create RSU Application
    NS_LOG_INFO("Create RSU Application");
    //Address localAddress(InetSocketAddress (Ipv4Address("255.255.255.255"), port));
    //Address localAddress(InetSocketAddress (interfaces.GetAddress(1), port));
    Address localAddress(InetSocketAddress (Ipv4Address::GetAny(), port));
    RsuAppHelper rsu_1(true, localAddress);
    rsu_1.SetPeersAddresses(peerAddress);
    rsu_1.SetAttribute("DataRate", DataRateValue(DataRate("5Mb/s")));
    ApplicationContainer rsuApp = rsu_1.Install(allNodes.Get(0));
    
    rsuApp.Start(Seconds(1.0));
    rsuApp.Stop(Seconds(10.0));

    
    //Create Vehicle Application
    NS_LOG_INFO("Create Vehicle Application");
    Address rsuAddress(InetSocketAddress (Ipv4Address::GetAny(), port));
    RsuVehicleHelper rsuVehi(true, rsuAddress);
    rsuVehi.SetPeersAddresses(rsupeerAddress);
    rsuVehi.SetAttribute("DataRate", DataRateValue(DataRate("5Mb/s")));
    ApplicationContainer rsuVehiApp = rsuVehi.Install(allNodes.Get(1));

    rsuVehiApp.Start(Seconds(0.0));
    rsuVehiApp.Stop(Seconds(10.0));
    
    
    AnimationInterface anim("rsu_test_v2.xml");
    anim.SetConstantPosition(allNodes.Get(0), 10, 5);
    anim.SetConstantPosition(allNodes.Get(1), 10, 10);
    //anim.SetConstantPosition(vehicles.Get(1), 10, 10);
    
    p2p.EnablePcapAll("rsu_test_2_packet", false);

    NS_LOG_INFO("Run Simulation");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    return 0;


    


}