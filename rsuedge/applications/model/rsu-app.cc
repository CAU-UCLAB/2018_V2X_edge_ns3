#include "ns3/log.h"
#include "rsu-app.h"
#include "ns3/rsu-header.h"
#include "ns3/udp-socket.h"

namespace ns3{

    NS_LOG_COMPONENT_DEFINE ("RsuApp");
    NS_OBJECT_ENSURE_REGISTERED(RsuApp);

    TypeId RsuApp::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::RsuApp")
            .SetParent<Application>()
            .AddConstructor<RsuApp>()
            .AddAttribute("Mode", "The mode: Normal(True), Multi-hop(false)",
                BooleanValue(true), MakeBooleanAccessor(&RsuApp::m_mode),
                MakeBooleanChecker())
            .AddAttribute("Local", "The address on which to bind the rx socket",
                AddressValue(),
                MakeAddressAccessor(&RsuApp::m_local),
                MakeAddressChecker())
            .AddAttribute("DataRate", "The data rate",
                DataRateValue(DataRate("500kb/s")),
                MakeDataRateAccessor(&RsuApp::m_dataRate),
                MakeDataRateChecker())
            .AddTraceSource("Tx", "A new packet is created and is sent",
                MakeTraceSourceAccessor(&RsuApp::m_txTrace),
                "ns3::Packet::TracedCallback")
            .AddTraceSource("Rx", "A packet has been received",
                MakeTraceSourceAccessor(&RsuApp::m_rxTrace),
                "ns3::Packet::TracedCallback");
        
        return tid;
    }

    RsuApp::RsuApp()
        :m_socket(0),
        m_packetSize(1000),
        m_packetsSent(0)
    {
        NS_LOG_FUNCTION(this);
    }

    RsuApp::~RsuApp()
    {
        NS_LOG_FUNCTION(this);
    }

    std::vector<Ipv4Address>
    RsuApp::GetPeersAdresses(void) const
    {
        NS_LOG_FUNCTION(this);
        return m_peersAddresses;
    }

    std::vector<Ipv4Address>
    RsuApp::GetSelectedEdges(void) const
    {
        NS_LOG_FUNCTION(this);
        return m_selectedEdges;
    }

    void
    RsuApp::SetPeersAddresses(const std::vector<Ipv4Address> &peers)
    {
        NS_LOG_FUNCTION(this);
        NS_LOG_INFO("Set peersAddresses");
        m_peersAddresses = peers;
        m_numberOfPeers = m_peersAddresses.size();
    }

    void
    RsuApp::SetSelectedEdges(int numberOfEdges)
    {
        //choose edges randomly
    }

    void 
    RsuApp::StartApplication(void)
    {
        NS_LOG_FUNCTION(this);

        NS_LOG_INFO("Node " << GetNode()->GetId() <<" peers are");
        for(auto it = m_peersAddresses.begin(); it != m_peersAddresses.end(); it++)
        {
            NS_LOG_INFO("\t" << *it);
        }
       
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        if(!m_socket)
        {   
            NS_LOG_INFO("Node " << GetNode()->GetId() << " Create Socket");
            m_socket = Socket::CreateSocket(GetNode(), tid);
            m_socket->SetAllowBroadcast(true);
            m_socket->Bind(m_local);
            m_socket->Listen();
            m_socket->ShutdownSend();
            if(addressUtils::IsMulticast(m_local))
            {
                Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
                if(udpSocket)
                {
                    udpSocket->MulticastJoinGroup(0, m_local);
                }
                else
                {
                    NS_FATAL_ERROR("Error: joining multicast on a non-UDP socket");
                }
            }
        }
        
        m_socket->SetRecvCallback(MakeCallback(&RsuApp::HandleRead, this));
        m_socket->SetAcceptCallback(
            MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
            MakeCallback (&RsuApp::HandleAccept, this));
        m_socket->SetCloseCallbacks(
            MakeCallback (&RsuApp::HandlePeerClose, this),
            MakeCallback (&RsuApp::HandlePeerError, this));

        if(m_peersAddresses.size() == 0)
        {
            NS_LOG_INFO("No peer");
        }
        else
        {
            for(std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end() ; ++i)
            {
                NS_LOG_INFO("Node " << GetNode()->GetId() << " connect peer:" << *i);
                m_peersSockets[*i] = Socket::CreateSocket(GetNode(), tid);
                m_peersSockets[*i]->Connect(*i);
            }
        }

        
        ScheduleNextAuction();
        
    } 

    void 
    RsuApp::StopApplication()
    {
        NS_LOG_FUNCTION(this);
        
        if(m_sendEvent.IsRunning()){
            Simulator::Cancel(m_sendEvent);
        }

        for(std::vector<Ipv4Address>::iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i)
        {
            m_peersSockets[*i]->Close();
        }

        if(m_socket)
        {
            m_socket->Close();
            m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }

    }

    void 
    RsuApp::SendPacket(uint64_t msgType)
    {
        NS_LOG_FUNCTION(this);
        Ptr<Packet> packet = Create<Packet> (m_packetSize);
        RsuHeader rHeader;
        rHeader.SetMessageType(msgType);
        packet->AddHeader(rHeader);

        m_txTrace(packet);


        if(msgType == 1)
        {
            //InetSocketAddress broad = InetSocketAddress(Ipv4Address("10.1.1.2"));
            m_socket->Bind();
            m_socket->Connect(m_local);
            m_socket->Send(packet);

            NS_LOG_INFO("Node " << GetNode()->GetId() << " send REQUEST to " 
                << InetSocketAddress::ConvertFrom(m_local).GetIpv4());
            
        }
        else if(msgType == 3)
        {
            
            m_socket->Bind();
            m_socket->Connect(m_local);
            m_socket->Send(packet);

            NS_LOG_INFO("Node " << GetNode()->GetId() << " send RESULT to "
                << InetSocketAddress::ConvertFrom(m_local).GetIpv4());
        }
        
        ScheduleNextAuction();
    }

    void 
    RsuApp::ScheduleNextAuction(void)
    {
        NS_LOG_FUNCTION(this);
        Time tNext (Seconds(3.0));
        m_sendEvent = Simulator::Schedule(tNext, &RsuApp::SendPacket, this, 1);
    }

    void 
    RsuApp::HandleRead (Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this);
        Ptr<Packet> packet;
        Address from;
        RsuHeader rHeader;
        uint64_t msgType;

        while((packet = m_socket->RecvFrom(from))){
            
            NS_LOG_INFO("At time" << Simulator::Now().GetSeconds()
                <<"s packet received" << packet->GetSize()
                << "bytes from" << InetSocketAddress::ConvertFrom(from).GetIpv4());
            packet->RemoveHeader(rHeader);
            msgType = rHeader.GetMessageType();
            m_rxTrace(packet);
            if(msgType == 2)
            {
                //Wait and do Auction
                NS_LOG_INFO("Receive AUC_RELPY");
                //m_rxTrace(packet);
            }
            else
            {
                //remove packet
            }

        }

    }

    void
    RsuApp::HandleAccept (Ptr<Socket> socket, const Address& from)
    {
        NS_LOG_FUNCTION(this << socket <<from);
        socket->SetRecvCallback(MakeCallback(&RsuApp::HandleRead, this));
    }

    void
    RsuApp::HandlePeerClose (Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this << socket);
    }
    
    void
    RsuApp::HandlePeerError (Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this << socket);
    }


}