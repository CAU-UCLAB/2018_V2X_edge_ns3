#include "rsu-vehicle-helper.h"
#include "ns3/names.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("RsuVehicleAppHelper");
    RsuVehicleHelper::RsuVehicleHelper (bool mode, Address local)
    {
        m_factory.SetTypeId("ns3::RsuVehicleApp");
        m_factory.Set ("Mode", BooleanValue(mode));
        m_factory.Set ("Local", AddressValue(local));

    }

    void
    RsuVehicleHelper::SetAttribute (std::string name, const AttributeValue &value)
    {
        m_factory.Set(name, value);
    }
    void 
    RsuVehicleHelper::SetPeersAddresses(std::vector<Ipv4Address> &peersAddresses)
    {
        NS_LOG_INFO("peer size:" << peersAddresses.size());
        m_peersAddresses = peersAddresses;
    }

    ApplicationContainer
    RsuVehicleHelper::Install (Ptr<Node> node) const
    {
        return ApplicationContainer(InstallPriv(node));
    } 

    ApplicationContainer
    RsuVehicleHelper::Install (std::string nodeName) const
    {
        Ptr<Node> node = Names::Find<Node> (nodeName);
        return ApplicationContainer(InstallPriv(node));
    }

    ApplicationContainer
    RsuVehicleHelper::Install (NodeContainer c) const
    {
        ApplicationContainer apps;
        for(NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
        {
            apps.Add(InstallPriv(*i));
        }
        return apps;
    }

    Ptr<Application>
    RsuVehicleHelper::InstallPriv (Ptr<Node> node) const
    {
        Ptr<RsuVehicleApp> app = m_factory.Create<RsuVehicleApp>();
        app->SetPeersAddresses(m_peersAddresses);
        node->AddApplication(app);
        return app;
    }

}