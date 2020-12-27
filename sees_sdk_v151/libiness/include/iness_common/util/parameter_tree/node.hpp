/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Wed Dec 13 2017
 */

#ifndef INESS_COMMON_UTIL_PARAMETER_HPP_
#define INESS_COMMON_UTIL_PARAMETER_HPP_

#include <memory>
#include <map>
#include <cassert>
#include <functional>
#include "iness_common/util/std_map_iterator.hpp"
#include "iness_common/util/memory.hpp"

namespace iness
{
namespace ptree
{

/*! @brief Main class of our parameter tree functionality.
 *  It defines the main interfaces and provides the functionality to build up and iterate through
 *  parameter trees. Every node in the tree is derived from it.
 */
class Node: public std::enable_shared_from_this<Node>
{
public:
  //! Used to provide type information to figure out at runtime to which type a cast is possible
  enum class Type
  {
    FANCY, //!< Everything but...
    SIMPLE, //!< Can be casted to SimpleBase
    VECTOR //!< Can be casted to SimpleVectorBase
  };

  //! Indicates the read/write capabilites the parameter supports
  enum struct RwType
  {
    READ_AND_WRITE, //!< Parameter can be read and written.
    READ_ONLY, //!< Parameter is read only.
    WRITE_ONLY //!< Parameter is write only.
  };

  //! Provides state information about reading
  enum class ReadState
  {
    SUCCESSFUL = 0, //!< Reading was successful.
    STALLED=1, //!< The value that was attempted to be read couldn't be read on that level of the tree, instead the readStalled method of a parent node must be called. It is expected that if readStalled succeeded exactly one call to read will be successful (or fail) and that for a second call to read readStalled has to be called again. The first call after the last call to readStalled() on the parent must never return STALLED.
    FAILED = 2, //!< Reading was not successful
    UNSUPPORTED = 3 //!< Node doesn't support reading.
  };

  //! Provides state information about writing
  enum class WriteState
  {
    SUCCESSFUL = 0, //!< Writing was successful
    STALLED = 1, //!< The value that was attempted to be written was not written through, but stalled along the way. To write it through, the writeStalled method likely needs to be called on a parent in the tree.
    FAILED = 2, //!< Writing failed
    UNSUPPORTED = 3 //!< Writing is not supported by the parameter
  };

  typedef std::shared_ptr<Node> Ptr;
  typedef std::shared_ptr<const Node> ConstPtr;
  typedef util::StdMapIterator< std::map<unsigned int,Ptr> > Iterator;
  typedef util::ConstStdMapIterator< std::map<unsigned int,Ptr> > ConstIterator;

public:

  //! Factory function for a Node::Ptr
  static Node::Ptr create(RwType _rw_type = RwType::READ_AND_WRITE);
  
  //! Copy assignment operator
  Node& operator=(const Node&);

  //! Destructor
  virtual ~Node();

  //! Casts the node to the given type (The validity of the cast is only checked in debug mode).
  template<typename T_TARGET_TYPE>
  std::shared_ptr<T_TARGET_TYPE> as();

  //! Casts the node to the given type (The validity of the cast is only checked in debug mode).
  template<typename T_TARGET_TYPE>
  std::shared_ptr<const T_TARGET_TYPE> as() const;

  //! Returns shared ptr to the node itself
  Ptr self();

  //! Returns shared ptr to the node itself
  ConstPtr self() const;

  //! Returns a reference to the member. If not yet existing, an entry of type Node will be created and returned.
  Node& operator[]( std::string _key );

  //! Returns a constant reference to the member. If not yet existing, an std::invalid_argument exception will be thrown.
  const Node& operator[]( std::string _key ) const;

  //! Returns a pointer to the entry if it exists, a nullptr otherwise */
  Ptr entry( std::string _key );

  //! Returns a pointer to the entry if it exists, a nullptr otherwise */
  ConstPtr entry( std::string _key ) const;

  //! Returns whether an entry exist
  bool hasEntry( std::string _key ) const;

  //! Adds a new child parameter to the parameter: Returns true if successful
  bool addEntry( std::string _key, Ptr _new_entry = create() );

