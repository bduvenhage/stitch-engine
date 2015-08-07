/*
 * $Id: Line.h 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Line.h
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

#ifndef STITCH_LINE_H
#define STITCH_LINE_H

namespace stitch {
	struct Line;
}

#include "Vec3.h"
#include "Plane.h"
#include "MathUtil.h"

namespace stitch {
	
	//! A line is defined by the intersection of two planes, two connected vertices or a vertex and a direction.
	struct Line
	{
	public:
        //!Construct line from the intersection of two planes.
		explicit Line(const Plane &plane0, const Plane &plane1);
        
        //!Construct line from two coordinates.
		explicit Line(const Vec3 &vStart, const Vec3 &vEnd, const bool extendToInfinity=false);
        
        //!Copy contructor.
		Line(const Line &lValue);
        
        //!Assignment operator.
		Line & operator=(const Line &lValue);
        
        //!Virtual destructor.
        virtual ~Line() {}
        
        virtual void normalise()
        {
            float d=dir_.normalise_rt();
            
            if (tStart_>(-FLT_MAX)) tStart_/=d;
            if (tEnd_<FLT_MAX) tEnd_/=d;
        }
        
        //!Trim the line to be on the negative/inside of the plane.
		inline void trim(const Plane &plane)
        {
            //! Distance of vStart from plane.
            float dStart;
            
            //! Distance of vEnd from plane.
            float dEnd;
            
            //! Cosine of angle between plane normal and line.
            const float cosTheta=plane.normal_ * dir_;
            
            if (cosTheta==0.0f)
            {//Plane parallel to line.
                dStart=dEnd=vertex_*plane.normal_-plane.d_;
            } else
            {
                if (tStart_>-((float)FLT_MAX))
                {
                    dStart=plane.normal_*getVStart()-plane.d_;
                } else
                {
                    dStart=cosTheta>0.0f ? (-((float)FLT_MAX)) : ((float)FLT_MAX);
                }
                
                if (tEnd_<((float)FLT_MAX))
                {
                    dEnd=plane.normal_*getVEnd()-plane.d_;
                } else
                {
                    dEnd=cosTheta>0.0f ? ((float)FLT_MAX) : (-((float)FLT_MAX));
                }
            }
            
            if ((dStart>0.0f) && (dEnd>0.0f))
            {//line outside of this brush plane.
                tStart_=tEnd_=0.0f;
            } else
                if ((dStart<=0.0f)&&(dEnd>0.0f))
                {//line begins inside bush plane, but ends outside.
                    tEnd_=plane.calcIntersectDist(vertex_, dir_);
                } else
                    if ((dEnd<=0.0f)&&(dStart>0.0f))
                    {//line begins outside brush plane, but ends inside.
                        tStart_=plane.calcIntersectDist(vertex_, dir_);
                    }
        }
        
        
        //! Return the start vertex of the line. vStart_ = vertex_ if tStart at neginf.
        inline stitch::Vec3 getVStart() const
        {
            return Vec3(vertex_, dir_, tStart_);
        }
        
        //! Return the start parameter of the line.
        inline float getTStart() const
        {
            return tStart_;
        }
        
        //! Return the end vertex of the line. vEnd_ = vertex_ if tEnd at posinf.
        inline stitch::Vec3 getVEnd() const
        {
            return Vec3(vertex_, dir_, tEnd_);
        }
        
        //! Return the end parameter of the line.
        inline float getTEnd() const
        {
            return tEnd_;
        }
        
        //! Return the distance of a point to this line segement. The nearest point to the query point is also returned.
        inline float calcDistToPointSq(const Vec3 &point, Vec3 &nearestToPoint) const
        {
            const float tNearest=(point-vertex_)*dir_;
            
            if (tNearest<tStart_)
            {
                nearestToPoint=getVStart();
            } else
                if (tNearest>tEnd_)
                {
                    nearestToPoint=getVEnd();
                }
                else
                {
                    nearestToPoint=stitch::Vec3(vertex_, dir_, tNearest);
                }
            
            return Vec3::calcDistToPointSq(point, nearestToPoint);
        }
		
        //! Check if line is valid. A normDir_ of (0,0,0) indicates that the constructor did not result in a line.
        inline bool isValid() const
        {
            return dir_.lengthSq()>0.0f;
        }
        
        inline const Vec3 &getVertex() const
        {
            return vertex_;
        }
        
        inline const Vec3 &getDir() const
        {
            return dir_;
        }
        
        inline float getDomain() const
        {
            if ((tStart_>(-FLT_MAX))&&(tEnd_<FLT_MAX))
            {
                return (tEnd_ - tStart_);
            } else
            {
                return FLT_MAX;
            }
        }
        
        inline Vec3 calcCoord(const float t) const
        {
            return Vec3(vertex_,dir_,t);
        }
        
    private:
		Vec3 dir_;
		Vec3 vertex_;
		
		//Vec3 vStart_;
		float tStart_;
        
		//Vec3 vEnd_;
		float tEnd_;
	};
}

#endif// STITCH_LINE_H
