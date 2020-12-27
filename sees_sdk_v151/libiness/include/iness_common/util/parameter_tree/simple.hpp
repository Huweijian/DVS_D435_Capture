/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Thu Dec 14 2017
 */

#ifndef INESS_COMMON_UTIL_PARAMETER_TREE_SIMPLE_PARAMETER_HPP_
#define INESS_COMMON_UTIL_PARAMETER_TREE_SIMPLE_PARAMETER_HPP_

#include "iness_common/util/parameter_tree/node.hpp"
#include "iness_common/util/type_registry.hpp"

#include <iostream>
#include <fstream>

namespace iness
{
namespace ptree
{

class SimpleBase: public Node
{
public:
  typedef std::shared_ptr<SimpleBase> Ptr;
  typedef std::shared_ptr<const SimpleBase> ConstPtr;

public:
  virtual ~SimpleBase() = default;

  //! Returns the type id of the encapsulated parameter.
  virtual util::ExtendedTypeRegistry::Id typeId() const =0;

protected:
  SimpleBase() = default;
};

/*! Template definition for simple parameter types */
template<typename T_TYPE>
class Simple: public SimpleBase
{
public:
  typedef std::shared_ptr<Simple<T_TYPE>> Ptr;
  typedef std::shared_ptr<const Simple<T_TYPE>> ConstPtr;

public:
  virtual ~Simple() = default;

  //! Returns information about the parameter type
  virtual Type type() const override;

  //! Returns a type identifier for the given templated type
  util::ExtendedTypeRegistry::Id typeId() const override;

  /*! Write parameter
   * @param _to_write Value that is to be written
   */
  virtual Node::WriteState write( T_TYPE _to_write );

  /*! Read parameter
   * @param _to_read[out] Where the value will be stored to
   */
  virtual Node::ReadState read( T_TYPE& _to_read ) const;

protected:
  Simple() = default;
};

//! Convenience function to write to a parameter node of type SIMPLE
/*! Might end up with a stalled write */
template<typename T_VALUE_TYPE>
Node::WriteState write(Node& _target, T_VALUE_TYPE _to_write);

//! Convenience function to write to a parameter node of type SIMPLE
/*! Might end up with a stalled write */
template<typename T_VALUE_TYPE>
Node::WriteState write(Node::Ptr _target, T_VALUE_TYPE _to_write);

//! Convenience function to write to a parameter node of type SIMPLE
/*! Might end up with a stalled write */
template<typename T_VALUE_TYPE>
Node::WriteState write(Node* _target, T_VALUE_TYPE _to_write);

//! Convenience function to write to a parameter node of type SIMPLE, without stalling.
/*! Will call stalled writes on parent nodes where necessary to actually write the data */
template<typename T_VALUE_TYPE>
Node::WriteState directWrite(Node& _target, T_VALUE_TYPE _to_write);

//! Convenience function to write to a parameter node of type SIMPLE, without stalling.
/*! Will call stalled writes on parent nodes where necessary to actually write the data */
template<typename T_VALUE_TYPE>
Node::WriteState directWrite(Node::Ptr _target, T_VALUE_TYPE _to_write);

//! Convenience function to write to a parameter node of type SIMPLE, without stalling.
/*! Will call stalled writes on parent nodes where necessary to actually write the data */
template<typename T_VALUE_TYPE>
Node::WriteState directWrite(Node* _target, T_VALUE_TYPE _to_write);

//! Convenience function to read from a parameter node of type SIMPLE
/*! Might end up with a stalled read */
template<typename T_VALUE_TYPE>
Node::ReadState read(const Node& _target, T_VALUE_TYPE& _to_read);

//! Convenience function to read from a parameter node of type SIMPLE
/*! Might end up with a stalled read */
template<typename T_VALUE_TYPE>
Node::ReadState read(Node::ConstPtr _target, T_VALUE_TYPE& _to_read);

//! Convenience function to read from a parameter node of type SIMPLE
/*! Might end up with a stalled read */
template<typename T_VALUE_TYPE>
Node::ReadState read(const Node* _target, T_VALUE_TYPE& _to_read);

//! Convenience function to read from a parameter node of type SIMPLE, without stalling
/*! Will call stalled reads on parent nodes before attempting to read the data */
template<typename T_VALUE_TYPE>
Node::ReadState directRead(const Node& _target, T_VALUE_TYPE& _to_read);

//! Convenience function to read from a parameter node of type SIMPLE, without stalling
/*! Will call stalled reads on parent nodes before attempting to read the data */
template<typename T_VALUE_TYPE>
Node::ReadState directRead(const Node::Ptr _target, T_VALUE_TYPE& _to_read);

//! Convenience function to read from a parameter node of type SIMPLE, without stalling
/*! Will call stalled reads on parent nodes before attempting to read the data */
template<typename T_VALUE_TYPE>
Node::ReadState directRead(const Node* _target, T_VALUE_TYPE& _to_read);


}
}

#include "impl/simple.inl"


#endif // INESS_COMMON_UTIL_PARAMETER_TREE_SIMPLE_PARAMETER_HPP_