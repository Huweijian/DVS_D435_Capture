/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Mon Feb 12 2018
 */


#ifndef INESS_COMMON_UTIL_PARAMETER_TREE_PARAMETER_TREE_PROVIDER_HPP_
#define INESS_COMMON_UTIL_PARAMETER_TREE_PARAMETER_TREE_PROVIDER_HPP_

#include "iness_common/util/parameter_tree/node.hpp"

namespace iness
{
namespace ptree
{

//! Interface definition for a class that provides a parameter tree.
class Provider
{
public:
  Provider() = default;
  virtual ~Provider() = default;

  //! Returns a pointer to the objects' parameter tree.
  virtual Node::Ptr parameterTree() =0;

  //! Returns a pointer to a const parameter tree
  virtual Node::ConstPtr parameterTree() const{ return const_cast<Provider*>(this)->parameterTree(); }
};

}
}

#endif // INESS_COMMON_UTIL_PARAMETER_TREE_PARAMETER_TREE_PROVIDER_HPP_