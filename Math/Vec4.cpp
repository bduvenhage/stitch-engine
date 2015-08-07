/*
 * $Id: Vec4.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Vec4.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2009/09/17.
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

#include "Vec4.h"
#include "Vec3.h"
#include "VecN.h"

float stitch::Vec4::abyss_=0.0f;

stitch::Vec4 stitch::Vec4::allZeros()
{
    return Vec4();//Zeroed in constructor;
}

stitch::Vec4 stitch::Vec4::allOnes()
{
    return Vec4(1.0f);
}

stitch::Vec4 stitch::Vec4::rand()
{
    return Vec4(GlobalRand::uniformSampler()*2.0f-1.0f,
                GlobalRand::uniformSampler()*2.0f-1.0f,
                GlobalRand::uniformSampler()*2.0f-1.0f,
                GlobalRand::uniformSampler()*2.0f-1.0f);
}

stitch::Vec4 stitch::Vec4::randBall()
{
    Vec4 r;
    
    do {
        r=Vec4(GlobalRand::uniformSampler()*2.0f-1.0f,
               GlobalRand::uniformSampler()*2.0f-1.0f,
               GlobalRand::uniformSampler()*2.0f-1.0f,
               GlobalRand::uniformSampler()*2.0f-1.0f);
        
    } while (r.lengthSq()>1.0);
    
    return r;
}

stitch::Vec4 stitch::Vec4::randNorm()
{
    Vec4 r;
    
    do {
        r=Vec4(GlobalRand::uniformSampler()*2.0f-1.0f,
               GlobalRand::uniformSampler()*2.0f-1.0f,
               GlobalRand::uniformSampler()*2.0f-1.0f,
               GlobalRand::uniformSampler()*2.0f-1.0f);
        
    } while ((r.lengthSq()>1.0)||(r.lengthSq()<0.1));
    
    r.normalise();
    
    return r;
}

//=======================================================================//
stitch::Vec4::Vec4(const stitch::Vec3 &lValue, float w)
{
    x_=lValue.x();
    y_=lValue.y();
    z_=lValue.z();
    w_=w;
}

//=======================================================================//
stitch::Vec4::Vec4(const stitch::Vec3 &lValue)
{
    x_=lValue.x();
    y_=lValue.y();
    z_=lValue.z();
    w_=0.0f;
}

//=======================================================================//
stitch::Vec4 & stitch::Vec4::operator = (const stitch::Vec3 &lValue)
{
    x_=lValue.x();
    y_=lValue.y();
    z_=lValue.z();
    w_=0.0f;
    return (*this);
}


//=======================================================================//
stitch::Vec4::Vec4(const stitch::VecN &lValue)
{
    x_=lValue.x();
    y_=lValue.y();
    z_=lValue.z();
    w_=lValue.w();
}

//=======================================================================//
stitch::Vec4 & stitch::Vec4::operator = (const stitch::VecN &lValue)
{
    x_=lValue.x();
    y_=lValue.y();
    z_=lValue.z();
    w_=lValue.w();
    return (*this);
}


