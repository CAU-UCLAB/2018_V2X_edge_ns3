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
#include "ns3/wifi-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RsuTest_v3");


int main (int argc, char *argv[])
{
    
    uint32_t N = 2;
    
    //1. Create RSU and Vehicles
    
    NS_LOG_INFO("Create Nodes.");
    NodeContainer rsuNodes;
    NodeContainer vehicleNodes;
    rsuNodes.Create(1);
    vehicleNodes.Create(N);

    NodeContainer allNodes = NodeContainer(rsuNodes, vehicleNodes);

    //2. Crate PHY Layer (wireless channel)
    NS_LOG_INFO("Create Channels.");
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
    //phy.SetPcapDataLinkType(Wi)
    phy.SetChannel(channel.Create());

    //3. Create MAC Layer
    WifiMacHelper mac;
    Ssid ssid = Ssid("RsuTest3");
    mac.SetType("ns3::StaWifiMac",
        "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    
    //4. Create WLAN setting
    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211n_5GHZ);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", 
        "DataMode", StringValue("HtMcs7"), "ControlMode", StringValue("HtMcs0"));
    
    //5. Create NetDevices
    NetDeviceContainer rsuDevice;
    rsuDevice = wifi.Install(phy, mac, rsuNodes);

    mac.SetType("ns3::ApWifiMac",
            "Ssid", SsidValue(ssid),
            "BeaconInterval", TimeValue(MicroSeconds(102400)),
            "BeaconGeneration", BooleanValue(true));
    
    NetDeviceContainer vehicleDevice;
    vehicleDevice = wifi.Install(phy, mac, vehicleNodes);

    //6. Create Network layer
    NS_LOG_INFO("Assign IP Addresses.");
    InternetStackHelper stack;
    stack.Install(allNodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    std::vector<Ipv4Address> peerAddress(N);
    std::vector<Ipv4Address> rsuAddress;
    peerAddress.clear();
    rsuAddress.clear();
    Ipv4InterfaceContainer rsuInterface = ipv4.Assign(rsuDevice);
    Ipv4InterfaceContainer vehicleInterface = ipv4.Assign(vehicleDevice);
    for(uint32_t i = 0; i < N ; ++i)
    {
        NS_LOG_INFO("Add IP (" << vehicleInterface.GetAddress(i) << ") address to vector");
        peerAddress.push_back(vehicleInterface.GetAddress(i));
    }
    rsuAddress.push_back(rsuInterface.GetAddress(0));
    NS_LOG_INFO("number of vehicles: " << peerAddress.size());

    // 7. Locate nodes
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

    positionAlloc->Add(Vector(0,0,0));
    positionAlloc->Add(Vector(1,0,0));
    positionAlloc->Add(Vector(1,1,0));
    mobility.SetPositionAllocator(positionAlloc);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    mobility.Install(allNodes);
    //mobility.Install(vehicleNodes);


    // 8. Create Rsu application.
    NS_LOG_INFO("Create RSU Application");
    uint16_t port = 8080;
    Address localAddress(InetSocketAddress (Ipv4Address::GetAny(), port));
    RsuAppHelper rsu_1(true, localAddress);
    rsu_1.SetPeersAddresses(peerAddress);
    rsu_1.SetAttribute("DataRate", DataRateValue(DataRate("5Mb/s")));
    ApplicationContainer rsuApp = rsu_1.Install(rsuNodes);
    
    rsuApp.Start(Seconds(1.0));
    rsuApp.Stop(Seconds(10.0));

    // 9. Create Vehicle application.
    NS_LOG_INFO("Create Vehicle Application");
    //Address rsuAddress(InetSocketAddress (Ipv4Address::GetAny(), port));
    RsuVehicleHelper rsuVehi(true, localAddress);
    rsuVehi.SetPeersAddresses(rsuAddress);
    rsuVehi.SetAttribute("DataRate", DataRateValue(DataRate("5Mb/s")));
    ApplicationContainer rsuVehiApp = rsuVehi.Install(vehicleNodes);

    rsuVehiApp.Start(Seconds(0.0));
    rsuVehiApp.Stop(Seconds(10.0));

    
    AnimationInterface anim("rsu_test_v3.xml");
    anim.SetMaxPktsPerTraceFile(1000000);
    anim.SetConstantPosition(rsuNodes.Get(0), 10, 5);
    anim.SetConstantPosition(vehicleNodes.Get(0), 10, 10);
    anim.SetConstantPosition(vehicleNodes.Get(1), 5, 10);
    
    //p2p.EnablePcapAll("rsu_test_3_packet", false);
    phy.EnablePcap ("rsu_test_3_packet", rsuDevice);

    NS_LOG_INFO("Run Simulation");
    Simulator::Stop(Seconds(11));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    return 0;



}