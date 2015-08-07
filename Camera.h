/*
 * $Id: Camera.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Camera.h
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

#ifndef STITCH_CAMERA_H
#define STITCH_CAMERA_H

namespace stitch {
	class Camera;
	class SimplePinholeCamera;
}

#include "Math/Vec3.h"
#include "Math/Ray.h"

#ifdef USE_CXX11
#include <cstdint>
#else
#include <stdint.h>
#endif

namespace stitch
{
    //! Abstract base class for cameras.
    class Camera
    {
    public:
        Vec3 m_position_;
		Vec3 m_backward_;
		Vec3 m_up_;
        Vec3 m_right_;
        
        //! Copy contructor.
		Camera(const Camera &lValue);
        
        //! Assignment operator overload.
		virtual Camera & operator=(const Camera &lValue);
        
        /*! Virtual constructor idiom. Clone operator. */
        virtual Camera * clone() const = 0;  //Uses the copy constructor.
        
		virtual ~Camera() {}
        
        Camera(const Vec3 &i_position, const Vec3 &i_lookAt, const Vec3 &i_up=Vec3(0.0, 1.0, 0.0));
        
        virtual Ray getPrimaryRay(const uint32_t rayID0, const uint16_t rayID1, const float i_s, const float i_t) const = 0;
        
        virtual Vec3 getFocalPlaneIntersect(const Vec3 &i_position) const=0;
        
        virtual bool operator == (const Camera &lValue) const;
        
        virtual bool operator != (const Camera &lValue) const;
    };
    
    //! Simple concrete pinhole camera.
    class SimplePinholeCamera : public Camera
    {
    public:
        float m_filmDist_;
        
		SimplePinholeCamera(const SimplePinholeCamera &lValue);
        
		virtual SimplePinholeCamera & operator=(const SimplePinholeCamera &lValue);
        
        /*! Virtual constructor idiom. Clone operator. */
        virtual SimplePinholeCamera * clone() const
        {
            return new SimplePinholeCamera(*this);
        }

		virtual ~SimplePinholeCamera() {}
        
        SimplePinholeCamera(const Vec3 &i_position, const Vec3 &i_lookAt, const Vec3 &i_up=Vec3(0.0, 1.0, 0.0));
        
        virtual Ray getPrimaryRay(const uint32_t rayID0, const uint16_t rayID1, const float i_s, const float i_t) const
        {
            return stitch::Ray(rayID0, rayID1, stitch::Vec3::sumScaleAndNormalise(m_right_, i_s, m_up_, -i_t, m_backward_, -m_filmDist_), m_position_);
        }
        
        virtual Vec3 getFocalPlaneIntersect(const Vec3 &i_position) const;
    };
    //===========================================================================================//
}
#endif
