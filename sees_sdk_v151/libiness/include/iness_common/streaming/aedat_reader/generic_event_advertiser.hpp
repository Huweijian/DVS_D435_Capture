/*
*   Copyright (C) Insightness AG, Switzerland - All Rights Reserved
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   Proprietary and confidential
*   Written by Nathan Baumli <nathan@insightness.com>, August 2017
*/

#ifndef AEDAT_READER_GENERIC_EVENT_ADVERTISER_H
#define AEDAT_READER_GENERIC_EVENT_ADVERTISER_H

#include "iness_common/streaming/aedat_reader/i_shared_raw_event_data_handler.hpp"
#include "iness_common/device/sees_base.hpp"
#include "iness_common/types/eventhandler.h"


namespace iness
{
namespace aedat
{
class GenericEventAdvertiser: public ISharedRawEventDataHandler
{
public:
    GenericEventAdvertiser(
        std::function<void(std::shared_ptr<aedat::IEventPacket>)> _packet_cb_func,
        std::function<void(std::shared_ptr<aedat::IEventPacket> _packet, unsigned int event_index)> _event_cb_func);
    virtual ~GenericEventAdvertiser();

public:
    virtual void handleEventPacketData(std::shared_ptr<aedat::IEventPacket> _packet) override;
    virtual void handleEventData(std::shared_ptr<aedat::IEventPacket> _packet, unsigned int _event_index) override;

private:
    std::function<void(std::shared_ptr<aedat::IEventPacket>)> packet_cb_function_;
    std::function<void(std::shared_ptr<aedat::IEventPacket> _packet, unsigned int event_index)> event_cb_function_;
};

}
}

#endif // AEDAT_READER_GENERIC_EVENT_ADVERTISER_H
