/*
 * $Id: Line.cpp 298 2015-03-25 13:00:40Z bernardt.duvenhage $
 */
/*
 *  Line.cpp
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

#include "Line.h"
#include "Math/MathUtil.h"


//=======================================================================//
stitch::Line::Line(const stitch::Plane &plane0, const stitch::Plane &plane1) :
dir_(plane0.normal_ ^ plane1.normal_),
vertex_(),
tStart_(-((float)FLT_MAX)),
tEnd_(((float)FLT_MAX))
{
	if (dir_.lengthSq()>0.0f)
	{//The planes have a line intersect.
		float ndx=dir_.v_[0];//plane1.normal_.z() * plane0.normal_.y() - plane0.normal_.z() * plane1.normal_.y();
		float ndy=dir_.v_[1];//plane1.normal_.x() * plane0.normal_.z() - plane0.normal_.x() * plane1.normal_.z();
		float ndz=dir_.v_[2];//plane1.normal_.y() * plane0.normal_.x() - plane0.normal_.y() * plane1.normal_.x();
		
		//normDir_.normalise();
        
		float fabsNDY=fabsf(ndy);
		float fabsNDZ=fabsf(ndz);
		float fabsNDX=fabsf(ndx);
		
		float x=0.0f,y=0.0f,z=0.0f;
		
		if ((fabsNDY>=fabsNDZ)&&(fabsNDY>=fabsNDX))
		{
			y=0.0f;
			x=(plane1.d_*plane0.normal_.z() - plane0.d_*plane1.normal_.z())/ndy;
			
			if (fabs(plane0.normal_.z())>fabs(plane1.normal_.z()))
			{
				z=(plane0.d_-x*plane0.normal_.x() - y*plane0.normal_.y())/plane0.normal_.z();
			} else {
				z=(plane1.d_-x*plane1.normal_.x() - y*plane1.normal_.y())/plane1.normal_.z();
			}
		} else
			if (fabsNDZ>=fabsNDX)
			{
				z=0.0f;
				y=(plane1.d_*plane0.normal_.x() - plane0.d_*plane1.normal_.x())/ndz;
				
				if (fabs(plane0.normal_.x())>fabs(plane1.normal_.x()))
				{
					x=(plane0.d_-y*plane0.normal_.y() - z*plane0.normal_.z())/plane0.normal_.x();
				} else {
					x=(plane1.d_-y*plane1.normal_.y() - z*plane1.normal_.z())/plane1.normal_.x();
				}
				
			} else
				{
					x=0.0f;
					z=(plane1.d_*plane0.normal_.y() - plane0.d_*plane1.normal_.y())/ndx;
					
					if (fabs(plane0.normal_.y())>fabs(plane1.normal_.y()))
					{
						y=(plane0.d_-z*plane0.normal_.z() - x*plane0.normal_.x())/plane0.normal_.y();
					} else {
						y=(plane1.d_-z*plane1.normal_.z() - x*plane1.normal_.x())/plane1.normal_.y();
					}
				}
		
		vertex_.set(x, y, z);
	}
}


//=======================================================================//
stitch::Line::Line(const Vec3 &vStart, const Vec3 &vEnd, const bool extendToInfinity) :
dir_(vStart, vEnd),
vertex_(vStart),
tStart_(extendToInfinity ? (-((float)FLT_MAX)) : 0.0f)
{
    if (extendToInfinity)
    {
        tEnd_=((float)FLT_MAX);
        //normDir_.normalise();
    } else
    {
        tEnd_=1.0f;//normDir_.normalise_rt();
    }
}


//=======================================================================//
stitch::Line::Line(const Line &lValue) :
dir_(lValue.dir_),
vertex_(lValue.vertex_),
tStart_(lValue.tStart_),
tEnd_(lValue.tEnd_)
{
}


//=======================================================================//
stitch::Line & stitch::Line::operator=(const Line &lValue)
{
    dir_=lValue.dir_;
    vertex_=lValue.vertex_;

    tStart_=lValue.tStart_;

    tEnd_=lValue.tEnd_;
    
    return (*this);
}


