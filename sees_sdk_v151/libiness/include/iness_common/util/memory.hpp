/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Mon Sep 11 2017
 */

#ifndef INESS_COMMON_UTIL_MEMORY_HPP_
#define INESS_COMMON_UTIL_MEMORY_HPP_

#include <memory>

namespace iness
{

/*! Iness make_shared implementation: Actually uses the classes' new operator and is thus Eigen-safe
 *  (as opposed to the one in the std library which isn't)
 */
template<typename T_CLASS_TYPE, typename ... T_VAR_ARGS>
std::shared_ptr<T_CLASS_TYPE> make_shared( T_VAR_ARGS ... _var_args )
{
  return std::shared_ptr<T_CLASS_TYPE>( new T_CLASS_TYPE(_var_args...) );
}

/*! Iness make_unique implementation
 */
template<typename T_CLASS_TYPE, typename ... T_VAR_ARGS>
std::unique_ptr<T_CLASS_TYPE> make_unique( T_VAR_ARGS ... _var_args )
{
  std::unique_ptr<T_CLASS_TYPE>( new T_CLASS_TYPE(_var_args...) );
}

}


#endif // INESS_COMMON_UTIL_MEMORY_HPP_