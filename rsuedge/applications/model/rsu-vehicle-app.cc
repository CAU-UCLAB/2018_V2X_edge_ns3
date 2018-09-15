#include "ns3/log.h"
#include "rsu-vehicle-app.h"
#include "ns3/rsu-header.h"
#include "ns3/udp-socket.h"

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
    RsuVehicleApp::StartApplication (void)
    {
        NS_LOG_FUNCTION(this);

        NS_LOG_INFO("Node " << GetNode()->GetId());
        NS_LOG_INFO("m_local : " << InetSocketAddress::ConvertFrom(m_local).GetIpv4());
    
        if(!m_socket)
        {
            NS_LOG_INFO("Node " << GetNode()->GetId() << " Create Socket");
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
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
        
        m_socket->SetRecvCallback(MakeCallback(&RsuVehicleApp::HandleRead, this));
        m_socket->SetAcceptCallback(
            MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
            MakeCallback (&RsuVehicleApp::HandleAccept, this));
        m_socket->SetCloseCallbacks(
            MakeCallback (&RsuVehicleApp::HandlePeerClose, this),
            MakeCallback (&RsuVehicleApp::HandlePeerError, this));
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
        m_socket->Send(packet);

        NS_LOG_INFO("Node " << GetNode()->GetId() << " send REPLY to"
        << InetSocketAddress::ConvertFrom(m_local).GetIpv4());

    }

    void
    RsuVehicleApp::ScheduleTx(void)
    {
        //
    }

    void
    RsuVehicleApp::HandleRead(Ptr<Socket> socket)
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
            if(msgType == 1)
            {
                //receive AUC_REQUEST
                NS_LOG_INFO("Receive AUC_REQUSET");
                
                if(m_mode == true)
                {
                    //Choose to bid
                    double bidTime = rand()%2*0.01f;
                    Simulator::Schedule(Seconds(bidTime), &RsuVehicleApp::SendPacket, this, 2);
                }

            }
            else if(msgType == 3)
            {
                //receive AUC_RESULT
                NS_LOG_INFO("Receive AUC_RESULT");
                //m_rxTrace(packet);
                m_mode = false;

            }
            else
            {
                //remove packet
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