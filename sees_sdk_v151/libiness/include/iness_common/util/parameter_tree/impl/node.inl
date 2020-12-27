/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Thu Feb 15 2018
 */


#include "iness_common/util/parameter_tree/simple_value.hpp"

namespace iness
{
namespace ptree
{

template<typename T_TARGET_TYPE>
std::shared_ptr<T_TARGET_TYPE> Node::as()
{
  assert(std::dynamic_pointer_cast<T_TARGET_TYPE>(shared_from_this()) && "Node::as<T_TARGET_TYPE>():: You attempted to cast to a type that doesn't inherit from ptree::Node.");
  return std::static_pointer_cast<T_TARGET_TYPE>( shared_from_this() );
}

template<typename T_TARGET_TYPE>
std::shared_ptr<const T_TARGET_TYPE> Node::as() const
{
  assert(std::dynamic_pointer_cast<const T_TARGET_TYPE>(shared_from_this()) && "Node::as<T_TARGET_TYPE>() const:: You attempted to cast to a type that doesn't inherit from ptree::Node.");
  return std::static_pointer_cast<const T_TARGET_TYPE>(shared_from_this());
}

template<typename T_INFO_TYPE>
bool Node::addPropertyEntry( std::string _name, T_INFO_TYPE _info_element )
{
  return getPropertyTree()->addEntry(_name, SimpleValue<T_INFO_TYPE>::create(_info_element) );
}

template<typename T_INFO_TYPE>
T_INFO_TYPE Node::getPropertyEntry( std::string _name ) const
{
  const auto query_entry = getPropertyTree()->entry(_name);
  assert(query_entry && "Node::getInfo:: You tried to call the function with a key name of a nonexistent member");
  auto cast_query = query_entry->as<SimpleValue<T_INFO_TYPE>>();
  T_INFO_TYPE to_read;
  cast_query->read(to_read);
  return to_read;
}

}
}