/*
 * $Id: VecN.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  VecN.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2011/04/29.
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

#include "VecN.h"
#include "Vec3.h"
#include "Vec4.h"

float stitch::VecN::abyss_=0.0f;

stitch::VecN stitch::VecN::rand(const size_t numComponents)
{
#ifdef USE_STL_VEC_VECN
    VecN r(numComponents);
#else
    VecN r;
#endif
    
#ifdef USE_STL_VEC_VECN
        const size_t size=r.getNumComponents();
#else
        const size_t size=VECN_SIZE;
#endif
        
        for (size_t i=0; i<size; ++i)
        {
            r[i]=GlobalRand::uniformSampler()*2.0f-1.0f;
        }
        
    return r;
}

stitch::VecN stitch::VecN::randBall(const size_t numComponents)
{
#ifdef USE_STL_VEC_VECN
    VecN r(numComponents);
#else
    VecN r;
#endif
    
    do {
#ifdef USE_STL_VEC_VECN
        const size_t size=r.getNumComponents();
#else
        const size_t size=VECN_SIZE;
#endif
        
        for (size_t i=0; i<size; ++i)
        {
            r[i]=GlobalRand::uniformSampler()*2.0f-1.0f;
        }
        
    } while (r.lengthSq()>1.0);
    
    return r;
}

stitch::VecN stitch::VecN::randNorm(const size_t numComponents)
{
#ifdef USE_STL_VEC_VECN        
    VecN r(numComponents);
#else
    VecN r;
#endif
    
    do {
#ifdef USE_STL_VEC_VECN        
        const size_t size=r.getNumComponents();
#else
        const size_t size=VECN_SIZE;
#endif
        
        for (size_t i=0; i<size; ++i)
        {
            r[i]=GlobalRand::uniformSampler()*2.0f-1.0f;
        }
        
    } while ((r.lengthSq()>1.0)||(r.lengthSq()<0.1));
    
    r.normalise();
    
    return r;
}

//=======================================================================//
stitch::VecN::VecN(const stitch::Vec3 &lValue)
{
#ifdef USE_STL_VEC_VECN        
    c_.push_back(lValue.x());
    c_.push_back(lValue.y());
    c_.push_back(lValue.z());
#else
    c_=new float[VECN_SIZE];
    
    c_[0]=lValue.x();
    c_[1]=lValue.y();
    c_[2]=lValue.z();
    
    for (size_t i=3; i<VECN_SIZE; ++i)
    {
        c_[i]=0.0f;
    }
#endif
}

//=======================================================================//
stitch::VecN & stitch::VecN::operator = (const stitch::Vec3 &lValue)
{
#ifdef USE_STL_VEC_VECN        
    c_.clear();
    
    c_.push_back(lValue.x());
    c_.push_back(lValue.y());
    c_.push_back(lValue.z());
#else
    c_=new float[VECN_SIZE];
    
    c_[0]=lValue.x();
    c_[1]=lValue.y();
    c_[2]=lValue.z();

    for (size_t i=3; i<VECN_SIZE; ++i)
    {
        c_[i]=0.0f;
    }
#endif
    
    return (*this);
}

//=======================================================================//
stitch::VecN::VecN(const stitch::Vec4 &lValue)
{
#ifdef USE_STL_VEC_VECN        
    c_.push_back(lValue.x());
    c_.push_back(lValue.y());
    c_.push_back(lValue.z());
    c_.push_back(lValue.w());
#else
    c_=new float[VECN_SIZE];
    
    c_[0]=lValue.x();
    c_[1]=lValue.y();
    c_[2]=lValue.z();
    c_[3]=lValue.w();

    for (size_t i=4; i<VECN_SIZE; ++i)
    {
        c_[i]=0.0f;
    }
#endif
}

//=======================================================================//
stitch::VecN & stitch::VecN::operator = (const stitch::Vec4 &lValue)
{
#ifdef USE_STL_VEC_VECN        
    c_.clear();
    
    c_.push_back(lValue.x());
    c_.push_back(lValue.y());
    c_.push_back(lValue.z());
    c_.push_back(lValue.w());
#else
    c_=new float[VECN_SIZE];
    
    c_[0]=lValue.x();
    c_[1]=lValue.y();
    c_[2]=lValue.z();
    c_[3]=lValue.w();
    
    for (size_t i=4; i<VECN_SIZE; ++i)
    {
        c_[i]=0.0f;
    }
#endif
    
    return (*this);
}

