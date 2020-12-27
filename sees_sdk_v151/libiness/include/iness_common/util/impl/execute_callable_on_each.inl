/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Tue Feb 20 2018
 */


namespace iness
{
namespace util
{

template<template <typename> class T_CALLABLE, typename T_CALL_TYPE_SET >
template<typename ... T_VAR_ARGS>
void ExecuteCallableOnEach<T_CALLABLE,T_CALL_TYPE_SET>::call( T_VAR_ARGS ... _func_obj_constructor_args )
{
  ExecuteCallableOnEachAt<T_CALLABLE, T_CALL_TYPE_SET ,0>::call(_func_obj_constructor_args...);
  return;
}

template<template <typename> class T_TEMPLATE_FUNCTION_TYPE, typename T_CALL_TYPE_SET, std::size_t T_POS>
template<typename ... T_VAR_ARGS>
void ExecuteCallableOnEachAt<T_TEMPLATE_FUNCTION_TYPE,T_CALL_TYPE_SET,T_POS>::call( T_VAR_ARGS ... _func_obj_constructor_args )
{
  typedef typename std::tuple_element<T_POS, typename T_CALL_TYPE_SET::type_list >::type CurrentSwitchType;

  T_TEMPLATE_FUNCTION_TYPE<CurrentSwitchType>::call(_func_obj_constructor_args...);
  
  return ExecuteCallableOnEachAt<T_TEMPLATE_FUNCTION_TYPE,T_CALL_TYPE_SET,T_POS+1>::call(_func_obj_constructor_args...);
}

template<template <typename> class T_TEMPLATE_FUNCTION_TYPE, typename T_CALL_TYPE_SET >
template<typename ... T_VAR_ARGS>
void ExecuteCallableOnEachAt<T_TEMPLATE_FUNCTION_TYPE, T_CALL_TYPE_SET, std::tuple_size<typename T_CALL_TYPE_SET::type_list>::value >::call( T_VAR_ARGS ... )
{
  return;
}


}
}