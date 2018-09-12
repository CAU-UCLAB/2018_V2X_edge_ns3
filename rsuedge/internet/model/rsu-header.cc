#include "rsu-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE("RsuHeader");

using namespace ns3;

NS_OBJECT_ENSURE_REGISTERED(RsuHeader);

TypeId
RsuHeader::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::RsuHeader")
        .SetParent<Header>()
        .AddConstructor<RsuHeader> ()
        ;
    return tid;
}

TypeId
RsuHeader::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

uint32_t
RsuHeader::GetSerializedSize(void) const
{
    return 8;
}

void
RsuHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU64(m_msg);
}

uint32_t
RsuHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    m_msg = i.ReadNtohU64();
    return 8;
}

void
RsuHeader::Print(std::ostream &os) const
{
    os << "Message type : " << m_msg;
}

void
RsuHeader::SetMessageType(uint64_t msg)
{
    m_msg = msg;
}

uint64_t
RsuHeader::GetMessageType(void) const
{
    return m_msg;
}



