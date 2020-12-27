/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Thu Dec 14 2017
 */

#ifndef INESS_COMMON_UTIL_STD_MAP_ITERATOR_HPP_
#define INESS_COMMON_UTIL_STD_MAP_ITERATOR_HPP_

namespace iness
{
namespace util
{

/*! These are definitions for templated iterators that allow direct access to second pair members of std::map<...> iterators, that is
 *  they circumvent the need for access through ->first and ->second.
 */


template<typename T_MAP_TYPE, typename T_CRTP_CHILD, typename T_ENCAPSULATED_ITERATOR_TYPE>
class StdMapIteratorBase
{
public:
  StdMapIteratorBase( T_ENCAPSULATED_ITERATOR_TYPE _it ): it_(_it){};

	/*! Advances the iterator by one.
	 */
	T_CRTP_CHILD& operator++(){ ++it_; return static_cast<T_CRTP_CHILD&>(*this); }

	/*! Advances the iterator by one.
	 */
	T_CRTP_CHILD operator++(int){ T_CRTP_CHILD cpy = static_cast<T_CRTP_CHILD&>(*this); ++it_; return cpy; }

	/*! Decrements iterator.
	 */
	T_CRTP_CHILD& operator--(){ --it_; return static_cast<T_CRTP_CHILD&>(*this); }

	//! Decrements iterator.
	T_CRTP_CHILD operator--(int){ T_CRTP_CHILD cpy = static_cast<T_CRTP_CHILD&>(*this); --it_; return cpy; }

	//! Iterator comparison.
	bool operator==(T_CRTP_CHILD _it) const{ return it_==_it.it_;}

	//! Iterator comparison.
	bool operator!=(T_CRTP_CHILD _it) const{ return it_!=_it.it_;}

  //! Accessor for key
  typename T_MAP_TYPE::key_type key() const{ return it_->first; }

protected:
  T_ENCAPSULATED_ITERATOR_TYPE it_; //! Instance of map iterator

};

template<typename T_MAP_TYPE>
class ConstStdMapIterator: public StdMapIteratorBase<T_MAP_TYPE,ConstStdMapIterator<T_MAP_TYPE>,typename T_MAP_TYPE::const_iterator>
{
public:
  ConstStdMapIterator( typename T_MAP_TYPE::const_iterator _it ): StdMapIteratorBase<T_MAP_TYPE,ConstStdMapIterator<T_MAP_TYPE>,typename T_MAP_TYPE::const_iterator>(_it){};

	//! Element access.
	const typename T_MAP_TYPE::mapped_type& operator*() const { return this->it_->second; }

	//! Element access.
	const typename T_MAP_TYPE::mapped_type* operator->() const{ return &this->it_->second; }
};

template<typename T_MAP_TYPE>
class StdMapIterator: public StdMapIteratorBase<T_MAP_TYPE,StdMapIterator<T_MAP_TYPE>,typename T_MAP_TYPE::iterator>
{
public:
  StdMapIterator( typename T_MAP_TYPE::iterator _it ): StdMapIteratorBase<T_MAP_TYPE,StdMapIterator<T_MAP_TYPE>,typename T_MAP_TYPE::iterator>(_it){};

	//! Conversion to ConstIterator
	operator ConstStdMapIterator<T_MAP_TYPE>() const{ return ConstStdMapIterator<T_MAP_TYPE>(T_MAP_TYPE::const_iterator(this->it_) );}

	//! Element access.
	typename T_MAP_TYPE::mapped_type& operator*(){ return this->it_->second; }

	//! Element access.
	typename T_MAP_TYPE::mapped_type* operator->(){ return &this->it_->second; }
};

}
}


#endif // INESS_COMMON_UTIL_STD_MAP_ITERATOR_HPP_