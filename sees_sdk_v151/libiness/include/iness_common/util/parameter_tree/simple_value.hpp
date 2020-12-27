/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Thu Feb 08 2018
 */

#ifndef INESS_COMMON_UTIL_PARAMETER_TREE_SIMPLE_PARAMETER_VALUE_HPP_
#define INESS_COMMON_UTIL_PARAMETER_TREE_SIMPLE_PARAMETER_VALUE_HPP_

#include "iness_common/util/parameter_tree/simple.hpp"
#include "iness_common/util/memory.hpp"

namespace iness
{
namespace ptree
{

//! Template definition for a simple parameter tree node that holds its own value 
template<typename T_TYPE>
class SimpleValue: public Simple<T_TYPE>
{
public:
  typedef std::shared_ptr< SimpleValue<T_TYPE> > Ptr;
  typedef std::shared_ptr< const SimpleValue<T_TYPE> > ConstPtr;

public:
  /*! Creates a new SimpleValue
   * @param _init_val Initial value of the hold member.
   * @param _rw_type Read/Write access type.
   * @param _support_on_change_callback Whether the node should support on change callbacks or not [default:true]
   */
  static Node::Ptr create( T_TYPE _init_val = T_TYPE(), Node::RwType _rw_type = Node::RwType::READ_AND_WRITE, bool _support_on_change_callback = true );

  //! Returns information about the parameter type
  virtual Node::Type type() const override;

  /*! @brief Write parameter
   * @param _to_write Value that is to be written
   */
  virtual Node::WriteState write( T_TYPE _to_write ) override;

  /*! @brief Read parameter
   * @param _to_read[out] Where the value will be stored to
   */
  virtual Node::ReadState read( T_TYPE& _to_read ) const override;

  //! Returns a reference to the represented value
  virtual T_TYPE& value();

  //! Returns the current read state
  virtual Node::ReadState getReadState() const override;

  //! Returns the current write state
  virtual Node::WriteState getWriteState() const override;

protected:
  /*! Creates a new SimpleValue
   * @param _init_val Initial value of the hold member.
   * @param _rw_type Read/Write access type.
   * @param _support_on_change_callback Whether the node should support on change callbacks or not [default:true]
   */
  SimpleValue( T_TYPE _init_val = T_TYPE(), Node::RwType _rw_type = Node::RwType::READ_AND_WRITE, bool _support_on_change_callback = true );
  
  friend Ptr make_shared<SimpleValue<T_TYPE>>();
  friend Ptr make_shared<SimpleValue<T_TYPE>>(T_TYPE);
  friend Ptr make_shared<SimpleValue<T_TYPE>>(T_TYPE, Node::RwType);
  friend Ptr make_shared<SimpleValue<T_TYPE>>(T_TYPE, Node::RwType, bool);
  

private:
  T_TYPE value_; //!< Member
};

}
}

#include "impl/simple_value.inl"

#endif // INESS_COMMON_UTIL_PARAMETER_TREE_SIMPLE_PARAMETER_VALUE_HPP_