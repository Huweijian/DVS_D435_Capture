/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Fri Dec 15 2017
 */

#ifndef INESS_COMMON_UTIL_TYPE_REGISTRY_HPP_
#define INESS_COMMON_UTIL_TYPE_REGISTRY_HPP_

#include <tuple>
#include <type_traits>
#include <string>
#include <cstdint>
#include <vector>
#include <functional>

namespace iness
{
namespace util
{


/*! Provides compile time type information (ids) on a fixed, configurable set of types and functionality to instantiate and call templated objects automatically with the registered
 *  types.
 */
template<typename ... T_SUPPORTED_TYPES>
class TypeRegistry
{

public:
  typedef std::size_t Id;
  typedef std::tuple<T_SUPPORTED_TYPES...> type_list;

public:

  //! Returns a unique type id for each registered type (note that if the same type is registered twice, both will get the same id)
  template<typename T_QUERY_TYPE>
  static std::size_t getId();

};


template<typename ... T_TYPE_LIST>
struct TypeList
{
  typedef std::tuple<T_TYPE_LIST...> type_list;
};

typedef TypeRegistry<std::string, bool, float, double, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, std::vector<std::function<void()>> > ExtendedTypeRegistry;
typedef TypeList<std::string> StringType;
typedef TypeList<bool, float, double, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t> NumericTypes;
typedef TypeList<std::string,bool, float, double, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t> StdStreamSupportedTypes;



}
}

#endif // INESS_COMMON_UTIL_TYPE_REGISTRY_HPP_