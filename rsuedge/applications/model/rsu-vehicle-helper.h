#ifndef RSU_VEHICLE_HELPER_H
#define RSU_VEHICLE_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/rsu-vehicle-app.h"
#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/address.h"

namespace ns3{

    class DataRate;

    class RsuVehicleHelper{
        public:
            RsuVehicleHelper(bool mode, Address local);
            void SetAttribute(std:: string name, const AttributeValue &value);
            ApplicationContainer Install (Ptr<Node> node) const;
            ApplicationContainer Install (std::string nodeName) const;
            ApplicationContainer Install (NodeContainer c) const;

        private:
            Ptr<Application> InstallPriv (Ptr<Node> node) const;
            ObjectFactory m_factory;

    };

}
#endif