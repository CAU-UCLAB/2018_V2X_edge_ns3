#ifndef RSU_VEHICLE_H
#define RSU_VEHICLE_H

#include "ns3/application.h"
#include "ns3/network-module.h"
#include <vector>

namespace ns3{

    class Socket;
    class Packet;

    class RsuVehicleApp : public Application
    {
        public:
            static TypeId GetTypeId(void);
            RsuVehicleApp();
            virtual ~RsuVehicleApp();

        private:
            virtual void StartApplication (void);
            virtual void StopAllication (void);

            void SendPacket (uint64_t msgType);
            void ScheduleTx (void);
            void HandleRead (Ptr<Socket> socket);
            void HandleAccept (Ptr<Socket> socket, const Address& from);
            void HandlePeerClose (Ptr<Socket> socket);
            void HandlePeerError (Ptr<Socket> socket);

            bool        m_mode;         //Normal : True, Edge : False
            Address     m_local;
            DataRate    m_dataRate;
            Ptr<Socket> m_socket;
            uint32_t    m_packetSize;
            uint32_t    m_packetSent;
            EventId     m_sendEvent;

            TracedCallback<Ptr<const Packet>> m_txTrace;
            TracedCallback<Ptr<const Packet>> m_rxTrace;
    };
}

#endif