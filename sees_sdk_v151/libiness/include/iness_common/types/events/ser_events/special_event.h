#ifndef SPECIAL_EVENT_H
#define SPECIAL_EVENT_H

#include <stdint.h>
#include <libcaer-iness/events/special_type.h>
#include "iness_common/types/generated/aedat_type_generated.h"

#include "event_helper.h"

namespace iness
{

/**
 * @brief The SpecialEvent class.
 */
class SpecialEvent
{
public:
    SpecialEvent();

    /**
     * @brief Return timestamp
     * @param _overflow_timestamp Overflow counter from event packet header.
     * @return The timestamp in microseconds.
     */
    time::TimeUs getTimestampUs(int32_t _overflow_timestamp) const;

    //! Serialize event to buffer.
    void serialize(char *_output_buffer) const;

    //! Get the AedatType
    static iness::serialization::AedatType aedatType();

    /**
     * @brief Get the event type.
     */
    caer_special_event_types getType() const;

    /**
     * @brief Get the special event payload.
     */
    uint32_t getPayload() const;

public:
    static const unsigned int ts_offset = sizeof(uint32_t); //!< size of data_

private:
    uint32_t data_;
    int32_t timestamp_;
};

}

#endif // SPECIAL_EVENT_H
