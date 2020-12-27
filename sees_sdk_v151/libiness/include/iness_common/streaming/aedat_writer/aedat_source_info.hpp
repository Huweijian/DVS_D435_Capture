/*
*   Copyright (C) Insightness AG, Switzerland - All Rights Reserved
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   Proprietary and confidential
*   Written by Michael Gassner <michael@insightness.com>, February 2018
*/

#ifndef AEDAT_WRITER_AEDAT_SOURCE_INFO_H
#define AEDAT_WRITER_AEDAT_SOURCE_INFO_H

#include <string>
#include <iness_common/types/generated/aedat_type_generated.h>
#include <iness_common/util/parameter_tree/node.hpp>

namespace iness
{
namespace aedat
{

/**
 * @brief Struct containing required information about an AEDAT data source. This information must be
 * contained in the AEDAT file header (check AEDAT 3.1 https://inilabs.com/support/software/fileformat/ for more information).
 */
struct AedatSourceInfo
{
    unsigned int source_id; //!< Numerical source ID as definded in AEDAT 3.1 (https://inilabs.com/support/software/fileformat/).
    std::string source_description; //!< The source description as defined in AEDAT 3.1, for our devices it is the module name (e.g. Seem1)
    iness::ptree::Node::Ptr source_info; //!< Any additional information.
};

}
}

#endif // AEDAT_WRITER_AEDAT_SOURCE_INFO_H
