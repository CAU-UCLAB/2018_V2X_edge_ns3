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

            bool        m_mode;         //Normal : True, Edge : False
            Address     m_address;
            DataRate    m_dataRate;
            Ptr<Socket> m_socket;
            uint32_t    m_pakcetSize;
            uint32_t    m_packetSent;
            EventId     m_sendEvent;

    };
}

#endif