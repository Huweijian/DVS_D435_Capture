/*
*   Copyright (C) Insightness AG, Switzerland - All Rights Reserved
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   Proprietary and confidential
*   Written by Jonas Strubel <jonas@insightness.com>, April 2017
*/

#ifndef I_EVENT_H
#define I_EVENT_H

#include "iness_common/time/definitions.hpp"

class IEvent
{
public:
    virtual iness::time::TimeUs getTimestamp() const = 0;
    virtual void serialize(char* _output_buffer) = 0;
    virtual void deserialize(int32_t _overflow_timestamp, char* _input_buffer) = 0;
};

#endif // I_EVENT_H
