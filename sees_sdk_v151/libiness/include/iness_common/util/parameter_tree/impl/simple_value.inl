/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Thu Feb 15 2018
 */


#include <vector>
#include "iness_common/util/memory.hpp"


namespace iness
{
namespace ptree
{

#define TEMPT template<typename T_TYPE>
#define CSCOPE SimpleValue<T_TYPE>

TEMPT
CSCOPE::SimpleValue( T_TYPE _init_val, Node::RwType _rw_type, bool _support_on_change_callback )
: value_(_init_val)
{ 
  this->rwTypeRef() = _rw_type;

  if( _support_on_change_callback )
  {
    this->properties()->addEntry("SupportsOnChangeCallback", SimpleValue<bool>::create(true, Node::RwType::READ_ONLY, false)  );
  }
}

TEMPT
Node::Ptr CSCOPE::create( T_TYPE _init_val, Node::RwType _rw_type, bool _support_on_change_callback )
{ 
  return std::shared_ptr<CSCOPE>( new CSCOPE(_init_val,_rw_type,_support_on_change_callback) );
}

TEMPT
Node::Type CSCOPE::type() const
{
  return Node::Type::SIMPLE;
}

TEMPT
Node::WriteState CSCOPE::write( T_TYPE _to_write )
{
  value_=_to_write;
  if( this->hasPropertyEntry("OnChangeCallback") )
  {
    std::vector<std::function<void()>> callbacks = this->template getPropertyEntry< std::vector<std::function<void()>> >("OnChangeCallback");
    for( auto func: callbacks )
      func();
  }

  return Node::WriteState::SUCCESSFUL;
}

TEMPT
Node::ReadState CSCOPE::read( T_TYPE& _to_read ) const
{
  _to_read = value_;
  return Node::ReadState::SUCCESSFUL;
}

TEMPT
T_TYPE& CSCOPE::value()
{
  return value_;
}

TEMPT
Node::ReadState CSCOPE::getReadState() const
{
  return Node::ReadState::SUCCESSFUL;
}

TEMPT
Node::WriteState CSCOPE::getWriteState() const
{
  return Node::WriteState::SUCCESSFUL;
}

#undef TEMPT
#undef CSCOPE

}
}