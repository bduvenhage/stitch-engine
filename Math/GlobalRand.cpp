/*
 * $Id: GlobalRand.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  GlobalRand.cpp
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

#include "GlobalRand.h"

#ifdef USE_CXX11
std::random_device stitch::GlobalRand::rndDev;
std::mt19937_64 stitch::GlobalRand::rndGen(/*rndDev()*/1);
std::uniform_real_distribution<float> stitch::GlobalRand::uniformDist(0.0, 1.0);//Inclusive range.
std::normal_distribution<float> stitch::GlobalRand::normalDist(0.0, 1.0);//Mean, standard deviation.
#else
boost::mt19937 stitch::GlobalRand::rndGen(1);
boost::uniform_real<float> stitch::GlobalRand::uniformDist(0.0, 1.0);
boost::normal_distribution<float> stitch::GlobalRand::normalDist(0.0, 1.0);
boost::variate_generator<boost::mt19937&, boost::uniform_real<float> >  stitch::GlobalRand::uniformSampler(stitch::GlobalRand::rndGen, stitch::GlobalRand::uniformDist);
boost::variate_generator<boost::mt19937&, boost::normal_distribution<float> >  stitch::GlobalRand::normalSampler(stitch::GlobalRand::rndGen, stitch::GlobalRand::normalDist);
#endif

//=====================

#define UNIFORM_SAMPLER_ARRAY_SIZE 100000000
float uniformSamplerArray[UNIFORM_SAMPLER_ARRAY_SIZE];
size_t uniformSamplerArrayIndex=0;

float stitch::GlobalRand::uniformSamplerFromArray()
{
    ++uniformSamplerArrayIndex;
    
    if (uniformSamplerArrayIndex>=UNIFORM_SAMPLER_ARRAY_SIZE) uniformSamplerArrayIndex=0;
    
    return uniformSamplerArray[uniformSamplerArrayIndex];
}

void stitch::GlobalRand::initialiseUniformSamplerArray()
{
    for (size_t i=0; i<UNIFORM_SAMPLER_ARRAY_SIZE; ++i)
    {
        uniformSamplerArray[i]=stitch::GlobalRand::uniformSampler();
    }
}
