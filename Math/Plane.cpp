/*
 * $Id: Plane.cpp 299 2015-08-07 14:57:10Z bernardt.duvenhage $
 */
/*
 *  Plane.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2012/12/05.
 *  Copyright $Date: 2015-08-07 16:57:10 +0200 (Fri, 07 Aug 2015) $ Bernardt Duvenhage. All rights reserved.
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

#include "Plane.h"

stitch::Plane::Plane(const stitch::Vec3 &vert0, const Vec3 &vert1, const Vec3 &vert2)
{
    normal_=stitch::Vec3::crossNormalised(vert1, vert2, vert0);
    d_=vert0*normal_;
}

stitch::Plane::Plane(const Vec3 &normal, const float d) :
normal_(normal),
d_(d)
{}

stitch::Plane::Plane(const Vec3 &normal, const Vec3 &point) :
normal_(normal),
d_(normal*point)
{
}

stitch::Plane::Plane(const Plane &lValue) :
normal_(lValue.normal_),
d_(lValue.d_)
{}

#ifdef USE_CXX11
stitch::Plane::Plane(Plane &&rValue) noexcept:
normal_(std::move(rValue.normal_)),
d_(rValue.d_)
{}
#endif //USE_CXX11

stitch::Plane::~Plane()
{}

stitch::Plane &stitch::Plane::operator=(const Plane &lValue)
{
    normal_=lValue.normal_;
    d_=lValue.d_;
    
    return (*this);
}

#ifdef USE_CXX11
stitch::Plane &stitch::Plane::operator=(Plane &&rValue) noexcept
{
    normal_=std::move(rValue.normal_);
    d_=rValue.d_;
    
    return (*this);
}
#endif //USE_CXX11

float stitch::Plane::calcDistToPoint(const Vec3 &point) const
{
    return point*normal_-d_;
}
