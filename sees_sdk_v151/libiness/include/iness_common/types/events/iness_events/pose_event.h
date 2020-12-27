/*
*   Copyright (C) Insightness AG, Switzerland - All Rights Reserved
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   Proprietary and confidential
*   Written by Jonas Strubel <jonas@insightness.com>, April 2017
*/

#ifndef POSE_EVENT_H
#define POSE_EVENT_H

#include "i_event.h"
#include "iness_common/types/generated/aedat_type_generated.h"


namespace iness {

#pragma pack(push, 1)
class PoseEvent
{
public:
    PoseEvent();
    PoseEvent(int32_t _timestamp, float _qw, float _qx, float _qy, float _qz, float _x, float _y, float _z);

    // IEvent interface
public:
    iness::time::TimeUs getTimestamp(int32_t _overflow_timestamp) const;
    void setTimestamp(int32_t _timestamp);
    void serialize(char *_output_buffer) const;
    void deserialize(char *_input_buffer);
    //! Get the AedatType
    static iness::serialization::AedatType aedatType();

    float getQw() const;
    void setQw(float qw);

    float getQx() const;
    void setQx(float qx);

    float getQy() const;
    void setQy(float qy);

    float getQz() const;
    void setQz(float qz);

    float getX() const;
    void setX(float x);

    float getY() const;
    void setY(float y);

    float getZ() const;
    void setZ(float z);

public:
    static const unsigned int ts_offset = sizeof(uint32_t); //!< size of info_

private:
    uint32_t info_;
    int32_t timestamp_us_;

    //Rotation (Quaternion)
    float qw_;
    float qx_;
    float qy_;
    float qz_;

    //Translation
    float x_;
    float y_;
    float z_;
};
#pragma pack(pop)
}

#endif // POSE_EVENT_H