  //! After this call no new entries can be added to the node and all its childs
  void lockMapping();

  //! Returns information about the parameter type
  virtual Type type() const;

  //! Returns the current read state
  virtual ReadState getReadState() const;

  //! Returns the current write state
  virtual WriteState getWriteState() const;

  //! Attempts to write stalled data through for all childs, returns successful if all stalls were resolved.
  virtual WriteState writeStalled();
  
  //! Attempts to read values of child nodes whose reading was stalled.
  virtual ReadState readStalled() const;

  //! Read the parameter name
  std::string name() const;

  //! Returns additional properties of the parameter
  /*! The following keys are defined and should be used according to the definition (Their existence is not mandatory though):
   *  - Description [std::string] Describes in text what the parameter is.
   *  - NumberTextFormat [std::string] Can be used to define in what format a number is to be written to text. Defined choices: default, hexadecimal, 
   *  - HexNumberBytes [unsigned int] Number of types used to display hex as hex number
   *  - SupportsOnChangeCallback [bool] If defined the parameter support the OnChangeCallback member
   *  - OnChangeCallback [std::vector<std::function<void()>>] Can be used to register functions that will be called after every write operation. Only supported if 'SupportsOnChangeCallback' is defined.
   *  - ReadableName [std::string] Short human readable name for the parameter, to be used on GUI's etc
   *  - Limits [ParameterNode] must have two childs : "Min" and "Max" [T_VALUE_TYPE] Defines allowed range for the parameter
   *  - StepSize [T_VALUE_TYPE] If GUI provides stepping functionality, this suggests a step size
   */
  Ptr properties();

  //! Adds a property to the tree's first level (convenience function)
  /*! @return True if property was successfully added
   */
  template<typename T_INFO_TYPE>
  bool addPropertyEntry( std::string _name, T_INFO_TYPE _info_element );

  //! Convenience function to check if a property entry exists
  bool hasPropertyEntry( std::string _name ) const;

  //! Convenience function to retrieve property entries of known type.
  /*! The existence of the entry is not checked, attempting to access a non existing entry will result in a runtime error.
   *  Use hasPropertyEntry() to check its existence first.
   */
  template<typename T_INFO_TYPE>
  T_INFO_TYPE getPropertyEntry( std::string _name ) const;

  //! Returns the number of child nodes.
  size_t nrOfChildNodes() const;

  //! Returns an iterator for child parameter nodes (ordered by creation time)
  Iterator begin();

  //! Returns an end iterator for child parameter nodes (ordered by creation time)
  Iterator end();

  //! Returns a const iterator for child parameter nodes (ordered by creation time)
  ConstIterator begin() const;

  //! Returns a const end iterator for child parameter nodes (ordered by creation time)
  ConstIterator end() const;
  
  //! Parent accessor
  Node* parent() const;

  //! Returns the read/write capabilities of the parameter.
  RwType rwType() const;

  //! Convenience function to add onChange callback function
  void addOnChangeCallback( std::function<void()> _func );

  //! Convencience function that calls all registered on change callback functions, if any.
  void callOnChangeCallbacks();

protected:
  //! Constructor
  Node(RwType _rw_type = RwType::READ_AND_WRITE);

  //! Copy constructor
  Node( const Node& );

  friend Ptr make_shared<Node>();
  friend Ptr make_shared<Node>(RwType);

  //! Gives childs access to alter the RwType
  RwType& rwTypeRef();


private:
  //! Returns the info pointer, initializes it if not yet initialized
  Node::Ptr getPropertyTree();

  //! Returns the info pointer, initializes it if not yet initialized
  Node::ConstPtr getPropertyTree() const;

  //! Sets the parameter name
  void setName( std::string _name );

  //! Sets the parameter parent
  void setParent( Node* _parent );

private:
  struct Members;
  std::unique_ptr<Members> members_; //!< Holds all private members of the type.
};

}
}

#include "impl/node.inl"

#endif // INESS_COMMON_UTIL_PARAMETER_HPP_
