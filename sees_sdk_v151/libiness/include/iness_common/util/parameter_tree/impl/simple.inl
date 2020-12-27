/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Thu Feb 15 2018
 */


namespace iness
{
namespace ptree
{

#define TEMPT template<typename T_TYPE>
#define CSCOPE Simple<T_TYPE>

TEMPT
typename CSCOPE::Type CSCOPE::type() const
{
  return Node::Type::SIMPLE;
}

TEMPT
util::ExtendedTypeRegistry::Id CSCOPE::typeId() const
{
  return util::ExtendedTypeRegistry::getId<T_TYPE>();
}

TEMPT
Node::WriteState CSCOPE::write( T_TYPE )
{
  return Node::WriteState::UNSUPPORTED;
}

TEMPT
Node::ReadState CSCOPE::read( T_TYPE& ) const
{
  return Node::ReadState::UNSUPPORTED;
}

#undef TEMPT
#undef CSCOPE



template<typename T_VALUE_TYPE>
Node::WriteState write(Node& _target, T_VALUE_TYPE _to_write)
{ 
  return write(&_target,_to_write);
};

template<typename T_VALUE_TYPE>
Node::WriteState write(Node::Ptr _target, T_VALUE_TYPE _to_write)
{ 
  assert(_target && "Node::WriteState write(Node::Ptr _target, T_VALUE_TYPE _to_write):: Target was a nullptr!");

  if(!_target)
    return Node::WriteState::FAILED;

  auto target = _target->as< Simple<T_VALUE_TYPE> >();
  auto ret = target->write(_to_write);
  return ret;
};


template<typename T_VALUE_TYPE>
Node::WriteState write(Node* _target, T_VALUE_TYPE _to_write)
{ 
  assert(_target && "Node::WriteState write(Node::Ptr _target, T_VALUE_TYPE _to_write):: Target was a nullptr!");

  if(!_target)
    return Node::WriteState::FAILED;

  auto target = _target->as< Simple<T_VALUE_TYPE> >();
  auto ret = target->write(_to_write);
  return ret;
};

template<typename T_VALUE_TYPE>
Node::WriteState directWrite(Node& _target, T_VALUE_TYPE _to_write)
{
  return directWrite(&_target, _to_write );
}

template<typename T_VALUE_TYPE>
Node::WriteState directWrite(Node::Ptr _target, T_VALUE_TYPE _to_write)
{
  return directWrite(_target.get(), _to_write );
}

template<typename T_VALUE_TYPE>
Node::WriteState directWrite(Node* _target, T_VALUE_TYPE _to_write)
{
  Node::WriteState ret_val = write<T_VALUE_TYPE>(_target,_to_write);

  Node* current = _target;

  while(ret_val == Node::WriteState::STALLED)
  {
    Node* parent = current->parent();
    if( parent==nullptr )
      break;

    ret_val = parent->writeStalled();
    current = parent;
  }
  return ret_val;
}


template<typename T_VALUE_TYPE>
Node::ReadState read(const Node& _target, T_VALUE_TYPE& _to_read)
{
  return read(&_target,_to_read); 
}

template<typename T_VALUE_TYPE>
Node::ReadState read(Node::ConstPtr _target, T_VALUE_TYPE& _to_read)
{ 
  assert(_target && "Node::ReadState read(Node::Ptr _target, T_VALUE_TYPE& _to_read) Target was a nullptr!");

  if(!_target)
    return Node::ReadState::FAILED;
    
  auto target = _target->as< Simple<T_VALUE_TYPE> >();
  return target->read(_to_read);
};

template<typename T_VALUE_TYPE>
Node::ReadState read(const Node* _target, T_VALUE_TYPE& _to_read)
{ 
  assert(_target && "Node::ReadState read(Node::Ptr _target, T_VALUE_TYPE& _to_read) Target was a nullptr!");

  if(!_target)
    return Node::ReadState::FAILED;
    
  assert(dynamic_cast<const Simple<T_VALUE_TYPE>*>(_target) && "Node::ReadState read(Node::Ptr _target, T_VALUE_TYPE _to_read):: _target must be a Simple<T_VALUE_TYPE> type.");
  
  auto target = static_cast< const Simple<T_VALUE_TYPE>* >(_target);
  return target->read(_to_read);
};

template<typename T_VALUE_TYPE>
Node::ReadState directRead(const Node& _target, T_VALUE_TYPE& _to_read)
{
  return directRead(&_target,_to_read);
}

template<typename T_VALUE_TYPE>
Node::ReadState directRead(const Node::Ptr _target, T_VALUE_TYPE& _to_read)
{
  return directRead(_target.get(),_to_read);
}

template<typename T_VALUE_TYPE>
Node::ReadState directRead(const Node* _target, T_VALUE_TYPE& _to_read)
{
  Node* parent = _target->parent();
  while( parent!=nullptr )
  {
    parent->readStalled();
    parent = parent->parent();
  }

  return read<T_VALUE_TYPE>(_target,_to_read);
}



}
}