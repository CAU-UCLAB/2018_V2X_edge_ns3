#ifndef RSU_HEADER_H
#define RSU_HEADER_H

#include "ns3/header.h"

using namespace ns3;
/*
enum Messages
{
    AUC_REQUEST,
    AUC_REPLY,
    AUC_RESULT
};
*/

class RsuHeader : public Header
{
    public:
        static TypeId GetTypeId(void);
        virtual TypeId GetInstanceTypeId(void) const;
        virtual uint32_t GetSerializedSize(void) const;
        virtual void Serialize (Buffer::Iterator start) const;
        virtual uint32_t Deserialize (Buffer::Iterator start);
        virtual void Print(std::ostream &os) const;

        void SetMessageType(uint64_t msg);
        uint64_t GetMessageType(void) const;
        //enum Messages GetMessageType(void) const;

    private:
        uint64_t    m_msg;         // 1: AUC_REQUEST , 2:AUC_REPLY, 3:AUC_RESULT
        //enum Messages m_msg;
};


#endif