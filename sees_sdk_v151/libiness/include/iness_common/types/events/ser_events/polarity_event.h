#ifndef POLARITY_EVENT_H
#define POLARITY_EVENT_H

#include <stdint.h>
#include <libcaer-iness/events/polarity_type.h>

#include "iness_common/types/events/event_packet_header.h"
#include "event_helper.h"

namespace iness
{

typedef caer_polarity_event PolarityEventRaw;
typedef caer_polarity_event* PolarityEventRawPtr;

//! Polarity type definition
enum struct Polarity: unsigned char
{
        NEG=0,
        POS=1
};

/*! Wrapper class around libcaer polarity events, providing object-style setters and getters
 */
#pragma pack(push, 1)
class PolarityEvent
{
public:
    typedef uint16_t CoordinateInt;

public:
    /**
     * @brief Default ctor.
     * @param _ev
     */
    PolarityEvent() = default;

    /**
     * @brief Copy data from libcaer polarity event
     * @param _ev libcaer struct
     */
    PolarityEvent( const PolarityEventRaw& _ev );

    //! Serialize the event to a buffer.
    void serialize(char *_output_buffer) const;

    //! Get the AedatType
    static iness::serialization::AedatType aedatType();

    /**
     * @brief Get the x-coordinate of the event
     */
    CoordinateInt getX() const;

    /**
     * @brief Set the x-coordinate of the event
     * @param _x
     */
    void setX(CoordinateInt _x);

    /**
     * @brief Return the y-coordinate of the event
     */
    CoordinateInt getY() const;

    /**
     * @brief Set the y-coordinate of the event
     * @param _y
     */
    void setY(CoordinateInt _y);

    /**
     * @brief Get the polarity of the event
     */
    Polarity getPolarity() const;

    /**
     * @brief Set the polarity of the event
     * @param _pol
     */
    void setPolarity( Polarity _pol );

    /**
     * @brief Get the full 64 bit timestamp which includes the overflow counter
     * @param _overflow_timestamp The timestamp overflow counter
     */
    time::TimeUs getTimestampUs(int32_t _overflow_timestamp) const;

    /**
     * @brief Set the 32 bit timestamp
     * @param _timestamp The 32 bit timestamp
     */
    void setTimestamp(int32_t _timestamp);

    /**
     * @brief Get the validity
     * @return
     */
    bool getValidity() const;

    /**
     * @brief Sets the validity
     * @param _validity
     */
    void setValidity( bool _validity );

public:
    static const unsigned int ts_offset = sizeof(uint32_t); //!< size of data_

private:
    uint32_t data_{0};
    int32_t timestamp_us_{0};
};
#pragma pack(pop)

/*! Class merges packet header information and event information in cases where both is necessary
 * (e.g. for overflow-corrected timestamp calculation)
 */
class PolarityEventWithContext
{
public:
    typedef PolarityEvent::CoordinateInt CoordinateInt;
public:
    PolarityEventWithContext()=default;
    PolarityEventWithContext(PolarityEvent& _event, EventPacketHeader& _header);

    /**
     * @brief Sets a new event pointer
     * @param _event
     */
    void setEvent(PolarityEvent* _event);

    /**
     * @brief Get the current event.
     */
    PolarityEvent* getEvent();
    const PolarityEvent* getEvent() const;

    /**
     * @brief Sets a new packet header
     * @param _pkt_header
     */
    void setHeader(EventPacketHeader* _pkt_header );

    /**
     * @brief Get the current header.
     */
    EventPacketHeader* getHeader();

    /**
     * @brief Gets the (ts-overflow-corrected) 64bit timestamp of the event.
     */
    time::TimeUs getTimestampUs() const;

    /**
     * @brief Get the x-coordinate of the event
     */
    CoordinateInt getX() const;

    /**
     * @brief Set the x-coordinate of the event
     * @param _x
     */
    void setX(CoordinateInt _x);

    /**
     * @brief Get the y-coordinate of the event
     * @return
     */
    CoordinateInt getY() const;

    /**
     * @brief Set the y-coordinate of the event
     * @param _y
     */
    void setY(CoordinateInt _y);

    /**
     * @brief Get event polarity
     * @return
     */
    Polarity getPolarity() const;

    /**
     * @brief Set event polarity
     * @param _pol
     */
    void setPolarity( Polarity _pol );
private:
    PolarityEvent* event_{nullptr}; //! Pointer to an actual event
    EventPacketHeader* pkt_header_{nullptr}; //! Pointer to the header of the corresponding packet.
};

}

#endif // POLARITY_EVENT_H
