/**
 * @file
 * @author  Clay Hughes   <>, (C) 2008, 2009, 2010
 * @date    10/03/08
 * @brief   Helper functions.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * 
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PARAM_FUNCS_H
#define PARAM_FUNCS_H

#include <ctime>
#include <boost/random.hpp>

#include "param_types.h"

/**
 * @name getRDTSC
 * 
 * @note Read Time-Stamp Counter (not portable but...meh)
 * @param  
 * @return Clock value
 */
inline UINT_64 getRDTSC(void)
{
   UINT_32 eax_r, edx_r;
   __asm__ __volatile__("rdtsc" :"=a" (eax_r), "=d" (edx_r));

   return ((UINT_64)eax_r) | (((UINT_64)edx_r) << 32);
}

/**
 * @name uniformIntRV
 * @brief RV over the range [min, max)
 * 
 * @param min 
 * @param max 
 * @return Unsigned int
 */
inline UINT_32 uniformIntRV(UINT_32 min, UINT_32 max)
{
   static boost::lagged_fibonacci1279 generator(static_cast<unsigned> (getRDTSC()));
//    static boost::lagged_fibonacci1279 generator(static_cast<unsigned> (42));
   boost::uniform_int<> uniformDistribution(min, max);
   boost::variate_generator<boost::lagged_fibonacci1279&, boost::uniform_int<> >  randomVariable(generator, uniformDistribution);

   return randomVariable();
}

/**
 * @name uniformNormRV
 * @brief Uniform RV [0,1)
 * 
 * @param  
 * @return Double
 */
inline double uniformNormRV(void)
{
   static boost::lagged_fibonacci607 generator(static_cast<unsigned> (getRDTSC()));
//    static boost::lagged_fibonacci607 generator(static_cast<unsigned> (42));
   boost::uniform_real<> uniformDistribution(0, 1);
   boost::variate_generator<boost::lagged_fibonacci607&, boost::uniform_real<double> > randomVariable(generator, uniformDistribution);

   return randomVariable();
}

/**
 * @name roundFloat
 * 
 * @param x 
 * @return x rounded to nearest unsigned
 */
inline UINT_64 roundFloat(double x)
{
   if(x > 0.0)
      x = x + 0.5;
   else
      x = x - 0.5;

   return static_cast<int>(x);
}

/**
 * @name roundFloat_b
 * 
 * @param x 
 * @return x rounded to nearest unsigned
 */
inline UINT_64 roundFloat_b(double x)
{
   if(x > 0.0)
      x = x + 0.0001;

   return static_cast<int>(x);
}
#endif
