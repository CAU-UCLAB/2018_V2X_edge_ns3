#include "ns3/log.h"
#include "rsu-vehicle-app.h"
#include "ns3/rsu-header.h"
#include "ns3/udp-socket.h"
#include "ns3/random-variable-stream.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/config.h"

namespace ns3{

    NS_LOG_COMPONENT_DEFINE("RsuVehicleApp");
    NS_OBJECT_ENSURE_REGISTERED(RsuVehicleApp);

    TypeId
    RsuVehicleApp::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::RsuVehicleApp")
            .SetParent<Application> ()
            .AddConstructor<RsuVehicleApp> ()
            .AddAttribute("Mode", "The mode: Normal(True), Edge(false)",
                BooleanValue(true), MakeBooleanAccessor(&RsuVehicleApp::m_mode),
                MakeBooleanChecker())
            .AddAttribute("Local", "The address on which to bind the rx socket",
                AddressValue(),
                MakeAddressAccessor(&RsuVehicleApp::m_local),
                MakeAddressChecker())
            .AddAttribute("Protocol",
                "The type id of the protocol to use for the rx socket",
                TypeIdValue(TcpSocketFactory::GetTypeId()),
                MakeTypeIdAccessor(&RsuVehicleApp::m_tid),
                MakeTypeIdChecker())
            .AddAttribute("DataRate", "The data rate",
                DataRateValue(DataRate("500kb/s")),
                MakeDataRateAccessor(&RsuVehicleApp::m_dataRate),
                MakeDataRateChecker())
            .AddTraceSource("Tx", "A new packet is created and is sent",
                MakeTraceSourceAccessor(&RsuVehicleApp::m_txTrace),
                "ns3::Packet::TracedCallback")
            .AddTraceSource("Rx", "A packet has been received",
                MakeTraceSourceAccessor(&RsuVehicleApp::m_rxTrace),
                "ns3::Packet::TracedCallback");
        
