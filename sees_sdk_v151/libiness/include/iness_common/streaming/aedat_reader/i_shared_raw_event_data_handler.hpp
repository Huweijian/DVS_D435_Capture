/*
*   Copyright (C) Insightness AG, Switzerland - All Rights Reserved
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   Proprietary and confidential
*   Written by Jonas Strubel <jonas@insightness.com>, May 2017
*/

#ifndef AEDAT_READER_I_SHARED_RAW_EVENT_DATA_HANDLER_H
#define AEDAT_READER_I_SHARED_RAW_EVENT_DATA_HANDLER_H

#include "iness_common/types/events/i_event_packet.hpp"

namespace iness {
namespace aedat
{
/*!
 * \brief The ISharedRawEventDataHandler class defines the interface for processing raw event data (per-event and per-packet).
 * The interface methods are used to invoke registered callback methods on the raw event data in the event advertiser classes.
 */
class ISharedRawEventDataHandler
{
public:
    virtual ~ISharedRawEventDataHandler()=default;
    virtual void handleEventData(std::shared_ptr<aedat::IEventPacket> _packet, unsigned int _event_index) = 0;
    virtual void handleEventPacketData(std::shared_ptr<aedat::IEventPacket> _packet) = 0;
};

}
}

#endif // AEDAT_READER_I_SHARED_RAW_EVENT_DATA_HANDLER_H
