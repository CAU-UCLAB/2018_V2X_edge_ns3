#include "rsu-app-helper.h"
#include "ns3/names.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("RsuAppHelper");
    RsuAppHelper::RsuAppHelper (bool mode, Address local)
    {
        m_factory.SetTypeId("ns3::RsuApp");
        m_factory.Set ("Mode", BooleanValue(mode));
        m_factory.Set ("Local", AddressValue(local));
    }

    void
    RsuAppHelper::SetAttribute (std::string name, const AttributeValue &value)
    {
        m_factory.Set(name, value);
    }
    void 
    RsuAppHelper::SetPeersAddresses(std::vector<Ipv4Address> &peersAddresses)
    {
        NS_LOG_INFO("peer size:" << peersAddresses.size());
        m_peersAddresses = peersAddresses;
    }
    void
    RsuAppHelper::SetNumberOfEdges(int edges)
    {
        m_numberOfEdges = edges;
    }

    ApplicationContainer
    RsuAppHelper::Install (Ptr<Node> node) const
    {
        return ApplicationContainer(InstallPriv(node));
    } 

    ApplicationContainer
    RsuAppHelper::Install (std::string nodeName) const
    {
        Ptr<Node> node = Names::Find<Node> (nodeName);
        return ApplicationContainer(InstallPriv(node));
    }

    ApplicationContainer
    RsuAppHelper::Install (NodeContainer c) const
    {
        ApplicationContainer apps;
        for(NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
        {
            apps.Add(InstallPriv(*i));
        }
        return apps;
    }

    Ptr<Application>
    RsuAppHelper::InstallPriv (Ptr<Node> node) const
    {
        Ptr<RsuApp> app = m_factory.Create<RsuApp>();
        app->SetPeersAddresses(m_peersAddresses);
        app->SetNumberOfEdges(m_numberOfEdges);
        node->AddApplication(app);
        return app;
    }

}