        return tid;
    }

    RsuVehicleApp::RsuVehicleApp()
        : m_socket(0),
        m_packetSize(1000),
        m_packetSent(0)
    {
        NS_LOG_FUNCTION(this);
    }

    RsuVehicleApp::~RsuVehicleApp()
    {
        NS_LOG_FUNCTION(this);
    }
    void
    RsuVehicleApp::SetPeersAddresses(const std::vector<Ipv4Address> &peers)
    {
        NS_LOG_FUNCTION(this);
        NS_LOG_INFO("peers size: " << peers.size());
        m_peersAddresses = peers;
        m_numberOfPeers = m_peersAddresses.size();
        NS_LOG_INFO("Set peersAddresses (size: " <<  m_peersAddresses.size() << ")" );

    }

    void
    RsuVehicleApp::StartApplication (void)
    {
        NS_LOG_FUNCTION(this);

        NS_LOG_INFO("Node " << GetNode()->GetId() <<" peers are");
        for(auto it = m_peersAddresses.begin(); it != m_peersAddresses.end(); it++)
        {
            NS_LOG_INFO("\t" << *it);
        }

        //TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        TypeId tid = m_tid;

        if(!m_socket)
        {
            NS_LOG_INFO("Node " << GetNode()->GetId() << " Create Socket");
            m_socket = Socket::CreateSocket(GetNode(), tid);
            m_socket->SetAllowBroadcast(true);
            m_socket->Bind(m_local);
            m_socket->Listen();
            m_socket->ShutdownSend();
            
            /*
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
            */
        }
        
        m_socket->SetRecvCallback(MakeCallback(&RsuVehicleApp::HandleRead, this));
        
        m_socket->SetAcceptCallback(
            MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
            MakeCallback (&RsuVehicleApp::HandleAccept, this));
        m_socket->SetCloseCallbacks(
            MakeCallback (&RsuVehicleApp::HandlePeerClose, this),
            MakeCallback (&RsuVehicleApp::HandlePeerError, this));
        
        Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1460));
        Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(900000));
        Config::SetDefault("ns3::TcpSocket::SndBufSize" , UintegerValue(900000));

        if(m_peersAddresses.size() == 0)
        {
            NS_LOG_INFO("No peer");
        }
        else
        {
            for(std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end() ; ++i)
            {
                NS_LOG_INFO("Node " << GetNode()->GetId() << " connect peer:" << *i);
                m_peersSockets[*i] = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
                m_peersSockets[*i]->Connect(InetSocketAddress(*i, 8080));
                //m_peersUDPSockets[*i] = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
                //m_peersUDPSockets[*i]->Connect(InetSocketAddress(*i, 8070));
                //m_peersUDPSockets[*i]->SetAllowBroadcast(true);
            }
        }
        
    }

    void
    RsuVehicleApp::StopAllication (void)
    {
        NS_LOG_FUNCTION(this);

        if(m_socket)
        {
            m_socket->Close();
        }
    }

    void
    RsuVehicleApp::SendPacket (uint64_t msgType)
    {
        NS_LOG_FUNCTION(this);

        Ptr<Packet> packet = Create<Packet> (m_packetSize);
        RsuHeader rHeader;
        rHeader.SetMessageType(msgType);
        packet->AddHeader(rHeader);
        m_txTrace(packet);
        
        if(msgType == 2)
        {
            for(std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i)
            {
                m_peersSockets[*i]->Send(packet);
                NS_LOG_INFO("Node " << GetNode()->GetId() << " send AUC_REPLY to " 
                << *i);
            }
        }

    }

    void
    RsuVehicleApp::ScheduleEdge(void)
    {
        NS_LOG_FUNCTION(this);
        NS_LOG_INFO("Node " << GetNode()->GetId() << " stop edge computing.");
        m_mode = true;
    }

    void
    RsuVehicleApp::HandleRead(Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this);
        Ptr<Packet> packet;
        Address from;
        RsuHeader rHeader;
        uint64_t msgType;

        while((packet = socket->RecvFrom(from))){
            
            NS_LOG_INFO("Node " << GetNode()->GetId() 
                <<" At time " << Simulator::Now().GetSeconds()
                <<"s packet received" << packet->GetSize()
                << "bytes from" << InetSocketAddress::ConvertFrom(from).GetIpv4());
            packet->RemoveHeader(rHeader);
            msgType = rHeader.GetMessageType();
            m_rxTrace(packet);
            if(msgType == 1)
            {
                //receive AUC_REQUEST
                NS_LOG_INFO("Receive AUC_REQUSET");
                
                if(m_mode == true)
                {
                    double bidTime = rand()%4*0.01f;
                    Simulator::Schedule(Seconds(bidTime), &RsuVehicleApp::SendPacket, this, 2);
                }
                

                /* 
                 * Version 2: Random bidding
                 *
                Ptr<UniformRandomVariable> rv = CreateObject<UniformRandomVariable> ();
                uint32_t selectEdge = rv->GetInteger(1, 10);

                if(selectEdge > 6)
                {
                    if(m_mode == true)
                    {
                        //Choose to bid
                        double bidTime = rand()%2*0.01f;
                        Simulator::Schedule(Seconds(bidTime), &RsuVehicleApp::SendPacket, this, 2);
                    }
                }
                */
                
            }
            else if(msgType == 3)
            {
                //receive AUC_RESULT
                NS_LOG_INFO("Node " << GetNode()->GetId() << " receive AUC_RESULT");
                
                // Version 2. control edge computing time
                /*
                double edgeTime = rand()%4*0.01f+2;
                m_mode = false;
                Simulator::Schedule(Seconds(edgeTime), &RsuVehicleApp::ScheduleEdge, this);
                */
            }
        }
    }

    
    void
    RsuVehicleApp::HandleAccept (Ptr<Socket> socket, const Address& from)
    {
        NS_LOG_FUNCTION(this << socket <<from);
        socket->SetRecvCallback(MakeCallback(&RsuVehicleApp::HandleRead, this));
    }

    void
    RsuVehicleApp::HandlePeerClose (Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this << socket);
    }
    
    void
    RsuVehicleApp::HandlePeerError (Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this << socket);
    }




}