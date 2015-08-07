/*
 * $Id: Camera.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Camera.cpp
 *  StitchEngine
 *
 *  Created by Bernardt Duvenhage on 2010/03/27.
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

#include "Camera.h"


//=======================================================================//
stitch::Camera::Camera(const stitch::Vec3 &i_position, const stitch::Vec3 &i_lookAt, const stitch::Vec3 &i_up) :
m_position_(i_position), m_up_(i_up)
{
    Vec3 lookDirection=i_lookAt-m_position_;
    
    m_right_=lookDirection^m_up_;
    m_up_=m_right_^lookDirection;
    m_backward_=lookDirection*(-1.0f);
    
    m_right_=m_right_.normalised();
    m_backward_=m_backward_.normalised();
    m_up_=m_up_.normalised();
}

//=======================================================================//
stitch::Camera::Camera(const Camera &lValue) :
m_position_(lValue.m_position_),
m_backward_(lValue.m_backward_),
m_up_(lValue.m_up_),
m_right_(lValue.m_right_)
{}

//=======================================================================//
stitch::Camera & stitch::Camera::operator=(const Camera &lValue)
{
    m_position_=lValue.m_position_;
    m_backward_=lValue.m_backward_;
    m_up_=lValue.m_up_;
    m_right_=lValue.m_right_;
    
    return (*this);
}


//=======================================================================//
bool stitch::Camera::operator == (const Camera &lValue) const
{
    return (m_position_ == lValue.m_position_)&&(m_backward_ == lValue.m_backward_);
}

//=======================================================================//
bool stitch::Camera::operator != (const Camera &lValue) const
{
    return !((*this)==lValue);
}


//=======================================================================//
stitch::SimplePinholeCamera::SimplePinholeCamera(const stitch::Vec3 &i_position, const stitch::Vec3 &i_lookAt, const stitch::Vec3 &i_up) :
Camera(i_position, i_lookAt, i_up), m_filmDist_(1.46f)
{}

//=======================================================================//
stitch::SimplePinholeCamera::SimplePinholeCamera(const SimplePinholeCamera &lValue) :
Camera(lValue),
m_filmDist_(lValue.m_filmDist_)
{}

//=======================================================================//
stitch::SimplePinholeCamera & stitch::SimplePinholeCamera::operator=(const SimplePinholeCamera &lValue)
{
    Camera::operator=(lValue);
    
    m_filmDist_=lValue.m_filmDist_;
    
    return (*this);
}

//=======================================================================//
stitch::Vec3 stitch::SimplePinholeCamera::getFocalPlaneIntersect(const Vec3 &i_position) const
{
	float x=(i_position-m_position_) * m_right_;
	float y=(i_position-m_position_) * m_up_;
	float z=(i_position-m_position_) * m_backward_;
    
    Vec3 fpIntersect;
	
	fpIntersect.x() = x/fabsf(z) * m_filmDist_;
	fpIntersect.y() = -1.0f * y/fabsf(z) * m_filmDist_;
	fpIntersect.z() = m_filmDist_;	
    
    return fpIntersect;
}
