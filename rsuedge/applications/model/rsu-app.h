#ifndef RSU_APP_H
#define RSU_APP_H

#include "ns3/application.h"
#include "ns3/network-module.h"
#include <vector>
#include <map>

namespace ns3{

    class Socket;
    class Packet;

    class RsuApp : public Application{

        public:
            static TypeId GetTypeId(void);
            RsuApp();
            virtual ~RsuApp();
            std::vector<Ipv4Address> GetPeersAdresses(void) const;
            std::vector<Ipv4Address> GetSelectedEdges(void) const;
            void SetPeersAddresses(const std::vector<Ipv4Address> &peers);
            void SetNumberOfEdges(int edges);
            void SetSelectedEdges(int numberOfEdges);
            void AuctionLatency(void);

        private:
            virtual void StartApplication (void);
            virtual void StopApplication (void);

            void SendPacket (uint64_t msgType);
            void CompleteCollect (Address from);
            void ScheduleNextAuction (void);
            void HandleRead (Ptr<Socket> socket);
            void HandleAccept (Ptr<Socket> socket, const Address& from);
            void HandlePeerClose (Ptr<Socket> socket);
            void HandlePeerError (Ptr<Socket> socket);

            bool                                m_mode;        //Tx: true, Rx: false
            int                                 m_numberOfPeers;
            int                                 m_numberOfEdges;
            double                              requestTime;
            double                              resultTime;
            std::vector<Ipv4Address>            m_peersAddresses;
            std::vector<Ipv4Address>            m_selectedEdges;
            std::vector<Ipv4Address>            m_receivedReply;
            std::map<Ipv4Address, Ptr<Socket>>  m_peersSockets;
            Ipv4Address                         m_broadcastAddress;
            Address                             m_local;
            DataRate                            m_dataRate;
            Ptr<Socket>                         m_socket;
            uint32_t                            m_packetSize;
            uint32_t                            m_packetsSent;
            EventId                             m_sendEvent;
            EventId                             m_waitEvent;
            

            TracedCallback<Ptr<const Packet>> m_txTrace;
            TracedCallback<Ptr<const Packet>> m_rxTrace;
    };

}
#endif