/*
 * $Id: GlobalRand.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  GlobalRand.h
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/07/01.
 *  Copyright $Date: 2015-03-25 15:00:40 +0200 (Wed, 25 Mar 2015) $ Bernardt Duvenhage. All rights reserved.
 *
 *
 *  This file is part of StitchEngine.
 
 *  StitchEngine is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 
 *  StitchEngine is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with StitchEngine.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef STITCH_GLOBAL_RAND_H
#define STITCH_GLOBAL_RAND_H


namespace stitch
{
	class GlobalRand;
}

#ifdef USE_CXX11
#include <random>
#else
#include <boost/random.hpp>
#define nullptr 0
#endif

namespace stitch
{
    //! Wrapper class for static random number generators.
	class GlobalRand
	{
	public:
#ifdef USE_CXX11
        static std::random_device rndDev;
		static std::mt19937_64 rndGen;
        
        static std::uniform_real_distribution<float> uniformDist;
		static std::normal_distribution<float> normalDist;
        
        //!Return a uniform float number in [0..1)
        inline static float uniformSampler()
        {
            return uniformDist(rndGen);
        }
        
        //!Return a normal distributed float with mean 0.0 and standard deviation 1.0.
        inline static float normalSampler()
        {
            return normalDist(rndGen);
        }
#else
        static boost::mt19937 rndGen;
        
        static boost::uniform_real<float> uniformDist;
        static boost::normal_distribution<float> normalDist;
        
        static boost::variate_generator<boost::mt19937&, boost::uniform_real<float> >  uniformSampler;
        static boost::variate_generator<boost::mt19937&, boost::normal_distribution<float> >  normalSampler;
                
#endif
        
        static float uniformSamplerFromArray();
        static void initialiseUniformSamplerArray();
	};
	
}

#endif// STITCH_GLOBAL_RAND_H
