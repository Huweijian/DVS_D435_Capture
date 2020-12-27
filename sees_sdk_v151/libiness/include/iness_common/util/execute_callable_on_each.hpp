/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Wed Dec 19 2018
 */

#ifndef INESS_COMMON_UTIL_EXECUTE_CALLABLE_ON_EACH_HPP_
#define INESS_COMMON_UTIL_EXECUTE_CALLABLE_ON_EACH_HPP_

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


/*! Allows automatically calling the call function of the templated callable object
  * with given arguments, the functor being
  * instantiated with all types in T_CALL_TYPE_SET. The callable object T_CALLABLE must
  * feature a static call function. T_CALL_TYPE_SET must have a tuple member named type_list
  * 
  * This means:
  *  - You create a callable object:
  * 
  *  template<typename TYPE>
  *  struct Callable
  *  {
  *    static void call(*any arguments you like*){};
  *  };
  * 
  *  - And if you do:
  *    ExtendedTypeRegistry::ExecuteCallableOnEach<Callable>::call(*your arguments*);
  * 
  *  - The template will instantiate function calls to the call function as follows:
  *    Callable<std::string>::call(*your arguments*);
  *    Callable<bool>::call(*your arguments*);
  *    Callable<float>::call(*your arguments*);
  *    ... and all other types stored within ExtendedTypeRegistry
  * 
  * */
template<template <typename> class T_CALLABLE, typename T_CALL_TYPE_SET >
struct ExecuteCallableOnEach
{
  template<typename ... T_VAR_ARGS>
  static void call( T_VAR_ARGS ... _func_obj_constructor_args );
};

template<template <typename> class T_TEMPLATE_FUNCTION_TYPE, typename T_CALL_TYPE_SET, std::size_t T_POS>
struct ExecuteCallableOnEachAt
{
  template<typename ... T_VAR_ARGS>
  static void call( T_VAR_ARGS ... _func_obj_constructor_args );
};

template<template <typename> class T_TEMPLATE_FUNCTION_TYPE, typename T_CALL_TYPE_SET >
struct ExecuteCallableOnEachAt<T_TEMPLATE_FUNCTION_TYPE, T_CALL_TYPE_SET, std::tuple_size<typename T_CALL_TYPE_SET::type_list>::value >
{
  template<typename ... T_VAR_ARGS>
  static void call( T_VAR_ARGS ... );
};

}
}

#include "impl/execute_callable_on_each.inl"

#endif // INESS_COMMON_UTIL_EXECUTE_CALLABLE_ON_EACH_HPP_